#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vector2.hpp"


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
    Vector2f position;            //!< 2D position of the vertex
    Color    color{Color::White}; //!< Color of the vertex
    Vector2f texCoords{};         //!< Coordinates of the texture's pixel to map to the vertex
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Vertex
/// \ingroup graphics
///
/// A vertex is an improved point. It has a position and other
/// extra attributes that will be used for drawing: in SFML,
/// vertices also have a color and a pair of texture coordinates.
///
/// The vertex is the building block of drawing. Everything which
/// is visible on screen is made of vertices. They are grouped
/// as 2D primitives (lines, triangles, ...), and these primitives
/// are grouped to create even more complex 2D entities such as
/// sprites, texts, etc.
///
/// If you use the graphical entities of SFML (sprite, text, shape)
/// you won't have to deal with vertices directly. But if you want
/// to define your own 2D entities, such as tiled maps or particle
/// systems, using vertices will allow you to get maximum performances.
///
/// Example:
/// \code
/// // define a 100x100 square, red, with a 10x10 texture mapped on it
/// sf::Vertex vertices[]
/// {
///     {{  0.f,   0.f}, sf::Color::Red, { 0.f,  0.f}},
///     {{  0.f, 100.f}, sf::Color::Red, { 0.f, 10.f}},
///     {{100.f, 100.f}, sf::Color::Red, {10.f, 10.f}},
///     {{  0.f,   0.f}, sf::Color::Red, { 0.f,  0.f}},
///     {{100.f, 100.f}, sf::Color::Red, {10.f, 10.f}},
///     {{100.f,   0.f}, sf::Color::Red, {10.f,  0.f}}
/// };
///
/// // draw it
/// window.draw(vertices, 6, sf::PrimitiveType::Triangles);
/// \endcode
///
///
/// It is recommended to use aggregate initialization to create vertex
/// objects, which initializes the members in order.
///
/// On a C++20-compliant compiler (or where supported as an extension)
/// it is possible to use "designated initializers" to only initialize
/// a subset of members, with the restriction of having to follow the
/// same order in which they are defined.
///
/// Example:
/// \code
/// // C++17 and above
/// sf::Vertex v0{{5.f, 5.f}};                               // explicit 'position', implicit 'color' and 'texCoords'
/// sf::Vertex v1{{5.f, 5.f}, sf::Color::Red};               // explicit 'position' and 'color', implicit 'texCoords'
/// sf::Vertex v2{{5.f, 5.f}, sf::Color::Red, {1.f, 1.f}}; // everything is explicitly specified
///
/// // C++20 and above (or compilers supporting "designated initializers" as an extension)
/// sf::Vertex v3{
///    .position{5.f, 5.f},
///    .texCoords{1.f, 1.f}
/// };
/// \endcode
///
/// Note: Although texture coordinates are supposed to be an integer
/// amount of pixels, their type is float because of some buggy graphics
/// drivers that are not able to process integer coordinates correctly.
///
/// \see `sf::VertexBuffer`
///
////////////////////////////////////////////////////////////

// TODO P0: investigate what it would take to make `sf::Vertex` customizable!
