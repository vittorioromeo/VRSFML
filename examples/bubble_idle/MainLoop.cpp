

#include "BubbleIdleMain.hpp"
#include "ExampleProfiler/Profiler.hpp"
#include "FrameViewState.hpp"
#include "InputHelper.hpp"
#include "PlayerInput.hpp"
#include "Playthrough.hpp"
#include "Serialization.hpp"
#include "Zancle/System/Time.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/MinMax.hpp"

#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/MathUtils.hpp"

namespace
{
struct GameLoopFrameState
{
    bool             shouldDrawUI{false};
    zb::U8           shouldDrawUIAlpha{0u};
    FrameInput       input;
    FrameUpdateState update;
    za::Time         deltaTime;
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
    frame.shouldDrawUI = !inPrestigeTransition && splashCountdown.time <= 0.f;
    frame.shouldDrawUIAlpha = inPrestigeTransition || splashCountdown.asProgress().getElapsed() < 0.75f
                                  ? static_cast<zb::U8>(0u)
                                  : static_cast<zb::U8>(
                                        remap(easeInOutSine(splashCountdown.asProgress().getElapsed()), 0.75f, 1.f, 0.f, 255.f));

    fps = 1.f / fpsClock.getElapsedTime().asSeconds();
    fpsClock.restart();

    inputHelper.beginNewFrame();
    if (!gameLoopHandleEvents(frame.input, frame.shouldDrawUI))
        return false;

    frame.deltaTime   = deltaClock.restart();
    frame.deltaTimeMs = zb::min(24.f, static_cast<float>(frame.deltaTime.asMicroseconds()) / 1000.f);
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
    const zb::StringView loadMessage = loadPlaythroughFromFile(ptMain, "userdata/playthrough.json");

    if (!loadMessage.empty())
        pushNotification("Playthrough loading info", "%s", loadMessage.data());

    reseedRNGs(pt->seed);
    shuffledCatNamesPerType = makeShuffledCatNames(rng);
}
