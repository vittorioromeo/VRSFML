#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Base/Unreachable.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct DrillDirection
{
    enum [[nodiscard]] Enum : za::U8
    {
        Left  = 0u,
        Right = 1u,
        Down  = 2u
    };
};


/////////////////////////////////////////////////////////////
inline constexpr za::SizeT drillDirectionCount = 3u;


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
            ZA_UNREACHABLE();
    }
}

} // namespace tsurv
