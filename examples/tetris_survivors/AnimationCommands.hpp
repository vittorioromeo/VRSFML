#pragma once


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Block.hpp"

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
struct [[nodiscard]] AnimHardDrop
{
    float duration;
    int   endY;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimSquish
{
    float duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimEmbed
{
    float duration;
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
        Block     block;
        sf::Vec2f position;
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
struct [[nodiscard]] AnimVerticalDrill
{
    float duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimHorizontalDrillLeft
{
    float duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimHorizontalDrillRight
{
    float duration;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] AnimColumnClear
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
using AnimationCommand = sf::base::Variant<
    AnimWait, //
    AnimHardDrop,
    AnimSquish,
    AnimEmbed,
    AnimClearLines,
    AnimFadeBlocks,
    AnimCollapseGrid,
    AnimVerticalDrill,
    AnimHorizontalDrillLeft,
    AnimHorizontalDrillRight,
    AnimColumnClear,
    AnimAction>;

} // namespace tsurv
