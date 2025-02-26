#pragma once

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
struct AchievementData
{
    const char* name;
    const char* description;
    bool        secret = false;
};

////////////////////////////////////////////////////////////
inline constexpr AchievementData achievementData[] = {
    {"Initiation Pop", "Pop your first bubble by hand."},
    {"Finger Fitness", "Pop 10 bubbles by hand."},
    {"Clicker Novice", "Pop 100 bubbles by hand."},
    {"Manual Labor", "Pop 1,000 bubbles by hand."},
    {"Carpal Tunnel Candidate", "Pop 10,000 bubbles by hand."},
    {"Popaholic", "Pop 100,000 bubbles by hand."},
    {"Finger of God", "Pop 1,000,000 bubbles by hand."},

    {"Paw Patrol", "Pop your first bubble via cats."},
    {"Lazy Overlord", "Pop 100 bubbles via cats."},
    {"Cat-alyst", "Pop 1,000 bubbles via cats."},
    {"Claw Enforcement", "Pop 10,000 bubbles via cats."},
    {"Meowtomation", "Pop 100,000 bubbles via cats."},
    {"Feline Workforce", "Pop 1,000,000 bubbles via cats."},
    {"Rube Goldberg Meowchine", "Pop 10,000,000 bubbles via cats."},
    {"Don't Need to Click Anymore", "Pop 100,000,000 bubbles via cats."},

    {"Combo Curious", "Purchase the \"Combo\" upgrade."},

    {"I Can Go Higher", "Purchase 5 upgrades of \"Longer Combos\"."},
    {"I Must Go Even Higher", "Purchase 10 upgrades of \"Longer Combos\"."},
    {"I Want Big Numbers", "Purchase 15 upgrades of \"Longer Combos\"."},
    {"I Need Huge Numbers", "Purchase 20 upgrades of \"Longer Combos\"."},

    {"What's Over There?", "Purchase the \"Map Scrolling\" upgrade."},
    {"Horizon Hoarder", "Extend the map up to 40%."},
    {"Border-line Addict", "Extend the map up to 60%."},
    {"Map Magnate", "Extend the map up to 80%."},
    {"SpyCat Satellite", "Extend the map up to 100%."},

    {"Keeping Kids Entertained", "Purchase the \"More Bubbles\" upgrade once."},
    {"The Screen Isn't Full Yet", "Purchase the \"More Bubbles\" upgrade 5 times."},
    {"There Still Are Some Gaps", "Purchase the \"More Bubbles\" upgrade 10 times."},
    {"Oh God, Maybe It's Too Much", "Purchase the \"More Bubbles\" upgrade 20 times."},
    {"It's All Bubbles. Always been.", "Purchase the \"More Bubbles\" upgrade 30 times."},

    {"Cat Person", "Purchase your first cat."},
    {"Cat Enthusiast", "Purchase 5 cats."},
    {"Crazy Cat Person", "Purchase 10 cats."},
    {"Cat Café Owner", "Purchase 20 cats."},
    {"Cat Hoarder", "Purchase 30 cats."},
    {"I Live to Serve Cats", "Purchase 40 cats."},

    {"Speedy Paws", "Improve cat cooldown once."},
    {"Pawmaxxing", "Improve cat cooldown 3 times."},
    {"Lightning Paws", "Improve cat cooldown 6 times."},
    {"How Can She Slap?", "Improve cat cooldown 9 times."},
    {"ORA ORA ORA ORA ORA", "Improve cat cooldown 12 times."},

    {"Pupil Dilation", "Improve cat range once."},
    {"Panoramic Paws", "Improve cat range 3 times."},
    {"Omnipawtent", "Improve cat range 6 times."},
    {"Gomu Gomu No Meow", "Improve cat range 9 times."},

    {"Taste The Rainbow", "Purchase your first Unicat."},
    {"Sparkle Squad", "Purchase 5 Unicats."},
    {"Glitter Gang", "Purchase 10 Unicats."},
    {"Celestial Herd", "Purchase 20 Unicats."},
    {"Galactic Uniclan", "Purchase 30 Unicats."},
    {"I'm On The Color Spectrum", "Purchase 40 Unicats."},

    {"Fireworks!", "Improve Unicat cooldown once."},
    {"Starlight Surge", "Improve Unicat cooldown 3 times."},
    {"Galactic Gait", "Improve Unicat cooldown 6 times."},
    {"Need More Shinies", "Improve Unicat cooldown 9 times."},
    {"Double-Slit Experiment", "Improve Unicat cooldown 12 times."},

    {"Lunar Leash", "Improve Unicat range once."},
    {"Solr Spana", "Improve Unicat range 3 times."},
    {"Constellation Coverage", "Improve Unicat range 6 times."},
    {"Celestial Dominion", "Improve Unicat range 9 times."},

    // TODO P0: unicat transcension

    {"Chaos Catalyst", "Purchase your first Devilcat."},
    {"Demonic Delegation", "Purchase 5 Devilcats."},
    {"Infernal Army", "Purchase 10 Devilcats."},
    {"Hell's Managers", "Purchase 20 Devilcats."},
    {"Apocalypse Now", "Purchase 30 Devilcats."},
    {"Satan's HR Department", " Purchase 40 Devilcats."},

    {"Strategic Bombing", "Improve Devilcat cooldown once."},
    {"Infernal Impulse", "Improve Devilcat cooldown 3 times."},
    {"Hellfire Refresh", "Improve Devilcat cooldown 6 times."},
    {"Carpet Bombing", "Improve Devilcat cooldown 9 times."},
    {"WOPR", "Improve Devilcat cooldown 12 times."},

    // {"Infernal Reach", "Improve Devilcat range once."},
    // {"Hellish Horizon", "Improve Devilcat range 3 times."},
    // {"Satanic Span", "Improve Devilcat range 6 times."},
    // {"Abyssal Ambition", "Improve Devilcat range 9 times."},

    {"Boom", "Improve Devilcat explosion radius once."},
    {"Boomer", "Improve Devilcat explosion radius 5 times."},
    {"Boomest", "Improve Devilcat explosion radius 10 times."},

    {"I Am A Leaf On The Wind", "Purchase your first Astrocat."},
    {"Engage", "Purchase 5 Astrocats."},
    {"Failure Is Not an Option", "Purchase 10 Astrocats."},
    {"Trans-Dimensional Council of Ricks", "Purchase 20 Astrocats."},
    {"DOOP", "Purchase 30 Astrocats."},
    {"Trisolaran Fleet", "Purchase 40 Astrocats."},

    {"Faster Than A Cheetah", "Improve Astrocat cooldown once."},
    {"Speed Of Sound", "Improve Astrocat cooldown 3 times."},
    {"Lightspeed", "Improve Astrocat cooldown 6 times."},
    {"Warp Drive", "Improve Astrocat cooldown 9 times."},
    {"Ramming Droplet", "Improve Astrocat cooldown 12 times."},

    {"Extremely Large Telescope", "Improve Astrocat range once."},
    {"Aperture Spherical Telescope", "Improve Astrocat range 3 times."},
    {"Hubble Space Telescope", "Improve Astrocat range 6 times."},
    {"Webb Space Telescope", "Improve Astrocat range 9 times."},

    {"Prestigious", "Prestige once."},
    {"Groundhog Day", "Prestige 2 times."},
    {"Triangle", "Prestige 3 times."},
    {"Edge of Tomeowrrow", "Prestige 5 times."},
    {"The Cat Who Leapt Through Time", "Prestige 10 times."},
    {"If I Had To Do It All Over Again", "Prestige 15 times."},
    {"Whatever It Took To Get The JoJ", "Prestige 20 times."},

    {"Locked'n'Loaded", "Purchase the \"Starter Pack\" permanent upgrade."},

    {"Pop Goes the Weasel", "Purchase the \"Multipop\" permanent upgrade."},
    {"Popcorn", "Improve multipop range once."},
    {"Area of Effect", "Improve multipop range 2 times."},
    {"More Pops, More Pops", "Improve multipop range 5 times."},
    {"Ok, Stop The Pops", "Improve multipop range 10 times."},

    {"Wind Beneath My Paws", "Purchase the \"Giant Fan\" permanent upgrade."},

    {"Stephen Meowking", "Purchase the \"Smart Cats\" permanent upgrade."},
    {"H2OGFat", "Purchase the \"Genius Cats\" permanent upgrade."},

    {"Interstellar Brainwashing", "Purchase the \"Space Propaganda\" permanent upgrade."},
    {"For The Cause", "Improve inspire duration once."},
    {"Bubblophobic", "Improve inspire duration 4 times."},
    {"Cationalist", "Improve inspire duration 8 times."},
    {"Radicatlized", "Improve inspire duration 12 times."},
    {"Fur Supremacy", "Improve inspire duration 16 times."},

    {"Claw Grip", "Reach x5 combo multiplier."},
    {"Aim Training", "Reach x10 combo multiplier."},
    {"You Likely Wear A Gaming Sleeve", "Reach x15 combo multiplier."},
    {"Voltaic Clicking", "Reach x20 combo multiplier."},
    {"Don't Need Kovaak's Anymore", "Reach x25 combo multiplier."},

    {"Hydrogen Bomb", "Pop a star bubble with at least x5 combo multiplier."},
    {"Tunguska Meteorite", "Pop a star bubble with at least x10 combo multiplier."},
    {"B41", "Pop a star bubble with at least x15 combo multiplier."},
    {"Tsar Bomba", "Pop a star bubble with at least x20 combo multiplier."},
    {"Anti-Matter Bomb", "Pop a star bubble with at least x25 combo multiplier."},

    {"Starry-Eyed", "Pop your first star bubble by hand."},
    {"Star Collector", "Pop 100 star bubbles by hand."},
    {"Stellar Hoarder", "Pop 1,000 star bubbles by hand."},
    {"Galactic Tycoon", "Pop 10,000 star bubbles by hand."},
    {"Supernova Supplier", "Pop 100,000 star bubbles by hand."},

    {"Paw-laris", "Pop your first star bubble via cats."},
    {"Star Boop", "Pop 100 star bubbles via cats."},
    {"Constellation Slap", "Pop 1,000 star bubbles via cats."},
    {"Galactic Scrub", "Pop 10,000 star bubbles via cats."},
    {"Stellar Cleanse", "Pop 100,000 star bubbles via cats."},
    {"Celestial Clean-Up", "Pop 1,000,000 star bubbles via cats."},
    {"Spotless Night Sky", "Pop 10,000,000 star bubbles via cats."},

    {"K-T Extinction Event", "Pop a nova bubble with at least x5 combo multiplier."},
    {"Shoemaker-Levy 9", "Pop a nova bubble with at least x10 combo multiplier."},
    {"Supernova", "Pop a nova bubble with at least x15 combo multiplier."},
    {"Gamma Ray Burst", "Pop a nova bubble with at least x20 combo multiplier."},
    {"Big Bang", "Pop a nova bubble with at least x25 combo multiplier."},

    {"Transcendental Click", "Pop your first nova bubble by hand."},
    {"Click The Otherwordly", "Pop 100 nova bubbles by hand."},
    {"Ethereal Clicking", "Pop 1,000 nova bubbles by hand."},
    {"Unearthly LMB", "Pop 10,000 nova bubbles by hand."},
    {"Metaphysical Mouse", "Pop 100,000 nova bubbles by hand."},

    {"Transcendental Pop", "Pop your first nova bubble via cats."},
    {"Otherwordly Swipes", "Pop 100 nova bubbles via cats."},
    {"Ethereal Cat Slaps", "Pop 1,000 nova bubbles via cats."},
    {"Unearthly Boops", "Pop 10,000 nova bubbles via cats."},
    {"Metaphysical Claws", "Pop 100,000 nova bubbles via cats."},
    {"Paranormal Taps", "Pop 1,000,000 nova bubbles via cats."},
    {"Inconceivable Swats", "Pop 10,000,000 nova bubbles via cats."},

    {"Hand Grenade", "Pop your first bomb by hand."},
    {"Explosive Management", "Pop 100 bombs by hand."},
    {"Dynamite Digits", "Pop 1,000 bombs by hand."},
    {"Manual Detonator", "Pop 10,000 bombs by hand."},

    {"Bomb Voyage", "Pop your first bomb via cats."},
    {"Clawed Demolition", "Pop 100 bombs via cats."},
    {"Feline Firestorm", "Pop 1,000 bombs via cats."},
    {"Purr-cussion Experts", "Pop 10,000 bombs via cats."},
    {"Meow-ton Blast", "Pop 100,000 bombs via cats."},

    {"Astro-Boom", "Have an Astrocat pop a bomb."},

    {"Team Spirit", "Have an Astrocat inspire a cat."},
    {"Celestial Synergy", "Have an Astrocat inspire a Unicat."},
    {"Hell's Ambassador", "Have an Astrocat inspire a Devilcat."},
    {"Bone Propaganda", "Have an Astrocat inspire the Witchcat."},
    {"Mewsterful Mentor", "Have an Astrocat inspire the Wizardcat."},
    {"Mini Mewtivation", "Have an Astrocat inspire the Mousecat."},
    {"Ingeni-cat Instigator", "Have an Astrocat inspire the Engicat."},
    {"Purrfect Pushback", "Have an Astrocat inspire the Repulsocat."},
    {"Attracting Ad", "Have an Astrocat inspire the Attractocat."},

    {"Paws of Awakening", "Activate the 1st shrine."},
    {"Twin Paws, Double Power", "Activate the 2nd shrine."},
    {"Three's a Purr-ty", "Activate the 3th shrine."},
    {"Fur-midable Foursome", "Activate the 4th shrine."},
    {"Fifth Purr-adox", "Activate the 5th shrine."},
    {"Six-Pawed Spectacle", "Activate the 6th shrine."},
    {"Lucky Seven Lives", "Activate the 7th shrine."},
    {"Octo-Paws Overture", "Activate the 8th shrine."},
    {"Nine Lives Activated", "Activate the 9th shrine."},

    {"Cat's Foot", "Complete the 1st shrine (\"Shrine of Voodoo\")."},
    {"Abraca-paw-dabra", "Complete the 2nd shrine (\"Shrine of Magic\")."},
    {"Purrfectly Clicked", "Complete the 3rd shrine (\"Shrine of Clicking\")."},
    {"Auto-Mew-tation", "Complete the 4th shrine (\"Shrine of Automation\")."},
    {"Repulsolution", "Complete the 5th shrine (\"Shrine of Repulsion\")."},
    {"Attract-a-Paw", "Complete the 6th shrine (\"Shrine of Attraction\")."},
    {"TODONAME", "Complete the 7th shrine (\"Shrine of TODO\")."},
    {"TODONAME", "Complete the 8th shrine (\"Shrine of TODO\")."},
    {"It's Finally Over", "Complete the 9th shrine (\"Shrine of Victory\")."},

    {"Unsealed. Moisturized. Happy.", "Unseal the Witchcat."},
    {"I Don't Recall Being in There", "Unseal the Wizardcat."},
    {"Clicked my Way Out", "Unseal the Mousecat."},
    {"Shrine Lid Unscrewed", "Unseal the Engicat."},
    {"There's No Ceiling", "Unseal the Repulsocat."},
    {"Electronic Lock", "Unseal the Attractocat."},

    {"No Pops Allowed", "Perform a voodoo ritual on a cat."},
    {"I Hate Rainbows", "Perform a voodoo ritual on a Unicat."},
    {"Demilitarization", "Perform a voodoo ritual on a Devilcat."},
    {"Out of Fuel", "Perform a voodoo ritual on an Astrocat."},
    {"Go to Bed, Gramps", "Perform a voodoo ritual on the Wizardcat."},
    {"Gamer Rage", "Perform a voodoo ritual on the Mousecat."},
    {"Hex Tech", "Perform a voodoo ritual on the Engicat."},
    {"Not a Fan", "Perform a voodoo ritual on the Repulsocat."},
    {"Demagnetized", "Perform a voodoo ritual on the Attractocat."},

    {"Black cat bone", "Perform 500 voodoo rituals on a cat."},
    {"Unibones", "Perform 100 voodoo rituals on a Unicat."},
    {"Hellbones", "Perform 100 voodoo rituals on a Devilcat."},
    {"Bone Voyage", "Perform 50 voodoo rituals on an Astrocat."},
    {"Old Bones", "Perform 10 voodoo rituals on a Wizardcat."},
    {"Bones And Circuits", "Perform 10 voodoo rituals on a Mousecat."},
    {"Nuts And Bones", "Perform 10 voodoo rituals on an Engicat."},
    {"Bones, Come Back!", "Perform 10 voodoo rituals on a Repulsocat."},
    {"Ferromagnetic Bones", "Perform 10 voodoo rituals on an Attractocat."},

    {"TODONAME", "Collect a voodoo doll."},
    {"TODONAME", "Collect 10 voodoo dolls."},
    {"TODONAME", "Collect 100 voodoo dolls."},
    {"TODONAME", "Collect 1,000 voodoo dolls."},
    {"TODONAME", "Collect 10,000 voodoo dolls."},

    {"TODONAME", "Improve Witchcat buff duration once."},
    {"TODONAME", "Improve Witchcat buff duration 3 times."},
    {"TODONAME", "Improve Witchcat buff duration 6 times."},
    {"TODONAME", "Improve Witchcat buff duration 9 times."},
    {"TODONAME", "Improve Witchcat buff duration 12 times."},

    {"Spiritual Clowder", "Purchase the \"Group Ritual\" permanent upgrade."},
    {"Cult Classic", "Purchase the \"Worldwide Cult\" permanent upgrade."},
    {"Doll Inflation", "Purchase the \"Material Shortage\" permanent upgrade."},
    {"Boomdoo", "Purchase the \"Flammable Dolls\" permanent upgrade."},
    {"Space Junk Cleanup", "Purchase the \"Orbital Dolls\" permanent upgrade."},

    {"Double Mojo", "Have two Witchcat ritual buffs active at once."},
    {"Triple Mojo", "Have three Witchcat ritual buffs active at once."},
    {"Quadra Mojo", "Have four Witchcat ritual buffs active at once."},

    {"Broomstick Boost", "Improve Witchcat cooldown once."},
    {"Triple Hocus Haste", "Improve Witchcat cooldown 3 times."},
    {"Hexed Haste", "Improve Witchcat cooldown 6 times."},
    {"Nimble Necromancy", "Improve Witchcat cooldown 9 times."},
    {"Witching Whirlwind", "Improve Witchcat cooldown 12 times."},

    {"Voodoo Vista", "Improve Witchcat range once."},
    {"Witchcat's Wide Wards", "Improve Witchcat range 3 times."},
    {"Bone Meridian", "Improve Witchcat range 6 times."},
    {"Spiritual Frontier", "Improve Witchcat range 9 times."},

    {"Cosmic Cat-sophy", "Absorb wisdom from a star bubble."},
    {"Centipaws of Knowledge", "Absorb wisdom from 100 star bubbles."},
    {"Kilo Claws of Wisdom", "Absorb wisdom from 1,000 star bubbles."},
    {"Ten Thousand Tails of Wisdom", "Absorb wisdom from 10,000 star bubbles."},
    {"Whisker Wisdom Overload", "Absorb wisdom from 100,000 star bubbles."},

    {"Mewmorable Magic", "Remember Wizardcat's first spell."},
    {"Double Dose of Dazzle", "Remember Wizardcat's second spell."},
    {"Triple Treat of Transcendence", "Remember Wizardcat's third spell."},
    {"Quartet of Quirky Conjuring", "Remember Wizardcat's fourth spell."},

    {"Starpaw Spark", "Cast \"Starpaw Conversion\" once."},
    {"Deca-Starpaw Dynamo", "Cast \"Starpaw Conversion\" 10 times."},
    {"Centipaw Catastrophe", "Cast \"Starpaw Conversion\" 100 times."},
    {"Kilo Starpaw Chaos", "Cast \"Starpaw Conversion\" 1,000 times."},

    {"Mewltiplier Moment", "Cast \"Mewltiplier Aura\" once."},
    {"Deca-Mewltiplier Dynamo", "Cast \"Mewltiplier Aura\" 10 times."},
    {"Centimewltiplier Catastrophe", "Cast \"Mewltiplier Aura\" 100 times."},
    {"Kilo Mewltiplier Chaos", "Cast \"Mewltiplier Aura\" 1,000 times."},

    {"TODONAME", "Cast \"TODOSPELL #2\" once."},
    {"TODONAME", "Cast \"TODOSPELL #2\" 10 times."},
    {"TODONAME", "Cast \"TODOSPELL #2\" 100 times."},
    {"TODONAME", "Cast \"TODOSPELL #2\" 1,000 times."},

    {"TODONAME", "Cast \"TODOSPELL #3\" once."},
    {"TODONAME", "Cast \"TODOSPELL #3\" 10 times."},
    {"TODONAME", "Cast \"TODOSPELL #3\" 100 times."},
    {"TODONAME", "Cast \"TODOSPELL #3\" 1,000 times."},

    // TODO P0: wizardcat wisdom upgrade achievmenets

    {"Pawsitively Wise", "Improve Wizardcat cooldown once."},
    {"Triple-Wise Tabbies", "Improve Wizardcat cooldown 3 times."},
    {"Sixth Sense of Speedy Wisdom", "Improve Wizardcat cooldown 6 times."},
    {"Nine-Fold Wisdom Nimbleness", "Improve Wizardcat cooldown 9 times."},
    {"2Wise2Furious", "Improve Wizardcat cooldown 12 times."},

    {"Far-Out Wisdom", "Improve Wizardcat range once."},
    {"Telescopic Wisdom", "Improve Wizardcat range 3 times."},
    {"Planetary Wisdom", "Improve Wizardcat range 6 times."},
    {"Universal Wisdom", "Improve Wizardcat range 9 times."},

    {"TODONAME", "Improve Wizardcat's mana cooldown once"},
    {"TODONAME", "Improve Wizardcat's mana cooldown 4 times"},
    {"TODONAME", "Improve Wizardcat's mana cooldown 8 times"},
    {"TODONAME", "Improve Wizardcat's mana cooldown 12 times"},
    {"TODONAME", "Improve Wizardcat's mana cooldown 16 times"},

    {"TODONAME", "Improve Wizardcat's mana limit once"},
    {"TODONAME", "Improve Wizardcat's mana limit 4 times"},
    {"TODONAME", "Improve Wizardcat's mana limit 8 times"},
    {"TODONAME", "Improve Wizardcat's mana limit 12 times"},
    {"TODONAME", "Improve Wizardcat's mana limit 16 times"},
    {"TODONAME", "Improve Wizardcat's mana limit 20 times"},

    {"Everyone But You", "Purchase the \"Selective Starpaw\" permanent upgrade."},
    {"It Takes Longer To Say It", "Purchase the \"Meeeeeewltiplier\" permanent upgrade."},

    // TODO P0: other wizardcat prestiges for other spells

    {"Modest Autoclicker", "Reach combo x25 with Mousecat."},
    {"Midway Autoclicker", "Reach combo x50 with Mousecat."},
    {"Majestic Autoclicker", "Reach combo x75 with Mousecat."},
    {"Mighty Autoclicker", "Reach combo x100 with Mousecat."},
    {"Monumental Autoclicker", "Reach combo x125 with Mousecat."},
    {"Mammoth Autoclicker", "Reach combo x150 with Mousecat."},
    {"Mythic Autoclicker", "Reach combo x175 with Mousecat."},
    {"Hello, Mouse? Mouse Broke", "Reach combo x999 with Mousecat."},

    {"Speedy Squeak", "Improve Mousecat cooldown once."},
    {"Triple Squeak Sprint", "Improve Mousecat cooldown 3 times."},
    {"Hexa-Hasty Squeaker", "Improve Mousecat cooldown 6 times."},
    {"Nine-Nimble Nibbler", "Improve Mousecat cooldown 9 times."},
    {"Dozen Dash of Squeak", "Improve Mousecat cooldown 12 times."},

    {"Mini Mewsight", "Improve Mousecat range once."},
    {"Triple Mewsion", "Improve Mousecat range 3 times."},
    {"Six-Star Squeakview", "Improve Mousecat range 6 times."},
    {"Maximum Mousecat Mews", "Improve Mousecat range 9 times."},

    {"Serial Port Mouse", "Improve Mousecat buff multiplier once."},
    {"PS/2 Mouse", "Improve Mousecat buff multiplier twice."},
    {"USB Mouse", "Improve Mousecat buff multiplier 6 times."},
    {"Bluetooth Mouse", "Improve Mousecat buff multiplier 10 times."},
    {"Trackball...?", "Improve Mousecat buff multiplier 14 times."},

    {"Purrfect Tune-Up", "Perform maintenance on cats once."},
    {"Deca-Domestic Detailing", "Perform maintenance on cats 10 times."},
    {"Century of Cat Care", "Perform maintenance on cats 100 times."},
    {"Thousand-Tailed Tending", "Perform maintenance on cats 1,000 times."},
    {"Ten-Thousand Tidy Tabbies", "Perform maintenance on cats 10,000 times."},
    {"Hundred-Kitten Cleanup", "Perform maintenance on cats 100,000 times."},
    {"Megapurr Maintenance", "Perform maintenance on cats 1,000,000 times."},

    {"Triple Cat Care Coordination", "Perform maintenance on 3 cats simultaneously."},
    {"Sixfold Feline Fix-Up", "Perform maintenance on 6 cats simultaneously."},
    {"Nine-Cat Nurturing", "Perform maintenance on 9 cats simultaneously."},
    {"Dozen Dapper Detailing", "Perform maintenance on 12 cats simultaneously."},
    {"Fifteen Furry Facelift", "Perform maintenance on 15 cats simultaneously."},

    {"Engineered for Speed", "Improve Engicat cooldown once."},
    {"Triple Turbo Tinkering", "Improve Engicat cooldown 3 times."},
    {"Six-Speed Servicing", "Improve Engicat cooldown 6 times."},
    {"Ninefold Nitro", "Improve Engicat cooldown 9 times."},
    {"Dozen-Driven Dash", "Improve Engicat cooldown 12 times."},

    {"Engineer's Extended Gaze", "Improve Engicat range once."},
    {"Triple-Volt Vision", "Improve Engicat range 3 times."},
    {"Hexa-Hued Horizon", "Improve Engicat range 6 times."},
    {"Nonagon Navigator", "Improve Engicat range 9 times."},

    {"TODONAME", "Improve Engicat buff multiplier once."},
    {"TODONAME", "Improve Engicat buff multiplier twice."},
    {"TODONAME", "Improve Engicat buff multiplier 6 times."},
    {"TODONAME", "Improve Engicat buff multiplier 10 times."},
    {"TODONAME", "Improve Engicat buff multiplier 14 times."},

    {"Rules? Meow off!", "Pop 300 bubbles without ever purchasing anything.", /* secret */ true},
};

////////////////////////////////////////////////////////////
enum : sf::base::SizeT
{
    nAchievements = sf::base::getArraySize(achievementData)
};

// TODO P1:
// - add achievements for every special cat
// - fill in missing names
// - link to Steam
// - come up with secret achivements
// - speedrun/milestone achievements
// ---- prestige 1 mid 5min, good 4min, insane 3.45min
// ---- prestige 2 mid 25min, good 22min, insane 18min
// ---- prestige 3? maybe test with auto combos
