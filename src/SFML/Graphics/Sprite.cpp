#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Sprite.hpp"

#include "SFML/Base/Math/Fabs.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
FloatRect Sprite::getLocalBounds() const
{
    return {{0.f, 0.f}, {SFML_BASE_MATH_FABSF(textureRect.size.x), SFML_BASE_MATH_FABSF(textureRect.size.y)}};
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}

} // namespace sf
