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
struct DrillDirection
{
    enum [[nodiscard]] Enum : sf::base::U8
    {
        Left  = 0u,
        Right = 1u,
        Down  = 2u
    };
};


/////////////////////////////////////////////////////////////
inline constexpr sf::base::SizeT drillDirectionCount = 3u;


/////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr sf::Vec2i drillDirectionToVec2i(const DrillDirection::Enum direction)
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
            SFML_BASE_UNREACHABLE();
    }
}

} // namespace tsurv
