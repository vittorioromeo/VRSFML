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
////////////////////////////////////////////////////////////
struct LaserDirection
{
    enum [[nodiscard]] Enum : za::U8
    {
        Left  = 0u,
        Right = 1u,
    };
};


////////////////////////////////////////////////////////////
inline constexpr za::SizeT laserDirectionCount = 2u;


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr za::Vec2i laserDirectionToVec2i(const LaserDirection::Enum direction)
{
    switch (direction)
    {
        case LaserDirection::Left:
            return {-1, 1};
        case LaserDirection::Right:
            return {1, 1};
        default:
            ZA_UNREACHABLE();
    }
}

} // namespace tsurv
