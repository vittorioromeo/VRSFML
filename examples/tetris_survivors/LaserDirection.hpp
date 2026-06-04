#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/Builtin/Unreachable.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct LaserDirection
{
    enum [[nodiscard]] Enum : zb::U8
    {
        Left  = 0u,
        Right = 1u,
    };
};


/////////////////////////////////////////////////////////////
inline constexpr zb::SizeT laserDirectionCount = 2u;


/////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr za::Vec2i laserDirectionToVec2i(const LaserDirection::Enum direction)
{
    switch (direction)
    {
        case LaserDirection::Left:
            return {-1, 1};
        case LaserDirection::Right:
            return {1, 1};
        default:
            ZB_UNREACHABLE();
    }
}

} // namespace tsurv
