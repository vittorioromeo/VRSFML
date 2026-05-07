#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp"

#include "SFML/Base/Assert.hpp"


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
    /// \brief Default-construct to 0 degrees
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr AutoWrapAngle() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from an `sf::Angle`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr /* implicit */ AutoWrapAngle(const Angle angle) :
        m_radians(angle.radians)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Assign an `sf::Angle`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator=(const Angle angle) noexcept
    {
        m_radians = angle.radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Implicitly convert to `sf::Angle` wrapped into `[0, 360)`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr operator Angle() const noexcept
    {
        return radians(m_radians).wrapUnsigned();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Wrapped value in degrees
    ///
    /// \see `asRadians`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float asDegrees() const
    {
        return operator Angle().asDegrees();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Wrapped value in radians
    ///
    /// \see `asDegrees`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float asRadians() const
    {
        return operator Angle().asRadians();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Rotate towards `other` by at most `speed`, wrapping first (see `sf::Angle::rotatedTowards`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle rotatedTowards(const Angle other,
                                                                                                     const float speed) const
    {
        return operator Angle().rotatedTowards(other, speed);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Equality of the wrapped angle values
    ///
    ////////////////////////////////////////////////////////////
    friend constexpr bool operator==(AutoWrapAngle lhs, AutoWrapAngle rhs)
    {
        return Angle(lhs) == Angle(rhs);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add `rhs` to the (unwrapped) angle value
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator+=(const Angle rhs)
    {
        m_radians += rhs.radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Subtract `rhs` from the (unwrapped) angle value
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator-=(const Angle rhs)
    {
        m_radians -= rhs.radians;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Multiply the (unwrapped) angle value by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator*=(const float rhs)
    {
        m_radians *= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Divide the (unwrapped) angle value by `rhs` (asserts `rhs != 0`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator/=(const float rhs)
    {
        SFML_BASE_ASSERT(rhs != 0.f);

        m_radians /= rhs;
        return *this;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Assign `*this % rhs`, wrapping the current angle first (see `sf::Angle::operator%`)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] inline constexpr AutoWrapAngle& operator%=(const Angle rhs)
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
