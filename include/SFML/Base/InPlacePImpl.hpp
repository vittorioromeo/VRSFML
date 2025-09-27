#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/MaxAlignT.hpp"
#include "SFML/Base/PlacementNew.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T, decltype(sizeof(int)) BufferSize>
class InPlacePImpl
{
private:
    alignas(MaxAlignT) char m_buffer[BufferSize];

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] T* operator->() noexcept
    {
        return SFML_BASE_LAUNDER_CAST(T*, m_buffer);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const T* operator->() const noexcept
    {
        return SFML_BASE_LAUNDER_CAST(const T*, m_buffer);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] T& operator*() noexcept
    {
        return *SFML_BASE_LAUNDER_CAST(T*, m_buffer);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const T& operator*() const noexcept
    {
        return *SFML_BASE_LAUNDER_CAST(const T*, m_buffer);
    }


    ////////////////////////////////////////////////////////////
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] explicit InPlacePImpl(Args&&... args)
    {
        static_assert(sizeof(T) <= BufferSize);
        SFML_BASE_PLACEMENT_NEW(m_buffer) T(static_cast<Args&&>(args)...);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(const InPlacePImpl& rhs)
    {
        SFML_BASE_PLACEMENT_NEW(m_buffer) T(*SFML_BASE_LAUNDER_CAST(const T*, rhs.m_buffer));
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlacePImpl(InPlacePImpl&& rhs) noexcept
    {
        SFML_BASE_PLACEMENT_NEW(m_buffer) T(static_cast<T&&>(*SFML_BASE_LAUNDER_CAST(T*, rhs.m_buffer)));
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
    [[gnu::always_inline]] InPlacePImpl& operator=(const InPlacePImpl& rhs)
    {
        // Rely on the inner type for self-assignment check.
        *SFML_BASE_LAUNDER_CAST(T*, m_buffer) = *SFML_BASE_LAUNDER_CAST(const T*, rhs.m_buffer);
        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] InPlacePImpl& operator=(InPlacePImpl&& rhs) noexcept
    {
        // Rely on the inner type for self-assignment check.
        *SFML_BASE_LAUNDER_CAST(T*, m_buffer) = static_cast<T&&>(*SFML_BASE_LAUNDER_CAST(T*, rhs.m_buffer));
        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~InPlacePImpl()
    {
        SFML_BASE_LAUNDER_CAST(T*, m_buffer)->~T();
    }
};

} // namespace sf::base
