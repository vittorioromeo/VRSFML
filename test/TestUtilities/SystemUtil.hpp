// Header for SFML unit tests.
//
// For a new system module test case, include this header.
// This ensures that string conversions are visible and can be used by Catch2 for debug output.

#pragma once

#include "SFML/System/Rect2.hpp"

#include <iosfwd>

// String conversions for Catch2
namespace sf::base
{
template <typename T, typename TDeleter>
class UniquePtr;
}

namespace sf
{
struct Angle;
class UnicodeString;
class Time;

template <typename>
class Vec2;

template <typename>
class Vec3;

void setStreamPrecision(std::ostream& os, int maxDigits10);

std::ostream& operator<<(std::ostream& os, const Angle& angle);
std::ostream& operator<<(std::ostream& os, const UnicodeString& string);
std::ostream& operator<<(std::ostream& os, Time time);

template <typename T>
std::ostream& operator<<(std::ostream& os, Vec2<T> vec);

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vec3<T>& vec);

template <typename T, typename Del>
std::ostream& operator<<(std::ostream& os, const sf::base::UniquePtr<T, Del>&)
{
    return os;
}
} // namespace sf

////////////////////////////////////////////////////////////
/// Class template for creating custom approximate comparisons.
/// To register a new type, simply implement a custom operator==
/// overload for that type.
////////////////////////////////////////////////////////////
template <typename T>
struct Approx
{
    explicit Approx(const T& t) : value(t)
    {
    }

    const T& value;
};

bool operator==(const float& lhs, const Approx<float>& rhs);
bool operator==(sf::Vec2<float> lhs, const Approx<sf::Vec2<float>>& rhs);
bool operator==(const sf::Vec3<float>& lhs, const Approx<sf::Vec3<float>>& rhs);
bool operator==(const sf::Angle& lhs, const Approx<sf::Angle>& rhs);
bool operator==(const sf::Rect2f& lhs, const Approx<sf::Rect2f>& rhs);

template <typename T>
std::ostream& operator<<(std::ostream& os, const Approx<T>& approx);
