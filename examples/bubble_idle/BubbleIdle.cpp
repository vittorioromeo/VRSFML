#include "BubbleIdleMain.hpp"

#include "SFML/Base/Builtins/Strcmp.hpp"
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    #include "Steam.hpp"
#endif


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main(int argc, const char** argv)
{
    if (argc >= 2 && SFML_BASE_STRCMP(argv[1], "dev") == 0)
        debugMode = true;

#ifdef BUBBLEBYTE_USE_STEAMWORKS
    hg::Steam::SteamManager steamMgr;
    steamMgr.requestStatsAndAchievements();
    steamMgr.runCallbacks();

    // Using a heap-allocation here because `Main` exceeds the stack size
    sf::base::makeUnique<Main>(steamMgr)->run();
#else
    sf::base::makeUnique<Main>()->run();
#endif
}

// TODO P1: review all tooltips
// TODO P1: instead of new BGMs, attracto/repulso could unlock speed/pitch shifting for BGMs
// TODO P1: tooltips for options, reorganize them
// TODO P1: credits somewhere
// TODO P1: more steam deck improvements
// TODO P1: drag click PP upgrade, stacks with multipop
// TODO P1: demo & sponsor on playmygame with ss

// TODO P2: idea for PP: when astrocat touches hellcat portal its buffed
// TODO P2: rested buff 1PP: 1.25x mult, enables after Xs of inactivity, can be upgraded with PPs naybe?
// TODO P2: configurable particle spawn chance
// TODO P2: configurable pop sound play chance
// TODO P2: maybe 64PP prestige buff for multipop that allows "misses"
// TODO P2: reduce size of game textures and try to reduce atlas size
// TODO P2: some sort of beacon cat that is only effective when near n-m cats but no less nor more
// TODO P2: decorations for unique cats (e.g. wizard cape, witch?, engi tesla coil,  ?)
// TODO P2: upgrade for ~512PPs "brain takes over" that turns cats into brains with 50x mult with corrupted zalgo names
// TODO P2: pp upgrade around 128pp that makes manually clicked bombs worth 100x (or maybe all bubbles)
// TODO P2: challenge runs, separate saves
