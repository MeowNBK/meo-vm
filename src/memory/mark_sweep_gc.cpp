#include "mark_sweep_gc.h"
#include "meow_vm.h"
#include "core/value.h"

MarkSweepGC::~MarkSweepGC() {
    for (auto const& [obj, data] : metadata) {
        delete obj;
    }
}

void MarkSweepGC::registerObject(const MeowObject* obj) {
    metadata.emplace(obj, GCMetadata{});
}

void MarkSweepGC::collect(MeowVM& vm_instance) noexcept {
    this->vm = &vm_instance;

    vm->traceRoots(*this);

    for (auto it = metadata.begin(); it != metadata.end();) {
        const MeowObject* obj = it->first;
        GCMetadata& data = it->second;

        if (data.isMarked) {
            data.isMarked = false;
            ++it;
        } else {

            delete obj;
            it = metadata.erase(it);
        }
    }
    
    this->vm = nullptr;
}

void MarkSweepGC::visit_value(const Value& value) noexcept {
    if (value.is_instance())      
        mark(value.get<Instance>());
    else if (value.is_function()) 
        mark(value.get<Function>());
    else if (value.is_class())    
        mark(value.get<Class>());
    else if (value.is_module())   
        mark(value.get<Module>());
    else if (value.is_bound_method())    
        mark(value.get<BoundMethod>());
    else if (value.is_proto())    
        mark(value.get<Proto>());
    else if (value.is_upvalue()) 
        mark(value.get<Upvalue>());
    else if (value.is_array()) {
        mark(value.get<Array>());
    } else if (value.is_hash()) {
        mark(value.get<Object>());
    }
}

void MarkSweepGC::visit_object(const MeowObject* object) noexcept {
    mark(object);
}

void MarkSweepGC::mark(const MeowObject* object) noexcept {
    if (object == nullptr) return;
    auto it = metadata.find(object); // find() is not noexcept
    if (it == metadata.end()) return;
    else if (it->second.isMarked) return;

    it->second.isMarked = true;

    object->trace(*this);
}


// From IDEAS.txt
// #include "memory/mark_sweep_gc.h"
// #include "core/value.h"
// #include "runtime/execution_context.h"
// #include "runtime/builtin_registry.h"

// using namespace meow::memory;

// MarkSweepGC::~MarkSweepGC() {
//     std::cout << "[destroy] Đang xử lí các object khi hủy GC" << std::endl;
//     for (auto const& [obj, data] : metadata_) {
//         delete obj;
//     }
// }

// void MarkSweepGC::register_object(const meow::core::MeowObject* object) {
//     std::cout << "[register] Đang đăng kí object: " << object << std::endl;
//     metadata_.emplace(object, GCMetadata{});
// }

// size_t MarkSweepGC::collect() noexcept {
//     std::cout << "[collect] Đang collect các object" << std::endl;

//     context_->trace(*this);
//     builtins_->trace(*this);

//     for (auto it = metadata_.begin(); it != metadata_.end();) {
//         const meow::core::MeowObject* object = it->first;
//         GCMetadata& data = it->second;

//         if (data.is_marked_) {
//             data.is_marked_ = false;
//             ++it;
//         } else {
//             delete object;
//             it = metadata_.erase(it);
//         }
//     }

//     return metadata_.size();
// }

// void MarkSweepGC::visit_value(const meow::core::Value& value) noexcept {
//     if (value.is_object()) mark(value.as_object());
// }

// void MarkSweepGC::visit_object(const meow::core::MeowObject* object) noexcept { mark(object); }

// void MarkSweepGC::mark(const meow::core::MeowObject* object) {
//     if (object == nullptr) return;
//     auto it = metadata_.find(object);
//     if (it == metadata_.end() || it->second.is_marked_) return;
//     it->second.is_marked_ = true;
//     object->trace(*this);
// }