////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Time.hpp> // NOLINT(misc-header-include-cycle)

#include <SFML/Base/Assert.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
constexpr Time::Time() = default;


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
constexpr bool operator==(Time left, Time right)
{
    return left.asMicroseconds() == right.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator!=(Time left, Time right)
{
    return left.asMicroseconds() != right.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator<(Time left, Time right)
{
    return left.asMicroseconds() < right.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator>(Time left, Time right)
{
    return left.asMicroseconds() > right.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator<=(Time left, Time right)
{
    return left.asMicroseconds() <= right.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr bool operator>=(Time left, Time right)
{
    return left.asMicroseconds() >= right.asMicroseconds();
}


////////////////////////////////////////////////////////////
constexpr Time operator-(Time right)
{
    return microseconds(-right.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time operator+(Time left, Time right)
{
    return microseconds(left.asMicroseconds() + right.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator+=(Time& left, Time right)
{
    return left = left + right;
}


////////////////////////////////////////////////////////////
constexpr Time operator-(Time left, Time right)
{
    return microseconds(left.asMicroseconds() - right.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator-=(Time& left, Time right)
{
    return left = left - right;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(Time left, float right)
{
    return seconds(left.asSeconds() * right);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(Time left, std::int64_t right)
{
    return microseconds(left.asMicroseconds() * right);
}


////////////////////////////////////////////////////////////
constexpr Time operator*(float left, Time right)
{
    return right * left;
}


////////////////////////////////////////////////////////////
constexpr Time operator*(std::int64_t left, Time right)
{
    return right * left;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& left, float right)
{
    return left = left * right;
}


////////////////////////////////////////////////////////////
constexpr Time& operator*=(Time& left, std::int64_t right)
{
    return left = left * right;
}


////////////////////////////////////////////////////////////
constexpr Time operator/(Time left, float right)
{
    SFML_BASE_ASSERT(right != 0 && "Time::operator/ cannot divide by 0");
    return seconds(left.asSeconds() / right);
}


////////////////////////////////////////////////////////////
constexpr Time operator/(Time left, std::int64_t right)
{
    SFML_BASE_ASSERT(right != 0 && "Time::operator/ cannot divide by 0");
    return microseconds(left.asMicroseconds() / right);
}


////////////////////////////////////////////////////////////
constexpr Time& operator/=(Time& left, float right)
{
    SFML_BASE_ASSERT(right != 0 && "Time::operator/= cannot divide by 0");
    return left = left / right;
}


////////////////////////////////////////////////////////////
constexpr Time& operator/=(Time& left, std::int64_t right)
{
    SFML_BASE_ASSERT(right != 0 && "Time::operator/= cannot divide by 0");
    return left = left / right;
}


////////////////////////////////////////////////////////////
constexpr float operator/(Time left, Time right)
{
    SFML_BASE_ASSERT(right.asMicroseconds() != 0 && "Time::operator/ cannot divide by 0");
    return left.asSeconds() / right.asSeconds();
}


////////////////////////////////////////////////////////////
constexpr Time operator%(Time left, Time right)
{
    SFML_BASE_ASSERT(right.asMicroseconds() != 0 && "Time::operator% cannot modulus by 0");
    return microseconds(left.asMicroseconds() % right.asMicroseconds());
}


////////////////////////////////////////////////////////////
constexpr Time& operator%=(Time& left, Time right)
{
    SFML_BASE_ASSERT(right.asMicroseconds() != 0 && "Time::operator%= cannot modulus by 0");
    return left = left % right;
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Time Time::Zero;

} // namespace sf
