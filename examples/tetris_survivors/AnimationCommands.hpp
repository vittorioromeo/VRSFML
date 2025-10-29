#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"
#include "DrillDirection.hpp"
#include "LaserDirection.hpp"
#include "Tetramino.hpp"

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"


namespace tsurv
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimWait
{
    float duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimHardDrop // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino tetramino;
    float     duration;
    int       endY;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimSquish // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino tetramino;
    float     duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimClearLines
{
    sf::base::Vector<sf::base::SizeT> rows;
    float                             duration;
    bool                              awardXP;
    bool                              forceClear;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimFadeBlocks
{
    struct FadingBlock // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        Block      block;
        sf::Vec2uz position;
    };

    sf::base::Vector<FadingBlock> fadingBlocks;
    float                         duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimCollapseGrid
{
    sf::base::Vector<sf::base::SizeT> clearedRows;
    float                             duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimDrill // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino            tetramino;
    DrillDirection::Enum direction;
    float                duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimColumnClear // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::Vec2uz position;
    float      duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimAction
{
    sf::base::FixedFunction<void(), 64> action;
    float                               duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimLaser // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino            tetramino;
    LaserDirection::Enum direction;
    sf::Vec2i            gridStartPos;
    sf::Vec2i            gridTargetPos;
    bool                 onlyVisual;
    float                duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimFadeAttachments // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    Tetramino tetramino;
    float     duration;
};

////////////////////////////////////////////////////////////
using AnimationCommandP0 = sf::base::Variant<AnimWait, //
                                             AnimDrill,
                                             AnimAction,
                                             AnimLaser,
                                             AnimFadeAttachments>;


////////////////////////////////////////////////////////////
using AnimationCommandP1 = sf::base::Variant<AnimHardDrop, //
                                             AnimSquish,
                                             AnimClearLines,
                                             AnimFadeBlocks,
                                             AnimCollapseGrid,
                                             AnimColumnClear>;

} // namespace tsurv
