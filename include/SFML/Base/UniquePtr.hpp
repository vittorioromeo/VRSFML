#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Trait/IsBaseOf.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/TrivialAbi.hpp"


namespace sf::base
{
// clang-format off
////////////////////////////////////////////////////////////
struct SFML_BASE_TRIVIAL_ABI UniquePtrDefaultDeleter
{
    template <typename T>
    [[gnu::always_inline]] constexpr void operator()(T* const ptr) const noexcept
    {
        static_assert(!SFML_BASE_IS_SAME(T, void), "can't delete pointer to incomplete type");

        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        static_assert(sizeof(T) > 0u, "can't delete pointer to incomplete type");

        delete ptr;
    }
};
// clang-format on


////////////////////////////////////////////////////////////
template <typename T, typename TDeleter = UniquePtrDefaultDeleter>
class SFML_BASE_TRIVIAL_ABI UniquePtr : private TDeleter
{
    template <typename, typename>
    friend class UniquePtr;

private:
    T* m_ptr;

public:
    ////////////////////////////////////////////////////////////
    enum : bool
    {
        enableTrivialRelocation = true
    };


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit UniquePtr() noexcept : m_ptr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ UniquePtr(decltype(nullptr)) noexcept : m_ptr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit UniquePtr(T* ptr) noexcept : m_ptr{ptr}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit UniquePtr(T* ptr, const TDeleter& deleter) noexcept :
        TDeleter{deleter},
        m_ptr{ptr}
    {
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr ~UniquePtr() noexcept
    {
        static_cast<TDeleter*>(this)->operator()(m_ptr);
    }


    ////////////////////////////////////////////////////////////
    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;


    ////////////////////////////////////////////////////////////
    template <typename U, typename UDeleter>
    [[nodiscard, gnu::always_inline]] constexpr UniquePtr(UniquePtr<U, UDeleter>&& rhs) noexcept
        requires(isSame<T, U> || isBaseOf<T, U>)
        : TDeleter{static_cast<UDeleter&&>(rhs)}, m_ptr{rhs.m_ptr}
    {
        rhs.m_ptr = nullptr;
    }


    ////////////////////////////////////////////////////////////
    template <typename U, typename UDeleter>
    [[gnu::always_inline, gnu::flatten]] constexpr UniquePtr& operator=(UniquePtr<U, UDeleter>&& rhs) noexcept
        requires(isSame<T, U> || isBaseOf<T, U>)
    {
        (*static_cast<TDeleter*>(this)) = static_cast<UDeleter&&>(rhs);

        reset(rhs.m_ptr);
        rhs.m_ptr = nullptr;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* get() const noexcept
    {
        return m_ptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T& operator*() const noexcept
    {
        SFML_BASE_ASSERT(m_ptr != nullptr);
        return *m_ptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T* operator->() const noexcept
    {
        SFML_BASE_ASSERT(m_ptr != nullptr);
        return m_ptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(decltype(nullptr)) const noexcept
    {
        return m_ptr == nullptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator!=(decltype(nullptr)) const noexcept
    {
        return m_ptr != nullptr;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void reset(T* const ptr = nullptr) noexcept
    {
        static_cast<TDeleter*>(this)->operator()(m_ptr);
        m_ptr = ptr;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr T* release() noexcept
    {
        T* const ptr = m_ptr;
        m_ptr        = nullptr;
        return ptr;
    }
};


////////////////////////////////////////////////////////////
template <typename T, typename... Ts>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr UniquePtr<T> makeUnique(Ts&&... xs)
{
    return UniquePtr<T>{new T{static_cast<Ts&&>(xs)...}};
}

} // namespace sf::base
