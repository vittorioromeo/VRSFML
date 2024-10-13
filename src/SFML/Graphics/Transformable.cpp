#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Transformable.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/FastSinCos.hpp"


// The code below could be `constexpr` but the `FastSinCos` header is a heavy
// compilation time hit.

namespace sf
{
////////////////////////////////////////////////////////////
Transform Transformable::getTransform() const
{
    const auto [sine, cosine] = base::fastSinCos(rotation.asRadians());
    return Transform::from(position, scale, origin, sine, cosine);
}


////////////////////////////////////////////////////////////
Transform Transformable::getInverseTransform() const
{
    return getTransform().getInverse();
}

} // namespace sf
