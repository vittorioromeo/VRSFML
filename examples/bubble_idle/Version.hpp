#pragma once


////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY_IMPL(...) #__VA_ARGS__

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY(...) BUBBLEBYTE_STRINGIFY_IMPL(__VA_ARGS__)

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MAJOR 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MINOR 6

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

    constexpr bool operator==(const Version& other) const = default;
};

////////////////////////////////////////////////////////////
inline constexpr Version currentVersion{BUBBLEBYTE_VERSION_MAJOR, BUBBLEBYTE_VERSION_MINOR, BUBBLEBYTE_VERSION_PATCH};


// TODO P0: CHANGELOG (already created, must make public)
// - Added left control as another valid key for AOE selection
// - Performing an AOE selection near the edge of the screen will now scroll
// - Wisdom point counter in "Magic" menu now uses digit separators
// - It is now possible to update spells with wisdom points while the wizard is busy (e.g. absorbing)
// - The multipop circle range indicator is now hidden while dragging cats
// - Added a SFX volume slider
// - Fixed a crash related to the Wizard's Mewltiplier Aura spell
// - Changed #astrocats achievements fromm [1,5,10,20,30,40] to [1,5,10,20,25,30]
// - Improved statistics panel to avoid text going out of bounds
// - Added checkbox in in-game achivements menu to show/hide completed achievements

// TODO P0: ADD TO PATCH NOTES
// - In-game achievements are now always synced with Steam
