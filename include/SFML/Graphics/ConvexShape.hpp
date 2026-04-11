#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Priv/ShapeMacros.hpp"
#include "SFML/Graphics/Priv/TransformableMacros.hpp"
#include "SFML/Graphics/Shape.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Specialized shape representing a convex polygon
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API ConvexShape : public Shape
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Initialization data for `sf::ConvexShape`
    ///
    /// In addition to the standard transformable and shape
    /// appearance members, `Data` carries the polygon's initial
    /// point count. After construction, the actual point
    /// positions are set with `setPoint`.
    ///
    /// Example:
    /// \code
    /// sf::ConvexShape pentagon{{
    ///     .fillColor        = sf::Color::Green,
    ///     .outlineColor     = sf::Color::Black,
    ///     .outlineThickness = 2.f,
    ///     .pointCount       = 5u, // pentagon
    /// }};
    ///
    /// pentagon.setPoint(0u, {  0.f,   0.f});
    /// pentagon.setPoint(1u, { 50.f, -50.f});
    /// // ...
    /// \endcode
    ///
    /// \see `ConvexShape(const Data&)`
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Data
    {
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

        base::SizeT pointCount{0u}; //!< Initial number of points of the polygon
    };

    ////////////////////////////////////////////////////////////
    /// \brief Construct a convex polygon from a `Data` initializer
    ///
    /// The polygon is created with `data.pointCount` points, all
    /// at position `(0, 0)`. Use `setPoint` to assign their
    /// actual coordinates.
    ///
    /// \param data Geometry, transform, and appearance settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit ConvexShape(const Data& data);

    ////////////////////////////////////////////////////////////
    /// \brief Set the number of points of the polygon
    ///
    /// For the shape to be rendered as expected, `count` must
    /// be greater or equal to 3.
    ///
    /// \param count New number of points of the polygon
    ///
    /// \see `getPointCount`
    ///
    ////////////////////////////////////////////////////////////
    void setPointCount(base::SizeT count);

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of points of the polygon
    ///
    /// \return Number of points of the polygon
    ///
    /// \see `setPointCount`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT getPointCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the position of a point
    ///
    /// Don't forget that the shape must be convex and the
    /// order of points matters. Points should not overlap.
    /// This applies to rendering; it is explicitly allowed
    /// to temporarily have non-convex or degenerate shapes
    /// when not drawn (e.g. during shape initialization).
    ///
    /// Point count must be specified beforehand. The behavior is
    /// undefined if `index` is greater than or equal to getPointCount.
    ///
    /// \param index Index of the point to change, in range [0 .. getPointCount() - 1]
    /// \param point New position of the point
    ///
    /// \see `getPoint`
    ///
    ////////////////////////////////////////////////////////////
    void setPoint(base::SizeT index, Vec2f point);

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of a point
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    /// The result is undefined if `index` is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    ///
    /// \return Position of the `index`-th point of the polygon
    ///
    /// \see `setPoint`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f getPoint(base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the geometric center of the convex shape
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    ///
    /// \return The geometric center of the shape
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f getGeometricCenter() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::Vector<Vec2f> m_points; //!< Points composing the convex polygon
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::ConvexShape
/// \ingroup graphics
///
/// `sf::ConvexShape` is the concrete `sf::Shape` subclass for
/// drawing arbitrary convex polygons. The point count is set at
/// construction time (or via `setPointCount`) and the
/// coordinates of each vertex are then assigned with `setPoint`.
///
/// Important: the polygon **must remain convex** at draw time,
/// and the points must be supplied in a consistent winding
/// order. A non-convex polygon may render with visual artifacts.
/// It is fine to temporarily have a non-convex or degenerate
/// polygon while you are still building it -- the constraint
/// applies only when the shape is actually drawn.
///
/// Usage example:
/// \code
/// sf::ConvexShape polygon{{
///     .position         = {10.f, 20.f},
///     .outlineColor     = sf::Color::Red,
///     .outlineThickness = 5.f,
///     .pointCount       = 3u,
/// }};
///
/// polygon.setPoint(0u, { 0.f,  0.f});
/// polygon.setPoint(1u, { 0.f, 10.f});
/// polygon.setPoint(2u, {25.f,  5.f});
///
/// window.draw(polygon);
/// \endcode
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
