#pragma once

class Value;
class MeowObject;

class GCVisitor {
public:
    virtual ~GCVisitor() = default;
    virtual void visit_value(const Value& value) noexcept = 0;
    virtual void visit_object(const MeowObject* object) noexcept = 0;
};

class MeowObject {
public:
    virtual ~MeowObject() = default;
    virtual void trace(GCVisitor& visitor) const noexcept = 0;
};