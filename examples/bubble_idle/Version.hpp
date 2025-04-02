#pragma once


////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY_IMPL(...) #__VA_ARGS__

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_STRINGIFY(...) BUBBLEBYTE_STRINGIFY_IMPL(__VA_ARGS__)

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MAJOR 1

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_MINOR 7

////////////////////////////////////////////////////////////
#define BUBBLEBYTE_VERSION_PATCH 0

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


// x - Added left control as another valid key for AOE selection
// x - Performing an AOE selection near the edge of the screen will now scroll
// x - Wisdom point counter in "Magic" menu now uses digit separators
// x - It is now possible to update spells with wisdom points while the wizard is busy (e.g. absorbing)
// x - The multipop circle range indicator is now hidden while dragging cats
// x - Added a SFX volume slider
// x - Fixed a crash related to the Wizard's Mewltiplier Aura spell
// x - Improved statistics panel to avoid text going out of bounds
// x - Made it possible to bulk purchase PPs in end game (show screenshot)
// x - Long button labels are now scaled to avoid the text going out of bounds
// x - Maxed-out button tooltips will now show "MAXED OUT" instead of calculating one more step
// x - Sligthly increased menu scrollbar thickness
// x - Added ability to hide maxed-out purchasables and category separators in shop ("Interface" tab)
// x - Fixed "Remember to buy combo upgrade tip" showing up after prestige
// x - Slightly increased default cursor scale
// x - In-game achievements are now always synced with Steam
// x - Achievement progress for most achievements is now displayed in-game
// x - Changed #astrocats achievements from [1,5,10,20,30,40] to [1,5,10,20,25,30]
// x - Added checkbox in in-game achivements menu to show/hide completed achievements
// x - Added "UNDO" button to undo PP purchases, can be used multiple times within 10s
