#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Standalone helpers and small drawing-config types extracted
// from `BubbleIdleMain.hpp` so they can be reused without
// pulling in the full `Main` struct definition.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Color.hpp"

#include "Zancle/Geometry/Vec2.hpp"

#include "Zancle/Math/Pow.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class CPUDrawableBatch;
} // namespace za


////////////////////////////////////////////////////////////
struct CloudFrameDrawSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    float time;

    za::Vec2f mins;
    za::Vec2f maxs;

    int xSteps;
    int ySteps;

    float     scaleMult;
    float     outwardOffsetMult;
    za::Color color = za::Color::White;

    za::CPUDrawableBatch* batch;
};


////////////////////////////////////////////////////////////
inline constexpr auto playerComboDecay      = 0.95f;
inline constexpr auto playerComboDecayLaser = 0.995f;
inline constexpr auto mouseCatComboDecay    = 0.995f; // higher decay for mousecat (higher reward)


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr float getComboValueMult(const int n, const float decay)
{
    return (1.f - za::pow(decay, static_cast<float>(n))) / (1.f - decay);
}
