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
// - highest $/s is now displayed in the stats screen
// - shrine text is now always draw in front of cat text
// - added spell mana requirement to "Remember Spell" tooltip in "magic" tab
// - added option to invert mouse buttons in "interface" tab under "settings"
// - automatically scroll screen when dragging cats near the edge, also tip about shift drag
// - prevent clicks done on top of UI from being registered as clicks on the game screen
// - prevent cat from being dragged when clicking offscreen
