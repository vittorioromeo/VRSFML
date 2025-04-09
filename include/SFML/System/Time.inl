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
constexpr Time::Time(const base::I64 microseconds) : m_microseconds(microseconds)
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
constexpr Time seconds(const float amount)
{
    return Time(static_cast<base::I64>(amount * 1'000'000.f));
}


////////////////////////////////////////////////////////////
constexpr Time milliseconds(const base::I32 amount)
{
    return Time(amount * 1000);
}


////////////////////////////////////////////////////////////
constexpr Time microseconds(const base::I64 amount)
{
    return Time(amount);
}


////////////////////////////////////////////////////////////
constexpr bool operator<(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() < rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator>(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() > rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator<=(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() <= rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator>=(const Time lhs, const Time rhs)
{
    return lhs.asMicroseconds() >= rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr Time operator-(Time rhs)
{
    return microseconds(-rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time operator+(const Time lhs, const Time rhs)
{
    return microseconds(lhs.asMicroseconds() + rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator+=(Time& lhs, const Time rhs)
{
    return lhs = lhs + rhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator-(const Time lhs, const Time rhs)
{
    return microseconds(lhs.asMicroseconds() - rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator-=(Time& lhs, const Time rhs)
{
    return lhs = lhs - rhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(const Time lhs, const float rhs)
{
    return seconds(lhs.asSeconds() * rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(const Time lhs, const base::I64 rhs)
{
    return microseconds(lhs.asMicroseconds() * rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(const float lhs, const Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(const base::I64 lhs, const Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& lhs, const float rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& lhs, const base::I64 rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator/(const Time lhs, const float rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
    return seconds(lhs.asSeconds() / rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator/(const Time lhs, const base::I64 rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/ cannot divide by 0");
    return microseconds(lhs.asMicroseconds() / rhs);
}


////////////////////////////////////////////////////////////
constexpr Time& operator/=(Time& lhs, const float rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
    return lhs = lhs / rhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator/=(Time& lhs, const base::I64 rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Time::operator/= cannot divide by 0");
    return lhs = lhs / rhs;
}


////////////////////////////////////////////////////////////
constexpr float operator/(const Time lhs, const Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator/ cannot divide by 0");
    return lhs.asSeconds() / rhs.asSeconds();
}


////////////////////////////////////////////////////////////
constexpr Time operator%(const Time lhs, const Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator% cannot modulus by 0");
    return microseconds(lhs.asMicroseconds() % rhs.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator%=(Time& lhs, const Time rhs)
{
    SFML_BASE_ASSERT(rhs.asMicroseconds() != 0 && "Time::operator%= cannot modulus by 0");
    return lhs = lhs % rhs;
}

} // namespace sf
