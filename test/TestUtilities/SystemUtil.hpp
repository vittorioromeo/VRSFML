// Header for SFML unit tests.
//
// For a new system module test case, include this header.
// This specializes `doctest::StringMaker` so doctest can stringify
// SFML types for failure output without dragging `<ostream>`.

#pragma once


#include <DoctestFwd.hpp>


// Forward declarations
namespace sf::base
{
class String;

template <typename T, typename TDeleter>
class UniquePtr;
} // namespace sf::base

namespace sf
{
class Angle;
class AutoWrapAngle;
class UnicodeString;
class Time;

template <typename>
struct Vec2;

template <typename>
struct Vec3;

template <typename>
class Rect2;
} // namespace sf


namespace doctest
{
template <>
struct StringMaker<sf::Angle>
{
    static String convert(const sf::Angle& angle);
};

template <>
struct StringMaker<sf::AutoWrapAngle>
{
    static String convert(const sf::AutoWrapAngle& angle);
};

template <>
struct StringMaker<sf::UnicodeString>
{
    static String convert(const sf::UnicodeString& string);
};

template <>
struct StringMaker<sf::Time>
{
    static String convert(sf::Time time);
};

template <typename T>
struct StringMaker<sf::Vec2<T>>
{
    static String convert(const sf::Vec2<T>& vec);
};

template <typename T>
struct StringMaker<sf::Vec3<T>>
{
    static String convert(const sf::Vec3<T>& vec);
};

template <typename T, typename Del>
struct StringMaker<sf::base::UniquePtr<T, Del>>
{
    static String convert(const sf::base::UniquePtr<T, Del>&)
    {
        return "";
    }
};
} // namespace doctest


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
bool operator==(const sf::Rect2<float>& lhs, const Approx<sf::Rect2<float>>& rhs);


namespace doctest
{
template <typename T>
struct StringMaker<::Approx<T>>
{
    static String convert(const ::Approx<T>& approx)
    {
        return toString(approx.value);
    }
};
} // namespace doctest
