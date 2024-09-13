#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Math/Fmod.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
inline constexpr float pi  = 3.141592654f;
inline constexpr float tau = pi * 2.f;


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float positiveRemainder(float a, float b)
{
    SFML_BASE_ASSERT(b > 0.f && "Cannot calculate remainder with non-positive divisor");

    const float val = a - static_cast<float>(static_cast<int>(a / b)) * b;
    return val >= 0.f ? val : val + b;
}

} // namespace sf::priv

namespace sf
{
////////////////////////////////////////////////////////////
constexpr float Angle::asDegrees() const
{
    return m_radians * (180.f / priv::pi);
}


////////////////////////////////////////////////////////////
constexpr float Angle::asRadians() const
{
    return m_radians;
}


////////////////////////////////////////////////////////////
constexpr Angle Angle::wrapSigned() const
{
    return radians(priv::positiveRemainder(m_radians + priv::pi, priv::tau) - priv::pi);
}


////////////////////////////////////////////////////////////
constexpr Angle Angle::wrapUnsigned() const
{
    return radians(priv::positiveRemainder(m_radians, priv::tau));
}


////////////////////////////////////////////////////////////
constexpr Angle::Angle(float radians) : m_radians(radians)
{
}


////////////////////////////////////////////////////////////
constexpr Angle degrees(float angle)
{
    return Angle(angle * (priv::pi / 180.f));
}


////////////////////////////////////////////////////////////
constexpr Angle radians(float angle)
{
    return Angle(angle);
}


////////////////////////////////////////////////////////////
constexpr bool operator==(Angle left, Angle right)
{
    return left.asRadians() == right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr bool operator!=(Angle left, Angle right)
{
    return left.asRadians() != right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr bool operator<(Angle left, Angle right)
{
    return left.asRadians() < right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr bool operator>(Angle left, Angle right)
{
    return left.asRadians() > right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr bool operator<=(Angle left, Angle right)
{
    return left.asRadians() <= right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr bool operator>=(Angle left, Angle right)
{
    return left.asRadians() >= right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr Angle operator-(Angle right)
{
    return radians(-right.asRadians());
}


////////////////////////////////////////////////////////////
constexpr Angle operator+(Angle left, Angle right)
{
    return radians(left.asRadians() + right.asRadians());
}


////////////////////////////////////////////////////////////
constexpr Angle& operator+=(Angle& left, Angle right)
{
    return left = left + right;
}


////////////////////////////////////////////////////////////
constexpr Angle operator-(Angle left, Angle right)
{
    return radians(left.asRadians() - right.asRadians());
}


////////////////////////////////////////////////////////////
constexpr Angle& operator-=(Angle& left, Angle right)
{
    return left = left - right;
}


////////////////////////////////////////////////////////////
constexpr Angle operator*(Angle left, float right)
{
    return radians(left.asRadians() * right);
}


////////////////////////////////////////////////////////////
constexpr Angle operator*(float left, Angle right)
{
    return right * left;
}


////////////////////////////////////////////////////////////
constexpr Angle& operator*=(Angle& left, float right)
{
    return left = left * right;
}


////////////////////////////////////////////////////////////
constexpr Angle operator/(Angle left, float right)
{
    SFML_BASE_ASSERT(right != 0.f && "Angle::operator/ cannot divide by 0");
    return radians(left.asRadians() / right);
}


////////////////////////////////////////////////////////////
constexpr Angle& operator/=(Angle& left, float right)
{
    SFML_BASE_ASSERT(right != 0.f && "Angle::operator/= cannot divide by 0");
    return left = left / right;
}


////////////////////////////////////////////////////////////
constexpr float operator/(Angle left, Angle right)
{
    SFML_BASE_ASSERT(right.asRadians() != 0.f && "Angle::operator/ cannot divide by 0");
    return left.asRadians() / right.asRadians();
}


////////////////////////////////////////////////////////////
constexpr Angle operator%(Angle left, Angle right)
{
    SFML_BASE_ASSERT(right.asRadians() != 0.f && "Angle::operator% cannot modulus by 0");
    return radians(priv::positiveRemainder(left.asRadians(), right.asRadians()));
}


////////////////////////////////////////////////////////////
constexpr Angle& operator%=(Angle& left, Angle right)
{
    SFML_BASE_ASSERT(right.asRadians() != 0.f && "Angle::operator%= cannot modulus by 0");
    return left = left % right;
}

namespace Literals
{

////////////////////////////////////////////////////////////
constexpr Angle operator""_deg(long double angle)
{
    return degrees(static_cast<float>(angle));
}


////////////////////////////////////////////////////////////
constexpr Angle operator""_deg(unsigned long long angle)
{
    return degrees(static_cast<float>(angle));
}


////////////////////////////////////////////////////////////
constexpr Angle operator""_rad(long double angle)
{
    return radians(static_cast<float>(angle));
}


////////////////////////////////////////////////////////////
constexpr Angle operator""_rad(unsigned long long angle)
{
    return radians(static_cast<float>(angle));
}

} // namespace Literals


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Angle Angle::Zero;

} // namespace sf
