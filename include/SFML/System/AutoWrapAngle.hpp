#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] AutoWrapAngle
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr AutoWrapAngle() = default;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr explicit(false) AutoWrapAngle(Angle angle) :
    m_radians(angle.m_radians)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator=(Angle angle) noexcept
    {
        m_radians = angle.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr operator Angle() const noexcept
    {
        return Angle(m_radians).wrapUnsigned();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Return the angle's value in degrees
    ///
    /// \return Angle in degrees
    ///
    /// \see `asRadians`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float asDegrees() const
    {
        return operator Angle().asDegrees();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Return the angle's value in radians
    ///
    /// \return Angle in radians
    ///
    /// \see `asDegrees`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float asRadians() const
    {
        return operator Angle().asRadians();
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle rotatedTowards(Angle other, float speed) const
    {
        return operator Angle().rotatedTowards(other, speed);
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    friend constexpr bool operator==(AutoWrapAngle lhs, AutoWrapAngle rhs) = default;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator+=(Angle rhs)
    {
        m_radians += rhs.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator-=(Angle rhs)
    {
        m_radians -= rhs.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator*=(float rhs)
    {
        m_radians *= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator/=(float rhs)
    {
        m_radians /= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator%=(Angle rhs)
    {
        return *this = (operator Angle() % rhs);
    }

private:
    float m_radians{0.f};
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::AutoWrapAngle
/// \ingroup system
///
/// TODO P1: docs
///
////////////////////////////////////////////////////////////
