#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2Base.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Point with color and texture coordinates
///
/// By default, the vertex color is white and texture coordinates are (0, 0).
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] Vertex
{
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vec2f position;            //!< 2D position of the vertex
    Color color{Color::White}; //!< Color of the vertex
    Vec2f texCoords{};         //!< Coordinates of the texture's pixel to map to the vertex
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Vertex
/// \ingroup graphics
///
/// A vertex is an "improved point". It has a position plus a few
/// extra attributes used for drawing: in VRSFML, vertices also carry
/// a color and a pair of texture coordinates.
///
/// The vertex is the basic building block of all drawing. Everything
/// visible on screen is ultimately made of vertices. They are grouped
/// into 2D primitives (lines, triangles, etc.), and these primitives
/// are grouped to create more complex 2D entities such as sprites,
/// texts, and shapes.
///
/// When using the higher-level graphical entities of VRSFML (`sf::Sprite`,
/// `sf::Text`, `sf::Shape`, ...) you do not need to deal with vertices
/// directly. However, if you want to define your own 2D entities --
/// such as tiled maps or particle systems -- working with vertices
/// directly is the most efficient option.
///
/// Example:
/// \code
/// // Define a 100x100 square, red, with a 10x10 texture mapped on it.
/// const sf::Vertex vertices[]
/// {
///     {{  0.f,   0.f}, sf::Color::Red, { 0.f,  0.f}},
///     {{  0.f, 100.f}, sf::Color::Red, { 0.f, 10.f}},
///     {{100.f, 100.f}, sf::Color::Red, {10.f, 10.f}},
///     {{  0.f,   0.f}, sf::Color::Red, { 0.f,  0.f}},
///     {{100.f, 100.f}, sf::Color::Red, {10.f, 10.f}},
///     {{100.f,   0.f}, sf::Color::Red, {10.f,  0.f}}
/// };
///
/// // Draw the vertex array as triangles.
/// window.draw(vertices, sf::PrimitiveType::Triangles);
/// \endcode
///
/// `sf::Vertex` is an aggregate, so it can be brace-initialized in
/// member-declaration order, or with C++20 designated initializers
/// (which VRSFML requires) to set only a subset of members:
///
/// \code
/// sf::Vertex v0{{5.f, 5.f}};                             // explicit 'position', defaulted 'color' and 'texCoords'
/// sf::Vertex v1{{5.f, 5.f}, sf::Color::Red};             // explicit 'position' and 'color', defaulted 'texCoords'
/// sf::Vertex v2{{5.f, 5.f}, sf::Color::Red, {1.f, 1.f}}; // everything is explicitly specified
///
/// sf::Vertex v3{
///    .position{5.f, 5.f},
///    .texCoords{1.f, 1.f}
/// };
/// \endcode
///
/// Note: although texture coordinates conceptually represent an
/// integer pixel position, their type is `float` because some graphics
/// drivers do not handle integer texture coordinates correctly.
///
/// \see `sf::VertexBuffer`, `sf::DrawableBatch`
///
////////////////////////////////////////////////////////////

// TODO P1: investigate what it would take to make `sf::Vertex` customizable!
