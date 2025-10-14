#pragma once

#include "value.h"
#include "op_codes.h"
#include "meow_object.h"
#include "common/type_definitions.h"
#include "pch.h"

struct Instruction {
    OpCode op;
    std::vector<Int> args;
    Instruction(OpCode op = OpCode::HALT, std::vector<Int> args = {}) : op(op), args(std::move(args)) {}
};

struct UpvalueDesc {
    Bool isLocal;
    Int index;
    UpvalueDesc(Bool local = false, Int idx = 0) : isLocal(local), index(idx) {}
};

struct ExceptionHandler {
    Int catchIp;
    Int frameDepth;
    Int stackDepth;
    ExceptionHandler(Int c = 0, Int f = 0, Int s = 0) : catchIp(c), frameDepth(f), stackDepth(s) {}
};

struct ObjFunctionProto : public MeowObject {
    Int numRegisters = 0;
    Int numUpvalues = 0;
    Str sourceName = "<anon>";
    std::vector<Instruction> code;
    std::vector<Value> constantPool;
    std::vector<UpvalueDesc> upvalueDescs;
    std::unordered_map<Str, Int> labels;
    std::vector<std::tuple<Int, Int, Str>> pendingJumps;

    ObjFunctionProto(Int regs = 0, Int ups = 0, Str name = "<anon>")
        : numRegisters(regs), numUpvalues(ups), sourceName(std::move(name)) {}

    void trace(GCVisitor& visitor) override {
        for (auto& constant : constantPool) {
            visitor.visitValue(constant);
        }
    }
};

struct ObjModule : public MeowObject {
    Str name;
    Str path;
    std::unordered_map<Str, Value> globals;
    std::unordered_map<Str, Value> exports;
    Bool isExecuted = false;
    Bool isBinary = false;

    Proto mainProto;
    Bool hasMain = false;

    Bool isExecuting = false;

    ObjModule(Str n = "", Str p = "", Bool b = false)
        : name(std::move(n)), path(std::move(p)), isBinary(b) {}

    void trace(GCVisitor& visitor) override {
        for (auto& kv : globals) visitor.visitValue(kv.second);
        for (auto& kv : exports) visitor.visitValue(kv.second);
        visitor.visitObject(mainProto);
    }
};

struct ObjUpvalue : public MeowObject {
    enum class State { OPEN, CLOSED };
    State state = State::OPEN;
    Int slotIndex = 0;
    Value closed = Null{};
    ObjUpvalue(Int idx = 0) : slotIndex(idx) {}
    void close(const Value& v) { 
        closed = v; 
        state = State::CLOSED; 
    }

    void trace(GCVisitor& visitor) override {
        visitor.visitValue(closed);
    }
};

struct ObjClosure : public MeowObject {
    Proto proto;
    std::vector<Upvalue> upvalues;
    ObjClosure(Proto p = nullptr) : proto(p), upvalues(p ? p->numUpvalues : 0) {}

    void trace(GCVisitor& visitor) override {
        visitor.visitObject(proto);
        for (auto& uv : upvalues) {
            visitor.visitObject(uv);
        }
    }
};

struct CallFrame {
    Function closure;
    Int slotStart;
    Module module;
    Int ip;
    Int retReg;
    CallFrame(Function c, Int start, Module m, Int ip_, Int ret)
        : closure(c), slotStart(start), module(m), ip(ip_), retReg(ret) {}
};

struct ObjClass : public MeowObject {
    Str name;
    std::optional<Class> superclass;
    std::unordered_map<Str, Value> methods;
    ObjClass(Str n = "") : name(std::move(n)) {}

    void trace(GCVisitor& visitor) override {
        if (superclass) {
            visitor.visitObject(*superclass);
        }
        for (auto& method : methods) {
            visitor.visitValue(method.second);
        }
    }
};

struct ObjInstance : public MeowObject {
    Class klass;
    std::unordered_map<Str, Value> fields;
    ObjInstance(Class k = nullptr) : klass(k) {}

    void trace(GCVisitor& visitor) override {
        visitor.visitObject(klass);
        for (auto& field : fields) {
            visitor.visitValue(field.second);
        }
    }
};

struct ObjBoundMethod : public MeowObject {
    Instance receiver;
    Function callable;
    ObjBoundMethod(Instance r = nullptr, Function c = nullptr) : receiver(r), callable(c) {}

    void trace(GCVisitor& visitor) override {
        visitor.visitObject(receiver);
        visitor.visitObject(callable);
    }
};

// struct ObjArray : public MeowObject {
//     std::vector<Value> elements;
//     ObjArray() = default;
//     ObjArray(std::vector<Value> v) : elements(std::move(v)) {}

//     void trace(GCVisitor& visitor) override {
//         for (auto& element : elements) {
//             visitor.visitValue(element);
//         }
//     }
// };

class ObjArray : public MeowObject {
private:
    using value_t = Value;
    using reference_t = value_t&;
    using const_reference_t = const value_t&;
    using container_t = std::vector<value_t>;

    container_t elements_;
public:
    // --- Constructors & destructor ---
    ObjArray() = default;
    explicit ObjArray(const container_t& elements) : elements_(elements) {}
    explicit ObjArray(container_t&& elements) noexcept : elements_(std::move(elements)) {}
    explicit ObjArray(std::initializer_list<value_t> elements) : elements_(elements) {}

