#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Math/MinMaxMacros.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Compute the intersection of two rectangles, handling negative sizes correctly
///
/// \return Intersection rectangle if they overlap, `za::nullOpt` otherwise
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::pure]] constexpr za::Optional<Rect2<T>> findIntersection(const Rect2<T>& rect0, const Rect2<T>& rect1)
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Precompute right/bottom edges
    const T r0Right  = rect0.position.x + rect0.size.x;
    const T r0Bottom = rect0.position.y + rect0.size.y;
    const T r1Right  = rect1.position.x + rect1.size.x;
    const T r1Bottom = rect1.position.y + rect1.size.y;

    // Compute the min and max of the first rectangle on both axes
    const T r0MinX = ZA_MIN(rect0.position.x, r0Right);
    const T r0MaxX = ZA_MAX(rect0.position.x, r0Right);
    const T r0MinY = ZA_MIN(rect0.position.y, r0Bottom);
    const T r0MaxY = ZA_MAX(rect0.position.y, r0Bottom);

    // Compute the min and max of the second rectangle on both axes
    const T r1MinX = ZA_MIN(rect1.position.x, r1Right);
    const T r1MaxX = ZA_MAX(rect1.position.x, r1Right);
    const T r1MinY = ZA_MIN(rect1.position.y, r1Bottom);
    const T r1MaxY = ZA_MAX(rect1.position.y, r1Bottom);

    // Compute the intersection boundaries for the X axis
    const T interLeft  = ZA_MAX(r0MinX, r1MinX);
    const T interRight = ZA_MIN(r0MaxX, r1MaxX);

    // Early exit if no overlap on X axis
    if (interLeft >= interRight)
        return za::nullOpt;

    // Compute the intersection boundaries for the Y axis
    const T interTop    = ZA_MAX(r0MinY, r1MinY);
    const T interBottom = ZA_MIN(r0MaxY, r1MaxY);

    // Check for overlap on Y axis
    if (interTop >= interBottom)
        return za::nullOpt;

    // Intersection found
    return za::makeOptional<Rect2<T>>(Vec2<T>{interLeft, interTop}, Vec2<T>{interRight - interLeft, interBottom - interTop});
}

} // namespace za


////////////////////////////////////////////////////////////
/// \fn `za::findIntersection(const Rect2<T>&, const Rect2<T>&)`
/// \ingroup system
///
/// Checks if two rectangles overlap and, if they do, returns the
/// rectangle representing their intersection.
/// Handles rectangles with negative sizes correctly.
///
////////////////////////////////////////////////////////////
