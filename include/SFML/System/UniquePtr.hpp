// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <cassert>

#if !__has_builtin(__is_base_of)

#include <type_traits>

namespace sf::priv
{
template <typename B, typename D>
using IsBaseOf = std::is_base_of<B, D>;
}

#endif

namespace sf::priv
{

// clang-format off
template<bool, typename = void> struct EnableIfImpl          { };
template<typename T>            struct EnableIfImpl<true, T> { using type = T; };
// clang-format on

template <bool B, typename T = void>
using EnableIf = typename EnableIfImpl<B, T>::type;

struct UniquePtrDefaultDeleter
{
    template <typename T>
    [[gnu::always_inline]] void operator()(T* const ptr) const noexcept
    {
        delete ptr;
    }
};

template <typename T, typename TDeleter = UniquePtrDefaultDeleter>
class UniquePtr : private TDeleter
{
    template <typename, typename>
    friend class UniquePtr;

private:
    T* m_ptr;

public:
    [[nodiscard, gnu::always_inline]] explicit UniquePtr() noexcept : m_ptr{nullptr}
    {
    }

    [[nodiscard, gnu::always_inline]] /* implicit */ UniquePtr(decltype(nullptr)) noexcept : m_ptr{nullptr}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit UniquePtr(T* ptr) noexcept : m_ptr{ptr}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit UniquePtr(T* ptr, const TDeleter& deleter) noexcept :
    TDeleter{deleter},
    m_ptr{ptr}
    {
    }

    [[gnu::always_inline]] ~UniquePtr() noexcept
    {
        if (m_ptr != nullptr)
        {
            static_cast<TDeleter*>(this)->operator()(m_ptr);
        }
    }

    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    template <typename U,
              typename UDeleter,
#if __has_builtin(__is_base_of)
              typename = EnableIf<__is_base_of(T, U)>
#else
              typename = EnableIf<IsBaseOf<T, U>::value>
#endif
              >
    [[nodiscard, gnu::always_inline]] UniquePtr(UniquePtr<U, UDeleter>&& rhs) noexcept :
    TDeleter{static_cast<UDeleter&&>(rhs)},
    m_ptr{rhs.m_ptr}
    {
        rhs.m_ptr = nullptr;
    }

    template <typename U,
              typename UDeleter,
#if __has_builtin(__is_base_of)
              typename = EnableIf<__is_base_of(T, U)>
#else
              typename = EnableIf<IsBaseOf<T, U>::value>
#endif
              >
    [[gnu::always_inline]] UniquePtr& operator=(UniquePtr<U, UDeleter>&& rhs) noexcept
    {
        (*static_cast<TDeleter*>(this)) = static_cast<UDeleter&&>(rhs);

        reset(rhs.m_ptr);
        rhs.m_ptr = nullptr;

        return *this;
    }

    [[nodiscard, gnu::always_inline]] T* get() const noexcept
    {
        return m_ptr;
    }

    [[nodiscard, gnu::always_inline]] T& operator*() const noexcept
    {
        assert(m_ptr != nullptr);
        return *m_ptr;
    }

    [[nodiscard, gnu::always_inline]] T* operator->() const noexcept
    {
        assert(m_ptr != nullptr);
        return m_ptr;
    }

    [[nodiscard, gnu::always_inline]] explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator==(const T* ptr) const noexcept
    {
        return m_ptr == ptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator!=(const T* ptr) const noexcept
    {
        return m_ptr != ptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator==(decltype(nullptr)) const noexcept
    {
        return m_ptr == nullptr;
    }

    [[nodiscard, gnu::always_inline]] bool operator!=(decltype(nullptr)) const noexcept
    {
        return m_ptr != nullptr;
    }

    [[gnu::always_inline]] void reset(T* const ptr = nullptr) noexcept
    {
        if (m_ptr != nullptr)
        {
            static_cast<TDeleter*>(this)->operator()(m_ptr);
        }

        m_ptr = ptr;
    }
};

template <typename T, typename... Ts>
[[nodiscard, gnu::always_inline]] inline UniquePtr<T> makeUnique(Ts&&... xs)
{
    return UniquePtr<T>{new T{static_cast<Ts&&>(xs)...}};
}

} // namespace sf::priv
