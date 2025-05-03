#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief A wrapper around `sf::Angle` that automatically wraps the angle value
///
/// This class behaves similarly to `sf::Angle` but automatically wraps
/// the angle to the range `[0, 360)` degrees (or `[0, 2*Pi)` radians)
/// whenever its value is accessed or used in comparisons/operations.
///
/// This is useful for representing properties like rotation where only
/// the final orientation matters, regardless of the number of full turns.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] AutoWrapAngle
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Initializes the angle to 0 degrees.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr AutoWrapAngle() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an `sf::Angle`
    ///
    /// \param angle Angle to initialize from.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr explicit(false) AutoWrapAngle(Angle angle) :
    m_radians(angle.m_radians)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Assign an `sf::Angle`
    ///
    /// \param angle Angle to assign.
    /// \return Reference to `*this`.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator=(Angle angle) noexcept
    {
        m_radians = angle.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Implicit conversion to `sf::Angle`
    ///
    /// Returns the angle value, wrapped to the range `[0, 360)`.
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
    /// \brief Rotate towards another angle by a maximum step.
    ///
    /// Wraps the current angle before performing the rotation.
    /// See `sf::Angle::rotatedTowards` for details.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle rotatedTowards(Angle other, float speed) const
    {
        return operator Angle().rotatedTowards(other, speed);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Equality comparison operator
    ///
    /// Compares the underlying radian values without wrapping.
    ///
    ////////////////////////////////////////////////////////////
    friend constexpr bool operator==(AutoWrapAngle lhs, AutoWrapAngle rhs) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Addition assignment operator
    ///
    /// Adds `rhs` to the current angle value.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator+=(Angle rhs)
    {
        m_radians += rhs.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Subtraction assignment operator
    ///
    /// Subtracts `rhs` from the current angle value.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator-=(Angle rhs)
    {
        m_radians -= rhs.m_radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Multiplication assignment operator
    ///
    /// Multiplies the current angle value by `rhs`.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator*=(float rhs)
    {
        m_radians *= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Division assignment operator
    ///
    /// Divides the current angle value by `rhs`. Asserts if `rhs` is 0.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator/=(float rhs)
    {
        m_radians /= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Modulo assignment operator
    ///
    /// Assigns the result of `*this % rhs` to `*this`. Wraps the current angle
    /// before performing the modulo operation. See `sf::Angle::operator%`.
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
/// `sf::AutoWrapAngle` is a wrapper around `sf::Angle` that automatically
/// normalizes the angle to the range `[0, 360)` degrees (or `[0, 2*Pi)` radians)
/// upon access or use in operations.
///
/// This is particularly useful for representing properties like rotation
/// where angles outside the standard range are equivalent (e.g., 450 degrees
/// is the same orientation as 90 degrees). Using `AutoWrapAngle` ensures
/// that comparisons and operations work intuitively in such cases.
///
/// \see `sf::Angle`
///
////////////////////////////////////////////////////////////
