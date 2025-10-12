#pragma once

#include "pch.h"

struct ObjString;
struct ObjArray;
struct ObjObject;

struct ObjClass;
struct ObjInstance;
struct ObjClosure;
struct ObjFunctionProto;
struct ObjModule;
struct ObjUpvalue;
struct ObjBoundMethod;

class MeowEngine;
class Value;

using Arguments = const std::vector<Value>&;

using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

using Uint8 = uint8_t;
using Uint16 = uint16_t;
using Uint32 = uint32_t;
using Uint64 = uint64_t;

using Float32 = float;
using Float64 = double;
using Float128 = long double;

using Null = std::monostate;
using Int = Int64;
using Real = Float64;
using Bool = bool;
using Str = std::string;
using Array = ObjArray*;

using Object = ObjObject*;

using Instance = ObjInstance*;
using Class = ObjClass*;
using Upvalue = ObjUpvalue*;
using Function = ObjClosure*;
using Module = ObjModule*;
using BoundMethod = ObjBoundMethod*;
using Proto = ObjFunctionProto*;

using NativeFnSimple = std::function<Value(Arguments)>;
using NativeFnAdvanced = std::function<Value(MeowEngine*, Arguments)>;
using NativeFn = std::variant<NativeFnSimple, NativeFnAdvanced>;

using BaseValue = std::variant<
    Null,
    Int,
    Real,
    Bool,
    Str,
    Array,
    Object,
    Instance,
    Class,
    Upvalue,
    Function,
    Module,
    BoundMethod,
    Proto,
    NativeFn
>;

class Value {
private:
    BaseValue data_;
public:
    Value() : data_(Null{}) {}
    template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Value>>>
    Value(T&& t) : data_(std::forward<T>(t)) {}

    Value(const Value& other) : data_(other.data_) {}
    Value(Value&& other) : data_(other.data_) {}
    inline Value& operator=(const Value& other) {
        if (this == &other) return *this;
        data_ = other.data_;
        return *this;
    }
    inline Value& operator=(Value&& other) noexcept {
        if (this == &other) return *this;
        data_ = std::move(other.data_);
        return *this;
    }
    ~Value() noexcept = default;

    [[nodiscard]] inline bool is_null() const noexcept { return is<Null>(); }
    [[nodiscard]] inline bool is_bool() const noexcept { return is<Bool>(); }
    [[nodiscard]] inline bool is_int() const noexcept { return is<Int>(); }
    [[nodiscard]] inline bool is_real() const noexcept { return is<Real>(); }
    [[nodiscard]] inline bool is_array() const noexcept { return is<Array>(); }
    [[nodiscard]] inline bool is_string() const noexcept{ return is<Str>(); }
    [[nodiscard]] inline bool is_hash() const noexcept { return is<Object>(); }
    [[nodiscard]] inline bool is_class() const noexcept { return is<Class>(); }
    [[nodiscard]] inline bool is_instance() const noexcept { return is<Instance>(); }
    [[nodiscard]] inline bool is_bound_method() const noexcept { return is<BoundMethod>(); }
    [[nodiscard]] inline bool is_upvalue() const noexcept { return is<Upvalue>(); }
    [[nodiscard]] inline bool is_proto() const noexcept { return is<Proto>(); }
    [[nodiscard]] inline bool is_function() const noexcept { return is<Function>(); }
    [[nodiscard]] inline bool is_module() const noexcept { return is<Module>(); }
    [[nodiscard]] inline bool is_native_fn() const noexcept { return is<NativeFn>(); }

    template<typename T>
    const T& get() const { return std::get<T>(data_); }

    template<typename T>
    T& get() { return std::get<T>(data_); }

    template<typename T>
    const T* get_if() const { return std::get_if<T>(&data_); }

    template<typename T>
    T* get_if() { return std::get_if<T>(&data_); }
    
    template<typename T>
    bool is() const noexcept { return std::holds_alternative<T>(data_); }
};

enum class ValueType {
    Null, Int, Real, Bool, String,
    Array, HashTable, Upvalue, Function,
    Class, Instance, BoundMethod,
    Proto, NativeFn, TotalValueTypes
};