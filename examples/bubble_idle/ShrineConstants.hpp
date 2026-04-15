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
        "Shrine Of Victory",
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
        -70.f,  // Repulsion
        150.f,  // Attraction
        0.f,    // Camouflage
        60.f,   // Victory
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    const char*,
    shrineTooltipsByType,
    nShrineTypes,
    {
        R"(
~~ Shrine Of Voodoo ~~

Spirits emanate from this shrine.

A Witchcat is sealed inside, performing rituals on nearby cats and scattering voodoo dolls across the map.

The spirits demand a cat sacrifice. Only one. Careful getting too close.

)",
        R"(
~~ Shrine Of Magic ~~

Legend says a powerful Wizardcat is sealed inside, capable of absorbing wisdom from star bubbles and casting spells on demand.

Apparently not powerful enough to avoid being sealed in the first place.

The shrine absorbs star bubble magic, nullifying their bonus. Time to rethink your strategy.)",
        R"(
~~ Shrine Of Clicking ~~

Rumor has it a sneaky Mousecat is sealed inside, clutching a stolen Logicat gaming mouse that somehow still works unplugged.

The shrine repels cats. You'll have to prove your worth by clicking. Hope your finger is ready.)",
        R"(
~~ Shrine Of Automation ~~

Stories tell of a crafty Engicat sealed inside, endlessly performing maintenance on machinery that doesn't exist.

Bubbles near the shrine are immune to clicks. Let your cats handle this one.)",
        R"(
~~ Shrine Of Repulsion ~~

Experiments reveal a Repulsocat sealed inside, continuously pushing bubbles away with a portable USB fan of alarming power.

Other cats are too fat to be affected by the wind.

The fan works even from inside the shrine. Can't explain that one.)",
        R"(
~~ Shrine Of Attraction ~~

Electromagnetism readings suggest an Attractocat is sealed inside, pulling bubbles toward the shrine with a magnet of unreasonable strength.

Cats are unaffected, despite presumably containing metal parts -- at least, according to the Engicat.

The magnetic field extends well beyond the shrine walls.)",
        R"(
~~ Shrine Of Camouflage ~~

If you squint, you can see the elusive Copycat hiding inside, ready to mimic any unique cat at will.

They're so good at camouflage that they lack their own identity. And color.)",
        R"(
~~ Shrine Of Victory ~~

It's almost over.

There is definitely something inside, but you cannot tell what is it from here.

Only one way to know.)",
    });
