#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/RectUtils.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/MinMax.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
base::Optional<Rect<T>> findIntersection(const Rect<T>& rect0, const Rect<T>& rect1)
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Compute the min and max of the first rectangle on both axes
    const T r1MinX = base::min(rect0.position.x, static_cast<T>(rect0.position.x + rect0.size.x));
    const T r1MaxX = base::max(rect0.position.x, static_cast<T>(rect0.position.x + rect0.size.x));
    const T r1MinY = base::min(rect0.position.y, static_cast<T>(rect0.position.y + rect0.size.y));
    const T r1MaxY = base::max(rect0.position.y, static_cast<T>(rect0.position.y + rect0.size.y));

    // Compute the min and max of the second rectangle on both axes
    const T r2MinX = base::min(rect1.position.x, static_cast<T>(rect1.position.x + rect1.size.x));
    const T r2MaxX = base::max(rect1.position.x, static_cast<T>(rect1.position.x + rect1.size.x));
    const T r2MinY = base::min(rect1.position.y, static_cast<T>(rect1.position.y + rect1.size.y));
    const T r2MaxY = base::max(rect1.position.y, static_cast<T>(rect1.position.y + rect1.size.y));

    // Compute the intersection boundaries
    const T interLeft   = base::max(r1MinX, r2MinX);
    const T interTop    = base::max(r1MinY, r2MinY);
    const T interRight  = base::min(r1MaxX, r2MaxX);
    const T interBottom = base::min(r1MaxY, r2MaxY);

    // If the intersection is valid (positive non zero area), then there is an intersection
    if ((interLeft < interRight) && (interTop < interBottom))
    {
        return base::makeOptional<Rect<T>>(Vector2<T>{interLeft, interTop},
                                           Vector2<T>{interRight - interLeft, interBottom - interTop});
    }

    return base::nullOpt;
}

} // namespace sf
