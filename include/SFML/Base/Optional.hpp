#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/Traits/IsCopyAssignable.hpp"
#include "SFML/Base/Traits/IsCopyConstructible.hpp"
#include "SFML/Base/Traits/IsMoveAssignable.hpp"
#include "SFML/Base/Traits/IsMoveConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"


namespace sf::base::priv
{
///////////////////////////////////////////////////////// ///
[[gnu::cold]] void throwIfNotEngaged();

} // namespace sf::base::priv

namespace sf::base
{
// clang-format off
////////////////////////////////////////////////////////////
struct BadOptionalAccess { };


////////////////////////////////////////////////////////////
inline constexpr struct InPlace  { } inPlace;
inline constexpr struct NullOpt  { } nullOpt;
inline constexpr struct FromFunc { } fromFunc;

// clang-format on


////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, engaged, buffer) \
    do                                                            \
    {                                                             \
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T))    \
        {                                                         \
            if (engaged)                                          \
                buffer.obj.~T();                                  \
        }                                                         \
    } while (false)
// NOLINTEND(bugprone-macro-parentheses)


////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_PRIV_OPTIONAL_DESTROY(T, buffer)                  \
    do                                                         \
    {                                                          \
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T)) \
        {                                                      \
            buffer.obj.~T();                                   \
        }                                                      \
    } while (false)

// NOLINTEND(bugprone-macro-parentheses)


