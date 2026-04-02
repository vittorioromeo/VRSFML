

#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "Serialization.hpp"
#include "Version.hpp"

#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/Profiler.hpp"
#include "ExampleUtils/ProfilerImGui.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Lround.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include <utility>

namespace
{
struct GameLoopFrameState
{
    bool          shouldDrawUI{false};
    sf::base::U8  shouldDrawUIAlpha{0u};
    FrameInput    input;
    sf::Time      deltaTime;
    float         deltaTimeMs{0.f};
    sf::base::I64 elapsedUs{0};
    float         cursorGrow{0.f};
};

struct GameLoopViewState
{
    sf::Vec2f resolution;
    sf::View  scaledTopGameView;
    sf::View  gameBackgroundView;
};

void updateGameLoopWorld(Main& main, GameLoopFrameState& frame)
{
    main.gameLoopUpdateTransitions(frame.deltaTimeMs);

    main.sweepAndPrune.clear();
    main.sweepAndPrune.populate(main.pt->bubbles);

    main.frameProcThisFrame = (main.frameProcCd.updateAndLoop(frame.deltaTimeMs, 20.f) == CountdownStatusLoop::Looping);

    main.gameLoopUpdateBubbles(frame.deltaTimeMs);
    main.gameLoopUpdateAttractoBuff(frame.deltaTimeMs);

    const bool anyBubblePoppedByClicking = main.gameLoopUpdateBubbleClick(frame.input.clickPosition);
    frame.cursorGrow = main.gameLoopUpdateCursorGrowthEffect(frame.deltaTimeMs, anyBubblePoppedByClicking);
    main.gameLoopUpdateCombo(frame.deltaTimeMs, anyBubblePoppedByClicking, frame.input.mousePos, frame.input.clickPosition);

    main.gameLoopUpdateCollisionsBubbleBubble(frame.deltaTimeMs);
    main.gameLoopUpdateCollisionsCatCat(frame.deltaTimeMs);
    main.gameLoopUpdateCollisionsCatShrine(frame.deltaTimeMs);
    main.gameLoopUpdateCollisionsCatDoll();
    main.gameLoopUpdateCollisionsBubbleHellPortal();

    for (Cat& cat : main.pt->cats)
    {
        constexpr float maxDragTime  = 1000.f;
        constexpr float dragInSpeed  = 1.f;
        constexpr float dragOutSpeed = 2.5f;

        const float dragDelta = main.isCatBeingDragged(cat) ? frame.deltaTimeMs * dragInSpeed
                                                            : -frame.deltaTimeMs * dragOutSpeed;
        cat.dragTime          = sf::base::clamp(cat.dragTime + dragDelta, 0.f, maxDragTime);
    }

    main.gameLoopUpdateCatDragging(frame.deltaTimeMs, frame.input.downFingers.size(), frame.input.mousePos);
    main.gameLoopUpdateCatActions(frame.deltaTimeMs);
    main.gameLoopUpdateShrines(frame.deltaTimeMs);
    main.gameLoopUpdateDolls(frame.deltaTimeMs, frame.input.mousePos);
    main.gameLoopUpdateCopyDolls(frame.deltaTimeMs, frame.input.mousePos);
    main.gameLoopUpdateHellPortals(frame.deltaTimeMs);
    main.gameLoopUpdateWitchBuffs(frame.deltaTimeMs);
    main.gameLoopUpdateMana(frame.deltaTimeMs);
    main.gameLoopUpdateAutocast();

    for (auto& [delayCountdown, func] : main.delayedActions)
        if (delayCountdown.updateAndStop(frame.deltaTimeMs) == CountdownStatusStop::JustFinished)
            func();

    sf::base::vectorEraseIf(main.delayedActions, [](const auto& delayedAction) {
        return delayedAction.delayCountdown.isDone();
    });

    main.gameLoopUpdateScreenShake(frame.deltaTimeMs);
    main.gameLoopUpdateParticlesAndTextParticles(frame.deltaTimeMs);
    main.gameLoopUpdateSounds(frame.deltaTimeMs, frame.input.mousePos);

    frame.elapsedUs = main.playedClock.getElapsedTime().asMicroseconds();
    main.playedClock.restart();

    main.gameLoopUpdateTimePlayed(frame.elapsedUs);
    main.gameLoopUpdateAutosave(frame.elapsedUs);
    main.gameLoopUpdateMilestones();
    main.gameLoopUpdateAchievements();
    main.gameLoopUpdateSplits();
}

void updateGameLoopUi(Main& main, const GameLoopFrameState& frame)
{
    main.imGuiContext.update(main.window, frame.deltaTime);

    if (main.undoPPPurchaseTimer.updateAndStop(frame.deltaTimeMs) == CountdownStatusStop::JustFinished)
        main.undoPPPurchase.clear();

    main.cpuCloudUiDrawableBatch.clear();
    main.uiDraw(frame.input.mousePos);

#ifdef SFEX_PROFILER_ENABLED
    ImGui::Begin("SFEX Profiler");
    sfex::showImguiProfiler();
    ImGui::End();
#endif
}

[[nodiscard]] GameLoopViewState computeGameLoopViews(Main& main)
{
    GameLoopViewState out;
    out.resolution = main.getResolution();

    const auto screenShake = main.profile.enableScreenShake
                                 ? main.rngFast.getVec2f({-main.screenShakeAmount, -main.screenShakeAmount},
                                                         {main.screenShakeAmount, main.screenShakeAmount})
                                 : sf::Vec2f{0.f, 0.f};

    main.nonScaledHUDView = {.center = out.resolution / 2.f, .size = out.resolution};
    main.scaledHUDView    = main.makeScaledHUDView(out.resolution, main.profile.hudScale);

    main.gameView                     = main.createScaledGameView(gameScreenSize, out.resolution);
    main.gameView.viewport.position.x = 0.f;
    main.gameView.center              = main.getViewCenter() + screenShake;

    const sf::Vec2u backgroundResolution = main.gameView.size.toVec2u();
    if (main.rtBackground.getSize() != backgroundResolution)
        main.recreateBackgroundRenderTexture(backgroundResolution);

    out.scaledTopGameView        = main.createScaledTopGameView(gameScreenSize, out.resolution);
    out.scaledTopGameView.center = main.gameView.center - (main.gameView.viewport.position +
                                                           main.gameView.viewport.size * 0.5f - sf::Vec2f{0.5f, 0.5f})
                                                              .componentWiseMul(out.scaledTopGameView.size);

    out.gameBackgroundView = {.center = main.getViewCenterWithoutScroll() + screenShake, .size = main.gameView.size};
    return out;
}

void renderGameLoopFrame(Main& main, const GameLoopFrameState& frame, const GameLoopViewState& views)
{
    main.rtGame.clear(sf::Color::Transparent);

    main.gameLoopUpdateAndDrawFixedMenuBackground(frame.deltaTimeMs, frame.elapsedUs);
    main.gameLoopUpdateAndDrawBackground(frame.deltaTimeMs, views.gameBackgroundView);

    main.bubbleDrawableBatch.clear();
    main.starBubbleDrawableBatch.clear();
    main.bombBubbleDrawableBatch.clear();

    if (main.profile.showBubbles)
        main.gameLoopDrawBubbles();

    if (main.profile.useBubbleShader)
        main.gameLoopDisplayBubblesWithShader();
    else
        main.gameLoopDisplayBubblesWithoutShader();

    main.cpuCloudDrawableBatch.clear();

    {
        sf::Vec2f p_min{main.pt->getMapLimit(), 0.f};
        sf::Vec2f p_max{main.pt->getMapLimit() + views.resolution.x, views.resolution.y};

        main.cpuCloudDrawableBatch.add(sf::RectangleShapeData{
            .position  = p_min.addX(10.f),
            .fillColor = sf::Color::White,
            .size      = p_max - p_min,
        });

        p_min.x += 40.f;

        main.drawCloudFrame({
            .time              = main.shaderTime,
            .mins              = p_min,
            .maxs              = p_max,
            .xSteps            = 6,
            .ySteps            = 12,
            .scaleMult         = 4.f,
            .outwardOffsetMult = 1.f,
            .batch             = &main.cpuCloudDrawableBatch,
        });
    }

    main.gameLoopDrawCursorTrail(frame.input.mousePos);
    main.gameLoopDrawMinimapIcons();

    main.cpuTopCloudDrawableBatch.clear();
    main.cpuDrawableBatchBeforeCats.clear();
    main.cpuDrawableBatch.clear();
    main.cpuDrawableBatchAfterCats.clear();
    main.cpuDrawableBatchAdditive.clear();
    main.cpuTopDrawableBatch.clear();
    main.catTextDrawableBatch.clear();
    main.catTextTopDrawableBatch.clear();
    main.cpuCloudHudDrawableBatch.clear();
    main.hexedCatDrawCommands.clear();

    if (main.pt->multiPopEnabled && main.playerInputState.draggedCats.empty())
    {
        const auto range = main.pt->psvPPMultiPopRange.currentValue() * 0.9f;

        main.cpuDrawableBatchBeforeCats.add(sf::CircleShapeData{
            .position           = frame.input.mousePos,
            .origin             = {range, range},
            .outlineTextureRect = main.txrWhiteDot,
            .fillColor          = sf::Color::Transparent,
            .outlineColor       = (main.outlineHueColor.withAlpha(105u).withLightness(0.75f)),
            .outlineThickness   = 1.5f,
            .radius             = range,
            .pointCount         = static_cast<unsigned int>(range / 2.f),
        });
    }

    main.gameLoopDrawHellPortals();
    main.gameLoopDrawCats(frame.input.mousePos, frame.deltaTimeMs);
    main.gameLoopDrawShrines(frame.input.mousePos);
    main.gameLoopDrawDolls(frame.input.mousePos);
    main.gameLoopDrawParticles();
    main.gameLoopDrawTextParticles();
    main.gameLoopDisplayCloudBatch(main.cpuCloudDrawableBatch, main.gameView);
    main.drawBatch(main.cpuDrawableBatchBeforeCats,
                   {.view = main.gameView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    main.drawBatch(main.cpuDrawableBatch,
                   {.view = main.gameView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    main.drawHexedCatDrawCommands(main.gameView, /* top */ false);
    main.drawBatch(main.cpuDrawableBatchAfterCats,
                   {.view = main.gameView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    main.drawBatch(main.cpuDrawableBatchAdditive,
                   {.blendMode = sf::BlendAdd,
                    .view      = main.gameView,
                    .texture   = &main.textureAtlas.getTexture(),
                    .shader    = &main.shader});
    main.drawBatch(main.catTextDrawableBatch,
                   {.view = main.gameView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});

    main.gameLoopDrawScrollArrowHint(frame.deltaTimeMs);

    if (const auto dragRect = main.getAoEDragRect(frame.input.mousePos); dragRect.hasValue())
        main.rtGame.draw(sf::RectangleShapeData{.position         = dragRect->position,
                                                .origin           = {0.f, 0.f},
                                                .fillColor        = sf::Color::whiteMask(64u),
                                                .outlineColor     = sf::Color::whiteMask(176u),
                                                .outlineThickness = 4.f,
                                                .size             = dragRect->size},
                         {.view = main.gameView});

    const float yBelowMinimap = main.pt->mapPurchased ? (boundaries.y / main.profile.minimapScale) + 12.f : 0.f;

    if (frame.shouldDrawUI)
    {
        sf::Vec2f mins{10.f, 10.f};
        sf::Vec2f maxs{20.f + static_cast<float>(main.moneyText.getString().getSize()) * 15.f,
                       main.pt->comboPurchased ? 60.f : 40.f};

        mins.y += yBelowMinimap;
        maxs.y += yBelowMinimap;

        main.drawCloudFrame({
            .time              = main.shaderTime,
            .mins              = mins,
            .maxs              = maxs,
            .xSteps            = 16,
            .ySteps            = 8,
            .scaleMult         = 1.f,
            .outwardOffsetMult = 1.f,
            .batch             = &main.cpuCloudHudDrawableBatch,
        });

        main.hudBottomDrawableBatch.clear();
        main.gameLoopDrawHUDBottomParticles();
        main.drawBatch(main.hudBottomDrawableBatch,
                       {.view = main.nonScaledHUDView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    }

    if (frame.shouldDrawUI)
    {
        main.hudDrawableBatch.clear();

        if (!main.uiState.debugHideUI)
            main.gameLoopDrawHUDParticles();

        main.gameLoopDrawEarnedCoinParticles();
        main.drawBatch(main.hudDrawableBatch,
                       {.view = main.scaledHUDView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    }

    if constexpr (isDemoVersion)
    {
        const float xStartOverlay = main.gameView.viewport.size.x * views.resolution.x / main.profile.hudScale;

        main.demoText.setGlobalTopRight({xStartOverlay - 15.f, 15.f});
        main.demoText.setOutlineColor(main.outlineHueColor);
        main.rtGame.draw(main.demoText, {.view = main.scaledHUDView});

        sf::TextData demoInfoTextData{.position         = {},
                                      .string           = "",
                                      .characterSize    = 24u,
                                      .fillColor        = sf::Color::White,
                                      .outlineColor     = main.outlineHueColor,
                                      .outlineThickness = 2.f};

        const float lineSpacing = main.fontSuperBakery.getLineSpacing(demoInfoTextData.characterSize);

        const sf::base::StringView lines[3] = {"Only one prestige and two shrines",
                                               "Full version available on Steam",
                                               "Your progress will carry over!"};

        for (sf::base::SizeT i = 0u; i < 3u; ++i)
        {
            demoInfoTextData.string = lines[i].data();
            demoInfoTextData.origin
                .x = sf::TextUtils::precomputeTextLocalBounds(main.fontSuperBakery, demoInfoTextData).size.x;
            demoInfoTextData.position = main.demoText.getGlobalBottomRight().addY(
                10.f + (static_cast<float>(i) * lineSpacing));

            main.rtGame.draw(main.fontSuperBakery, demoInfoTextData, {.view = main.scaledHUDView});
        }
    }

    main.gameLoopUpdateMoneyText(frame.deltaTimeMs, yBelowMinimap);
    main.gameLoopUpdateSpentMoneyEffect(frame.deltaTimeMs);
    main.gameLoopUpdateComboText(frame.deltaTimeMs, yBelowMinimap);

    if (main.isDevilcatHellsingedActive() && main.pt->buffCountdownsPerType[asIdx(CatType::Devil)].value > 0.f)
    {
        if (main.portalStormTimer.updateAndLoop(frame.deltaTimeMs, 10.f) == CountdownStatusLoop::Looping &&
            main.rng.getF(0.f, 100.f) <= main.pt->psvPPDevilRitualBuffPercentage.currentValue())
        {
            const float offset    = 64.f;
            const auto  portalPos = main.rng.getVec2f({offset, offset},
                                                      {main.pt->getMapLimit() - offset, boundaries.y - offset});

            main.pt->hellPortals.pushBack({
                .position = portalPos,
                .life     = Countdown{.value = 1750.f},
                .catIdx   = 100'000u,
            });

            main.sounds.makeBomb.settings.position = {portalPos.x, portalPos.y};
            main.playSound(main.sounds.portalon);
        }
    }

    main.gameLoopUpdateBuffText();

    if (!main.comboState.buffText.getString().isEmpty())
    {
        const sf::Vec2f offset{10.f, 10.f};

        auto mins = main.comboState.buffText.getGlobalTopLeft() + offset;
        auto maxs = main.comboState.buffText.getGlobalBottomRight() - offset - sf::Vec2{0.f, 20.f};

        mins = (mins.toVec2i() / 20 * 20).toVec2f();
        maxs = (maxs.toVec2i() / 20 * 20).toVec2f().addX(20.f).addY(5.f);

        main.drawCloudFrame({
            .time              = main.shaderTime,
            .mins              = mins,
            .maxs              = maxs,
            .xSteps            = 16,
            .ySteps            = 8,
            .scaleMult         = 1.f,
            .outwardOffsetMult = 1.f,
            .batch             = &main.cpuCloudHudDrawableBatch,
        });
    }

    if (frame.shouldDrawUI && !main.uiState.debugHideUI)
        if (main.comboState.comboCountdown.value > 25.f)
            main.rtGame.draw(
                sf::RoundedRectangleShapeData{
                    .position     = {main.comboState.comboText.getGlobalCenterRight().x + 3.f, yBelowMinimap + 51.f},
                    .fillColor    = sf::Color{75, 75, 75, 255},
                    .size         = {100.f * main.comboState.comboCountdown.value / 700.f, 20.f},
                    .cornerRadius = 6.f,
                },
                {.view = main.scaledHUDView});

    if (!main.uiState.debugHideUI && main.pt->mapPurchased)
        main.drawMinimap(/* back */ true, main.rtGame, main.scaledHUDView, views.resolution, frame.shouldDrawUIAlpha);

    main.gameLoopDisplayCloudBatch(main.cpuCloudUiDrawableBatch, main.nonScaledHUDView);
    main.gameLoopDisplayCloudBatch(main.cpuCloudHudDrawableBatch, main.scaledHUDView);

    if (!main.uiState.debugHideUI && main.pt->mapPurchased)
    {
        main.drawMinimap(/* back */ false, main.rtGame, main.scaledHUDView, views.resolution, frame.shouldDrawUIAlpha);

        const auto p = main.scaledHUDView.screenToWorld(frame.input.windowSpaceMouseOrFingerPos.toVec2f(),
                                                        main.window.getSize().toVec2f());

        if (main.uiState.minimapRect.contains(p) && main.mBtnDown(sf::Mouse::Button::Left, /* penetrateUI */ true))
        {
            const auto minimapPos = p - main.uiState.minimapRect.position;
            main.playerInputState.scroll = minimapPos.x * 0.5f * main.pt->getMapLimit() / main.uiState.minimapRect.size.x -
                                           main.gameView.size.x * 0.25f;
        }
    }

    if (!main.uiState.debugHideUI)
    {
        main.moneyText.setFillColorAlpha(frame.shouldDrawUIAlpha);
        main.moneyText.setOutlineColorAlpha(frame.shouldDrawUIAlpha);
        main.rtGame.draw(main.moneyText, {.view = main.scaledHUDView});
    }

    if (!main.uiState.debugHideUI && main.pt->comboPurchased)
    {
        main.comboState.comboText.setFillColorAlpha(frame.shouldDrawUIAlpha);
        main.comboState.comboText.setOutlineColorAlpha(frame.shouldDrawUIAlpha);
        main.rtGame.draw(main.comboState.comboText, {.view = main.scaledHUDView});
    }

    if (!main.uiState.debugHideUI)
    {
        main.comboState.buffText.setFillColorAlpha(frame.shouldDrawUIAlpha);
        main.comboState.buffText.setOutlineColorAlpha(frame.shouldDrawUIAlpha);
        main.rtGame.draw(main.comboState.buffText, {.view = main.scaledHUDView});
    }

    main.gameLoopDrawImGui(frame.shouldDrawUIAlpha);

    main.gameLoopDisplayCloudBatch(main.cpuTopCloudDrawableBatch, views.scaledTopGameView);
    main.drawBatch(main.cpuTopDrawableBatch,
                   {.view = views.scaledTopGameView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    main.drawHexedCatDrawCommands(views.scaledTopGameView, /* top */ true);
    main.drawBatch(main.catTextTopDrawableBatch,
                   {.view = views.scaledTopGameView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});

    if (frame.shouldDrawUI)
        main.gameLoopUpdatePurchaseUnlockedEffects(frame.deltaTimeMs);

    if (frame.shouldDrawUI)
    {
        main.hudTopDrawableBatch.clear();
        main.gameLoopDrawHUDTopParticles();
        main.drawBatch(main.hudTopDrawableBatch,
                       {.view = main.nonScaledHUDView, .texture = &main.textureAtlas.getTexture(), .shader = &main.shader});
    }

    main.gameLoopDrawCursor(frame.deltaTimeMs, frame.cursorGrow);
    main.gameLoopDrawCursorComboText(frame.deltaTimeMs, frame.cursorGrow);
    main.gameLoopDrawCursorComboBar();

    if (main.splashCountdown.value > 0.f)
        main.drawSplashScreen(main.rtGame, main.scaledHUDView, views.resolution, main.profile.hudScale);

    if (main.victoryTC.hasValue())
    {
        if (main.victoryTC->updateAndStop(frame.deltaTimeMs) == CountdownStatusStop::JustFinished)
        {
            main.cdLetterAppear.value = 4000.f;
            main.delayedActions.emplaceBack(Countdown{.value = 4000.f}, [&main] { main.playSound(main.sounds.paper); });
        }

        if (main.victoryTC->isDone())
        {
            if (main.cdLetterAppear.updateAndStop(frame.deltaTimeMs) == CountdownStatusStop::JustFinished)
                main.cdLetterText.value = 10'000.f;

            const float progress = main.cdLetterAppear.getProgressBounced(4000.f);

            main.rtGame.draw(sf::Sprite{.position = views.resolution / 2.f / main.profile.hudScale,
                                        .scale    = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(progress)) /
                                                    main.profile.hudScale * 2.f,
                                        .origin   = main.txLetter.getSize().toVec2f() / 2.f,
                                        .textureRect = main.txLetter.getRect(),
                                        .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(progress) * 255.f))},
                             {.view = main.scaledHUDView, .texture = &main.txLetter});
        }

        (void)main.cdLetterText.updateAndStop(frame.deltaTimeMs);

        const float textProgress = main.cdLetterText.value > 9000.f
                                       ? remap(main.cdLetterText.value, 9000.f, 10'000.f, 1.f, 0.f)
                                   : main.cdLetterText.value < 1000.f ? main.cdLetterText.value / 1000.f
                                                                      : 1.f;

        main.rtGame.draw(sf::Sprite{.position = views.resolution / 2.f / main.profile.hudScale,
                                    .scale    = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(textProgress)) /
                                                main.profile.hudScale * 1.45f,
                                    .origin   = main.txLetterText.getSize().toVec2f() / 2.f,
                                    .textureRect = main.txLetterText.getRect(),
                                    .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(textProgress) * 255.f))},
                         {.view = main.scaledHUDView, .texture = &main.txLetterText});
    }

    if (!main.pt->dolls.empty())
        main.gameLoopDrawDollParticleBorder(0.f);

    if (!main.pt->copyDolls.empty())
        main.gameLoopDrawDollParticleBorder(180.f);

    main.gameLoopTips(frame.deltaTimeMs);
    main.gameLoopPrestigeAvailableReminder();
    main.gameLoopReminderBuyCombo();
    main.gameLoopReminderSpendPPs();
    main.gameLoopUpdateDpsSampler(frame.elapsedUs);

    if (frame.shouldDrawUI)
        main.gameLoopUpdateNotificationQueue(frame.deltaTimeMs);
}

void presentGameLoopFrame(Main& main, const GameLoopViewState& views)
{
    if (main.flushBeforeDisplay)
        main.rtGame.flushGPUCommands();

    if (main.finishBeforeDisplay)
        main.rtGame.finishGPUCommands();

    main.rtGame.display();

    if (main.flushAfterDisplay)
        main.rtGame.flushGPUCommands();

    if (main.finishAfterDisplay)
        main.rtGame.finishGPUCommands();

    main.window.clear();

    auto gameViewNoScroll   = main.gameView;
    gameViewNoScroll.center = main.getViewCenterWithoutScroll();

    {
        const float ratio = views.resolution.x / 1250.f;
        const float fixedBgScroll = main.txFixedBg.getSize().toVec2f().x * 0.5f * sf::base::remainder(main.fixedBgSlide, 3.f);

        main.window.draw(main.txFixedBg,
                         {
                             .position    = {0.f, 0.f},
                             .scale       = {ratio, ratio},
                             .textureRect = {{fixedBgScroll - main.playerInputState.actualScroll / 20.f, 0.f},
                                             {views.resolution.x / ratio, views.resolution.y / ratio}},
                             .color       = sf::Color::White,
                         },
                         {.view = main.nonScaledHUDView});
    }

    main.window.draw(main.rtBackgroundProcessed.getTexture(),
                     {.textureRect{{0.f, 0.f}, gameViewNoScroll.size}},
                     {.view = gameViewNoScroll});

    main.drawActivatedShrineBackgroundEffects(main.window, gameViewNoScroll, main.gameView.center);

    main.setPostProcessUniforms(main.profile.ppSVibrance,
                                main.profile.ppSSaturation,
                                main.profile.ppSLightness,
                                main.profile.ppSSharpness,
                                main.profile.ppSBlur);

    constexpr sf::BlendMode premultipliedAlphaBlend(sf::BlendMode::Factor::One,
                                                    sf::BlendMode::Factor::OneMinusSrcAlpha,
                                                    sf::BlendMode::Equation::Add,
                                                    sf::BlendMode::Factor::One,
                                                    sf::BlendMode::Factor::OneMinusSrcAlpha,
                                                    sf::BlendMode::Equation::Add);

    main.window.draw(main.rtGame.getTexture(), {.blendMode = premultipliedAlphaBlend, .shader = &main.shaderPostProcess});

    if (main.flushBeforeDisplay)
        main.rtGame.flushGPUCommands();

    if (main.finishBeforeDisplay)
        main.rtGame.finishGPUCommands();

    {
        SFEX_PROFILE_SCOPE("window.display()");
        main.window.display();
    }

    if (main.flushAfterDisplay)
        main.rtGame.flushGPUCommands();

    if (main.finishAfterDisplay)
        main.rtGame.finishGPUCommands();
}
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
    updateGameLoopWorld(*this, frame);
    updateGameLoopUi(*this, frame);

    const GameLoopViewState views = computeGameLoopViews(*this);
    renderGameLoopFrame(*this, frame, views);
    presentGameLoopFrame(*this, views);

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
