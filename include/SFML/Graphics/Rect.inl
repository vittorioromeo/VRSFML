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
#include <SFML/Graphics/Rect.hpp> // NOLINT(misc-header-include-cycle)

#include <SFML/System/AlgorithmUtils.hpp>

namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr Rect<T>::Rect() = default;


////////////////////////////////////////////////////////////
template <typename T>
constexpr Rect<T>::Rect(const Vector2<T>& thePosition, const Vector2<T>& theSize) : position(thePosition), size(theSize)
{
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename U>
constexpr Rect<T>::Rect(const Rect<U>& rectangle) : position(rectangle.position), size(rectangle.size)
{
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool Rect<T>::contains(const Vector2<T>& point) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Compute the real min and max of the rectangle on both axes
    const T minX = priv::min(position.x, static_cast<T>(position.x + size.x));
    const T maxX = priv::max(position.x, static_cast<T>(position.x + size.x));
    const T minY = priv::min(position.y, static_cast<T>(position.y + size.y));
    const T maxY = priv::max(position.y, static_cast<T>(position.y + size.y));

    return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr std::optional<Rect<T>> Rect<T>::findIntersection(const Rect<T>& rectangle) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Compute the min and max of the first rectangle on both axes
    const T r1MinX = priv::min(position.x, static_cast<T>(position.x + size.x));
    const T r1MaxX = priv::max(position.x, static_cast<T>(position.x + size.x));
    const T r1MinY = priv::min(position.y, static_cast<T>(position.y + size.y));
    const T r1MaxY = priv::max(position.y, static_cast<T>(position.y + size.y));

    // Compute the min and max of the second rectangle on both axes
    const T r2MinX = priv::min(rectangle.position.x, static_cast<T>(rectangle.position.x + rectangle.size.x));
    const T r2MaxX = priv::max(rectangle.position.x, static_cast<T>(rectangle.position.x + rectangle.size.x));
    const T r2MinY = priv::min(rectangle.position.y, static_cast<T>(rectangle.position.y + rectangle.size.y));
    const T r2MaxY = priv::max(rectangle.position.y, static_cast<T>(rectangle.position.y + rectangle.size.y));

    // Compute the intersection boundaries
    const T interLeft   = priv::max(r1MinX, r2MinX);
    const T interTop    = priv::max(r1MinY, r2MinY);
    const T interRight  = priv::min(r1MaxX, r2MaxX);
    const T interBottom = priv::min(r1MaxY, r2MaxY);

    // If the intersection is valid (positive non zero area), then there is an intersection
    if ((interLeft < interRight) && (interTop < interBottom))
    {
        return std::make_optional<Rect<T>>(Vector2<T>{interLeft, interTop},
                                           Vector2<T>{interRight - interLeft, interBottom - interTop});
    }

    return std::nullopt;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Rect<T>::getCenter() const
{
    return position + size / T{2};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator==(const Rect<T>& lhs, const Rect<T>& rhs)
{
    return (lhs.position == rhs.position) && (lhs.size == rhs.size);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator!=(const Rect<T>& lhs, const Rect<T>& rhs)
{
    return !(lhs == rhs);
}

} // namespace sf
