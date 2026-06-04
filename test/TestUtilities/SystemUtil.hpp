// Header for SFML unit tests.
//
// For a new system module test case, include this header.
// Declares the `stringifyValue` ADL overloads so the bespoke testing
// library can render SFML system types for failure output.

#pragma once

#include "SFML/Base/SizeT.hpp"


// Forward declarations
namespace sf
{
class Angle;
class AutoWrapAngle;
class Utf8String;
class Time;

template <typename>
struct Vec2;

template <typename>
struct Vec3;

template <typename>
class Rect2;
} // namespace sf


////////////////////////////////////////////////////////////
// Legacy per-test `::Approx<T>` wrapper -- predates the introduction
// of `tst::Approx` and is kept for source compatibility with existing
// tests that pattern-match on it via the `bool operator==` overloads
// below.
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


////////////////////////////////////////////////////////////
// `stringifyValue` ADL overloads -- found when these operands need
// rendering for a failed assertion. (`Vec2`/`Vec3`/`Rect2` are rendered
// by the framework itself, see `Tst/Detail/StringifyValue.hpp`.)
////////////////////////////////////////////////////////////
namespace sf
{
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const Angle& angle) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const AutoWrapAngle& angle) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const Utf8String& string) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const Time& time) noexcept;
} // namespace sf


////////////////////////////////////////////////////////////
// The legacy `::Approx<T>` wrapper renders as `Approx(<value>)`, recursing
// through the same dispatch for its nested value.
////////////////////////////////////////////////////////////
template <typename T>
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const ::Approx<T>& approx) noexcept;
