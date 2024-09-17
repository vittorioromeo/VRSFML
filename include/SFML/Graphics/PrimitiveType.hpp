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
enum class [[nodiscard]] PrimitiveType : unsigned int
{
    Points        = 0u, //!< List of individual points
    Lines         = 1u, //!< List of individual lines
    LineStrip     = 2u, //!< List of connected lines, a point uses the previous point to form a line
    Triangles     = 3u, //!< List of individual triangles
    TriangleStrip = 4u, //!< List of connected triangles, a point uses the two previous points to form a triangle
    TriangleFan = 5u //!< List of connected triangles, a point uses the common center and the previous point to form a triangle
};

} // namespace sf
