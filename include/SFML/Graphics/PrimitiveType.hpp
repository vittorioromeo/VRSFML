#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup graphics
/// \brief Types of primitives that a range of `sf::Vertex` can render
///
/// Points and lines have no area, therefore their thickness
/// will always be 1 pixel, regardless the current transform
/// and view.
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] PrimitiveType
{
    Points,        //!< List of individual points
    Lines,         //!< List of individual lines
    LineStrip,     //!< List of connected lines, a point uses the previous point to form a line
    Triangles,     //!< List of individual triangles
    TriangleStrip, //!< List of connected triangles, a point uses the two previous points to form a triangle
    TriangleFan //!< List of connected triangles, a point uses the common center and the previous point to form a triangle
};

} // namespace sf
