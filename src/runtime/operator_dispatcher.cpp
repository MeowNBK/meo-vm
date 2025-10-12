#include "runtime/operator_dispatcher.h"

OperatorDispatcher::OperatorDispatcher() noexcept {
    using VT = ValueType;
    using enum OpCode;
    binary_ops_[+ADD][+VT::Int][+VT::Int] = [](value_param_t lhs, value_param_t rhs) -> Value {
        
    };
}