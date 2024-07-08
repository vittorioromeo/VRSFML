// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/System/Launder.hpp>
#include <SFML/System/MaxAlignT.hpp>
#include <SFML/System/PlacementNew.hpp>
#include <SFML/System/SizeT.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T, SizeT BufferSize>
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class InPlacePImpl
{
private:
    alignas(MaxAlignT) char m_buffer[BufferSize];

public:
    [[nodiscard, gnu::always_inline, gnu::pure]] T* operator->() noexcept
    {
        return SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] const T* operator->() const noexcept
    {
        return SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] T& operator*() noexcept
    {
        return *SFML_PRIV_LAUNDER_CAST(T*, m_buffer);
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] const T& operator*() const noexcept
    {
        return *SFML_PRIV_LAUNDER_CAST(const T*, m_buffer);
    }

    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] explicit InPlacePImpl(Args&&... args)
    {
        static_assert(sizeof(T) <= BufferSize);
        SFML_PRIV_PLACEMENT_NEW(m_buffer) T(static_cast<Args&&>(args)...);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(const InPlacePImpl& rhs)
    {
        SFML_PRIV_PLACEMENT_NEW(m_buffer) T(*SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(InPlacePImpl&& rhs) noexcept
    {
        SFML_PRIV_PLACEMENT_NEW(m_buffer) T(static_cast<T&&>(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer)));
    }

    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
    [[gnu::always_inline]] InPlacePImpl& operator=(const InPlacePImpl& rhs)
    {
        // Rely on the inner type for self-assignment check.
        *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) = *SFML_PRIV_LAUNDER_CAST(const T*, rhs.m_buffer);
        return *this;
    }

    [[gnu::always_inline]] InPlacePImpl& operator=(InPlacePImpl&& rhs) noexcept
    {
        // Rely on the inner type for self-assignment check.
        *SFML_PRIV_LAUNDER_CAST(T*, m_buffer) = static_cast<T&&>(*SFML_PRIV_LAUNDER_CAST(T*, rhs.m_buffer));
        return *this;
    }

    [[gnu::always_inline]] ~InPlacePImpl()
    {
        SFML_PRIV_LAUNDER_CAST(T*, m_buffer)->~T();
    }
};

} // namespace sf::priv
