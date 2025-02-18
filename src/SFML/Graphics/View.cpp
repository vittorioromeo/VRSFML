#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/View.hpp"

#include "SFML/Base/FastSinCos.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
View View::fromRect(const FloatRect& rectangle)
{
    return {.center = rectangle.position + rectangle.size / 2.f, .size = rectangle.size};
}


////////////////////////////////////////////////////////////
Transform View::getTransform() const
{
    // Rotation components
    const float angle         = rotation.asRadians();
    const auto [sine, cosine] = base::fastSinCos(angle);

    const float tx = -center.x * cosine - center.y * sine + center.x;
    const float ty = center.x * sine - center.y * cosine + center.y;

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
