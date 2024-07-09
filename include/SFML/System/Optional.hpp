////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/Launder.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/PlacementNew.hpp>
#include <SFML/System/RemoveCVRef.hpp>

#include <cassert>


namespace sf
{
////////////////////////////////////////////////////////////
inline constexpr struct InPlace
{
} inPlace;

////////////////////////////////////////////////////////////
inline constexpr struct NullOpt
{
} nullOpt;

////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Optional
{
public:
    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional() noexcept : m_engaged{false}
    {
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr Optional(NullOpt) noexcept : m_engaged{false}
    {
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(const T& object) : m_engaged{true}
    {
        SFML_PRIV_PLACEMENT_NEW(m_buffer) T(object);
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(T&& object) noexcept : m_engaged{true}
    {
        SFML_PRIV_PLACEMENT_NEW(m_buffer) T(SFML_MOVE(object));
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(const Optional& rhs) : m_engaged{rhs.m_engaged}
    {
        if (m_engaged)
            SFML_PRIV_PLACEMENT_NEW(m_buffer) T(*SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer));
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr explicit(false) Optional(Optional&& rhs) noexcept :
    m_engaged{rhs.m_engaged}
    {
        if (m_engaged)
            SFML_PRIV_PLACEMENT_NEW(m_buffer) T(SFML_MOVE(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer)));
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[gnu::always_inline]] constexpr Optional& operator=(const Optional& rhs)
    {
        if (&rhs == this || (!m_engaged && !rhs.m_engaged))
            return *this;

        if (m_engaged && !rhs.m_engaged)
        {
            m_engaged = false;
            SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();
        }
        else if (!m_engaged && rhs.m_engaged)
        {
            m_engaged = true;
            SFML_PRIV_PLACEMENT_NEW(m_buffer) T(*SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer));
        }
        else
        {
            assert(m_engaged && rhs.m_engaged);
            *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) = *SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer);
        }

        return *this;
    }


    //////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[gnu::always_inline]] constexpr Optional& operator=(Optional&& rhs) noexcept
    {
        if (&rhs == this || (!m_engaged && !rhs.m_engaged))
            return *this;

        if (m_engaged && !rhs.m_engaged)
        {
            m_engaged = false;
            SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();
        }
        else if (!m_engaged && rhs.m_engaged)
        {
            m_engaged = true;
            SFML_PRIV_PLACEMENT_NEW(m_buffer) T(SFML_MOVE(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer)));
        }
        else
        {
            assert(m_engaged && rhs.m_engaged);
            *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) = SFML_MOVE(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer));
        }

        return *this;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, const Optional&) = delete;


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, Optional&&) = delete;


    //////////////////////////////////////////
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr explicit Optional(InPlace, Args&&... args) : m_engaged{true}
    {
        SFML_PRIV_PLACEMENT_NEW(m_buffer) T(SFML_FORWARD(args)...);
    }


    //////////////////////////////////////////
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[gnu::always_inline]] constexpr T& emplace(Args&&... args)
    {
        if (m_engaged)
            SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();

        m_engaged = true;
        return *(SFML_PRIV_PLACEMENT_NEW(m_buffer) T(SFML_FORWARD(args)...));
    }


    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr void reset() noexcept
    {
        if (m_engaged)
            SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();

        m_engaged = false;
    }


    //////////////////////////////////////////
    [[gnu::always_inline]] constexpr ~Optional() noexcept
    {
        if (m_engaged)
            SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T& value() &
    {
        if (!m_engaged)
            throw 0; // TODO

        return *SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr const T& value() const&
    {
        if (!m_engaged)
            throw 0; // TODO

        return *SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr T&& value() &&
    {
        if (!m_engaged)
            throw 0; // TODO

        return SFML_MOVE(*SFML_PRIV_LAUNDER_CAST(T*, m_buffer));
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& valueOr(T& defaultValue) & noexcept
    {
        return m_engaged ? *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) : defaultValue;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& valueOr(const T& defaultValue) const& noexcept
    {
        return m_engaged ? *SFML_PRIV_LAUNDER_CAST(const T*, m_buffer) : defaultValue;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T&& valueOr(T&& defaultValue) && noexcept
    {
        return SFML_MOVE(m_engaged ? *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) : defaultValue);
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
        assert(m_engaged);
        return SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* operator->() const& noexcept
    {
        assert(m_engaged);
        return SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator*() & noexcept
    {
        assert(m_engaged);
        return *SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T& operator*() const& noexcept
    {
        assert(m_engaged);
        return *SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T&& operator*() && noexcept
    {
        assert(m_engaged);
        return SFML_MOVE(*SFML_PRIV_LAUNDER_CAST(T*, m_buffer));
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* asPtr() noexcept
    {
        return m_engaged ? SFML_PRIV_LAUNDER_CAST(T*, m_buffer) : nullptr;
    }


    //////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* asPtr() const noexcept
    {
        return m_engaged ? SFML_PRIV_LAUNDER_CAST(const T*, m_buffer) : nullptr;
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
    alignas(T) char m_buffer[sizeof(T)];
    bool m_engaged;
};


////////////////////////////////////////////////////////////
template <typename Object>
[[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr auto makeOptional(Object&& object)
{
    return Optional<SFML_PRIV_REMOVE_CVREF(Object)>{SFML_FORWARD(object)};
}


////////////////////////////////////////////////////////////
template <typename T, typename... Args>
[[nodiscard, gnu::always_inline, gnu::pure]] inline constexpr Optional<T> makeOptional(Args&&... args)
{
    return Optional<T>{sf::inPlace, SFML_FORWARD(args)...};
}

} // namespace sf
