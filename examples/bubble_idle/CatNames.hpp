#pragma once

#include "CatType.hpp"
#include "ExactArray.hpp"
#include "ZancleBase/GetArraySize.hpp"
#include "ZancleBase/Span.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Vector.hpp"

#include "ExampleUtils/RNGFast.hpp"


////////////////////////////////////////////////////////////
inline constexpr const char* normalCatNames[] =
    {"Mochi",      "Biscuit", "Tuna",      "Waffle",    "Noodle",     "Pebble",    "Pickle",      "Pudding", "Sprout",
     "Boop",       "Nacho",   "Dandelion", "Sushi",     "Snickers",   "Muffin",    "Zippy",       "Pumpkin", "Gizmo",
     "Wobble",     "Cupcake", "Mittens",   "Doodle",    "Beans",      "Rawr",      "Marshmallow", "Fizz",    "Whiskers",
     "Pounce",     "Olive",   "Honey",     "Pepper",    "Nibble",     "Cinnamon",  "Jinx",        "Toffee",  "Zorro",
     "Squish",     "Pogo",    "Purrseus",  "Shadow",    "Pixel",      "Bubbly",    "Yoshi",       "Dizzy",   "Loki",
     "Taco",       "Ziggy",   "Snuggles",  "Cuddles",   "Gorgonzola", "Pistachio", "Mozzarella",  "Nutella", "Cookie",
     "Strawberry", "Cloud",   "Espresso",  "Peanut",    "Donut",      "Vader",     "Yoda",        "Gandalf", "Bilbo",
     "Leia",       "Sonic",   "Mario",     "Luigi",     "Zelda",      "Link",      "Neo",         "Tron",    "Stark",
     "Draco",      "Hodor",   "Khaleesi",  "Dovahkiin", "Nuka",       "Cortana",   "Tardis",      "Rogue",   "Frodo",
     "Gimli",      "Legolas", "Bender",    "Zim",       "Stan",       "Cartman",   "Kenny",       "Kyle",    "Butters",
     "Randy",      "Ike",     "Chef",      "Timmy",     "Homer",      "Bart",      "Marge",       "Lisa",    "Maggie",
     "Flanders",   "Reimu",   "Marisa",    "Cirno",     "Remilia",    "Sakuya",    "Youmu",       "Yuyuko",  "Reisen",
     "Tewi",       "Aya",     "Sanae",     "Kanako",    "Suwako",     "Beethro",   "Halph",       "Dugan",   "Denfry",
     "Tendry",     "Gunthro", "Cannolo",   "Braciola",  "Granita",    "Shodan"};


////////////////////////////////////////////////////////////
inline constexpr const char* uniCatNames[] =
    {"Orion",   "Cassiopeia",  "Andromeda", "Ursa",      "Scorpius", "Lyra",    "Cygnus",    "Pegasus",
     "Hydra",   "Capricornus", "Aquarius",  "Centaurus", "Corvus",   "Crux",    "Delphinus", "Eridanus",
     "Lepus",   "Monoceros",   "Ophiuchus", "Eris",      "Cassie",   "Capella", "Auriga",    "Pavo",
     "Atria",   "Aquila",      "Aries",     "Gemini",    "Leo",      "Libra",   "Pisces",    "Sagittarius",
     "Scorpio", "Taurus",      "Virgo",     "Polaris",   "Lucero",   "Halley",  "Comet",     "Astrid",
     "Satoru",  "Shun",        "Saki",      "Mamoru",    "Maria"};


////////////////////////////////////////////////////////////
inline constexpr const char* devilCatNames[] =
    {"Atronach",       "Inferno",     "Satan",      "Lucifer",   "Beelzebub", "Mephisto",   "Diablo",       "Baphomet",
     "Belial",         "Moloch",      "Asmodeus",   "Leviathan", "Behemoth",  "Krampus",    "Azazel",       "Abaddon",
     "Astaroth",       "Lilith",      "Mammon",     "Baal",      "Samael",    "Belphegor",  "Nyarlathotep", "Cthulhu",
     "Shub-Niggurath", "Yog-Sothoth", "Hastur",     "Dagon",     "Ithaqua",   "Tsathoggua", "Nodens",       "Yig",
     "Ghatanothoa",    "Yibb-Tstll",  "Zoth-Ommog", "Cthugha",   "Hypnos",    "Nug",        "Yogash",       "Zoth",
     "Karlach",        "Alfira",      "Zevlor",     "Dammon",    "Arabella",  "Raphael",    "Mizora"};


