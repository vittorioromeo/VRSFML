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
    [[nodiscard]] AutoWrapAngle() = default;

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
    [[gnu::always_inline, gnu::flatten]] constexpr AutoWrapAngle& operator+=(Angle rhs)
    {
        m_radians += rhs.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr AutoWrapAngle& operator-=(Angle rhs)
    {
        m_radians -= rhs.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr AutoWrapAngle& operator*=(float rhs)
    {
        m_radians *= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr AutoWrapAngle& operator/=(float rhs)
    {
        m_radians /= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr AutoWrapAngle& operator%=(Angle rhs)
    {
        return *this = (operator Angle() % rhs);
    }

private:
    float m_radians{0.f};
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Angle
/// \ingroup system
///
/// `sf::Angle` encapsulates an angle value in a flexible way.
/// It allows for defining an angle value either as a number
/// of degrees or radians. It also works the other way
/// around. You can read an angle value as either a number
/// of degrees or radians.
///
/// By using such a flexible interface, the API doesn't
/// impose any fixed type or unit for angle values and lets
/// the user choose their own preferred representation.
///
/// Angle values support the usual mathematical operations.
/// You can add or subtract two angles, multiply or divide
/// an angle by a number, compare two angles, etc.
///
/// Usage example:
/// \code
/// sf::Angle a1  = sf::degrees(90);
/// float radians = a1.asRadians(); // 1.5708f
///
/// sf::Angle a2 = sf::radians(3.141592654f);
/// float degrees = a2.asDegrees(); // 180.0f
///
/// using namespace sf::Literals;
/// sf::Angle a3 = 10_deg;   // 10 degrees
/// sf::Angle a4 = 1.5_deg;  // 1.5 degrees
/// sf::Angle a5 = 1_rad;    // 1 radians
/// sf::Angle a6 = 3.14_rad; // 3.14 radians
/// \endcode
///
////////////////////////////////////////////////////////////
