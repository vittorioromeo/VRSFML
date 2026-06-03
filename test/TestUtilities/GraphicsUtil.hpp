// Header for SFML unit tests.
//
// For a new graphics module test case, include this header.
// Declares the `stringifyValue` ADL overloads so the bespoke testing
// library can render SFML graphics types for failure output. (`Rect2` is
// rendered by the framework itself, see `Tst/Detail/StringifyValue.hpp`.)

#pragma once

#include "SystemUtil.hpp"


namespace sf
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
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const BlendMode& blendMode) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, StencilComparison comparison) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, StencilUpdateOperation updateOperation) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const StencilMode& stencilMode) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const Color& color) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const Transform& transform) noexcept;
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const View& view) noexcept;
} // namespace sf


bool operator==(const sf::Transform& lhs, const Approx<sf::Transform>& rhs);
bool operator==(const sf::Color& lhs, const Approx<sf::Color>& rhs);

template <typename T>
bool operator==(const sf::Rect2<T>& lhs, const Approx<sf::Rect2<T>>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}
