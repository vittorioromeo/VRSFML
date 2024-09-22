#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Time.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
constexpr Time::Time(std::int64_t microseconds) : m_microseconds(microseconds)
{
}


////////////////////////////////////////////////////////////
constexpr float Time::asSeconds() const
{
    return static_cast<float>(m_microseconds) / 1'000'000.f;
}


////////////////////////////////////////////////////////////
constexpr std::int32_t Time::asMilliseconds() const
{
    return static_cast<std::int32_t>(static_cast<float>(m_microseconds) / 1'000.f);
}


////////////////////////////////////////////////////////////
constexpr std::int64_t Time::asMicroseconds() const
{
    return m_microseconds;
}


////////////////////////////////////////////////////////////
constexpr Time seconds(float amount)
{
    return Time(static_cast<std::int64_t>(amount * 1'000'000.f));
}


////////////////////////////////////////////////////////////
constexpr Time milliseconds(std::int32_t amount)
{
    return Time(amount * 1'000);
}


////////////////////////////////////////////////////////////
constexpr Time microseconds(std::int64_t amount)
{
    return Time(amount);
}


////////////////////////////////////////////////////////////
constexpr bool operator==(Time lhs, Time rhs)
{
    return lhs.asMicroseconds() == rhs.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator!=(Time lhs, Time rhs)
{
    return lhs.asMicroseconds() != rhs.asMicroseconds();
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
constexpr Time operator*(Time lhs, std::int64_t rhs)
{
    return microseconds(lhs.asMicroseconds() * rhs);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(float lhs, Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(std::int64_t lhs, Time rhs)
{
    return rhs * lhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& lhs, float rhs)
{
    return lhs = lhs * rhs;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& lhs, std::int64_t rhs)
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
constexpr Time operator/(Time lhs, std::int64_t rhs)
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
constexpr Time& operator/=(Time& lhs, std::int64_t rhs)
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
