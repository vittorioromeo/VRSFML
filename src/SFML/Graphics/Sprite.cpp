#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Sprite.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Sprite::Sprite(const FloatRect& rectangle) : textureRect(rectangle)
{
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getLocalBounds() const
{
    return {{0.f, 0.f}, {base::fabs(textureRect.size.x), base::fabs(textureRect.size.y)}};
}


////////////////////////////////////////////////////////////
FloatRect Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}

} // namespace sf
