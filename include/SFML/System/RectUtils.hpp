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
    return base::makeOptional<Rect<T>>(Vec2<T>{interLeft, interTop}, Vec2<T>{interRight - interLeft, interBottom - interTop});
}

} // namespace sf


////////////////////////////////////////////////////////////
/// \fn `sf::findIntersection(const Rect<T>&, const Rect<T>&)`
/// \ingroup system
///
/// Checks if two rectangles overlap and, if they do, returns the
/// rectangle representing their intersection.
/// Handles rectangles with negative sizes correctly.
///
////////////////////////////////////////////////////////////
