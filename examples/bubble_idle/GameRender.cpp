#include "Aliases.hpp"
#include "BubbleIdleMain.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "PlayerInput.hpp"
#include "Version.hpp"

#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/Profiler.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"

#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"

////////////////////////////////////////////////////////////
[[nodiscard]] FrameViewState Main::gameLoopComputeViews()
{
    FrameViewState out;
    out.resolution = getResolution();

    const auto screenShake = profile.enableScreenShake ? rngFast.getVec2f({-screenShakeAmount, -screenShakeAmount},
                                                                          {screenShakeAmount, screenShakeAmount})
                                                       : sf::Vec2f{0.f, 0.f};

    nonScaledHUDView = {.center = out.resolution / 2.f, .size = out.resolution};
    scaledHUDView    = makeScaledHUDView(out.resolution, profile.hudScale);

    gameView                     = createScaledGameView(gameScreenSize, out.resolution);
    gameView.viewport.position.x = 0.f;
    gameView.center              = getViewCenter() + screenShake;

    const sf::Vec2u backgroundResolution = gameView.size.toVec2u();
    if (rtBackground.getSize() != backgroundResolution)
        recreateBackgroundRenderTexture(backgroundResolution);

    out.scaledTopGameView        = createScaledTopGameView(gameScreenSize, out.resolution);
    out.scaledTopGameView.center = gameView.center -
                                   (gameView.viewport.position + gameView.viewport.size * 0.5f - sf::Vec2f{0.5f, 0.5f})
                                       .componentWiseMul(out.scaledTopGameView.size);

    out.gameBackgroundView = {.center = getViewCenterWithoutScroll() + screenShake, .size = gameView.size};
    return out;
}

