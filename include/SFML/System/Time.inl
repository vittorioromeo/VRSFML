#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Time.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
constexpr Time::Time(base::I64 microseconds) : m_microseconds(microseconds)
{
}


////////////////////////////////////////////////////////////
constexpr float Time::asSeconds() const
{
    return static_cast<float>(m_microseconds) / 1'000'000.f;
}


////////////////////////////////////////////////////////////
constexpr base::I32 Time::asMilliseconds() const
{
    return static_cast<base::I32>(static_cast<float>(m_microseconds) / 1000.f);
}


////////////////////////////////////////////////////////////
constexpr base::I64 Time::asMicroseconds() const
{
    return m_microseconds;
}


////////////////////////////////////////////////////////////
constexpr Time seconds(float amount)
{
    return Time(static_cast<base::I64>(amount * 1'000'000.f));
}


////////////////////////////////////////////////////////////
constexpr Time milliseconds(base::I32 amount)
{
    return Time(amount * 1000);
}


////////////////////////////////////////////////////////////
constexpr Time microseconds(base::I64 amount)
{
    return Time(amount);
}


////////////////////////////////////////////////////////////
constexpr bool operator<(Time lhs, Time rhs)
{
    return lhs.asMicroseconds() < rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator>(Time lhs, Time rhs)
{
    return lhs.asMicroseconds() > rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator<=(Time lhs, Time rhs)
{
    return lhs.asMicroseconds() <= rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator>=(Time lhs, Time rhs)
{
    return lhs.asMicroseconds() >= rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr Time operator-(Time rhs)
{
    return microseconds(-rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time operator+(Time lhs, Time rhs)
{
    return microseconds(lhs.asMicroseconds() + rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator+=(Time& lhs, Time rhs)
{
    return lhs = lhs + rhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator-(Time lhs, Time rhs)
{
    return microseconds(lhs.asMicroseconds() - rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator-=(Time& lhs, Time rhs)
{
    return lhs = lhs - rhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(Time lhs, float rhs)
{
    return seconds(lhs.asSeconds() * rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(Time lhs, base::I64 rhs)
{
    return microseconds(lhs.asMicroseconds() * rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(float lhs, Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(base::I64 lhs, Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& lhs, float rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& lhs, base::I64 rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator/(Time lhs, float rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
    return seconds(lhs.asSeconds() / rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator/(Time lhs, base::I64 rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
    return microseconds(lhs.asMicroseconds() / rhs);
}


////////////////////////////////////////////////////////////
constexpr Time& operator/=(Time& lhs, float rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
    return lhs = lhs / rhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator/=(Time& lhs, base::I64 rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
    return lhs = lhs / rhs;
}


////////////////////////////////////////////////////////////
constexpr float operator/(Time lhs, Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator/ cannot divide by 0");
    return lhs.asSeconds() / rhs.asSeconds();
}


////////////////////////////////////////////////////////////
constexpr Time operator%(Time lhs, Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator% cannot modulus by 0");
    return microseconds(lhs.asMicroseconds() % rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator%=(Time& lhs, Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator%= cannot modulus by 0");
    return lhs = lhs % rhs;
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Time Time::Zero;

} // namespace sf
