// Header for SFML unit tests.
//
// For a new graphics module test case, include this header.
// Declares the `stringifyValue` ADL overloads so the bespoke testing
// library can render SFML graphics types for failure output. (`Rect2` is
// rendered by the framework itself, see `Tst/Detail/StringifyValue.hpp`.)

#pragma once

#include "SystemUtil.hpp"


namespace za
{
enum class StencilComparison : unsigned char;
enum class StencilUpdateOperation : unsigned char;

struct BlendMode;
struct Color;
struct StencilMode;
struct Transform;
struct View;

template <typename>
class Rect2;

// `stringifyValue` ADL overloads -- found when these operands need
// rendering for a failed assertion.
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const BlendMode& blendMode) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, StencilComparison comparison) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, StencilUpdateOperation updateOperation) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const StencilMode& stencilMode) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const Color& color) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const Transform& transform) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const View& view) noexcept;
} // namespace za


bool operator==(const za::Transform& lhs, const Approx<za::Transform>& rhs);
bool operator==(const za::Color& lhs, const Approx<za::Color>& rhs);

template <typename T>
bool operator==(const za::Rect2<T>& lhs, const Approx<za::Rect2<T>>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}
