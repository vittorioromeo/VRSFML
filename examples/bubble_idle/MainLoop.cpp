

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "PlayerInput.hpp"
#include "Serialization.hpp"

#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/Profiler.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"

namespace
{
struct GameLoopFrameState
{
    bool             shouldDrawUI{false};
    sf::base::U8     shouldDrawUIAlpha{0u};
    FrameInput       input;
    FrameUpdateState update;
    sf::Time         deltaTime;
    float            deltaTimeMs{0.f};
};

} // namespace

////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::gameLoop()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (mustExit)
        return false;

#ifdef BUBBLEBYTE_USE_STEAMWORKS
    steamMgr.runCallbacks();
#endif

    GameLoopFrameState frame;
    frame.shouldDrawUI      = !inPrestigeTransition && splashCountdown.value <= 0.f;
    frame.shouldDrawUIAlpha = inPrestigeTransition || splashCountdown.getProgress() < 0.75f
                                  ? static_cast<sf::base::U8>(0u)
                                  : static_cast<sf::base::U8>(
                                        remap(easeInOutSine(splashCountdown.getProgress()), 0.75f, 1.f, 0.f, 255.f));

    fps = 1.f / fpsClock.getElapsedTime().asSeconds();
    fpsClock.restart();

    inputHelper.beginNewFrame();
    if (!gameLoopHandleEvents(frame.input, frame.shouldDrawUI))
        return false;

    frame.deltaTime   = deltaClock.restart();
    frame.deltaTimeMs = sf::base::min(24.f, static_cast<float>(frame.deltaTime.asMicroseconds()) / 1000.f);
    shaderTime += frame.deltaTimeMs * 0.001f;

    gameLoopPrepareInput(frame.input, frame.deltaTimeMs);
    gameLoopUpdateFrameWorld(frame.deltaTimeMs, frame.input, frame.update);
    gameLoopUpdateFrameUi(frame.deltaTime, frame.deltaTimeMs, frame.input);

    const FrameViewState views = gameLoopComputeViews();
    gameLoopRenderFrame(frame.deltaTimeMs, frame.shouldDrawUI, frame.shouldDrawUIAlpha, frame.input, frame.update, views);
    gameLoopPresentFrame(views);

    playerInputState.lastMousePos = frame.input.mousePos;
    return true;
}


////////////////////////////////////////////////////////////
void Main::loadPlaythroughFromFileAndReseed()
{
    const sf::base::StringView loadMessage = loadPlaythroughFromFile(ptMain, "userdata/playthrough.json");

    if (!loadMessage.empty())
        pushNotification("Playthrough loading info", "%s", loadMessage.data());

    reseedRNGs(pt->seed);
    shuffledCatNamesPerType = makeShuffledCatNames(rng);
}
