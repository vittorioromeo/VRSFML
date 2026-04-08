#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Shape.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Specialized shape representing a circle
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API CircleShape : public Shape
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Initialization data for a `sf::CircleShape`
    ///
    /// This type alias points to `sf::CircleShapeData`, which
    /// holds all the configurable parameters for a circle shape.
    /// It is used when constructing a `sf::CircleShape`.
    ///
    /// \see sf::CircleShapeData
    ////////////////////////////////////////////////////////////
    using Data = CircleShapeData;

    ////////////////////////////////////////////////////////////
    /// \brief Construct a circle shape from a `Data` initializer
    ///
    /// \param data Geometry, transform, and appearance settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit CircleShape(const Data& data);

    ////////////////////////////////////////////////////////////
    /// \brief Set the radius of the circle
    ///
    /// \param radius New radius of the circle
    ///
    /// \see `getRadius`
    ///
    ////////////////////////////////////////////////////////////
    void setRadius(float radius);

    ////////////////////////////////////////////////////////////
    /// \brief Get the radius of the circle
    ///
    /// \return Radius of the circle
    ///
    /// \see `setRadius`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getRadius() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the number of points of the circle
    ///
    /// \param pointCount New number of points of the circle
    ///
    /// \see `getPointCount`
    ///
    ////////////////////////////////////////////////////////////
    void setPointCount(unsigned int pointCount);

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of points of the circle
    ///
    /// \return Number of points of the circle
    ///
    /// \see `setPointCount`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getPointCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a point of the circle
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    /// The result is undefined if `index` is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    ///
    /// \return `index`-th point of the shape
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f getPoint(base::SizeT index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the geometric center of the circle
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
    /// \brief Recompute the circle geometry
    ///
    ////////////////////////////////////////////////////////////
    void updateCircleGeometry();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    float        m_radius;     //!< Radius of the circle
    unsigned int m_pointCount; //!< Number of points composing the circle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::CircleShape
/// \ingroup graphics
///
/// `sf::CircleShape` is the concrete `sf::Shape` subclass for
/// drawing a circle (or, more generally, a regular polygon
/// approximation of one). It exposes the standard transform and
/// appearance members inherited from `sf::Shape` plus a `radius`
/// and `pointCount` (number of vertices used to approximate the
/// curve).
///
/// Usage example:
/// \code
/// sf::CircleShape circle{{
///     .position         = {10.f, 20.f},
///     .outlineColor     = sf::Color::Red,
///     .outlineThickness = 5.f,
///     .radius           = 150.f,
/// }};
///
/// window.draw(circle);
/// \endcode
///
/// Since the GPU cannot draw perfect circles, the circle is
/// approximated by `pointCount` connected triangles fanning out
/// from the center. Higher counts produce smoother circles at
/// the cost of more vertices.
///
/// `pointCount` can also be used to draw regular polygons --
/// pass a small value to get an equilateral triangle (3), a
/// square (4), a pentagon (5), a hexagon (6), and so on.
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::ConvexShape`,
///      `sf::CircleShapeData`
///
////////////////////////////////////////////////////////////
