#pragma once

#include "core/value.h"

class Value;
class MemoryManager;

class MeowEngine {
public:
    virtual ~MeowEngine() = default;

    virtual Value call(const Value& callee, Arguments args) = 0;
    virtual MemoryManager* get_heap() noexcept = 0;
    virtual void register_method(const std::string& type_name, const std::string& method_name, const Value& method) = 0;
    virtual void register_getter(const std::string& type_name, const std::string& property_name, const Value& getter) = 0;
    virtual const std::vector<std::string>& get_arguments() const noexcept = 0;
};