////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Optional
{
public:
    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional() noexcept : m_engaged{false}
    {
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(NullOpt) noexcept : m_engaged{false}
    {
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(const T& object) : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(object);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(T&& object) noexcept : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_MOVE(object));
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(const Optional& rhs)
        requires(!base::isTriviallyCopyConstructible<T> && base::isCopyConstructible<T>)
    : m_engaged{rhs.m_engaged}
    {
        if (m_engaged)
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(rhs.m_buffer.obj);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(const Optional& rhs)
        requires(base::isTriviallyCopyConstructible<T>)
    = default;

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(Optional&& rhs) noexcept
        requires(!base::isTriviallyMoveConstructible<T> && base::isMoveConstructible<T>)
    : m_engaged{rhs.m_engaged}
    {
        if (m_engaged)
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_MOVE(rhs.m_buffer.obj));
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(Optional&& rhs)
        requires(base::isTriviallyMoveConstructible<T>)
    = default;

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr ~Optional() noexcept
        requires(!base::isTriviallyDestructible<T>)
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
    }

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr ~Optional() noexcept
        requires(base::isTriviallyDestructible<T>)
    = default;

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(const Optional& rhs)
        requires(!base::isTriviallyCopyAssignable<T> && base::isCopyAssignable<T>)
    {
        if (&rhs == this || (!m_engaged && !rhs.m_engaged))
            return *this;

        if (m_engaged && !rhs.m_engaged)
        {
            m_engaged = false;
            SFML_PRIV_OPTIONAL_DESTROY(T, m_buffer);
        }
        else if (!m_engaged && rhs.m_engaged)
        {
            m_engaged = true;
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(rhs.m_buffer.obj);
        }
        else
        {
            SFML_BASE_ASSERT(m_engaged && rhs.m_engaged);
            m_buffer.obj = rhs.m_buffer.obj;
        }

        return *this;
    }

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(const Optional& rhs)
        requires(base::isTriviallyCopyAssignable<T>)
    = default;

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(Optional&& rhs) noexcept
        requires(!base::isTriviallyMoveAssignable<T> && base::isMoveAssignable<T>)
    {
        if (&rhs == this || (!m_engaged && !rhs.m_engaged))
            return *this;

        if (m_engaged && !rhs.m_engaged)
        {
            m_engaged = false;
            SFML_PRIV_OPTIONAL_DESTROY(T, m_buffer);
        }
        else if (!m_engaged && rhs.m_engaged)
        {
            m_engaged = true;
            SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_MOVE(rhs.m_buffer.obj));
        }
        else
        {
            SFML_BASE_ASSERT(m_engaged && rhs.m_engaged);
            m_buffer.obj = SFML_BASE_MOVE(rhs.m_buffer.obj);
        }

        return *this;
    }

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr Optional& operator=(Optional&& rhs)
        requires(base::isTriviallyMoveAssignable<T>)
    = default;


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, const Optional&) = delete;


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, Optional&&) = delete;

    //////////////////////////////////////////
    template <typename... Args>
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, Args&&... args) : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(args)...);
    }

    //////////////////////////////////////////
    template <typename F>
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(FromFunc, F&& func) : m_engaged{true}
    {
        SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(func)());
    }

    //////////////////////////////////////////
    template <typename... Args>
    [[gnu::always_inline]] constexpr T& emplace(Args&&... args)
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
        m_engaged = true;

        return *(SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(args)...));
    }

    //////////////////////////////////////////
    template <typename F>
    [[gnu::always_inline]] constexpr T& emplaceFromFunc(F&& func)
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
        m_engaged = true;

        return *(SFML_BASE_PLACEMENT_NEW(&m_buffer.obj) T(SFML_BASE_FORWARD(func)()));
    }

    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr void reset() noexcept
    {
        SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED(T, m_engaged, m_buffer);
        m_engaged = false;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T& value() &
    {
        if (!m_engaged) [[unlikely]]
            priv::throwIfNotEngaged();

        return m_buffer.obj;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const T& value() const&
    {
        if (!m_engaged) [[unlikely]]
            priv::throwIfNotEngaged();

        return m_buffer.obj;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T&& value() &&
    {
        if (!m_engaged) [[unlikely]]
            priv::throwIfNotEngaged();

        return SFML_BASE_MOVE(m_buffer.obj);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& valueOr(T& defaultValue) & noexcept
    {
        return m_engaged ? m_buffer.obj : defaultValue;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& valueOr(const T& defaultValue) const& noexcept
    {
        return m_engaged ? m_buffer.obj : defaultValue;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T&& valueOr(T&& defaultValue) && noexcept
    {
        return SFML_BASE_MOVE(m_engaged ? m_buffer.obj : defaultValue);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool hasValue() const noexcept
    {
        return m_engaged;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr explicit operator bool() const noexcept
    {
        return m_engaged;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* operator->() & noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return &m_buffer.obj;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* operator->() const& noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return &m_buffer.obj;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator*() & noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return m_buffer.obj;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& operator*() const& noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return m_buffer.obj;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T&& operator*() && noexcept
    {
        SFML_BASE_ASSERT(m_engaged);
        return SFML_BASE_MOVE(m_buffer.obj);
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* asPtr() noexcept
    {
        return m_engaged ? &m_buffer.obj : nullptr;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* asPtr() const noexcept
    {
        return m_engaged ? &m_buffer.obj : nullptr;
    }

    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator==(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
    {
        return lhs.m_engaged == rhs.m_engaged && (!lhs.m_engaged || *lhs == *rhs);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator!=(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
    {
        return lhs.m_engaged != rhs.m_engaged || (lhs.m_engaged && *lhs != *rhs);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator>=(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
    {
        return !rhs.m_engaged || (lhs.m_engaged && *lhs >= *rhs);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator<=(const Optional& lhs,
                                                                                         const Optional& rhs) noexcept
    {
        return !lhs.m_engaged || (rhs.m_engaged && *lhs <= *rhs);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator>(const Optional& lhs,
                                                                                        const Optional& rhs) noexcept
    {
        return lhs.m_engaged && (!rhs.m_engaged || *lhs > *rhs);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend inline constexpr bool operator<(const Optional& lhs,
                                                                                        const Optional& rhs) noexcept
    {
        return rhs.m_engaged && (!lhs.m_engaged || *lhs < *rhs);
    }

private:
    union Buffer
    {
        char dummy{}; // Needed by GCC for constant expression support
        T    obj;

        // clang-format off
        constexpr Buffer() requires(base::isTriviallyConstructible<T>) = default;
        constexpr Buffer() requires(!base::isTriviallyConstructible<T>) { }

        constexpr ~Buffer() requires(base::isTriviallyDestructible<T>) = default;
        constexpr ~Buffer() requires(!base::isTriviallyDestructible<T>) { }

        constexpr Buffer(const Buffer&) = default;
        constexpr Buffer& operator=(const Buffer&) = default;

        constexpr Buffer(Buffer&&) = default;
        constexpr Buffer& operator=(Buffer&&) = default;
        // clang-format on
    } m_buffer;

    bool m_engaged;
};

////////////////////////////////////////////////////////////
#undef SFML_PRIV_OPTIONAL_DESTROY
#undef SFML_PRIV_OPTIONAL_DESTROY_IF_ENGAGED

////////////////////////////////////////////////////////////
template <typename Object>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto makeOptional(Object&& object)
{
    return Optional<SFML_BASE_REMOVE_CVREF(Object)>{SFML_BASE_FORWARD(object)};
}

////////////////////////////////////////////////////////////
template <typename T, typename... Args>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Optional<T> makeOptional(Args&&... args)
{
    return Optional<T>{inPlace, SFML_BASE_FORWARD(args)...};
}

////////////////////////////////////////////////////////////
template <typename F>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr auto makeOptionalFromFunc(F&& f)
{
    return Optional<decltype(SFML_BASE_FORWARD(f)())>{fromFunc, SFML_BASE_FORWARD(f)};
}

} // namespace sf::base
