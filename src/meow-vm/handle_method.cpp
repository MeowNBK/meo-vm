#include "vm/meow_vm.h"

// Helper: tạo wrapper cho native function, push receiver vào đầu args
static NativeFnAdvanced make_native_wrapper(NativeFn orig, Value receiver) {
    return [orig, receiver](MeowEngine* engine, Arguments args) -> Value {
        std::vector<Value> newArgs;
        newArgs.reserve(1 + args.size());
        newArgs.push_back(receiver);
        newArgs.insert(newArgs.end(), args.begin(), args.end());
        return std::visit([&](auto&& fn) -> Value {
            using T = std::decay_t<decltype(fn)>;
            if constexpr (std::is_same_v<T, NativeFnSimple>) {
                return fn(newArgs);
            } else {
                return fn(engine, newArgs);
            }
        }, orig);
    };
}

// Helper: nếu receiver là Instance thì bind function/bound_method -> ObjBoundMethod;
// nếu là native_fn -> bọc native với receiver; nếu không thì trả về as-is
std::optional<Value> wrapValueForInstance(Instance inst, const Value& v, MemoryManager* memoryManager) {
    if (!inst) return std::nullopt;

    if (v.is_function()) {
        Function f = v.get<Function>();
        auto bm = memoryManager->newObject<ObjBoundMethod>(inst, f);
        return Value(bm);
    }

    if (v.is_bound_method()) {
        BoundMethod inbm = v.get<BoundMethod>();
        if (inbm && inbm->callable) {
            auto bm = memoryManager->newObject<ObjBoundMethod>(inst, inbm->callable);
            return Value(bm);
        }
    }

    if (v.is_native_fn()) {
        NativeFn orig = v.get<NativeFn>();
        NativeFnAdvanced wrapper = make_native_wrapper(orig, Value(inst));
        return Value(wrapper);
    }

    return Value(v);
}

// Helper: cho các receiver không phải Instance (Object/Array/String/Int/Real/Bool)
// - nếu value là native_fn -> bọc native với receiver Value
// - ngược lại trả Value as-is
std::optional<Value> wrapValueWithReceiverValue(const Value& receiver, const Value& v) {
    if (v.is_native_fn()) {
        NativeFn orig = v.get<NativeFn>();
        NativeFnAdvanced wrapper = make_native_wrapper(orig, receiver);
        return Value(wrapper);
    }
    return Value(v);
}

std::optional<Value> MeowVM::getMagicMethod(const Value& obj, const Str& name) {

    // --- INSTANCE case (fields -> class methods -> super) ---
    if (obj.is_instance()) {
        Instance inst = obj.get<Instance>();
        if (!inst) return std::nullopt;

        // 1) instance fields
        auto fit = inst->fields.find(name);
        if (fit != inst->fields.end()) {
            if (auto r = wrapValueForInstance(inst, fit->second, memoryManager.get())) return *r;
        }

        // 2) class methods (including walking superclass)
        Class cur = inst->klass;
        while (cur) {
            auto mit = cur->methods.find(name);
            if (mit != cur->methods.end()) {
                if (auto r = wrapValueForInstance(inst, mit->second, memoryManager.get())) return *r;
            }
            if (cur->superclass) cur = *cur->superclass;
            else break;
        }

        return std::nullopt;
    }

    // --- OBJECT (hash) ---
    if (obj.is_hash()) {
        Object objPtr = obj.get<Object>();
        if (!objPtr) return std::nullopt;

        auto fit = objPtr->fields.find(name);
        if (fit != objPtr->fields.end()) {
            if (auto r = wrapValueWithReceiverValue(Value(objPtr), fit->second)) return *r;
        }

        auto pgit = builtinGetters.find("Object");
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) return this->call(it->second, { obj });
        }
        auto pit = builtinMethods.find("Object");
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                if (auto r = wrapValueWithReceiverValue(Value(objPtr), it->second)) return *r;
            }
        }
        return std::nullopt;
    }

    // --- ARRAY ---
    if (obj.is_array()) {
        Array arr = obj.get<Array>();
        if (!arr) return std::nullopt;

        auto pgit = builtinGetters.find("Array");
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) return this->call(it->second, { obj });
        }
        auto pit = builtinMethods.find("Array");
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                if (auto r = wrapValueWithReceiverValue(Value(arr), it->second)) return *r;
            }
        }
        return std::nullopt;
    }

    // --- STRING ---
    if (obj.is_string()) {
        auto pgit = builtinGetters.find("String");
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) return this->call(it->second, { obj });
        }
        auto pit = builtinMethods.find("String");
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                if (auto r = wrapValueWithReceiverValue(obj, it->second)) return *r;
            }
        }
        return std::nullopt;
    }

    // --- PRIMITIVES: Int / Real / Bool ---
    if (obj.is_int() || obj.is_real() || obj.is_bool()) {
        Str typeName;
        if (obj.is_int()) typeName = "Int";
        else if (obj.is_real()) typeName = "Real";
        else typeName = "Bool";

        auto pgit = builtinGetters.find(typeName);
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) return this->call(it->second, { obj });
        }

        auto pit = builtinMethods.find(typeName);
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                if (auto r = wrapValueWithReceiverValue(obj, it->second)) return *r;
            }
        }
        return std::nullopt;
    }

    // --- CLASS (static methods) ---
    if (obj.is_class()) {
        Class klass = obj.get<Class>();
        if (!klass) return std::nullopt;
        auto mit = klass->methods.find(name);
        if (mit != klass->methods.end()) {
            return Value(mit->second);
        }
    }

    return std::nullopt;
}

void MeowVM::register_method(const Str& type_name, const Str& method_name, const Value& method) noexcept {
    builtinMethods[type_name][method_name] = method;
}

void MeowVM::register_getter(const Str& type_name, const Str& property_name, const Value& getter) noexcept {
    builtinGetters[type_name][property_name] = getter;
}