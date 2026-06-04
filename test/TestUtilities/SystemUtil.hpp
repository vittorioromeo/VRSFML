// Header for SFML unit tests.
//
// For a new system module test case, include this header.
// Declares the `stringifyValue` ADL overloads so the bespoke testing
// library can render SFML system types for failure output.

#pragma once

#include "ZancleBase/SizeT.hpp"


// Forward declarations
namespace za
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
} // namespace za


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
bool operator==(za::Vec2<float> lhs, const Approx<za::Vec2<float>>& rhs);
bool operator==(const za::Vec3<float>& lhs, const Approx<za::Vec3<float>>& rhs);
bool operator==(const za::Angle& lhs, const Approx<za::Angle>& rhs);
bool operator==(const za::Rect2<float>& lhs, const Approx<za::Rect2<float>>& rhs);


////////////////////////////////////////////////////////////
// `stringifyValue` ADL overloads -- found when these operands need
// rendering for a failed assertion. (`Vec2`/`Vec3`/`Rect2` are rendered
// by the framework itself, see `Tst/Detail/StringifyValue.hpp`.)
////////////////////////////////////////////////////////////
namespace za
{
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const Angle& angle) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const AutoWrapAngle& angle) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const Utf8String& string) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const Time& time) noexcept;
} // namespace za


////////////////////////////////////////////////////////////
// The legacy `::Approx<T>` wrapper renders as `Approx(<value>)`, recursing
// through the same dispatch for its nested value.
////////////////////////////////////////////////////////////
template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const ::Approx<T>& approx) noexcept;
