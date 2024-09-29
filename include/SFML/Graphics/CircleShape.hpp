#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Shape.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"


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
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Settings
    {
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_TRANSFORMABLE;
        SFML_PRIV_DEFINE_SETTINGS_DATA_MEMBERS_SHAPE;

        float        radius{0.f};     //!< Radius of the circle
        unsigned int pointCount{30u}; //!< Number of points composing the circle
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \param settings Settings of the circle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit CircleShape(const Settings& settings);

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
    /// The result is undefined if \a `index` is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    ///
    /// \return index-th point of the shape
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2f getPoint(base::SizeT index) const;

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
    [[nodiscard]] Vector2f getGeometricCenter() const;

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
/// This class inherits all the functions of `sf::Transformable`
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of `sf::Shape` (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::CircleShape circle(150.f);
/// circle.setOutlineColor(sf::Color::Red);
/// circle.setOutlineThickness(5);
/// circle.position = {10, 20};
/// ...
/// window.draw(circle);
/// \endcode
///
/// Since the graphics card can't draw perfect circles, we have to
/// fake them with multiple triangles connected to each other. The
/// "points count" property of `sf::CircleShape` defines how many of these
/// triangles to use, and therefore defines the quality of the circle.
///
/// The number of points can also be used for another purpose; with
/// small numbers you can create any regular polygon shape:
/// equilateral triangle, square, pentagon, hexagon, ...
///
/// \see `sf::Shape`, `sf::RectangleShape`, `sf::ConvexShape`
///
////////////////////////////////////////////////////////////
