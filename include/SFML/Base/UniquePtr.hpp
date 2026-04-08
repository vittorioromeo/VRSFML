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
/// \brief Default deleter for `UniquePtr`, calls `delete` on the pointer
///
/// Statically rejects deletion of pointers to incomplete or `void` types.
///
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
/// \brief Lightweight `std::unique_ptr` replacement
///
/// Owns a single heap-allocated `T` and destroys it via `TDeleter` on
/// destruction. The deleter is stored as a private base to take
/// advantage of empty base optimization, so a `UniquePtr` with a
/// stateless deleter is the size of a single pointer.
///
/// `UniquePtr` is annotated with `SFML_BASE_TRIVIAL_ABI` so it is
/// passed in registers like a raw pointer when ABI rules allow, and
/// is always trivially relocatable.
///
/// Move-only: copy construction and copy assignment are deleted.
///
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
    /// \brief Default constructor, creates a null pointer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit UniquePtr() noexcept : m_ptr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct a null pointer from `nullptr`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr /* implicit */ UniquePtr(decltype(nullptr)) noexcept : m_ptr{nullptr}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Take ownership of an existing raw pointer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit UniquePtr(T* ptr) noexcept : m_ptr{ptr}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Take ownership of an existing raw pointer with a custom deleter
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit UniquePtr(T* ptr, const TDeleter& deleter) noexcept :
        TDeleter{deleter},
        m_ptr{ptr}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Destructor, invokes the deleter on the held pointer
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr ~UniquePtr() noexcept
    {
        static_cast<TDeleter*>(this)->operator()(m_ptr);
    }


    ////////////////////////////////////////////////////////////
    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;


    ////////////////////////////////////////////////////////////
    /// \brief Move-construct from a derived/related `UniquePtr`
    ///
    /// Allows storing `UniquePtr<Derived>` in a `UniquePtr<Base>`.
    ///
    ////////////////////////////////////////////////////////////
    template <typename U, typename UDeleter>
    [[nodiscard, gnu::always_inline]] constexpr UniquePtr(UniquePtr<U, UDeleter>&& rhs) noexcept
        requires(isSame<T, U> || isBaseOf<T, U>)
        : TDeleter{static_cast<UDeleter&&>(rhs)}, m_ptr{rhs.m_ptr}
    {
        rhs.m_ptr = nullptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move-assign from a derived/related `UniquePtr`
    ///
    /// Destroys the previously held object before stealing the new one.
    ///
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
    /// \brief Get the underlying raw pointer (or `nullptr`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* get() const noexcept
    {
        return m_ptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dereference the held object (asserts non-null)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T& operator*() const noexcept
    {
        SFML_BASE_ASSERT(m_ptr != nullptr);
        return *m_ptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Member access on the held object (asserts non-null)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T* operator->() const noexcept
    {
        SFML_BASE_ASSERT(m_ptr != nullptr);
        return m_ptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Conversion to `bool`, true when the pointer is non-null
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Equality comparison with `nullptr`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(decltype(nullptr)) const noexcept
    {
        return m_ptr == nullptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Inequality comparison with `nullptr`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator!=(decltype(nullptr)) const noexcept
    {
        return m_ptr != nullptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Destroy the held object and optionally take ownership of a new one
    ///
    /// \param ptr New pointer to take ownership of (defaults to `nullptr`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void reset(T* const ptr = nullptr) noexcept
    {
        static_cast<TDeleter*>(this)->operator()(m_ptr);
        m_ptr = ptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Release ownership of the held pointer without destroying it
    ///
    /// \return The previously held pointer; the caller is now responsible for it
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr T* release() noexcept
    {
        T* const ptr = m_ptr;
        m_ptr        = nullptr;
        return ptr;
    }
};


////////////////////////////////////////////////////////////
/// \brief Construct a `UniquePtr<T>` by forwarding `xs...` to `T`'s constructor
///
/// Equivalent to `std::make_unique`. Always uses brace-initialization.
///
////////////////////////////////////////////////////////////
template <typename T, typename... Ts>
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr UniquePtr<T> makeUnique(Ts&&... xs)
{
    return UniquePtr<T>{new T{static_cast<Ts&&>(xs)...}};
}

} // namespace sf::base