////////////////////////////////////////////////////////////
inline constexpr const char* astroCatNames[] =
    {"Armstrong", "Buzz",  "Apollo",   "Lightyear", "Aldrin",   "Gagarin",       "Sputnik", "Vostok",
     "Fry",       "Leela", "Zoidberg", "Shenzhou",  "Tiangong", "Spock",         "Picard",  "Cooper",
     "Groot",     "Kirk",  "Spock",    "Hadfield",  "Jamison",  "Cristoforetti", "Solo",    "Shepard"};


////////////////////////////////////////////////////////////
// TODO: dedicated wardencat name list once they get their own art.
inline constexpr const char* wardenCatNames[] = {
    "Argus",
    "Patrol",
    "Sentry",
    "Vigil",
    "Watcher",
    "Reveille",
    "Bugle",
    "Dawn",
    "Roost",
    "Snoozer",
    "Reverie",
    "Awakener",
};


////////////////////////////////////////////////////////////
inline constexpr const char* witchCatNames[] = {
    "Bayou",
    "Roux",
    "Gumbo",
    "Maman",
    "Marinette",
    "Simbi",
    "Legba",
    "Erzulie",
    "Damballa",
    "Ayida",
    "Agwe",
    "Marassa",
    "Samedi",
    "Ethel",
};


////////////////////////////////////////////////////////////
inline constexpr const char* wizardCatNames[] = {
    "Gandalf",
    "Dumbledore",
    "Harry",
    "Merlin",
    "Xerath",
    "Zilean",
    "Veigar",
    "Ryze",
    "Gale",
    "Balthazar",
    "Elminster",
};


////////////////////////////////////////////////////////////
inline constexpr const char* mouseCatNames[] = {
    "Clicker",
    "Squeaky",
    "Nibbler",
    "Twitch",
    "Squealer",
    "Kiroumaru",
    "Yakomaru",
    "Splinter",
    "Remy",
    "Jerry",
    "Mickey",
    "Fievel",
};


////////////////////////////////////////////////////////////
inline constexpr const char* engiCatNames[] = {
    "Stark",
    "Tesla",
    "Edison",
    "Watt",
    "Volta",
    "Faraday",
    "Freeman",
    "Torbjorn",
    "Bulma",
    "Kaylee",
    "Scotty",
    "Brown",
    "Sanchez",
    "Ratchet",
    "Wily",
};


////////////////////////////////////////////////////////////
inline constexpr const char* repulsoCatNames[] = {
    "Janna",
    "Zephyr",
    "Storm",
    "Aang",
    "Korra",
    "Venti",
    "Sciangazza",
    "Aeolus",
};


////////////////////////////////////////////////////////////
inline constexpr const char* attractoCatNames[] = {
    "Magneto",
    "Polaris",
    "Magnemite",
    "Neodymium",
    "Ferrite",
};


////////////////////////////////////////////////////////////
inline constexpr const char* copyCatNames[] = {
    "Bond",
    "Chesire",
    "Snake",
    "Garrett",
    "Fisher",
    "Shadow",
    "Lupin",
    "Sly",
    "Fortyseven",
};


////////////////////////////////////////////////////////////
inline constexpr const char* duckCatNames[] = {
    "Victoria",
};


////////////////////////////////////////////////////////////
inline constexpr EXACT_ARRAY(
    zb::Span<const char* const>,
    catNamesPerType,
    nCatTypes,
    {
        {normalCatNames, zb::getArraySize(normalCatNames)}, // Normal
        {uniCatNames, zb::getArraySize(uniCatNames)},       // Uni
        {devilCatNames, zb::getArraySize(devilCatNames)},   // Devil
        {astroCatNames, zb::getArraySize(astroCatNames)},   // Astro
        {wardenCatNames, zb::getArraySize(wardenCatNames)}, // Warden

        {witchCatNames, zb::getArraySize(witchCatNames)},       // Witch
        {wizardCatNames, zb::getArraySize(wizardCatNames)},     // Wizard
        {mouseCatNames, zb::getArraySize(mouseCatNames)},       // Mouse
        {engiCatNames, zb::getArraySize(engiCatNames)},         // Engi
        {repulsoCatNames, zb::getArraySize(repulsoCatNames)},   // Repulso
        {attractoCatNames, zb::getArraySize(attractoCatNames)}, // Attracto
        {copyCatNames, zb::getArraySize(copyCatNames)},         // Copy
        {duckCatNames, zb::getArraySize(duckCatNames)},         // Duck
    });


////////////////////////////////////////////////////////////
[[nodiscard]] zb::Vector<zb::StringView> getShuffledCatNames(CatType catType, RNGFast& rng);
