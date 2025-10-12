#pragma once

#include "common/pch.h"
#include "common/value.h"

// --- Type alias ---
using value_t = Value;
using value_param_t = std::conditional_t<sizeof(value_t) <= sizeof(void*), const value_t, const value_t&>;
using index_t = size_t;

// --- Helper functions ---
[[nodiscard]] inline ValueType get_value_type(value_param_t value) noexcept {
    if (value.is_null()) return ValueType::Null;
    if (value.is_int()) return ValueType::Int;
    if (value.is_real()) return ValueType::Real;
    if (value.is_bool()) return ValueType::Bool;
    if (value.is_string()) return ValueType::String;
    if (value.is_array()) return ValueType::Array;
    if (value.is_hash()) return ValueType::HashTable;
    if (value.is_upvalue()) return ValueType::Upvalue;
    if (value.is_function()) return ValueType::Function;
    if (value.is_class()) return ValueType::Class;
    if (value.is_instance()) return ValueType::Instance;
    if (value.is_bound_method()) return ValueType::BoundMethod;
    if (value.is_proto()) return ValueType::Proto;
    if (value.is_native_fn()) return ValueType::NativeFn;
    return ValueType::Null;
}