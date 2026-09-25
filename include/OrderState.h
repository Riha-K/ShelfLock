#pragma once

#include <memory>
#include <string>

class OrderState {
public:
    virtual ~OrderState() = default;
    virtual const char* name() const = 0;
};

class CreatedState : public OrderState {
public:
    const char* name() const override;
};

class ReservedState : public OrderState {
public:
    const char* name() const override;
};

class PaidState : public OrderState {
public:
    const char* name() const override;
};

class ReleasedState : public OrderState {
public:
    const char* name() const override;
};

class ExpiredState : public OrderState {
public:
    const char* name() const override;
};

class OrderStateFactory {
public:
    static std::unique_ptr<OrderState> create(const std::string& state);
};
