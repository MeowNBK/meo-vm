#pragma once

namespace meow::core {
    class Value;
    class MeowObject;
}

namespace meow::memory {
    class GCVisitor {
    public:
        virtual ~GCVisitor() noexcept = default;
        virtual void visit_value(const meow::core::Value& value) noexcept = 0;
        virtual void visit_object(const meow::core::MeowObject* object) noexcept = 0;
    };
}