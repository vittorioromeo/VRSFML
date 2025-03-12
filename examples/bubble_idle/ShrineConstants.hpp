#pragma once

#include "ExactArray.hpp"
#include "ShrineType.hpp"


////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    const char*,
    shrineNames,
    nShrineTypes,
    {
        "Shrine Of Voodoo",
        "Shrine Of Magic",
        "Shrine Of Clicking",
        "Shrine Of Automation",
        "Shrine Of Repulsion",
        "Shrine Of Attraction",
        "Shrine Of Camouflage",
        "Shrine Of Victory", // TODO P0: implement rubber duck?
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    float,
    shrineHues,
    nShrineTypes,
    {
        30.f,   // Voodoo
        -139.f, // Magic
        130.f,  // Clicking
        -15.f,  // Automation
        180.f,  // Repulsion
        -80.f,  // Attraction
        0.f,    // Camouflage
        0.f,    // Victory
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    const char*,
    shrineTooltipsByType,
    nShrineTypes,
    {
        R"(
~~ Shrine Of Voodoo ~~

Spirits can be felt emanating from this shrine, where a Witchcat is sealed inside. The Witchcat performs rituals on nearby cats, hexing one of them and capturing their soul in voodoo dolls that appear around the map.

Collecting all the dolls will release the hex and trigger a powerful timed effect depending on the type of the cursed cat.

The spirits in the shrine seem to demand a cat... only one, at least. Careful getting too close!
)",
        R"(
~~ Shrine Of Magic ~~

The legend says that a powerful Wizardcat is sealed inside, capable of absorbing wisdom from star bubbles and casting spells on demand. Guess the Wizardcat wasn't powerful enough to avoid being sealed...

The magic of star bubbles seem to be absorbed by the shrine, nullifying their benefits. Switch up your strategy!)",
        R"(
~~ Shrine Of Clicking ~~

Rumor has it that a sneaky Mousecat is sealed inside, capable of clicking bubbles, keeping up combos, empowering nearby cats, and providing a global click reward value multiplier by merely existing. That's a mouthful.

The shrine repels cats, wanting you to prove your worth by clicking bubbles. Hope your finger is ready!)",
        R"(
~~ Shrine Of Automation ~~

Stories of a crafty Engicat being sealed inside this shrine are told, capable of performing engine maintenance on nearby cats, temporarily increasing their speed, and providing a global cat reward value multiplier by merely existing.

Bubbles near the shrine are immune to clicks. Prove your automation skils!)",
        R"(
~~ Shrine Of Repulsion ~~

Experiments reveal that a Repulsocat is sealed inside, who continuously pushes bubbles away with a gigantic portable USB fan. Thankfully, other cats are too fat to be affected by the wind.

Seems like the wind is powerful enough to repel bubbles even from within the shrine.)",
        R"(
~~ Shrine Of Attraction ~~

Electromagnetism readings suggest that an Attractocat is sealed inside, who continuously attracts bubbles with a huge magnet. Despite cats having an engine, they are not affected by the magnet -- can't really explain that one...

The magnet is so powerful that its effects are felt even near the shrine.)",
        R"(
~~ Shrine Of Camouflage ~~

If you squint, you can see the elusive Copycat hiding inside, who can transform into any other unique cat at will, mimicking their effects. The Copycat is so good at camouflage that they lacks their own identity (and color).
w
// TODO P0: shrine effects)",
        R"(
~~ Shrine Of Victory ~~

Effects: TODO P1: complete

Rewards: TODO P1: complete)",
    });
