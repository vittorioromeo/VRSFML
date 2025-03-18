#pragma once


////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY_IMPL(...) #__VA_ARGS__

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY(...) BUBBLEBYTE_STRINGIFY_IMPL(__VA_ARGS__)

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MAJOR 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MINOR 3

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_PATCH 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_STR                                                   \
    "v" BUBBLEBYTE_STRINGIFY(BUBBLEBYTE_VERSION_MAJOR) "." BUBBLEBYTE_STRINGIFY( \
        BUBBLEBYTE_VERSION_MINOR) "." BUBBLEBYTE_STRINGIFY(BUBBLEBYTE_VERSION_PATCH)

////////////////////////////////////////////////////////////
struct [[nodiscard]] Version
{
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
};

////////////////////////////////////////////////////////////
inline constexpr Version currentVersion{BUBBLEBYTE_VERSION_MAJOR, BUBBLEBYTE_VERSION_MINOR, BUBBLEBYTE_VERSION_PATCH};


// TODO P0:
// CHANGELOG:
// - Removed unused hidden achievements
// - Fixed Pawdestination achievement not registering correctly
// - Transcendent Unicats and Hellsinged Devilcats can now be toggled at will after purchasing
// - Added prestige points sink, 1 trillion to 100PPs
// - Fixed some Witchcat achievements having a missing name
// - A secret achievement related to bubbles is now obtainable even after prestiging
//     - If you are at the max level prestige already and want to obtain the achievement, added a button "Reset current
//     prestige" under "Data"
// - Fixed session statistics not being correctly reset after a prestige
// - Fixed astrocat pop achievement
