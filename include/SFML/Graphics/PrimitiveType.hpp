#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup graphics
/// \brief Types of primitives that a range of vertices can render
///
/// Points and lines have no area, therefore their thickness
/// will always be 1 pixel, regardless the current transform
/// and view.
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] PrimitiveType : unsigned int
{
    Points        = 0u, //!< Individual points
    Lines         = 1u, //!< Individual lines
    LineStrip     = 2u, //!< Connected lines, points use the previous point to form a line
    Triangles     = 3u, //!< Individual triangles
    TriangleStrip = 4u, //!< Connected triangles, points use the two previous points to form a triangle
    TriangleFan   = 5u  //!< Connected triangles, points use the common center and the previous point to form a triangle
};

} // namespace sf
