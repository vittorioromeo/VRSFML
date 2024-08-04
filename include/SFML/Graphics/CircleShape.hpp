#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <SFML/Graphics/Shape.hpp>

#include <SFML/System/Vector2.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <cstddef>


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
    /// \brief Default constructor
    ///
    /// \param radius     Radius of the circle
    /// \param pointCount Number of points composing the circle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit CircleShape(float radius = 0, std::size_t pointCount = 30);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~CircleShape();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    CircleShape(const CircleShape& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment operator
    ///
    ////////////////////////////////////////////////////////////
    CircleShape& operator=(const CircleShape&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    CircleShape(CircleShape&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment operator
    ///
    ////////////////////////////////////////////////////////////
    CircleShape& operator=(CircleShape&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Set the radius of the circle
    ///
    /// \param radius New radius of the circle
    ///
    /// \see getRadius
    ///
    ////////////////////////////////////////////////////////////
    void setRadius(float radius);

    ////////////////////////////////////////////////////////////
    /// \brief Get the radius of the circle
    ///
    /// \return Radius of the circle
    ///
    /// \see setRadius
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getRadius() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the number of points of the circle
    ///
    /// \param count New number of points of the circle
    ///
    /// \see getPointCount
    ///
    ////////////////////////////////////////////////////////////
    void setPointCount(std::size_t count);

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of points of the circle
    ///
    /// \return Number of points of the circle
    ///
    /// \see setPointCount
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::size_t getPointCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a point of the circle
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    /// The result is undefined if \a index is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
    ///
    /// \return index-th point of the shape
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2f getPoint(std::size_t index) const;

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
    void update(float radius, std::size_t pointCount);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 48> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::CircleShape
/// \ingroup graphics
///
/// This class inherits all the functions of sf::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of sf::Shape (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::CircleShape circle;
/// circle.setRadius(150);
/// circle.setOutlineColor(sf::Color::Red);
/// circle.setOutlineThickness(5);
/// circle.setPosition({10, 20});
/// ...
/// window.draw(circle);
/// \endcode
///
/// Since the graphics card can't draw perfect circles, we have to
/// fake them with multiple triangles connected to each other. The
/// "points count" property of sf::CircleShape defines how many of these
/// triangles to use, and therefore defines the quality of the circle.
///
/// The number of points can also be used for another purpose; with
/// small numbers you can create any regular polygon shape:
/// equilateral triangle, square, pentagon, hexagon, ...
///
/// \see sf::Shape, sf::RectangleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////
