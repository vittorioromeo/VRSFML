#pragma once


////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY_IMPL(...) #__VA_ARGS__

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY(...) BUBBLEBYTE_STRINGIFY_IMPL(__VA_ARGS__)

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MAJOR 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MINOR 0

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_PATCH 3

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

// TODO P0: changelog
// - fix the game crashing on Proton/Wine (but still needs attention)
// - range unicat upgrades are now only unlocked after buying aoe trascendence
// - made unicat transcendence more affordable
