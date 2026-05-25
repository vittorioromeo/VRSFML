// Header for SFML unit tests.
//
// For a new graphics module test case, include this header.
// Specializes `tst::StringMaker` so the bespoke testing library can
// stringify SFML graphics types for failure output.

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
} // namespace sf


namespace tst
{
template <>
struct StringMaker<sf::BlendMode>
{
    static sf::base::String convert(const sf::BlendMode& blendMode);
};

template <>
struct StringMaker<sf::StencilComparison>
{
    static sf::base::String convert(sf::StencilComparison comparison);
};

template <>
struct StringMaker<sf::StencilUpdateOperation>
{
    static sf::base::String convert(sf::StencilUpdateOperation updateOperation);
};

template <>
struct StringMaker<sf::StencilMode>
{
    static sf::base::String convert(const sf::StencilMode& stencilMode);
};

template <>
struct StringMaker<sf::Color>
{
    static sf::base::String convert(sf::Color color);
};

template <>
struct StringMaker<sf::Transform>
{
    static sf::base::String convert(const sf::Transform& transform);
};

template <>
struct StringMaker<sf::View>
{
    static sf::base::String convert(const sf::View& view);
};

template <typename T>
struct StringMaker<sf::Rect2<T>>
{
    static sf::base::String convert(const sf::Rect2<T>& rect);
};
} // namespace tst


bool operator==(const sf::Transform& lhs, const Approx<sf::Transform>& rhs);
bool operator==(const sf::Color& lhs, const Approx<sf::Color>& rhs);

template <typename T>
bool operator==(const sf::Rect2<T>& lhs, const Approx<sf::Rect2<T>>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}
