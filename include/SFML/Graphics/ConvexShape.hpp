#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Shape.hpp"

#include "SFML/System/Vec2.hpp"

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
    /// \brief Initialization data for a `sf::ConvexShape`
    ///
    /// This struct groups the parameters needed to initialize a
    /// `sf::ConvexShape`. It includes common shape properties
    /// (like transform, texture, fill color, outline) and
    /// convex shape specific properties, such as the initial
    /// point count.
    ///
    /// An instance of `Data` is passed to the constructor
    /// of `sf::ConvexShape`.
    ///
    /// Example:
    /// \code
    /// sf::ConvexShape::Data data;
    /// data.pointCount = 5; // For a pentagon
    /// data.fillColor = sf::Color::Green;
    /// data.outlineColor = sf::Color::Black;
    /// data.outlineThickness = 2.f;
    ///
    /// sf::ConvexShape pentagon(data);
    /// // Points still need to be set individually using `setPoint`
    /// \endcode
    ///
    /// \see ConvexShape(const Data&)
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Data
    {
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

        base::SizeT pointCount{0u}; //!< Number of points of the polygon
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \param settings Data of the polygon
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit ConvexShape(const Data& settings);

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
/// This class inherits all the functions of `sf::Transformable`
/// (position, rotation, scale, bounds, etc...) as well as the
/// functions of `sf::Shape` (outline, color, texture, etc...).
///
/// It is important to keep in mind that a convex shape must
/// always be... convex, otherwise it may not be drawn correctly.
/// Moreover, the points must be defined in order; using a random
/// order would result in an incorrect shape.
///
/// Usage example:
/// \code
/// sf::ConvexShape polygon;
/// polygon.setPointCount(3);
/// polygon.setPoint(0, sf::Vec2f{0, 0});
/// polygon.setPoint(1, sf::Vec2f{0, 10});
/// polygon.setPoint(2, sf::Vec2f{25, 5});
/// polygon.setOutlineColor(sf::Color::Red);
/// polygon.setOutlineThickness(5);
/// polygon.position = {10, 20};
/// ...
/// window.draw(polygon);
/// \endcode
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::CircleShape`
///
////////////////////////////////////////////////////////////
