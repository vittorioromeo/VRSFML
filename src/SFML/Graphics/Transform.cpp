#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/FastSinCos.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Transform& Transform::rotate(Angle angle)
{
    const auto [sine, cosine] = base::fastSinCos(angle.wrapUnsigned().asRadians());

    // clang-format off
    const Transform rotation(cosine, -sine, 0.f,
                             sine,  cosine, 0.f);
    // clang-format on

    return combine(rotation);
}


////////////////////////////////////////////////////////////
Transform& Transform::rotate(Angle angle, Vector2f center)
{
    const auto [sine, cosine] = base::fastSinCos(angle.wrapUnsigned().asRadians());

    // clang-format off
    const Transform rotation(cosine, -sine, center.x * (1.f - cosine) + center.y * sine,
                             sine,  cosine, center.y * (1.f - cosine) - center.x * sine);
    // clang-format on

    return combine(rotation);
}

} // namespace sf