    // --- Rule of 5 ---
    ObjArray(const ObjArray&) = delete;
    ObjArray(ObjArray&&) = delete;
    ObjArray& operator=(const ObjArray&) = delete;
    ObjArray& operator=(ObjArray&&) = delete;
    ~ObjArray() override = default;

    // --- Iterator types ---
    using iterator = container_t::iterator;
    using const_iterator = container_t::const_iterator;
    using reverse_iterator = container_t::reverse_iterator;
    using const_reverse_iterator = container_t::const_reverse_iterator;

    // --- Element access ---
    
    /// @brief Unchecked element access. For performance-critical code
    [[nodiscard]] inline const_reference_t get(size_t index) const noexcept {
        return elements_[index];
    }
    /// @brief Unchecked element modification. For performance-critical code
    template <typename T> inline void set(size_t index, T&& value) noexcept {
        elements_[index] = std::forward<T>(value);
    }
    /// @brief Checked element access. Throws if index is OOB
    [[nodiscard]] inline const_reference_t at(size_t index) const {
        return elements_.at(index);
    }
    inline const_reference_t operator[](size_t index) const noexcept { return elements_[index]; }
    inline reference_t operator[](size_t index) noexcept { return elements_[index]; }
    [[nodiscard]] inline const_reference_t front() const noexcept { return elements_.front(); }
    [[nodiscard]] inline reference_t front() noexcept { return elements_.front(); }
    [[nodiscard]] inline const_reference_t back() const noexcept { return elements_.back(); }
    [[nodiscard]] inline reference_t back() noexcept { return elements_.back(); }

    // --- Capacity ---
    [[nodiscard]] inline size_t size() const noexcept { return elements_.size(); }
    [[nodiscard]] inline bool empty() const noexcept { return elements_.empty(); }
    [[nodiscard]] inline size_t capacity() const noexcept { return elements_.capacity(); }

    // --- Modifiers ---
    template <typename T> inline void push(T&& value) { 
        elements_.emplace_back(std::forward<T>(value));
    }
    inline void pop() noexcept { elements_.pop_back(); }
    template <typename... Args> inline void emplace(Args&&... args) {
        elements_.emplace_back(std::forward<Args>(args)...);
    }
    inline void resize(size_t size) { elements_.resize(size); }
    inline void reserve(size_t capacity) { elements_.reserve(capacity); }
    inline void shrink() { elements_.shrink_to_fit(); }
    inline void clear() { elements_.clear(); }

    // --- Iterators ---
    inline iterator begin() noexcept { return elements_.begin(); }
    inline iterator end() noexcept { return elements_.end(); }
    inline const_iterator begin() const noexcept { return elements_.begin(); }
    inline const_iterator end() const noexcept { return elements_.end(); }
    inline reverse_iterator rbegin() noexcept { return elements_.rbegin(); }
    inline reverse_iterator rend() noexcept { return elements_.rend(); }
    inline const_reverse_iterator rbegin() const noexcept { return elements_.rbegin(); }
    inline const_reverse_iterator rend() const noexcept { return elements_.rend(); }

    inline void trace(GCVisitor& visitor) override {
        for (auto& element : elements_) {
            visitor.visitValue(element);
        }
    }
};

class ObjString : public MeowObject {
private:
    using string_t = std::string;
    string_t data_;
public:
    // --- Constructors & destructor ---
    ObjString() = default;
    explicit ObjString(const string_t& data): data_(data) {}
    explicit ObjString(string_t&& data) noexcept: data_(std::move(data)) {}
    explicit ObjString(const char* data): data_(data) {}

    // --- Rule of 5 ---
    ObjString(const ObjString&) = delete;
    ObjString(ObjString&&) = delete;
    ObjString& operator=(const ObjString&) = delete;
    ObjString& operator=(ObjString&&) = delete;
    ~ObjString() override = default;

    // --- Iterator types ---
    using const_iterator = string_t::const_iterator;
    using const_reverse_iterator = string_t::const_reverse_iterator;

    // --- Character access ---

    /// @brief Unchecked character access. For performance-critical code
    [[nodiscard]] inline char get(size_t index) const noexcept { return data_[index]; }
    /// @brief Checked character access. Throws if index is OOB
    [[nodiscard]] inline char at(size_t index) const { return data_.at(index); }

    // --- String access ---
    [[nodiscard]] inline const char* c_str() const noexcept { return data_.c_str(); }

    // --- Capacity ---
    [[nodiscard]] inline size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] inline bool empty() const noexcept { return data_.empty(); }

    // --- Iterators ---
    inline const_iterator begin() const noexcept { return data_.begin(); }
    inline const_iterator end() const noexcept { return data_.end(); }
    inline const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
    inline const_reverse_iterator rend() const noexcept { return data_.rend(); }

    inline void trace(GCVisitor&) noexcept override {}
};

struct ObjObject : public MeowObject {
    std::unordered_map<Str, Value> fields;
    ObjObject() = default;
    ObjObject(std::unordered_map<Str, Value> f) : fields(std::move(f)) {}

    void trace(GCVisitor& visitor) override {
        for (auto& field : fields) {
            visitor.visitValue(field.second);
        }
    }
};