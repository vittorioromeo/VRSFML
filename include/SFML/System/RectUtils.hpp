#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/System/Rect.hpp"

#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Check the intersection between two rectangles
///
/// \param rect0 First rectangle
/// \param rect1 Second rectangle
///
/// \return Intersection rectangle if intersecting, `base::nullOpt` otherwise
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::pure]] constexpr base::Optional<Rect<T>> findIntersection(const Rect<T>& rect0, const Rect<T>& rect1)
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Precompute right/bottom edges
    const T r0Right  = rect0.position.x + rect0.size.x;
    const T r0Bottom = rect0.position.y + rect0.size.y;
    const T r1Right  = rect1.position.x + rect1.size.x;
    const T r1Bottom = rect1.position.y + rect1.size.y;

    // Compute the min and max of the first rectangle on both axes
    const T r0MinX = SFML_BASE_MIN(rect0.position.x, r0Right);
    const T r0MaxX = SFML_BASE_MAX(rect0.position.x, r0Right);
    const T r0MinY = SFML_BASE_MIN(rect0.position.y, r0Bottom);
    const T r0MaxY = SFML_BASE_MAX(rect0.position.y, r0Bottom);

    // Compute the min and max of the second rectangle on both axes
    const T r1MinX = SFML_BASE_MIN(rect1.position.x, r1Right);
    const T r1MaxX = SFML_BASE_MAX(rect1.position.x, r1Right);
    const T r1MinY = SFML_BASE_MIN(rect1.position.y, r1Bottom);
    const T r1MaxY = SFML_BASE_MAX(rect1.position.y, r1Bottom);

    // Compute the intersection boundaries for the X axis
    const T interLeft  = SFML_BASE_MAX(r0MinX, r1MinX);
    const T interRight = SFML_BASE_MIN(r0MaxX, r1MaxX);

    // Early exit if no overlap on X axis
    if (interLeft >= interRight)
        return base::nullOpt;

    // Compute the intersection boundaries for the Y axis
    const T interTop    = SFML_BASE_MAX(r0MinY, r1MinY);
    const T interBottom = SFML_BASE_MIN(r0MaxY, r1MaxY);

    // Check for overlap on Y axis
    if (interTop >= interBottom)
        return base::nullOpt;

    // Intersection found
    return base::makeOptional<Rect<T>>(Vector2<T>{interLeft, interTop},
                                       Vector2<T>{interRight - interLeft, interBottom - interTop});
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Rect
/// \ingroup graphics
///
/// A rectangle is defined by its top-left corner and its size.
/// It is a very simple class defined for convenience, so
/// its member variables (position and size) are public
/// and can be accessed directly, just like the vector classes
/// (Vector2 and Vector3).
///
/// To keep things simple, `sf::Rect` doesn't define
/// functions to emulate the properties that are not directly
/// members (such as right, bottom, etc.), it rather
/// only provides intersection functions.
///
/// `sf::Rect` uses the usual rules for its boundaries:
/// \li The left and top edges are included in the rectangle's area
/// \li The right and bottom edges are excluded from the rectangle's area
///
/// This means that `sf::IntRect({0, 0}, {1, 1})` and `sf::IntRect({1, 1}, {1, 1})`
/// don't intersect.
///
/// sf::Rect is a template and may be used with any numeric type, but
/// for simplicity type aliases for the instantiations used by SFML are given:
/// \li sf::Rect<int> is sf::IntRect
/// \li sf::Rect<float> is sf::FloatRect
///
/// So that you don't have to care about the template syntax.
///
/// Usage example:
/// \code
/// // Define a rectangle, located at (0, 0) with a size of 20x5
/// sf::IntRect r1({0, 0}, {20, 5});
///
/// // Define another rectangle, located at (4, 2) with a size of 18x10
/// sf::Vector2i position(4, 2);
/// sf::Vector2i size(18, 10);
/// sf::IntRect r2(position, size);
///
/// // Test intersections with the point (3, 1)
/// bool b1 = r1.contains({3, 1}); // true
/// bool b2 = r2.contains({3, 1}); // false
///
/// // Test the intersection between r1 and r2
/// sf::base::Optional<sf::IntRect> result = r1.findIntersection(r2);
/// // result.hasValue() == true
/// // result.value() == sf::IntRect({4, 2}, {16, 3})
/// \endcode
///
////////////////////////////////////////////////////////////
