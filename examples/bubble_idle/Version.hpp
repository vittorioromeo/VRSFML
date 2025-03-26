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


// TODO P0: CHANGELOG
// - UI tabs can now be selected with keyboard shortcuts (1-5)
// - The UI can be closed altogether with the Backslash/Tilde key (before number key 1)
// - It is now possible to scroll the map using the scroll wheel (assuming you're not hovering UI)
// - Added several shortcuts for map navigation:
//     - Home/End: Jump to the start/end of the map
//     - PgUp/PgDn or Mouse Side Buttons: Jump to the previous/next shrine
// - It is now possible to scroll the map left/right using A/D keys as well
//     - Furthermore, if LShift is held, the map will scroll faster (including scroll wheel)
// - It is now possible to jump at a certain position by clicking on the minimap
// - Added subtle drop shadow effect to the mouse cursor to improve visibility
// - Fixed Mousecat multipop state being reset while prestiging (it now persists)
// - The Wizardcat can now be dragged while absorbing wisdom or while casting a spell
// - The screen shake effect can now be toggled (either via quick settings or Graphics tab)
// - Fixed typos in several tips
// - Added a new tip about dragging multiple cats at once with lshift when purchasing 3 cats
// - Made multiple cats dragging more intuitive, you can now release either lmb or lshift while dragging
// - Slightly increased tip duration and duration now scales with length of tip
// - Tips updated to reflect new shortcuts and features
// - Fixed astrocats losing their initial position after flying -- formations should now be stable
// - Added option to toggle cat bobbing animation (floating) in "Graphics"
// - Added option to increase cat range outline thickness in "Graphics"
// - Added option to tweak the thickness of the cat range circle indicator
// - Fixed multiple cats colliding with shrines while being dragged and ruining formations
// - Minor sprite art cleanup/improvements
// - Added "Y" as a click key (good for QWERTZ users)
// - Renamed "Stats" to "Info" and added new "Tips" tab containing the most useful tips
// - Mention bigger upcoming changes
// - Added a toggleable combo bar under the cursor
// - Avoid drawing cursor combo text and combo bar while dragging cats
// - Added text under inactive shrines to remind players how to activate them, only visible on 1st prestige
// - Added a subtle circle range indicator for when multipop is active
// - Added a plus sign to the cursor when multipop is active
// - Slightly increased default cursor scale
// - Fixed a secret achievement incorrectly unlocking when prestiging
