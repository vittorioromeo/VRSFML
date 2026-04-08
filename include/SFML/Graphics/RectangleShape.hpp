#pragma once

// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/Shape.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/SizeT.hpp"


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
    /// \brief Initialization data for `sf::RectangleShape`
    ///
    /// Alias for `sf::RectangleShapeData`.
    ///
    ////////////////////////////////////////////////////////////
    using Data = RectangleShapeData;

    ////////////////////////////////////////////////////////////
    /// \brief Construct a rectangle shape from a `Data` initializer
    ///
    /// \param data Geometry, transform, and appearance settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RectangleShape(const Data& data);

    ////////////////////////////////////////////////////////////
    /// \brief Set the size of the rectangle
    ///
    /// \param size New size of the rectangle
    ///
    /// \see `getSize`
    ///
    ////////////////////////////////////////////////////////////
    void setSize(Vec2f size);

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the rectangle
    ///
    /// \return Size of the rectangle
    ///
    /// \see `setSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of points defining the shape
    ///
    /// \return Number of points of the shape. For rectangle
    ///         shapes, this number is always 4.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr base::SizeT getPointCount() const
    {
        return 4u;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get a point of the rectangle
    ///
    /// The returned point is in local coordinates, that is,
    /// the shape's transforms (position, rotation, scale) are
    /// not taken into account.
    /// The result is undefined if `index` is out of the valid range.
    ///
    /// \param index Index of the point to get, in range [0 .. 3]
    ///
    /// \return `index`-th point of the shape
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f getPoint(base::SizeT index) const;

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
    [[nodiscard]] Vec2f getGeometricCenter() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vec2f m_size; //!< Size of the rectangle
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RectangleShape
/// \ingroup graphics
///
/// `sf::RectangleShape` is the concrete `sf::Shape` subclass for
/// drawing an axis-aligned rectangle. In addition to the
/// transform and appearance members inherited from `sf::Shape`,
/// it exposes a `size` (width and height) and the four corner
/// points it generates from it.
///
/// Usage example:
/// \code
/// const sf::RectangleShape rectangle{{
///     .position         = {10.f, 20.f},
///     .outlineColor     = sf::Color::Red,
///     .outlineThickness = 5.f,
///     .size             = {100.f, 50.f},
/// }};
///
/// window.draw(rectangle);
/// \endcode
///
/// \see `sf::Shape`, `sf::CircleShape`, `sf::ConvexShape`,
///      `sf::RectangleShapeData`
///
////////////////////////////////////////////////////////////
