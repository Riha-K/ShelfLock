#include "HoldHeap.h"

#include <exception>
#include <iostream>

HoldHeap::HoldHeap(ExpireFn on_expire) : on_expire_(std::move(on_expire)), thread_([this] { worker(); }) {}

HoldHeap::~HoldHeap() { stop(); }

void HoldHeap::push(std::int64_t hold_id, Clock::time_point expiry) {
    {
        std::lock_guard<std::mutex> lock(mu_);
        heap_.push(Item{expiry, hold_id});
    }
    cv_.notify_one();
}

void HoldHeap::stop() {
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (stop_) {
            return;
        }
        stop_ = true;
    }
    cv_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void HoldHeap::worker() {
    std::unique_lock<std::mutex> lock(mu_);
    while (!stop_) {
        if (heap_.empty()) {
            cv_.wait(lock, [this] { return stop_ || !heap_.empty(); });
            continue;
        }
        const Item next = heap_.top();
        if (cv_.wait_until(lock, next.expiry, [this, next] {
                return stop_ || (!heap_.empty() && heap_.top().expiry < next.expiry);
            })) {
            continue;
        }
        if (stop_) {
            break;
        }
        if (heap_.empty() || heap_.top().hold_id != next.hold_id) {
            continue;
        }
        heap_.pop();
        lock.unlock();
        try {
            on_expire_(next.hold_id);
        } catch (const std::exception& ex) {
            std::cerr << "hold expiry failed for " << next.hold_id << ": " << ex.what() << '\n';
        }
        lock.lock();
    }
}
