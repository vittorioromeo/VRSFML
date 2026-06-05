#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Container/InPlaceVector.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct [[nodiscard]] LaserableBlocksInfo
{
    za::InPlaceVector<za::Vec2i, 32> positions;
    za::InPlaceVector<za::Vec2i, 32> bouncePositions;
    za::Vec2i                        bouncePos;
};

} // namespace tsurv
