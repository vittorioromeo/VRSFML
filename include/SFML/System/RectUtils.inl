////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/RectUtils.hpp> // NOLINT(misc-header-include-cycle)

#include <SFML/Base/Algorithm.hpp>


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
        return sf::base::makeOptional<Rect<T>>(Vector2<T>{interLeft, interTop},
                                               Vector2<T>{interRight - interLeft, interBottom - interTop});
    }

    return base::nullOpt;
}

} // namespace sf
