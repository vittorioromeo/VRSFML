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
struct DrillDirection
{
    enum [[nodiscard]] Enum : zb::U8
    {
        Left  = 0u,
        Right = 1u,
        Down  = 2u
    };
};


/////////////////////////////////////////////////////////////
inline constexpr zb::SizeT drillDirectionCount = 3u;


/////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr za::Vec2i drillDirectionToVec2i(const DrillDirection::Enum direction)
{
    switch (direction)
    {
        case DrillDirection::Left:
            return {-1, 0};
        case DrillDirection::Right:
            return {1, 0};
        case DrillDirection::Down:
            return {0, 1};
        default:
            ZB_UNREACHABLE();
    }
}

} // namespace tsurv
