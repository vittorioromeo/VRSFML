// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/IsBaseOf.hpp>


namespace sf::base
{

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

    [[nodiscard, gnu::always_inline]] explicit(false) UniquePtr(decltype(nullptr)) noexcept : m_ptr{nullptr}
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

    template <typename U, typename UDeleter>
    [[nodiscard, gnu::always_inline]] UniquePtr(UniquePtr<U, UDeleter>&& rhs) noexcept
        requires(SFML_BASE_IS_BASE_OF(T, U)) :
    TDeleter{static_cast<UDeleter&&>(rhs)},
    m_ptr{rhs.m_ptr}
    {
        rhs.m_ptr = nullptr;
    }

    template <typename U, typename UDeleter>
    [[gnu::always_inline]] UniquePtr& operator=(UniquePtr<U, UDeleter>&& rhs) noexcept
        requires(SFML_BASE_IS_BASE_OF(T, U))
    {
        (*static_cast<TDeleter*>(this)) = static_cast<UDeleter&&>(rhs);

        reset(rhs.m_ptr);
        rhs.m_ptr = nullptr;

        return *this;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] T* get() const noexcept
    {
        return m_ptr;
    }

    [[nodiscard, gnu::always_inline]] T& operator*() const noexcept
    {
        SFML_BASE_ASSERT(m_ptr != nullptr);
        return *m_ptr;
    }

    [[nodiscard, gnu::always_inline]] T* operator->() const noexcept
    {
        SFML_BASE_ASSERT(m_ptr != nullptr);
        return m_ptr;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] bool operator==(const T* ptr) const noexcept
    {
        return m_ptr == ptr;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] bool operator!=(const T* ptr) const noexcept
    {
        return m_ptr != ptr;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] bool operator==(decltype(nullptr)) const noexcept
    {
        return m_ptr == nullptr;
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] bool operator!=(decltype(nullptr)) const noexcept
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
[[nodiscard, gnu::always_inline, gnu::pure]] inline UniquePtr<T> makeUnique(Ts&&... xs)
{
    return UniquePtr<T>{new T{static_cast<Ts&&>(xs)...}};
}

} // namespace sf::base
