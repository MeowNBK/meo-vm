#pragma once
#include "core/meow_object.h"

class MeowObject;
class MeowVM;

class GarbageCollector {
public:
    virtual ~GarbageCollector() = default;
    
    virtual void registerObject(const MeowObject* object) = 0;
    
    virtual void collect(MeowVM& vm) noexcept = 0;
};