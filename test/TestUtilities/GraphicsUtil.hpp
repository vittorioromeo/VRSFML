// Header for SFML unit tests.
//
// For a new graphics module test case, include this header.
// This specializes `doctest::StringMaker` so doctest can stringify
// SFML graphics types for failure output without dragging `<ostream>`.

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


// `View::ScissorRect` is a nested type that derives from `Rect2f`;
// its definition lives in `SFML/Graphics/View.hpp` which test files already include
// when they need it. The specialization's body in `GraphicsUtil.cpp` just slices
// to `Rect2f`, so only the forward declaration is needed here.
#include "SFML/Graphics/View.hpp" // IWYU pragma: keep


namespace doctest
{
template <>
struct StringMaker<sf::BlendMode>
{
    static String convert(const sf::BlendMode& blendMode);
};

template <>
struct StringMaker<sf::StencilComparison>
{
    static String convert(sf::StencilComparison comparison);
};

template <>
struct StringMaker<sf::StencilUpdateOperation>
{
    static String convert(sf::StencilUpdateOperation updateOperation);
};

template <>
struct StringMaker<sf::StencilMode>
{
    static String convert(const sf::StencilMode& stencilMode);
};

template <>
struct StringMaker<sf::Color>
{
    static String convert(sf::Color color);
};

template <>
struct StringMaker<sf::Transform>
{
    static String convert(const sf::Transform& transform);
};

template <>
struct StringMaker<sf::View>
{
    static String convert(const sf::View& view);
};

template <typename T>
struct StringMaker<sf::Rect2<T>>
{
    static String convert(const sf::Rect2<T>& rect);
};

template <>
struct StringMaker<sf::View::ScissorRect>
{
    static String convert(const sf::View::ScissorRect& scissorRect);
};
} // namespace doctest


bool operator==(const sf::Transform& lhs, const Approx<sf::Transform>& rhs);
bool operator==(const sf::Color& lhs, const Approx<sf::Color>& rhs);

template <typename T>
bool operator==(const sf::Rect2<T>& lhs, const Approx<sf::Rect2<T>>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}
