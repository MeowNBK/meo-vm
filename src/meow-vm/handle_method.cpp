#include "meow_vm.h"

Function MeowVM::wrapClosure(const Value& maybeCallable) {
    if (maybeCallable.is_function()) {
        return maybeCallable.get<Function>();
    }

    if (maybeCallable.is_bound_method()) {
        BoundMethod bm = maybeCallable.get<BoundMethod>();
        if (!bm || !bm->callable) throwVMError("wrapClosure: BoundMethod không có callable.");
        return bm->callable;
    }


    throwVMError("wrapClosure: Giá trị không phải Closure/BoundMethod.");
}

std::optional<Value> MeowVM::getMagicMethod(const Value& obj, const Str& name) {

    if (obj.is_instance()) {
        Instance inst = obj.get<Instance>();
        if (!inst) return std::nullopt;


        auto fit = inst->fields.find(name);
        if (fit != inst->fields.end()) {
            const Value& v = fit->second;

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
                NativeFnAdvanced wrapper = [orig, inst](MeowEngine* engine, Arguments args)->Value {
                    std::vector<Value> newArgs;
                    newArgs.reserve(1 + args.size());
                    newArgs.push_back(Value(inst));
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
                return Value(wrapper);
            }

            return Value(v);
        }


        Class cur = inst->klass;
        while (cur) {
            auto mit = cur->methods.find(name);
            if (mit != cur->methods.end()) {
                const Value& mv = mit->second;
                if (mv.is_function()) {
                    Function f = mv.get<Function>();
                    auto bm = memoryManager->newObject<ObjBoundMethod>(inst, f);
                    return Value(bm);
                }
                if (mv.is_bound_method()) {
                    BoundMethod inbm = mv.get<BoundMethod>();
                    if (inbm && inbm->callable) {
                        auto bm = memoryManager->newObject<ObjBoundMethod>(inst, inbm->callable);
                        return Value(bm);
                    }
                }
                if (mv.is_native_fn()) {
                    NativeFn orig = mv.get<NativeFn>();
                    NativeFnAdvanced wrapper = [orig, inst](MeowEngine* engine, Arguments args)->Value {
                        std::vector<Value> newArgs;
                        newArgs.reserve(1 + args.size());
                        newArgs.push_back(Value(inst));
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
                    return Value(wrapper);
                }

                return Value(mv);
            }
            if (cur->superclass) cur = *cur->superclass;
            else break;
        }
    }


    if (obj.is_hash()) {
        Object objPtr = obj.get<Object>();
        if (!objPtr) return std::nullopt;
        auto fit = objPtr->fields.find(name);
        if (fit != objPtr->fields.end()) return Value(fit->second);
        auto pgit = builtinGetters.find("Object");
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) {
                return this->call(it->second, { obj }); 
            }
        }
        auto pit = builtinMethods.find("Object");
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                const Value& mv = it->second;
                if (mv.is_native_fn()) {
                    NativeFn orig = mv.get<NativeFn>();
                    NativeFnAdvanced wrapper = [orig, objPtr](MeowEngine* engine, Arguments args)->Value {
                        std::vector<Value> newArgs;
                        newArgs.reserve(1 + args.size());
                        newArgs.push_back(Value(objPtr));
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
                    return Value(wrapper);
                }
                return Value(it->second);
            }
        }
        return std::nullopt;
    }


    if (obj.is_array()) {
        Array arr = obj.get<Array>();
        if (!arr) return std::nullopt;

        auto pgit = builtinGetters.find("Array");
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) {
                return this->call(it->second, { obj }); 
            }
        }
        auto pit = builtinMethods.find("Array");
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                const Value& mv = it->second;
                if (mv.is_native_fn()) {
                    NativeFn orig = mv.get<NativeFn>();
                    NativeFnAdvanced wrapper = [orig, arr](MeowEngine* engine, Arguments args)->Value {
                        std::vector<Value> newArgs;
                        newArgs.reserve(1 + args.size());
                        newArgs.push_back(Value(arr));
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
                    return Value(wrapper);
                }
                return Value(it->second);
            }
        }
        return std::nullopt;
    }


    if (obj.is_string()) {
        auto pgit = builtinGetters.find("String");
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) {
                return this->call(it->second, { obj }); 
            }
        }
        auto pit = builtinMethods.find("String");
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                const Value& mv = it->second;
                if (mv.is_native_fn()) {
                    NativeFn orig = mv.get<NativeFn>();
                    NativeFnAdvanced wrapper = [orig, obj](MeowEngine* engine, Arguments args)->Value {
                        std::vector<Value> newArgs;
                        newArgs.reserve(1 + args.size());
                        newArgs.push_back(obj);
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
                    return Value(wrapper);
                }
                return Value(it->second);
            }
        }
        return std::nullopt;
    }


    if (obj.is_int() || obj.is_real() || obj.is_bool()) {
        Str typeName;
        if (obj.is_int()) typeName = "Int";
        else if (obj.is_real()) typeName = "Real";
        else typeName = "Bool";

        auto pgit = builtinGetters.find(typeName);
        if (pgit != builtinGetters.end()) {
            auto it = pgit->second.find(name);
            if (it != pgit->second.end()) {
                return this->call(it->second, { obj }); 
            }
        }

        auto pit = builtinMethods.find(typeName);
        if (pit != builtinMethods.end()) {
            auto it = pit->second.find(name);
            if (it != pit->second.end()) {
                const Value& mv = it->second;
                if (mv.is_native_fn()) {
                    NativeFn orig = mv.get<NativeFn>();
                    NativeFnAdvanced wrapper = [orig, obj](MeowEngine* engine, Arguments args)->Value {
                        std::vector<Value> newArgs;
                        newArgs.reserve(1 + args.size());
                        newArgs.push_back(obj);
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
                    return Value(wrapper);
                }
                return Value(it->second);
            }
        }
        return std::nullopt;
    }


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

void MeowVM::registerMethod(const Str& typeName, const Str& methodName, const Value& method) {
    builtinMethods[typeName][methodName] = method;
}

void MeowVM::registerGetter(const Str& typeName, const Str& propName, const Value& getter) {
    builtinGetters[typeName][propName] = getter;
}