////////////////////////////////////////////////////////////
void Main::gameLoopRenderFrame(const float             deltaTimeMs,
                               const bool              shouldDrawUI,
                               const sf::base::U8      shouldDrawUIAlpha,
                               const FrameInput&       frameInput,
                               const FrameUpdateState& frameUpdate,
                               const FrameViewState&   frameViews)
{
    rtGame.clear(sf::Color::Transparent);

    gameLoopUpdateAndDrawFixedMenuBackground(deltaTimeMs, frameUpdate.elapsedUs);
    gameLoopUpdateAndDrawBackground(deltaTimeMs, frameViews.gameBackgroundView);

    bubbleDrawableBatch.clear();
    starBubbleDrawableBatch.clear();
    bombBubbleDrawableBatch.clear();

    if (profile.showBubbles)
        gameLoopDrawBubbles();

    if (profile.useBubbleShader)
        gameLoopDisplayBubblesWithShader();
    else
        gameLoopDisplayBubblesWithoutShader();

    cpuCloudDrawableBatch.clear();

    {
        sf::Vec2f p_min{pt->getMapLimit(), 0.f};
        sf::Vec2f p_max{pt->getMapLimit() + frameViews.resolution.x, frameViews.resolution.y};

        cpuCloudDrawableBatch.add(sf::RectangleShapeData{
            .position  = p_min.addX(10.f),
            .fillColor = sf::Color::White,
            .size      = p_max - p_min,
        });

        p_min.x += 40.f;

        drawCloudFrame({
            .time              = shaderTime,
            .mins              = p_min,
            .maxs              = p_max,
            .xSteps            = 6,
            .ySteps            = 12,
            .scaleMult         = 4.f,
            .outwardOffsetMult = 1.f,
            .batch             = &cpuCloudDrawableBatch,
        });
    }

    gameLoopDrawCursorTrail(frameInput.mousePos);
    gameLoopDrawMinimapIcons();

    cpuTopCloudDrawableBatch.clear();
    cpuDrawableBatchBeforeCats.clear();
    cpuDrawableBatch.clear();
    cpuDrawableBatchAfterCats.clear();
    cpuDrawableBatchAdditive.clear();
    cpuTopDrawableBatch.clear();
    catTextDrawableBatch.clear();
    catTextTopDrawableBatch.clear();
    cpuCloudHudDrawableBatch.clear();
    hexedCatDrawCommands.clear();

    if (pt->multiPopEnabled && playerInputState.draggedCats.empty())
    {
        const auto range = pt->psvPPMultiPopRange.currentValue() * 0.9f;

        cpuDrawableBatchBeforeCats.add(sf::CircleShapeData{
            .position           = frameInput.mousePos,
            .origin             = {range, range},
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::Transparent,
            .outlineColor       = (outlineHueColor.withAlpha(105u).withLightness(0.75f)),
            .outlineThickness   = 1.5f,
            .radius             = range,
            .pointCount         = static_cast<unsigned int>(range / 2.f),
        });
    }

    gameLoopDrawHellPortals();
    gameLoopDrawCats(frameInput.mousePos, deltaTimeMs);
    gameLoopDrawShrines(frameInput.mousePos);
    gameLoopDrawDolls(frameInput.mousePos);
    gameLoopDrawParticles();
    gameLoopDrawTextParticles();
    gameLoopDisplayCloudBatch(cpuCloudDrawableBatch, gameView);
    drawBatch(cpuDrawableBatchBeforeCats, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(cpuDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawHexedCatDrawCommands(gameView, /* top */ false);
    drawBatch(cpuDrawableBatchAfterCats, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(cpuDrawableBatchAdditive,
              {.blendMode = sf::BlendAdd, .view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(catTextDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});

    gameLoopDrawScrollArrowHint(deltaTimeMs);

    if (const auto dragRect = getAoEDragRect(frameInput.mousePos); dragRect.hasValue())
        rtGame.draw(sf::RectangleShapeData{.position         = dragRect->position,
                                           .origin           = {0.f, 0.f},
                                           .fillColor        = sf::Color::whiteWithAlpha(64u),
                                           .outlineColor     = sf::Color::whiteWithAlpha(176u),
                                           .outlineThickness = 4.f,
                                           .size             = dragRect->size},
                    {.view = gameView});

    const float yBelowMinimap = pt->mapPurchased ? (boundaries.y / profile.minimapScale) + 12.f : 0.f;

    if (shouldDrawUI)
    {
        sf::Vec2f mins{10.f, 10.f};
        sf::Vec2f maxs{20.f + static_cast<float>(moneyText.getString().getSize()) * 15.f, pt->comboPurchased ? 60.f : 40.f};

        mins.y += yBelowMinimap;
        maxs.y += yBelowMinimap;

        drawCloudFrame({
            .time              = shaderTime,
            .mins              = mins,
            .maxs              = maxs,
            .xSteps            = 16,
            .ySteps            = 8,
            .scaleMult         = 1.f,
            .outwardOffsetMult = 1.f,
            .batch             = &cpuCloudHudDrawableBatch,
        });

        hudBottomDrawableBatch.clear();
        gameLoopDrawHUDBottomParticles();
        drawBatch(hudBottomDrawableBatch,
                  {.view = nonScaledHUDView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    }

    if (shouldDrawUI)
    {
        hudDrawableBatch.clear();

        if (!uiState.debugHideUI)
            gameLoopDrawHUDParticles();

        gameLoopDrawEarnedCoinParticles();
        drawBatch(hudDrawableBatch, {.view = scaledHUDView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    }

    if constexpr (isDemoVersion)
    {
        const float xStartOverlay = gameView.viewport.size.x * frameViews.resolution.x / profile.hudScale;

        demoText.setGlobalTopRight({xStartOverlay - 15.f, 15.f});
        demoText.setOutlineColor(outlineHueColor);
        rtGame.draw(demoText, {.view = scaledHUDView});

        sf::TextData demoInfoTextData{.position         = {},
                                      .string           = "",
                                      .characterSize    = 24u,
                                      .fillColor        = sf::Color::White,
                                      .outlineColor     = outlineHueColor,
                                      .outlineThickness = 2.f};

        const float lineSpacing = fontSuperBakery.getLineSpacing(demoInfoTextData.characterSize);

        const sf::base::StringView lines[3] = {"Only one prestige and two shrines",
                                               "Full version available on Steam",
                                               "Your progress will carry over!"};

        for (sf::base::SizeT i = 0u; i < 3u; ++i)
        {
            demoInfoTextData.string = lines[i].data();
            demoInfoTextData.origin.x = sf::TextUtils::precomputeTextLocalBounds(fontSuperBakery, demoInfoTextData).size.x;
            demoInfoTextData.position = demoText.getGlobalBottomRight().addY(10.f + (static_cast<float>(i) * lineSpacing));

            rtGame.draw(fontSuperBakery, demoInfoTextData, {.view = scaledHUDView});
        }
    }

    gameLoopUpdateMoneyText(deltaTimeMs, yBelowMinimap);
    gameLoopUpdateSpentMoneyEffect(deltaTimeMs);
    gameLoopUpdateComboText(deltaTimeMs, yBelowMinimap);

    if (isDevilcatHellsingedActive() && pt->buffCountdownsPerType[asIdx(CatType::Devil)].value > 0.f)
    {
        if (portalStormTimer.updateAndLoop(deltaTimeMs, 10.f) == CountdownStatusLoop::Looping &&
            rng.getF(0.f, 100.f) <= pt->psvPPDevilRitualBuffPercentage.currentValue())
        {
            const float offset    = 64.f;
            const auto  portalPos = rng.getVec2f({offset, offset}, {pt->getMapLimit() - offset, boundaries.y - offset});

            pt->hellPortals.pushBack({
                .position = portalPos,
                .life     = Countdown{.value = 1750.f},
                .catIdx   = 100'000u,
            });

            sounds.makeBomb.settings.position = {portalPos.x, portalPos.y};
            playSound(sounds.portalon);
        }
    }

    gameLoopUpdateBuffText();

    if (!comboState.buffText.getString().isEmpty())
    {
        const sf::Vec2f offset{10.f, 10.f};

        auto mins = comboState.buffText.getGlobalTopLeft() + offset;
        auto maxs = comboState.buffText.getGlobalBottomRight() - offset - sf::Vec2{0.f, 20.f};

        mins = (mins.toVec2i() / 20 * 20).toVec2f();
        maxs = (maxs.toVec2i() / 20 * 20).toVec2f().addX(20.f).addY(5.f);

        drawCloudFrame({
            .time              = shaderTime,
            .mins              = mins,
            .maxs              = maxs,
            .xSteps            = 16,
            .ySteps            = 8,
            .scaleMult         = 1.f,
            .outwardOffsetMult = 1.f,
            .batch             = &cpuCloudHudDrawableBatch,
        });
    }

    if (shouldDrawUI && !uiState.debugHideUI)
        if (comboState.comboCountdown.value > 25.f)
            rtGame.draw(
                sf::RoundedRectangleShapeData{
                    .position     = {comboState.comboText.getGlobalCenterRight().x + 3.f, yBelowMinimap + 51.f},
                    .fillColor    = sf::Color{75, 75, 75, 255},
                    .size         = {100.f * comboState.comboCountdown.value / 700.f, 20.f},
                    .cornerRadius = 6.f,
                },
                {.view = scaledHUDView});

    if (!uiState.debugHideUI && pt->mapPurchased)
        drawMinimap(/* back */ true, rtGame, scaledHUDView, frameViews.resolution, shouldDrawUIAlpha);

    gameLoopDisplayCloudBatch(cpuCloudUiDrawableBatch, nonScaledHUDView);
    gameLoopDisplayCloudBatch(cpuCloudHudDrawableBatch, scaledHUDView);

    if (!uiState.debugHideUI && pt->mapPurchased)
    {
        drawMinimap(/* back */ false, rtGame, scaledHUDView, frameViews.resolution, shouldDrawUIAlpha);

        const auto p = scaledHUDView.screenToWorld(frameInput.windowSpaceMouseOrFingerPos.toVec2f(),
                                                   window.getSize().toVec2f());

        if (uiState.minimapRect.contains(p) && mBtnDown(sf::Mouse::Button::Left, /* penetrateUI */ true))
        {
            const auto minimapPos   = p - uiState.minimapRect.position;
            playerInputState.scroll = minimapPos.x * 0.5f * pt->getMapLimit() / uiState.minimapRect.size.x -
                                      gameView.size.x * 0.25f;
        }
    }

    if (!uiState.debugHideUI)
    {
        moneyText.setFillColorAlpha(shouldDrawUIAlpha);
        moneyText.setOutlineColorAlpha(shouldDrawUIAlpha);
        rtGame.draw(moneyText, {.view = scaledHUDView});
    }

    if (!uiState.debugHideUI && pt->comboPurchased)
    {
        comboState.comboText.setFillColorAlpha(shouldDrawUIAlpha);
        comboState.comboText.setOutlineColorAlpha(shouldDrawUIAlpha);
        rtGame.draw(comboState.comboText, {.view = scaledHUDView});
    }

    if (!uiState.debugHideUI)
    {
        comboState.buffText.setFillColorAlpha(shouldDrawUIAlpha);
        comboState.buffText.setOutlineColorAlpha(shouldDrawUIAlpha);
        rtGame.draw(comboState.buffText, {.view = scaledHUDView});
    }

    gameLoopDrawImGui(shouldDrawUIAlpha);

    gameLoopDisplayCloudBatch(cpuTopCloudDrawableBatch, frameViews.scaledTopGameView);
    drawBatch(cpuTopDrawableBatch,
              {.view = frameViews.scaledTopGameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawHexedCatDrawCommands(frameViews.scaledTopGameView, /* top */ true);
    drawBatch(catTextTopDrawableBatch,
              {.view = frameViews.scaledTopGameView, .texture = &textureAtlas.getTexture(), .shader = &shader});

    if (shouldDrawUI)
        gameLoopUpdatePurchaseUnlockedEffects(deltaTimeMs);

    if (shouldDrawUI)
    {
        hudTopDrawableBatch.clear();
        gameLoopDrawHUDTopParticles();
        drawBatch(hudTopDrawableBatch,
                  {.view = nonScaledHUDView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    }

    gameLoopDrawCursor(deltaTimeMs, frameUpdate.cursorGrow);
    gameLoopDrawCursorComboText(deltaTimeMs, frameUpdate.cursorGrow);
    gameLoopDrawCursorComboBar();

    if (splashCountdown.value > 0.f)
        drawSplashScreen(rtGame, scaledHUDView, frameViews.resolution, profile.hudScale);

    if (victoryTC.hasValue())
    {
        if (victoryTC->updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
        {
            cdLetterAppear.value = 4000.f;
            delayedActions.emplaceBack(Countdown{.value = 4000.f}, [this] { playSound(sounds.paper); });
        }

        if (victoryTC->isDone())
        {
            if (cdLetterAppear.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                cdLetterText.value = 10'000.f;

            const float progress = cdLetterAppear.getProgressBounced(4000.f);

            rtGame.draw(sf::Sprite{.position    = frameViews.resolution / 2.f / profile.hudScale,
                                   .scale       = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(progress)) /
                                                  profile.hudScale * 2.f,
                                   .origin      = txLetter.getSize().toVec2f() / 2.f,
                                   .textureRect = txLetter.getRect(),
                                   .color = sf::Color::whiteWithAlpha(static_cast<U8>(easeInOutQuint(progress) * 255.f))},
                        {.view = scaledHUDView, .texture = &txLetter});
        }

        (void)cdLetterText.updateAndStop(deltaTimeMs);

        const float textProgress = cdLetterText.value > 9000.f   ? remap(cdLetterText.value, 9000.f, 10'000.f, 1.f, 0.f)
                                   : cdLetterText.value < 1000.f ? cdLetterText.value / 1000.f
                                                                 : 1.f;

        rtGame.draw(sf::Sprite{.position    = frameViews.resolution / 2.f / profile.hudScale,
                               .scale       = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(textProgress)) /
                                              profile.hudScale * 1.45f,
                               .origin      = txLetterText.getSize().toVec2f() / 2.f,
                               .textureRect = txLetterText.getRect(),
                               .color = sf::Color::whiteWithAlpha(static_cast<U8>(easeInOutQuint(textProgress) * 255.f))},
                    {.view = scaledHUDView, .texture = &txLetterText});
    }

    if (!pt->dolls.empty())
        gameLoopDrawDollParticleBorder(0.f);

    if (!pt->copyDolls.empty())
        gameLoopDrawDollParticleBorder(180.f);

    gameLoopTips(deltaTimeMs);
    gameLoopPrestigeAvailableReminder();
    gameLoopReminderBuyCombo();
    gameLoopReminderSpendPPs();
    gameLoopUpdateDpsSampler(frameUpdate.elapsedUs);

    if (shouldDrawUI)
        gameLoopUpdateNotificationQueue(deltaTimeMs);
}

////////////////////////////////////////////////////////////
void Main::gameLoopPresentFrame(const FrameViewState& frameViews)
{
    if (flushBeforeDisplay)
        rtGame.invokeGlFlush();

    if (finishBeforeDisplay)
        rtGame.invokeGlFinish();

    rtGame.display();

    if (flushAfterDisplay)
        rtGame.invokeGlFlush();

    if (finishAfterDisplay)
        rtGame.invokeGlFinish();

    window.clear();

    auto gameViewNoScroll   = gameView;
    gameViewNoScroll.center = getViewCenterWithoutScroll();

    {
        const float ratio         = frameViews.resolution.x / 1250.f;
        const float fixedBgScroll = txFixedBg.getSize().toVec2f().x * 0.5f * sf::base::remainder(fixedBgSlide, 3.f);

        window.draw(txFixedBg,
                    {.position    = {0.f, 0.f},
                     .scale       = {ratio, ratio},
                     .textureRect = {{fixedBgScroll - playerInputState.actualScroll / 20.f, 0.f},
                                     {frameViews.resolution.x / ratio, frameViews.resolution.y / ratio}},
                     .color       = sf::Color::White},
                    {.view = nonScaledHUDView});
    }

    window.draw(rtBackgroundProcessed.getTexture(),
                {.textureRect{{0.f, 0.f}, gameViewNoScroll.size}},
                {.view = gameViewNoScroll});

    drawActivatedShrineBackgroundEffects(window, gameViewNoScroll, gameView.center);

    setPostProcessUniforms(profile.ppSVibrance,
                           profile.ppSSaturation,
                           profile.ppSLightness,
                           profile.ppSSharpness,
                           profile.ppSBlur);

    constexpr sf::BlendMode premultipliedAlphaBlend(sf::BlendMode::Factor::One,
                                                    sf::BlendMode::Factor::OneMinusSrcAlpha,
                                                    sf::BlendMode::Equation::Add,
                                                    sf::BlendMode::Factor::One,
                                                    sf::BlendMode::Factor::OneMinusSrcAlpha,
                                                    sf::BlendMode::Equation::Add);

    window.draw(rtGame.getTexture(), {.blendMode = premultipliedAlphaBlend, .shader = &shaderPostProcess});

    if (flushBeforeDisplay)
        rtGame.invokeGlFlush();

    if (finishBeforeDisplay)
        rtGame.invokeGlFinish();

    {
        SFEX_PROFILE_SCOPE("window.display()");
        window.display();
    }

    if (flushAfterDisplay)
        rtGame.invokeGlFlush();

    if (finishAfterDisplay)
        rtGame.invokeGlFinish();
}
