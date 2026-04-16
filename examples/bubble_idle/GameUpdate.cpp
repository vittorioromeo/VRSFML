#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "Countdown.hpp"
#include "PlayerInput.hpp"

#include "ExampleUtils/ProfilerImGui.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Clamp.hpp"

////////////////////////////////////////////////////////////
void Main::gameLoopUpdateFrameWorld(const float realDeltaTimeMs, FrameInput& frameInput, FrameUpdateState& frameUpdate)
{
    const float deltaTimeMs = debugMode ? realDeltaTimeMs * debugTimeScale : realDeltaTimeMs;

    gameLoopUpdateTransitions(deltaTimeMs);

    sweepAndPrune.populate(pt->bubbles);

    frameProcThisFrame = (frameProcCd.updateAndLoop(deltaTimeMs, 20.f) == CountdownStatusLoop::Looping);

    gameLoopUpdateBubbles(deltaTimeMs);
    gameLoopUpdateAttractoBuff(deltaTimeMs);

    const bool anyBubblePoppedByClicking = gameLoopUpdateBubbleClick(frameInput.clickPosition);
    frameUpdate.cursorGrow               = gameLoopUpdateCursorGrowthEffect(deltaTimeMs, anyBubblePoppedByClicking);
    gameLoopUpdateCombo(deltaTimeMs, anyBubblePoppedByClicking, frameInput.mousePos, frameInput.clickPosition);

    gameLoopUpdateCollisionsBubbleBubble(deltaTimeMs);
    gameLoopUpdateCollisionsCatCat(deltaTimeMs);
    gameLoopUpdateCollisionsCatShrine(deltaTimeMs);
    gameLoopUpdateCollisionsCatDoll();
    gameLoopUpdateCollisionsBubbleHellPortal();

    for (Cat& cat : pt->cats)
    {
        constexpr float maxDragTime  = 1000.f;
        constexpr float dragInSpeed  = 1.f;
        constexpr float dragOutSpeed = 2.5f;

        const float dragDelta = isCatBeingDragged(cat) ? deltaTimeMs * dragInSpeed : -deltaTimeMs * dragOutSpeed;
        cat.dragTime          = sf::base::clamp(cat.dragTime + dragDelta, 0.f, maxDragTime);
    }

    gameLoopUpdateCatDragging(deltaTimeMs, frameInput.downFingers.size(), frameInput.mousePos);
    gameLoopUpdateCatActions(deltaTimeMs);
    gameLoopUpdateShrines(deltaTimeMs);
    gameLoopUpdateDolls(deltaTimeMs, frameInput.mousePos);
    gameLoopUpdateCopyDolls(deltaTimeMs, frameInput.mousePos);
    gameLoopUpdateHellPortals(deltaTimeMs);
    gameLoopUpdateWitchBuffs(deltaTimeMs);
    gameLoopUpdateEvents(deltaTimeMs);
    gameLoopUpdateNapScheduler(deltaTimeMs);
    gameLoopUpdateMana(deltaTimeMs);
    gameLoopUpdateAutocast();

    for (auto& [delayCountdown, func] : delayedActions)
        if (delayCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            func();

    sf::base::vectorEraseIf(delayedActions, [](const auto& delayedAction) {
        return delayedAction.delayCountdown.isDone();
    });

    // Reap ephemeral bubbles (e.g. from Bubblefall) that fell off the bottom
    // this frame. Must run after all bubble collision passes so that the
    // indices captured by `sweepAndPrune.populate` stay valid.
    gameLoopReapEphemeralBubbles();
    gameLoopUpdateComboBubblePayouts(deltaTimeMs);

    gameLoopUpdateScreenShake(deltaTimeMs);
    gameLoopUpdateParticlesAndTextParticles(deltaTimeMs);
    gameLoopUpdateSounds(deltaTimeMs, frameInput.mousePos);

    frameUpdate.elapsedUs = playedClock.getElapsedTime().asMicroseconds();
    playedClock.restart();

    gameLoopUpdateTimePlayed(frameUpdate.elapsedUs);
    gameLoopUpdateAutosave(frameUpdate.elapsedUs);
    gameLoopUpdateMilestones();
    gameLoopUpdateAchievements();
    gameLoopUpdateSplits();
}

////////////////////////////////////////////////////////////
void Main::gameLoopUpdateFrameUi(const sf::Time deltaTime, const float deltaTimeMs, const FrameInput& frameInput)
{
    imGuiContext.update(window, deltaTime);

    if (undoPPPurchaseTimer.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
        undoPPPurchase.clear();

    cpuCloudUiDrawableBatch.clear();
    uiDraw(frameInput.mousePos);

#ifdef SFEX_PROFILER_ENABLED
    ImGui::Begin("SFEX Profiler");
    sfex::showImguiProfiler();
    ImGui::End();
#endif
}
