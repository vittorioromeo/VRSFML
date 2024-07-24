#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <SFML/Graphics/Shape.hpp>

#include <SFML/System/Vector2.hpp>

#include <cstddef>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Specialized shape representing a rectangle
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API RectangleShape : public Shape
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \param size Size of the rectangle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RectangleShape(Vector2f size = {});

    ////////////////////////////////////////////////////////////
    /// \brief Set the size of the rectangle
    ///
    /// \param size New size of the rectangle
    ///
    /// \see getSize
    ///
    ////////////////////////////////////////////////////////////
    void setSize(Vector2f size);

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the rectangle
    ///
    /// \return Size of the rectangle
    ///
    /// \see setSize
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2f getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of points defining the shape
    ///
    /// \return Number of points of the shape. For rectangle
    ///         shapes, this number is always 4.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::size_t getPointCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a point of the rectangle
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    /// The result is undefined if \a index is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. 3]
    ///
    /// \return index-th point of the shape
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2f getPoint(std::size_t index) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the geometric center of the rectangle
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
    // Member data
    ////////////////////////////////////////////////////////////
    Vector2f m_size;      //!< Size of the rectangle
    Vector2f m_points[4]; //!< Points composing the rectangle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RectangleShape
/// \ingroup graphics
///
/// This class inherits all the functions of sf::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of sf::Shape (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::RectangleShape rectangle;
/// rectangle.setSize(sf::Vector2f{100, 50});
/// rectangle.setOutlineColor(sf::Color::Red);
/// rectangle.setOutlineThickness(5);
/// rectangle.setPosition({10, 20});
/// ...
/// window.draw(rectangle);
/// \endcode
///
/// \see sf::Shape, sf::CircleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////
