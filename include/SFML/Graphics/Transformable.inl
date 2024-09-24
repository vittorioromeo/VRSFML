#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transformable.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
constexpr void Transformable::scaleBy(Vector2f factor)
{
    scale.x *= factor.x;
    scale.y *= factor.y;
}


////////////////////////////////////////////////////////////
constexpr Transform Transformable::getTransform() const
{
    const float angle  = -rotation.asRadians();
    const float cosine = base::cos(angle); // TODO P0: bottleneck
    const float sine   = base::sin(angle); // TODO P0: bottleneck
    const float sxc    = scale.x * cosine;
    const float syc    = scale.y * cosine;
    const float sxs    = scale.x * sine;
    const float sys    = scale.y * sine;
    const float tx     = -origin.x * sxc - origin.y * sys + position.x;
    const float ty     = origin.x * sxs - origin.y * syc + position.y;

    return {/* a00 */ sxc, /* a01 */ sys, /* a02 */ tx, -/* a10 */ sxs, /* a11 */ syc, /* a12 */ ty};
}


////////////////////////////////////////////////////////////
constexpr Transform Transformable::getInverseTransform() const
{
    return getTransform().getInverse();
}

} // namespace sf
