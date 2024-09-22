#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Assert.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
inline constexpr float pi  = 3.141592654f;
inline constexpr float tau = pi * 2.f;


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr float positiveRemainder(float a, float b)
{
    SFML_BASE_ASSERT(b > 0.f && "Cannot calculate remainder with non-positive divisor");

    // fmod seems to be slower
    const auto val = a - static_cast<float>(static_cast<int>(a / b)) * b;
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
    return Angle(priv::positiveRemainder(m_radians + priv::pi, priv::tau) - priv::pi);
}


////////////////////////////////////////////////////////////
constexpr Angle Angle::wrapUnsigned() const
{
    return Angle(priv::positiveRemainder(m_radians, priv::tau));
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
constexpr bool operator==(Angle lhs, Angle rhs)
{
    return lhs.m_radians == rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr bool operator!=(Angle lhs, Angle rhs)
{
    return lhs.m_radians != rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr bool operator<(Angle lhs, Angle rhs)
{
    return lhs.m_radians < rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr bool operator>(Angle lhs, Angle rhs)
{
    return lhs.m_radians > rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr bool operator<=(Angle lhs, Angle rhs)
{
    return lhs.m_radians <= rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr bool operator>=(Angle lhs, Angle rhs)
{
    return lhs.m_radians >= rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr Angle operator-(Angle rhs)
{
    return Angle(-rhs.m_radians);
}


////////////////////////////////////////////////////////////
constexpr Angle operator+(Angle lhs, Angle rhs)
{
    return Angle(lhs.m_radians + rhs.m_radians);
}


////////////////////////////////////////////////////////////
constexpr Angle& operator+=(Angle& lhs, Angle rhs)
{
    lhs.m_radians += rhs.m_radians;
    return lhs;
}


////////////////////////////////////////////////////////////
constexpr Angle operator-(Angle lhs, Angle rhs)
{
    return Angle(lhs.m_radians - rhs.m_radians);
}


////////////////////////////////////////////////////////////
constexpr Angle& operator-=(Angle& lhs, Angle rhs)
{
    lhs.m_radians -= rhs.m_radians;
    return lhs;
}


////////////////////////////////////////////////////////////
constexpr Angle operator*(Angle lhs, float rhs)
{
    return Angle(lhs.m_radians * rhs);
}


////////////////////////////////////////////////////////////
constexpr Angle operator*(float lhs, Angle rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Angle& operator*=(Angle& lhs, float rhs)
{
    lhs.m_radians *= rhs;
    return lhs;
}


////////////////////////////////////////////////////////////
constexpr Angle operator/(Angle lhs, float rhs)
{
    SFML_BASE_ASSERT(rhs != 0.f && "Angle::operator/ cannot divide by 0");
    return Angle(lhs.m_radians / rhs);
}


////////////////////////////////////////////////////////////
constexpr Angle& operator/=(Angle& lhs, float rhs)
{
    SFML_BASE_ASSERT(rhs != 0.f && "Angle::operator/= cannot divide by 0");
    lhs.m_radians /= rhs;
    return lhs;
}


////////////////////////////////////////////////////////////
constexpr float operator/(Angle lhs, Angle rhs)
{
    SFML_BASE_ASSERT(rhs.m_radians != 0.f && "Angle::operator/ cannot divide by 0");
    return lhs.m_radians / rhs.m_radians;
}


////////////////////////////////////////////////////////////
constexpr Angle operator%(Angle lhs, Angle rhs)
{
    SFML_BASE_ASSERT(rhs.m_radians != 0.f && "Angle::operator% cannot modulus by 0");
    return Angle(priv::positiveRemainder(lhs.m_radians, rhs.m_radians));
}


////////////////////////////////////////////////////////////
constexpr Angle& operator%=(Angle& lhs, Angle rhs)
{
    SFML_BASE_ASSERT(rhs.m_radians != 0.f && "Angle::operator%= cannot modulus by 0");
    lhs.m_radians = priv::positiveRemainder(lhs.m_radians, rhs.m_radians);
    return lhs;
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
