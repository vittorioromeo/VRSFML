#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
class [[nodiscard]] Angle
{
public:
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float asDegrees() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the angle's value in radians
    ///
    /// \return Angle in radians
    ///
    /// \see `asDegrees`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float asRadians() const;

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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle wrapSigned() const;

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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle wrapUnsigned() const;

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

private:
    friend AutoWrapAngle;

    friend constexpr Angle degrees(float angle);
    friend constexpr Angle radians(float angle);

    friend constexpr bool operator==(Angle lhs, Angle rhs);
    friend constexpr bool operator!=(Angle lhs, Angle rhs);
    friend constexpr bool operator<(Angle lhs, Angle rhs);
    friend constexpr bool operator>(Angle lhs, Angle rhs);
    friend constexpr bool operator<=(Angle lhs, Angle rhs);
    friend constexpr bool operator>=(Angle lhs, Angle rhs);

    friend constexpr Angle operator-(Angle rhs);

    friend constexpr Angle  operator+(Angle lhs, Angle rhs);
    friend constexpr Angle& operator+=(Angle& lhs, Angle rhs);

    friend constexpr Angle  operator-(Angle lhs, Angle rhs);
    friend constexpr Angle& operator-=(Angle& lhs, Angle rhs);

    friend constexpr Angle  operator*(Angle lhs, float rhs);
    friend constexpr Angle  operator*(float lhs, Angle rhs);
    friend constexpr Angle& operator*=(Angle& lhs, float rhs);

    friend constexpr Angle  operator/(Angle lhs, float rhs);
    friend constexpr Angle& operator/=(Angle& lhs, float rhs);
    friend constexpr float  operator/(Angle lhs, Angle rhs);

    friend constexpr Angle  operator%(Angle lhs, Angle rhs);
    friend constexpr Angle& operator%=(Angle& lhs, Angle rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a number of radians
    ///
    /// This function is internal. To construct angle values,
    /// use `sf::radians` or `sf::degrees` instead.
    ///
    /// \param radians Angle in radians
    ///
    ////////////////////////////////////////////////////////////
    constexpr explicit Angle(float radians);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    float m_radians{0.f}; //!< Angle value stored as radians
};

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle degrees(float angle);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle radians(float angle);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of `operator==` to compare two angle values
/// \note Does not automatically wrap the angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return `true` if both angle values are equal
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool operator==(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of `operator!=` to compare two angle values
/// \note Does not automatically wrap the angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return `true` if both angle values are different
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool operator!=(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of `operator<` to compare two angle values
/// \note Does not automatically wrap the angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return `true` if \a lhs is less than \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool operator<(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of `operator>` to compare two angle values
/// \note Does not automatically wrap the angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return `true` if \a lhs is greater than \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool operator>(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of `operator<=` to compare two angle values
/// \note Does not automatically wrap the angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return `true` if \a lhs is less than or equal to \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool operator<=(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of `operator>=` to compare two angle values
/// \note Does not automatically wrap the angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return `true` if \a lhs is greater than or equal to \a rhs
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool operator>=(Angle lhs, Angle rhs);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator-(Angle rhs);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator+(Angle lhs, Angle rhs);

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
[[gnu::always_inline, gnu::flatten]] constexpr Angle& operator+=(Angle& lhs, Angle rhs);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator-(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator-=` to subtract/assign two angle values
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return Difference of the two angle values
///
////////////////////////////////////////////////////////////ù
[[gnu::always_inline, gnu::flatten]] constexpr Angle& operator-=(Angle& lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator*` to scale an angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (a number)
///
/// \return \a `lhs` multiplied by \a `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator*(Angle lhs, float rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator*` to scale an angle value
///
/// \param lhs  Left operand (a number)
/// \param rhs Right operand (an angle)
///
/// \return \a `lhs` multiplied by \a `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator*(float lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator*=` to scale/assign an angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (a number)
///
/// \return \a `lhs` multiplied by \a `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] constexpr Angle& operator*=(Angle& lhs, float rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator/` to scale an angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (a number)
///
/// \return \a `lhs` divided by \a `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator/(Angle lhs, float rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator/=` to scale/assign an angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (a number)
///
/// \return \a `lhs` divided by \a `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] constexpr Angle& operator/=(Angle& lhs, float rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator/` to compute the ratio of two angle values
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return \a `lhs` divided by \a `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr float operator/(Angle lhs, Angle rhs);

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
/// \return \a `lhs` modulo \a `rhs`
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator%(Angle lhs, Angle rhs);

////////////////////////////////////////////////////////////
/// \relates Angle
/// \brief Overload of binary `operator%=` to compute/assign remainder of an angle value
///
/// \param lhs  Left operand (an angle)
/// \param rhs Right operand (an angle)
///
/// \return \a `lhs` modulo \a `rhs`
///
////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] constexpr Angle& operator%=(Angle& lhs, Angle rhs);

namespace Literals
{

////////////////////////////////////////////////////////////
/// \relates `sf::Angle`
/// \brief User defined literal for angles in degrees, e.g. `10.5_deg`
///
/// \param angle Angle in degrees
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator""_deg(long double angle);

////////////////////////////////////////////////////////////
/// \relates `sf::Angle`
/// \brief User defined literal for angles in degrees, e.g. `90_deg`
///
/// \param angle Angle in degrees
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator""_deg(unsigned long long int angle);

////////////////////////////////////////////////////////////
/// \relates `sf::Angle`
/// \brief User defined literal for angles in radians, e.g. `0.1_rad`
///
/// \param angle Angle in radians
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator""_rad(long double angle);

////////////////////////////////////////////////////////////
/// \relates `sf::Angle`
/// \brief User defined literal for angles in radians, e.g. `2_rad`
///
/// \param angle Angle in radians
///
/// \return Angle
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle operator""_rad(unsigned long long int angle);

} // namespace Literals
} // namespace sf

#include "SFML/System/Angle.inl"


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
