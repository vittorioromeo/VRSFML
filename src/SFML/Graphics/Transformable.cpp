#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/TransformableMixin.hpp"

#include "SFML/Base/FastSinCos.hpp"

// The code below could be `constexpr` but the `FastSinCos` header is a heavy
// compilation time hit.

namespace sf
{
Transform TransformableMixinBase::getTransform(const Vector2f position,
                                               const Vector2f scale,
                                               const Vector2f origin,
                                               const float    radians) const
{
    const auto [sine, cosine] = base::fastSinCos(radians);

    SFML_BASE_ASSUME(sine >= -1.f && sine <= 1.f);
    SFML_BASE_ASSUME(cosine >= -1.f && cosine <= 1.f);

    return Transform::from(position, scale, origin, sine, cosine);
}

} // namespace sf
