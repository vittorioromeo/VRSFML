// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Sprite.hpp"

#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Math/Fabs.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Rect2f Sprite::getLocalBounds() const
{
    return {{0.f, 0.f}, {ZA_MATH_FABSF(textureRect.size.x), ZA_MATH_FABSF(textureRect.size.y)}};
}


////////////////////////////////////////////////////////////
Rect2f Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}

} // namespace za
