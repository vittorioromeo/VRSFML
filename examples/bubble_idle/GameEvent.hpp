#pragma once

#include "SFML/Base/Variant.hpp"


////////////////////////////////////////////////////////////
// Each concrete event kind carries its own data. Positional events keep their
// region on the map; global events just need a duration. Adding a new kind is
// a matter of declaring a struct and extending the `GameEvent` variant below.
// Tuning (durations, spawn intervals, etc.) lives in `GameConstants::events`
// so it round-trips through `game_constants.json` and can be tweaked live.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Positional event: dense, fast-falling bubbles over a horizontal region.
struct [[nodiscard]] EBubblefall
{
    float regionCenterX{};
    float regionWidth{};
    float remainingMs{};
    float subTickMs{};
};


////////////////////////////////////////////////////////////
// Global one-shot event: spawns a single invincible Combo bubble at the top
// of the map. The bubble itself is the manifestation; the event entry just
// exists so the random scheduler can pick it. `remainingMs` ticks down to 0
// and the entry is dropped on the next frame.
struct [[nodiscard]] EInvincibleBubble
{
    float remainingMs{};
};


////////////////////////////////////////////////////////////
using GameEvent = sf::base::Variant<EBubblefall, EInvincibleBubble>;
