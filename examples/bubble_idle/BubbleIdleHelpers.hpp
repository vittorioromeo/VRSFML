#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Standalone helpers and small drawing-config types extracted
// from `BubbleIdleMain.hpp` so they can be reused without
// pulling in the full `Main` struct definition.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Math/Pow.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CPUDrawableBatch;
} // namespace sf


////////////////////////////////////////////////////////////
struct CloudFrameDrawSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    float time;

    sf::Vec2f mins;
    sf::Vec2f maxs;

    int xSteps;
    int ySteps;

    float     scaleMult;
    float     outwardOffsetMult;
    sf::Color color = sf::Color::White;

    sf::CPUDrawableBatch* batch;
};


////////////////////////////////////////////////////////////
inline constexpr auto playerComboDecay      = 0.95f;
inline constexpr auto playerComboDecayLaser = 0.995f;
inline constexpr auto mouseCatComboDecay    = 0.995f; // higher decay for mousecat (higher reward)


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr float getComboValueMult(const int n, const float decay)
{
    return (1.f - sf::base::pow(decay, static_cast<float>(n))) / (1.f - decay);
}
