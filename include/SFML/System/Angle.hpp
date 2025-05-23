#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Remainder.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class AutoWrapAngle;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Represents an angle value.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] Angle
{
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Sets the angle value to zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr Angle() = default;


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
        return radians * (180.f / base::pi);
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
        return radians;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Wrap to a range such that -180° <= angle < 180°
    ///
    /// Similar to a modulo operation, this returns a copy of the angle
    /// constrained to the range [-180°, 180°) == [-Pi, Pi).
    /// The resulting angle represents a rotation which is equivalent to `*this`.
    ///
    /// The name "signed" originates from the similarity to signed integers:
    /// <table>
    /// <tr>
    ///   <th></th>
    ///   <th>signed</th>
    ///   <th>unsigned</th>
    /// </tr>
    /// <tr>
    ///   <td>char</td>
    ///   <td>[-128, 128)</td>
    ///   <td>[0, 256)</td>
    /// </tr>
    /// <tr>
    ///   <td>Angle</td>
    ///   <td>[-180°, 180°)</td>
    ///   <td>[0°, 360°)</td>
    /// </tr>
    /// </table>
    ///
    /// \return Signed angle, wrapped to [-180°, 180°)
    ///
    /// \see `wrapUnsigned`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle wrapSigned() const
    {
        return Angle(base::positiveRemainder(radians + base::pi, base::tau) - base::pi);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Wrap to a range such that 0° <= angle < 360°
    ///
    /// Similar to a modulo operation, this returns a copy of the angle
    /// constrained to the range [0°, 360°) == [0, Tau) == [0, 2*Pi).
    /// The resulting angle represents a rotation which is equivalent to `*this`.
    ///
    /// The name "unsigned" originates from the similarity to unsigned integers:
    /// <table>
    /// <tr>
    ///   <th></th>
    ///   <th>signed</th>
    ///   <th>unsigned</th>
    /// </tr>
    /// <tr>
    ///   <td>char</td>
    ///   <td>[-128, 128)</td>
    ///   <td>[0, 256)</td>
    /// </tr>
    /// <tr>
    ///   <td>Angle</td>
    ///   <td>[-180°, 180°)</td>
    ///   <td>[0°, 360°)</td>
    /// </tr>
    /// </table>
    ///
    /// \return Unsigned angle, wrapped to [0°, 360°)
    ///
    /// \see `wrapSigned`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle wrapUnsigned() const
    {
        return Angle(base::positiveRemainder(radians, base::tau));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Rotate towards another angle by a maximum step.
    ///
    /// Calculates the shortest difference between `*this` and `other` (handling wrapping)
    /// and returns a new angle by rotating `*this` towards `other` by at most `speed` radians.
    /// If the shortest difference is less than or equal to `speed`, `other` is returned.
    /// The result is normalized to the range `[0, 2*Pi)`.
    ///
    /// \param other Target angle to rotate towards.
    /// \param speed Maximum rotation step in radians. Must be non-negative.
    ///
    /// \return Angle rotated towards `other`, clamped by `speed`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle rotatedTowards(const Angle other,
                                                                                                     const float speed) const
    {
        float diff = base::remainder(other.radians - radians, base::tau);

        if (diff > base::pi)
            diff -= base::tau;
        else if (diff < -base::pi)
            diff += base::tau;

        if (SFML_BASE_MATH_FABSF(diff) <= speed)
            return Angle{other.radians};

        float result = radians;

        if (diff > 0.f)
            result += speed;
        else
            result -= speed;

        // Normalize to [0, base::tau)
        result = base::remainder(result, base::tau);

        if (result < 0.f)
            result += base::tau;

        return Angle{result};
    }


    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const Angle Zero; //!< Predefined 0 degree angle value
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const Angle Quarter; //!< Predefined 90 degree angle value
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const Angle Half; //!< Predefined 180 degree angle value
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const Angle Full; //!< Predefined 360 degree angle value

    friend AutoWrapAngle;


    ////////////////////////////////////////////////////////////
    friend constexpr Angle degrees(float angle);
    friend constexpr Angle radians(float angle);


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of `operator==` to compare two angle values
    /// \note Does not automatically wrap the angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `true` if `lhs` is equal to `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr bool operator==(Angle lhs, Angle rhs) = default;


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of `operator<` to compare two angle values
    /// \note Does not automatically wrap the angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `true` if `lhs` is less than `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr bool operator<(const Angle lhs, const Angle rhs)
    {
        return lhs.radians < rhs.radians;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of `operator>` to compare two angle values
    /// \note Does not automatically wrap the angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `true` if `lhs` is greater than `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr bool operator>(const Angle lhs, const Angle rhs)
    {
        return lhs.radians > rhs.radians;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of `operator<=` to compare two angle values
    /// \note Does not automatically wrap the angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `true` if `lhs` is less than or equal to `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr bool operator<=(const Angle lhs, const Angle rhs)
    {
        return lhs.radians <= rhs.radians;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of `operator>=` to compare two angle values
    /// \note Does not automatically wrap the angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `true` if `lhs` is greater than or equal to `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr bool operator>=(const Angle lhs, const Angle rhs)
    {
        return lhs.radians >= rhs.radians;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of unary `operator-` to negate an angle value.
    ///
    /// Represents a rotation in the opposite direction.
    ///
    /// \param rhs Right operand (an angle)
    ///
    /// \return Negative of the angle value
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator-(const Angle rhs)
    {
        return Angle(-rhs.radians);
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator+` to add two angle values
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return Sum of the two angle values
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator+(const Angle lhs, const Angle rhs)
    {
        return Angle(lhs.radians + rhs.radians);
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator+=` to add/assign two angle values
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return Sum of the two angle values
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] friend constexpr Angle& operator+=(Angle& lhs, const Angle rhs)
    {
        lhs.radians += rhs.radians;
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator-` to subtract two angle values
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return Difference of the two angle values
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator-(const Angle lhs, const Angle rhs)
    {
        return Angle(lhs.radians - rhs.radians);
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator-=` to subtract/assign two angle values
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return Difference of the two angle values
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] friend constexpr Angle& operator-=(Angle& lhs, const Angle rhs)
    {
        lhs.radians -= rhs.radians;
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator*` to scale an angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (a number)
    ///
    /// \return `lhs` multiplied by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator*(const Angle lhs, const float rhs)
    {
        return Angle(lhs.radians * rhs);
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator*` to scale an angle value
    ///
    /// \param lhs  Left operand (a number)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `lhs` multiplied by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator*(const float lhs, const Angle rhs)
    {
        return rhs * lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator*=` to scale/assign an angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (a number)
    ///
    /// \return `lhs` multiplied by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] friend constexpr Angle& operator*=(Angle& lhs, const float rhs)
    {
        lhs.radians *= rhs;
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator/` to scale an angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (a number)
    ///
    /// \return `lhs` divided by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator/(const Angle lhs, const float rhs)
    {
        SFML_BASE_ASSERT(rhs != 0.f && "Angle::operator/ cannot divide by 0");
        return Angle(lhs.radians / rhs);
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator/=` to scale/assign an angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (a number)
    ///
    /// \return `lhs` divided by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] friend constexpr Angle& operator/=(Angle& lhs, const float rhs)
    {
        SFML_BASE_ASSERT(rhs != 0.f && "Angle::operator/= cannot divide by 0");
        lhs.radians /= rhs;
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator/` to compute the ratio of two angle values
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `lhs` divided by `rhs`
    ///
    ////////////////////////////////////////////////////////////
    friend constexpr float operator/(const Angle lhs, const Angle rhs)
    {
        SFML_BASE_ASSERT(rhs.radians != 0.f && "Angle::operator/ cannot divide by 0");
        return lhs.radians / rhs.radians;
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator%` to compute modulo of an angle value.
    ///
    /// Right hand angle must be greater than zero.
    ///
    /// Examples:
    /// \code
    /// sf::degrees(90) % sf::degrees(40)  // 10 degrees
    /// sf::degrees(-90) % sf::degrees(40) // 30 degrees (not -10)
    /// \endcode
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `lhs` modulo `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] friend constexpr Angle operator%(const Angle lhs, const Angle rhs)
    {
        SFML_BASE_ASSERT(rhs.radians != 0.f && "Angle::operator% cannot modulus by 0");
        return Angle(base::positiveRemainder(lhs.radians, rhs.radians));
    }


    ////////////////////////////////////////////////////////////
    /// \relates Angle
    /// \brief Overload of binary `operator%=` to compute/assign remainder of an angle value
    ///
    /// \param lhs  Left operand (an angle)
    /// \param rhs Right operand (an angle)
    ///
    /// \return `lhs` modulo `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] friend constexpr Angle& operator%=(Angle& lhs, const Angle rhs)
    {
        SFML_BASE_ASSERT(rhs.radians != 0.f && "Angle::operator%= cannot modulus by 0");
        lhs.radians = base::positiveRemainder(lhs.radians, rhs.radians);
        return lhs;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct from a number of radians
    ///
    /// This function is internal. To construct angle values,
    /// use `sf::radians` or `sf::degrees` instead.
    ///
    /// \param radians Angle in radians
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] constexpr explicit Angle(const float radians) : radians(radians)
    {
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    float radians{0.f}; //!< Angle value stored as radians
};


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr Angle Angle::Zero{}; //!< Predefined 0 degree angle value


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr Angle Angle::Quarter{base::halfPi}; //!< Predefined 90 degree angle value


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr Angle Angle::Half{base::pi}; //!< Predefined 180 degree angle value


////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr Angle Angle::Full{base::tau}; //!< Predefined 360 degree angle value


////////////////////////////////////////////////////////////
/// \brief Construct an angle value from a number of degrees
///
/// \param angle Number of degrees
///
/// \return Angle value constructed from the number of degrees
///
/// \see `radians`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Angle degrees(const float angle)
{
    return Angle(angle * (base::pi / 180.f));
}


////////////////////////////////////////////////////////////
/// \brief Construct an angle value from a number of radians
///
/// \param angle Number of radians
///
/// \return Angle value constructed from the number of radians
///
/// \see `degrees`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Angle radians(const float angle)
{
    return Angle(angle);
}


namespace Literals
{
////////////////////////////////////////////////////////////
/// \relates sf::Angle
/// \brief User defined literal for angles in degrees, e.g.\ `10.5_deg`
///
/// \param angle Angle in degrees
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline consteval Angle operator""_deg(const long double angle)
{
    return degrees(static_cast<float>(angle));
}


////////////////////////////////////////////////////////////
/// \relates sf::Angle
/// \brief User defined literal for angles in degrees, e.g.\ `90_deg`
///
/// \param angle Angle in degrees
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline consteval Angle operator""_deg(const unsigned long long int angle)
{
    return degrees(static_cast<float>(angle));
}


////////////////////////////////////////////////////////////
/// \relates sf::Angle
/// \brief User defined literal for angles in radians, e.g.\ `0.1_rad`
///
/// \param angle Angle in radians
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline consteval Angle operator""_rad(const long double angle)
{
    return radians(static_cast<float>(angle));
}

////////////////////////////////////////////////////////////
/// \relates sf::Angle
/// \brief User defined literal for angles in radians, e.g.\ `2_rad`
///
/// \param angle Angle in radians
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline consteval Angle operator""_rad(const unsigned long long int angle)
{
    return radians(static_cast<float>(angle));
}

} // namespace Literals
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
/// float degrees = a2.asDegrees(); // 180.f
///
/// using namespace sf::Literals;
/// sf::Angle a3 = 10_deg;   // 10 degrees
/// sf::Angle a4 = 1.5_deg;  // 1.5 degrees
/// sf::Angle a5 = 1_rad;    // 1 radians
/// sf::Angle a6 = 3.14_rad; // 3.14 radians
/// \endcode
///
////////////////////////////////////////////////////////////
