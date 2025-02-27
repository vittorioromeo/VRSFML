#pragma once

#include "CatType.hpp"
#include "ExactArray.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vector2.hpp"


namespace CatConstants
{
////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    sf::Color,
    colors,
    nCatTypes,
    {
        {192u, 147u, 109u}, // Normal
        {129u, 251u, 191u}, // Uni
        {191u, 61u, 61u},   // Devil
        sf::Color::White,   // Astro

        {90u, 155u, 48u},   // Witch
        {123u, 108u, 191u}, // Wizard
        {175u, 175u, 175u}, // Mouse
        {72u, 51u, 33u},    // Engi
        {182u, 110u, 135u}, // Repulso
        {132u, 132u, 132u}, // Attracto
    });


////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    const char*,
    actionNames,
    nCatTypes,
    {
        "Pops",    // Normal
        "Shines",  // Uni
        "IEDs",    // Devil
        "Flights", // Astro

        "Rituals", // Witch
        "Spells",  // Wizard
        "Clicks",  // Mouse
        "Boosts",  // Engi
        "N/A",     // Repulso
        "N/A",     // Attracto
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    float,
    baseCooldowns,
    nCatTypes,
    {
        750.f,    // Normal
        3000.f,   // Uni
        7000.f,   // Devil
        10'000.f, // Astro

        80'000.f, // Witch
        1500.f,   // Wizard
        400.f,    // Mouse
        10'000.f, // Engi
        5.f,      // Repulso
        5.f,      // Attracto
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    float,
    baseRanges,
    nCatTypes,
    {
        96.f, // Normal
        64.f, // Uni
        48.f, // Devil
        48.f, // Astro

        192.f, // Witch
        384.f, // Wizard
        160.f, // Mouse
        192.f, // Engi
        192.f, // Repulso
        192.f, // Attracto
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    sf::Vector2f,
    rangeOffsets,
    nCatTypes,
    {
        {0.f, 0.f},    // Normal
        {0.f, -100.f}, // Uni
        {0.f, 100.f},  // Devil
        {-64.f, 0.f},  // Astro

        {0.f, 0.f}, // Witch
        {0.f, 0.f}, // Wizard
        {0.f, 0.f}, // Mouse
        {0.f, 0.f}, // Engi
        {0.f, 0.f}, // Repulso
        {0.f, 0.f}, // Attracto
    });

////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    const char*,
    typeNames,
    nCatTypes,
    {
        "Normal",
        "Uni",
        "Devil",
        "Astro",

        "Witch",
        "Wizard",
        "Mouse",
        "Engi",
        "Repulso",
        "Attracto",
    });

} // namespace CatConstants
