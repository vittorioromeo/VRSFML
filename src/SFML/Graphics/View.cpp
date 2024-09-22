#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/View.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
View::View(const FloatRect& rectangle) : center(rectangle.getCenter()), size(rectangle.size)
{
}


////////////////////////////////////////////////////////////
View::View(Vector2f theCenter, Vector2f theSize) : center(theCenter), size(theSize)
{
}


////////////////////////////////////////////////////////////
void View::move(Vector2f offset)
{
    center += offset;
}


////////////////////////////////////////////////////////////
void View::rotate(Angle angle)
{
    rotation += angle;
}


////////////////////////////////////////////////////////////
void View::zoom(float factor)
{
    size *= factor;
}


////////////////////////////////////////////////////////////
Transform View::getTransform() const
{
    // Rotation components
    const float angle  = rotation.asRadians();
    const float cosine = base::cos(angle);
    const float sine   = base::sin(angle);
    const float tx     = -center.x * cosine - center.y * sine + center.x;
    const float ty     = center.x * sine - center.y * cosine + center.y;

    // Projection components
    const float a = 2.f / size.x;
    const float b = -2.f / size.y;
    const float c = -a * center.x;
    const float d = -b * center.y;

    // Rebuild the projection matrix
    return {a * cosine, a * sine, a * tx + c, -b * sine, b * cosine, b * ty + d};
}


////////////////////////////////////////////////////////////
Transform View::getInverseTransform() const
{
    return getTransform().getInverse();
}

} // namespace sf
