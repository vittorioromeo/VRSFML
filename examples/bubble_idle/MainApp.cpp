

#include "BubbleIdleApp.hpp"
#include "BubbleIdleMain.hpp"
#include "Steam.hpp"

#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"

////////////////////////////////////////////////////////////
bool debugMode = false;

////////////////////////////////////////////////////////////
void runBubbleIdleApp()
{
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


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
Main::Main(hg::Steam::SteamManager& xSteamMgr) : steamMgr(xSteamMgr), onSteamDeck(steamMgr.isOnSteamDeck())
#else
Main::Main() : onSteamDeck(false)
#endif
{
    sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

    if (onSteamDeck)
    {
        // borderless windowed
        profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
        profile.windowed = true;

        profile.uiScale = 1.25f;
    }

    //
    // Playthrough
    if (sf::Path{"userdata/playthrough.json"}.exists())
    {
        loadPlaythroughFromFileAndReseed();
        sf::cOut() << "Loaded playthrough from file on startup\n";
    }
    else
    {
        pt->seed = seed;
    }

    //
    // Reserve memory
    particles.reserve(512);
    spentCoinParticles.reserve(512);
    textParticles.reserve(256);
    pt->bubbles.reserve(32'768);
    pt->cats.reserve(512);

    //
    // Touch state
    fingerPositions.resize(10);
}

////////////////////////////////////////////////////////////
void Main::run()
{
    //
    // Startup (splash screen and meow)
    splashCountdown.restart();
    playSound(sounds.byteMeow);

    //
    //
    // Background music
    auto& [entries, selectedIndex] = getBGMSelectorData();
    selectBGM(entries, selectedIndex);
    switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ true);

    //
    // Game loop
    playedClock.start();

    while (true)
        if (!gameLoop())
            return;
}
