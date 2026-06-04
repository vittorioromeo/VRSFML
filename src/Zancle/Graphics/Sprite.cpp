// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Sprite.hpp"
#include "Zancle/System/Rect2.hpp"
#include "ZancleBase/Math/Fabs.hpp"


namespace za
{
////////////////////////////////////////////////////////////
Rect2f Sprite::getLocalBounds() const
{
    return {{0.f, 0.f}, {ZB_MATH_FABSF(textureRect.size.x), ZB_MATH_FABSF(textureRect.size.y)}};
}


////////////////////////////////////////////////////////////
Rect2f Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}

} // namespace za
