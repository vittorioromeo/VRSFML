#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transformable.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/FastSinCos.hpp"


// The code below could be `constexpr` but the `FastSinCos` header is a heavy
// compilation time hit.

namespace sf
{
////////////////////////////////////////////////////////////
Transform Transformable::getTransform() const
{
    const float angle         = rotation.asRadians();
    const auto [sine, cosine] = base::fastSinCos(angle);
    const float sxc           = scale.x * cosine;
    const float syc           = scale.y * cosine;
    const float sxs           = scale.x * -sine;
    const float sys           = scale.y * -sine;
    const float tx            = -origin.x * sxc - origin.y * sys + position.x;
    const float ty            = origin.x * sxs - origin.y * syc + position.y;

    return {/* a00 */ sxc, /* a01 */ sys, /* a02 */ tx, -/* a10 */ sxs, /* a11 */ syc, /* a12 */ ty};
}


////////////////////////////////////////////////////////////
Transform Transformable::getInverseTransform() const
{
    return getTransform().getInverse();
}

} // namespace sf
