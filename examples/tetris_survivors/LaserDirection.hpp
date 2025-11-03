#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Builtin/Unreachable.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"


namespace tsurv
{
/////////////////////////////////////////////////////////////
struct LaserDirection
{
    enum [[nodiscard]] Enum : sf::base::U8
    {
        Left  = 0u,
        Right = 1u,
    };
};


/////////////////////////////////////////////////////////////
inline constexpr sf::base::SizeT laserDirectionCount = 2u;


/////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr sf::Vec2i laserDirectionToVec2i(const LaserDirection::Enum direction)
{
    switch (direction)
    {
        case LaserDirection::Left:
            return {-1, 1};
        case LaserDirection::Right:
            return {1, 1};
        default:
            SFML_BASE_UNREACHABLE();
    }
}

} // namespace tsurv
