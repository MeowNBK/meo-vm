#pragma once

#include "common/pch.h"
#include "core/op_codes.h"
#include "core/value.h"
#include "core/definitions.h"

// --- Constants ---
constexpr size_t NUM_OP_CODES = static_cast<size_t>(OpCode::TOTAL_OPCODES);
constexpr size_t NUM_VALUE_TYPES = static_cast<size_t>(ValueType::TotalValueTypes);

// --- Simple castings ---
[[nodiscard]] inline constexpr size_t operator+(ValueType value) noexcept {
    return static_cast<size_t>(value);
}
[[nodiscard]] inline constexpr size_t operator+(OpCode op_code) noexcept {
    return static_cast<size_t>(op_code);
}

class MemoryManager;

class OperatorDispatcher {
private:
    // --- Definitions ---
    using binary_fn_t = Value(*)(value_param_t, value_param_t);
    using unary_fn_t = Value(*)(value_param_t);

    // --- Methods --- 
    binary_fn_t binary_ops_[NUM_OP_CODES][NUM_VALUE_TYPES][NUM_VALUE_TYPES] = {};
    unary_fn_t unary_ops_[NUM_OP_CODES][NUM_VALUE_TYPES] = {};
    MemoryManager* heap_ = nullptr;
public:
    // --- Constructors & destructor ---
    OperatorDispatcher() noexcept;

    OperatorDispatcher(const OperatorDispatcher&) = delete;
    OperatorDispatcher(OperatorDispatcher&&) = delete;
    OperatorDispatcher& operator=(const OperatorDispatcher&) = delete;
    OperatorDispatcher& operator=(OperatorDispatcher&&) = delete;
    ~OperatorDispatcher() = default;

    // --- Main API ---
    [[nodiscard]] inline constexpr const binary_fn_t* find(OpCode op_code, value_param_t left, value_param_t right) const noexcept {
        auto left_type = get_value_type(left);
        auto right_type = get_value_type(right);
        const binary_fn_t* function = &binary_ops_[+op_code][+left_type][+right_type];
        // return (*function) ? function : nullptr;
        return function;
    }
    [[nodiscard]] inline constexpr const unary_fn_t* find(OpCode op_code, value_param_t value) const noexcept {
        auto value_type = get_value_type(value);
        const unary_fn_t* function = &unary_ops_[+op_code][+value_type];
        return function;
    }
};