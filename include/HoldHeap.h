#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class HoldHeap {
public:
    using Clock = std::chrono::steady_clock;
    using ExpireFn = std::function<void(std::int64_t hold_id)>;

    explicit HoldHeap(ExpireFn on_expire);
    ~HoldHeap();

    HoldHeap(const HoldHeap&) = delete;
    HoldHeap& operator=(const HoldHeap&) = delete;

    void push(std::int64_t hold_id, Clock::time_point expiry);
    void stop();

private:
    struct Item {
        Clock::time_point expiry;
        std::int64_t hold_id;
        bool operator>(const Item& other) const { return expiry > other.expiry; }
    };

    void worker();

    ExpireFn on_expire_;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> heap_;
    std::mutex mu_;
    std::condition_variable cv_;
    bool stop_ = false;
    std::thread thread_;
};
