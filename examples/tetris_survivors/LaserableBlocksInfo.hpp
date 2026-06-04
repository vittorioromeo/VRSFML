#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/InPlaceVector.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct [[nodiscard]] LaserableBlocksInfo
{
    zb::InPlaceVector<za::Vec2i, 32> positions;
    zb::InPlaceVector<za::Vec2i, 32> bouncePositions;
    za::Vec2i                        bouncePos;
};

} // namespace tsurv
