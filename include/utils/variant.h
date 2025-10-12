#pragma once

#include "common/pch.h"

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#  define MEOW_BYTE_ORDER __BYTE_ORDER__
#  define MEOW_ORDER_LITTLE __ORDER_LITTLE_ENDIAN__
#endif

#if (defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)) && (defined(MEOW_BYTE_ORDER) && MEOW_BYTE_ORDER == MEOW_ORDER_LITTLE)
#  define MEOW_CAN_USE_NAN_BOXING 1
#else
#  define MEOW_CAN_USE_NAN_BOXING 0
#endif

#include "utils/variant/variant_fallback.h" 
#include "utils/variant/variant_nanbox.h"

namespace meo {
template <typename... Args>
class variant {
private:
    // --- Metadata ---
    static constexpr bool should_use_nan_box =
        MEOW_CAN_USE_NAN_BOXING &&
        (sizeof...(Args) <= 8) &&
        all_nanboxable_impl<flattened_unique_t<Args...>>::value;

    using implementation_t = std::conditional_t<
        should_use_nan_box,
        NaNBoxedVariant<Args...>,
        FallbackVariant<Args...>
    >;

    implementation_t storage_;

public:
    // --- Constructors ---
    variant() = default;
    variant(const variant&) = default;
    variant(variant&&) = default;
    template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, variant>>>
    variant(T&& value) noexcept(noexcept(implementation_t(std::forward<T>(value))))
        : storage_(std::forward<T>(value)) {}
    
    // --- Assignment operators ---
    variant& operator=(const variant&) = default;
    variant& operator=(variant&&) = default;
    template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, variant>>>
    variant& operator=(T&& value) noexcept(noexcept(storage_ = std::forward<T>(value))) {
        storage_ = std::forward<T>(value);
        return *this;
    }

    // --- Queries ---
    [[nodiscard]] std::size_t index() const noexcept { return storage_.index(); }
    [[nodiscard]] bool valueless() const noexcept { return storage_.valueless(); }

    template <typename T>
    [[nodiscard]] bool holds() const noexcept { return storage_.template holds<T>(); }

    template <typename T>
    [[nodiscard]] bool is() const noexcept { return holds<T>(); }

    // --- Accessors ---
    template <typename T>
    [[nodiscard]] decltype(auto) get() { return storage_.template get<T>(); }

    template <typename T>
    [[nodiscard]] decltype(auto) get() const { return storage_.template get<T>(); }
    
    template <typename T>
    [[nodiscard]] decltype(auto) safe_get() { return storage_.template safe_get<T>(); }
    
    template <typename T>
    [[nodiscard]] decltype(auto) safe_get() const { return storage_.template safe_get<T>(); }

    template <typename T>
    [[nodiscard]] auto* get_if() noexcept { return storage_.template get_if<T>(); }

    template <typename T>
    [[nodiscard]] const auto* get_if() const noexcept { return storage_.template get_if<T>(); }

    // --- Modifiers ---
    template <typename T, typename... CArgs>
    decltype(auto) emplace(CArgs&&... args) {
        return storage_.template emplace<T>(std::forward<CArgs>(args)...);
    }
    
    template <std::size_t I, typename... CArgs>
    decltype(auto) emplace_index(CArgs&&... args) {
        return storage_.template emplace_index<I>(std::forward<CArgs>(args)...);
    }

    void swap(variant& other) noexcept { storage_.swap(other.storage_); }

    // --- Visit ---
    template <typename Visitor>
    decltype(auto) visit(Visitor&& vis) {
        return storage_.visit(std::forward<Visitor>(vis));
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& vis) const {
        return storage_.visit(std::forward<Visitor>(vis));
    }
    
    // --- Comparison ---
    bool operator==(const variant& other) const { return storage_ == other.storage_; }
    bool operator!=(const variant& other) const { return storage_ != other.storage_; }
};

// --- Non-member utilities ---

template <typename... Ts>
void swap(variant<Ts...>& a, variant<Ts...>& b) noexcept {
    a.swap(b);
}

template <typename... Ts, typename... Fs>
decltype(auto) visit(variant<Ts...>& v, Fs&&... fs) {
    return v.visit(overload{ std::forward<Fs>(fs)... });
}

template <typename... Ts, typename... Fs>
decltype(auto) visit(const variant<Ts...>& v, Fs&&... fs) {
    return v.visit(overload{ std::forward<Fs>(fs)... });
}

template <typename... Ts, typename... Fs>
decltype(auto) visit(variant<Ts...>&& v, Fs&&... fs) {
    return std::move(v).visit(overload{ std::forward<Fs>(fs)... });
}

}