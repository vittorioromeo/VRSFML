#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Rect.hpp> // NOLINT(misc-header-include-cycle)

#include <SFML/Base/Algorithm.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr bool Rect<T>::contains(Vector2<T> point) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Compute the real min and max of the rectangle on both axes
    const T minX = base::min(position.x, static_cast<T>(position.x + size.x));
    const T maxX = base::max(position.x, static_cast<T>(position.x + size.x));
    const T minY = base::min(position.y, static_cast<T>(position.y + size.y));
    const T maxY = base::max(position.y, static_cast<T>(position.y + size.y));

    return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Rect<T>::getCenter() const
{
    return position + size / T{2};
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename OtherRect>
constexpr OtherRect Rect<T>::to() const
{
    using ValueType = decltype(OtherRect{}.position.x);
    return Rect<ValueType>{position.template to<Vector2<ValueType>>(), size.template to<Vector2<ValueType>>()};
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
