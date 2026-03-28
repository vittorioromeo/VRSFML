

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

////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::gameLoop()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (mustExit)
        return false;

    //
    // Only draw UI elements if not in prestige transition and splash is done
    const bool shouldDrawUI = !inPrestigeTransition && splashCountdown.value <= 0.f;
    const auto shouldDrawUIAlpha = inPrestigeTransition || splashCountdown.getProgress() < 0.75f
                                       ? static_cast<sf::base::U8>(0u)
                                       : static_cast<sf::base::U8>(
                                             remap(easeInOutSine(splashCountdown.getProgress()), 0.75f, 1.f, 0.f, 255.f));

#ifdef BUBBLEBYTE_USE_STEAMWORKS
    steamMgr.runCallbacks();
#endif

    fps = 1.f / fpsClock.getElapsedTime().asSeconds();
    fpsClock.restart();

    sf::base::Optional<sf::Vec2f> clickPosition;

    inputHelper.beginNewFrame();

    while (const sf::base::Optional event = window.pollEvent())
    {
        inputHelper.applyEvent(*event);
        imGuiContext.processEvent(window, *event);

        if (shouldDrawUI && event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)
        {
            if (!escWasPressed)
            {
                playSound(sounds.btnswitch);
                escWasPressed = true;
            }
        }

        if (event->is<sf::Event::Closed>())
            return false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
        if (const auto* e0 = event->getIf<sf::Event::TouchBegan>())
        {
            fingerPositions[e0->finger].emplace(e0->position.toVec2f());

            if (!clickPosition.hasValue())
                clickPosition.emplace(e0->position.toVec2f());
        }
        else if (const auto* e1 = event->getIf<sf::Event::TouchEnded>())
        {
            fingerPositions[e1->finger].reset();
        }
        else if (const auto* e2 = event->getIf<sf::Event::TouchMoved>())
        {
            fingerPositions[e2->finger].emplace(e2->position.toVec2f());

            if (pt->laserPopEnabled)
                if (!clickPosition.hasValue())
                    clickPosition.emplace(e2->position.toVec2f());
        }
        else if (const auto* e3 = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (e3->button == getLMB())
                clickPosition.emplace(e3->position.toVec2f());

            if (e3->button == getRMB() && !dragPosition.hasValue())
            {
                clickPosition.reset();

                dragPosition.emplace(e3->position.toVec2f());
                dragPosition->x += scroll;
            }
        }
        else if (const auto* e4 = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (e4->button == getRMB())
                dragPosition.reset();
        }
        else if (const auto* e5 = event->getIf<sf::Event::MouseMoved>())
        {
            if (pt->mapPurchased && dragPosition.hasValue())
                scroll = dragPosition->x - static_cast<float>(e5->position.x);
        }
        else if (const auto* e6 = event->getIf<sf::Event::Resized>())
        {
            recreateBackgroundRenderTexture(getExpandedGameViewSize(gameScreenSize, e6->size.toVec2f()).toVec2u());
            recreateImGuiRenderTexture(e6->size);
            recreateGameRenderTexture(e6->size);

            hudTopParticles.clear();
            hudBottomParticles.clear();
        }
        else if (const auto* e7 = event->getIf<sf::Event::KeyPressed>())
        {
            if (e7->code == sf::Keyboard::Key::Z || e7->code == sf::Keyboard::Key::X || e7->code == sf::Keyboard::Key::Y)
                clickPosition.emplace(sf::Mouse::getPosition(window).toVec2f());
        }
        else if (const auto* e8 = event->getIf<sf::Event::MouseWheelScrolled>())
        {
            const float scrollMult = keyDown(sf::Keyboard::Key::LShift) ? 200.f : 100.f;

            if (!ImGui::GetIO().WantCaptureMouse)
                scroll += e8->delta * scrollMult;
        }
    }

    if (ImGui::GetIO().WantCaptureMouse)
        clickPosition.reset();

    const auto deltaTime   = deltaClock.restart();
    const auto deltaTimeMs = sf::base::min(24.f, static_cast<float>(deltaTime.asMicroseconds()) / 1000.f);
    shaderTime += deltaTimeMs * 0.001f;

    gameLoopCheats();

    // Demo limitations clamping
    if constexpr (isDemoVersion)
    {
        const auto clampNPurchases = [](auto& psv)
        { psv.nPurchases = sf::base::min(psv.nPurchases, psv.data->nMaxPurchases); };

        clampNPurchases(pt->psvMapExtension);
        clampNPurchases(pt->psvShrineActivation);
        clampNPurchases(pt->psvBubbleValue);

        sf::base::vectorEraseIf(pt->cats, [](const Cat& cat) {
            return cat.type >= CatType::Mouse && cat.type <= CatType::Duck;
        });
    }

    //
    // TODO PO laser cursor
    if (pt->laserPopEnabled)
        if (keyDown(sf::Keyboard::Key::Z) || keyDown(sf::Keyboard::Key::X) || keyDown(sf::Keyboard::Key::Y) ||
            mBtnDown(getLMB(), /* penetrateUI */ false))
        {
            if (!clickPosition.hasValue())
                clickPosition.emplace(sf::Mouse::getPosition(window).toVec2f());
        }

    //
    // Number of fingers
    sf::base::Vector<sf::Vec2f> downFingers;
    for (const auto maybeFinger : fingerPositions)
        if (maybeFinger.hasValue())
            downFingers.pushBack(*maybeFinger);

    //
    // Map scrolling via keyboard and touch
    if (pt->mapPurchased)
    {
        // Jump to beginning/end of map
        if (inputHelper.wasKeyJustPressed(sf::Keyboard::Key::Home))
            scroll = 0.f;
        else if (inputHelper.wasKeyJustPressed(sf::Keyboard::Key::End))
            scroll = static_cast<float>(pt->getMapLimitIncreases()) * gameScreenSize.x * 0.5f;

        const auto currentScrollScreenIndex = static_cast<sf::base::SizeT>(
            sf::base::lround(scroll / (gameScreenSize.x * 0.5f)));

        // Jump to previous/next screen
        if (inputHelper.wasKeyJustPressed(sf::Keyboard::Key::PageDown) ||
            inputHelper.wasMouseButtonJustPressed(sf::Mouse::Button::Extra2))
        {
            const auto nextScrollScreenIndex = sf::base::min(currentScrollScreenIndex + 1u, pt->getMapLimitIncreases());
            scroll                           = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
        }
        else if ((inputHelper.wasKeyJustPressed(sf::Keyboard::Key::PageUp) ||
                  inputHelper.wasMouseButtonJustPressed(sf::Mouse::Button::Extra1)) &&
                 currentScrollScreenIndex > 0u)
        {
            const auto nextScrollScreenIndex = sf::base::max(static_cast<sf::base::SizeT>(0u), currentScrollScreenIndex - 1u);

            scroll = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
        }

        const float scrollMult = keyDown(sf::Keyboard::Key::LShift) ? 4.f : 2.f;

        // Left/right scrolling with keyboard
        if (keyDown(sf::Keyboard::Key::Left) || keyDown(sf::Keyboard::Key::A))
        {
            dragPosition.reset();
            scroll -= scrollMult * deltaTimeMs;
        }
        else if (keyDown(sf::Keyboard::Key::Right) || keyDown(sf::Keyboard::Key::D))
        {
            dragPosition.reset();
            scroll += scrollMult * deltaTimeMs;
        }
        else if (downFingers.size() == 2)
        {
            // TODO P2: check fingers distance
            const auto [fingerPos0, fingerPos1] = [&]
            {
                std::pair<sf::base::Optional<sf::Vec2f>, sf::base::Optional<sf::Vec2f>> result;

                for (const auto& fingerPosition : fingerPositions)
                {
                    if (fingerPosition.hasValue())
                    {
                        if (!result.first.hasValue())
                            result.first.emplace(*fingerPosition);
                        else if (!result.second.hasValue())
                            result.second.emplace(*fingerPosition);
                    }
                }

                return result;
            }();

            const auto avg = (*fingerPos0 + *fingerPos1) / 2.f;

            if (dragPosition.hasValue())
            {
                scroll = dragPosition->x - avg.x;
            }
            else
            {
                dragPosition.emplace(avg);
                dragPosition->x += scroll;
            }
        }
    }

    //
    // Cache unique cats
    cachedWitchCat    = findFirstCatByType(CatType::Witch);
    cachedWizardCat   = findFirstCatByType(CatType::Wizard);
    cachedMouseCat    = findFirstCatByType(CatType::Mouse);
    cachedEngiCat     = findFirstCatByType(CatType::Engi);
    cachedRepulsoCat  = findFirstCatByType(CatType::Repulso);
    cachedAttractoCat = findFirstCatByType(CatType::Attracto);
    cachedCopyCat     = findFirstCatByType(CatType::Copy);

    //
    // Scrolling
    gameLoopUpdateScrolling(deltaTimeMs, downFingers);

    //
    // Culling boundaries
    const sf::Vec2f resolution = getResolution();

    hudCullingBoundaries      = {0.f, resolution.x, 0.f, resolution.y};
    particleCullingBoundaries = getViewCullingBoundaries(/* offset */ 0.f);
    bubbleCullingBoundaries   = getViewCullingBoundaries(/* offset */ -64.f);

    //
    // World-space mouse position
    const auto windowSpaceMouseOrFingerPos = downFingers.size() == 1u ? downFingers[0].toVec2i()
                                                                      : sf::Mouse::getPosition(window);

    const auto mousePos = gameView.screenToWorld(windowSpaceMouseOrFingerPos.toVec2f(), window.getSize().toVec2f());

    //
    // Game startup, prestige transitions, etc...
    gameLoopUpdateTransitions(deltaTimeMs);

    //
    // Update spatial partitioning (needs to be done before updating bubbles)
    sweepAndPrune.clear();
    sweepAndPrune.populate(pt->bubbles);

    //
    // Update frameproc
    frameProcThisFrame = (frameProcCd.updateAndLoop(deltaTimeMs, 20.f) == CountdownStatusLoop::Looping);

    //
    // Update bubbles
    gameLoopUpdateBubbles(deltaTimeMs);

    //
    // Demonic attraction buff
    gameLoopUpdateAttractoBuff(deltaTimeMs);

    //
    // Process clicks
    const bool anyBubblePoppedByClicking = gameLoopUpdateBubbleClick(clickPosition);

    //
    // Cursor grow effect on click
    const float cursorGrow = gameLoopUpdateCursorGrowthEffect(deltaTimeMs, anyBubblePoppedByClicking);

    //
    // Combo failure due to timer end
    gameLoopUpdateCombo(deltaTimeMs, anyBubblePoppedByClicking, mousePos, clickPosition);

    //
    // Update collisions
    gameLoopUpdateCollisionsBubbleBubble(deltaTimeMs);
    gameLoopUpdateCollisionsCatCat(deltaTimeMs);
    gameLoopUpdateCollisionsCatShrine(deltaTimeMs);
    gameLoopUpdateCollisionsCatDoll();
    gameLoopUpdateCollisionsBubbleHellPortal();

    //
    // Update cats, shrines, dolls, buffs, and magic

    {
        for (Cat& cat : pt->cats)
        {
            constexpr float maxDragTime = 1000.f;

            constexpr float dragInSpeed  = 1.f;
            constexpr float dragOutSpeed = 2.5f;

            const float dragDelta = isCatBeingDragged(cat) ? deltaTimeMs * dragInSpeed : -deltaTimeMs * dragOutSpeed;

            cat.dragTime = sf::base::clamp(cat.dragTime + dragDelta, 0.f, maxDragTime);
        }
    }

    gameLoopUpdateCatDragging(deltaTimeMs, downFingers.size(), mousePos);
    gameLoopUpdateCatActions(deltaTimeMs);
    gameLoopUpdateShrines(deltaTimeMs);
    gameLoopUpdateDolls(deltaTimeMs, mousePos);
    gameLoopUpdateCopyDolls(deltaTimeMs, mousePos);
    gameLoopUpdateHellPortals(deltaTimeMs);
    gameLoopUpdateWitchBuffs(deltaTimeMs);
    gameLoopUpdateMana(deltaTimeMs);
    gameLoopUpdateAutocast();

    //
    // Delayed actions
    for (auto& [delayCountdown, func] : delayedActions)
        if (delayCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            func();

    sf::base::vectorEraseIf(delayedActions, [](const auto& delayedAction) {
        return delayedAction.delayCountdown.isDone();
    });

    //
    // Screen shake
    gameLoopUpdateScreenShake(deltaTimeMs);

    //
    // Particles and text particles
    gameLoopUpdateParticlesAndTextParticles(deltaTimeMs);

    //
    // Sounds and volume
    gameLoopUpdateSounds(deltaTimeMs, mousePos); // also updates listener

    //
    // Time played in microseconds
    const auto elapsedUs = playedClock.getElapsedTime().asMicroseconds();
    playedClock.restart();

    //
    // Played time
    gameLoopUpdateTimePlayed(elapsedUs);

    //
    // Autosave
    gameLoopUpdateAutosave(elapsedUs);

    //
    // Milestones and achievements
    gameLoopUpdateMilestones();
    gameLoopUpdateAchievements();

    //
    // Speedrunning splits
    gameLoopUpdateSplits();

    //
    // Update ImGui
    imGuiContext.update(window, deltaTime);

    //
    // Update PP undo button
    if (undoPPPurchaseTimer.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
        undoPPPurchase.clear();

    //
    // Draw ImGui menu
    cpuCloudUiDrawableBatch.clear();
    uiDraw(mousePos);


//
// Draw profiler
#ifdef SFEX_PROFILER_ENABLED
    ImGui::Begin("SFEX Profiler");
    sfex::showImguiProfiler();
    ImGui::End();
#endif

    //
    // Compute views
    const auto screenShake = profile.enableScreenShake ? rngFast.getVec2f({-screenShakeAmount, -screenShakeAmount},
                                                                          {screenShakeAmount, screenShakeAmount})
                                                       : sf::Vec2f{0.f, 0.f};

    nonScaledHUDView = {.center = resolution / 2.f, .size = resolution};
    scaledHUDView    = makeScaledHUDView(resolution, profile.hudScale);

    gameView                     = createScaledGameView(gameScreenSize, resolution);
    gameView.viewport.position.x = 0.f;
    gameView.center              = getViewCenter() + screenShake;

    const sf::Vec2u backgroundResolution = gameView.size.toVec2u();
    if (rtBackground.getSize() != backgroundResolution)
        recreateBackgroundRenderTexture(backgroundResolution);

    sf::View scaledTopGameView = createScaledTopGameView(gameScreenSize, resolution);
    scaledTopGameView.center   = gameView.center -
                                 (gameView.viewport.position + gameView.viewport.size * 0.5f - sf::Vec2f{0.5f, 0.5f})
                                     .componentWiseMul(scaledTopGameView.size);

    sf::View gameBackgroundView{.center = getViewCenterWithoutScroll() + screenShake, .size = gameView.size};

    //
    // Clear window
    rtGame.clear(sf::Color::Transparent);

    //
    // Underlying menu background
    gameLoopUpdateAndDrawFixedMenuBackground(deltaTimeMs, elapsedUs);

    //
    // Game background
    gameLoopUpdateAndDrawBackground(deltaTimeMs, gameBackgroundView);

    //
    // Draw bubbles (separate batch to avoid showing in minimap and for shader support)
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
        sf::Vec2f p_max{pt->getMapLimit() + resolution.x, resolution.y};


        cpuCloudDrawableBatch.add(sf::RectangleShapeData{
            .position  = p_min.addX(10.f),
            .fillColor = sf::Color::White,
            .size      = p_max - p_min,
        });

        // 4. Draw the Gradient (Top-Left, Top-Right, Bottom-Right, Bottom-Left)
        // draw_list->AddRectFilledMultiColor(p_min, p_max, col_top, col_top, col_bot, col_bot);

        // float rounding = 8.f;
        // draw_list->AddRectFilled(p_min, p_max, col_top, rounding, ImDrawFlags_RoundCornersAll);

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

    //
    // Combo trail
    gameLoopDrawCursorTrail(mousePos);

    //
    // Draw minimap stuff
    gameLoopDrawMinimapIcons();

    //
    // Draw cats, shrines, dolls, particles, and text particles
    cpuTopCloudDrawableBatch.clear();
    cpuDrawableBatch.clear();
    cpuDrawableBatchAdditive.clear();
    cpuTopDrawableBatch.clear();
    catTextDrawableBatch.clear();
    catTextTopDrawableBatch.clear();
    cpuCloudHudDrawableBatch.clear();

    // Draw multipop range
    if (pt->multiPopEnabled && draggedCats.empty())
    {
        const auto range = pt->psvPPMultiPopRange.currentValue() * 0.9f;

        cpuDrawableBatch.add(sf::CircleShapeData{
            .position           = mousePos,
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
    gameLoopDrawCats(mousePos, deltaTimeMs);
    gameLoopDrawShrines(mousePos);
    gameLoopDrawDolls(mousePos);
    gameLoopDrawParticles();
    gameLoopDrawTextParticles();
    gameLoopDisplayCloudBatch(cpuCloudDrawableBatch, gameView);
    drawBatch(cpuDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(cpuDrawableBatchAdditive,
              {.blendMode = sf::BlendAdd, .view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(catTextDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});

    //
    // Scroll arrow hint
    gameLoopDrawScrollArrowHint(deltaTimeMs);

    //
    // AoE Dragging Reticle
    if (const auto dragRect = getAoEDragRect(mousePos); dragRect.hasValue())
        rtGame.draw(sf::RectangleShapeData{.position         = dragRect->position,
                                           .origin           = {0.f, 0.f},
                                           .fillColor        = sf::Color::whiteMask(64u),
                                           .outlineColor     = sf::Color::whiteMask(176u),
                                           .outlineThickness = 4.f,
                                           .size             = dragRect->size},
                    {.view = nonScaledHUDView});

    //
    // Y coordinate below minimap to position money, combo, and buff texts
    const float yBelowMinimap = pt->mapPurchased ? (boundaries.y / profile.minimapScale) + 12.f : 0.f;

    //
    // Draw border around gameview
    // Bottom-level hud particles
    if (shouldDrawUI)
    {
        sf::Vec2f offset{-10.f, -10.f};
        sf::Vec2f mins{10.f, 10.f};
        sf::Vec2f maxs{20.f + moneyText.getString().getSize() * 15.f, pt->comboPurchased ? 60.f : 40.f};

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

    if (0)
        rtGame.draw(sf::RectangleShapeData{.position         = gameView.viewport.position.componentWiseMul(resolution),
                                           .fillColor        = sf::Color::Transparent,
                                           .outlineColor     = outlineHueColor,
                                           .outlineThickness = 4.f,
                                           .size             = gameView.viewport.size.componentWiseMul(resolution)},
                    {.view = nonScaledHUDView});

    if (shouldDrawUI)
    {
        hudDrawableBatch.clear();

        if (!debugHideUI)
            gameLoopDrawHUDParticles();

        gameLoopDrawEarnedCoinParticles();
        drawBatch(hudDrawableBatch, {.view = scaledHUDView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    }


    //
    // Demo text (TODO P2: cleanup)
    if constexpr (isDemoVersion)
    {
        const float xStartOverlay = gameView.viewport.size.x * resolution.x / profile.hudScale;

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

    //
    // Money text & spent money effect
    gameLoopUpdateMoneyText(deltaTimeMs, yBelowMinimap);
    gameLoopUpdateSpentMoneyEffect(deltaTimeMs); // handles both text smoothly doing down and particles


    //
    // Combo text
    gameLoopUpdateComboText(deltaTimeMs, yBelowMinimap);

    //
    // Portal storm buff
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
                .catIdx   = 100'000u, // invalid
            });

            sounds.makeBomb.settings.position = {portalPos.x, portalPos.y};
            playSound(sounds.portalon);
        }
    }

    //
    // Buff text
    gameLoopUpdateBuffText();

    if (!buffText.getString().isEmpty())
    {
        const sf::Vec2f offset{10.f, 10.f};

        auto mins = buffText.getGlobalTopLeft() + offset;
        auto maxs = buffText.getGlobalBottomRight() - offset - sf::Vec2{0.f, 20.f};

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

    //
    // Combo bar
    if (shouldDrawUI && !debugHideUI)
    {
        if (comboCountdown.value > 25.f)
            rtGame.draw(
                sf::RoundedRectangleShapeData{
                    .position     = {comboText.getGlobalCenterRight().x + 3.f, yBelowMinimap + 51.f},
                    .fillColor    = sf::Color{75, 75, 75, 255},
                    .size         = {100.f * comboCountdown.value / 700.f, 20.f},
                    .cornerRadius = 6.f,
                },
                {.view = scaledHUDView});
    }

    //
    // Minimap
    if (!debugHideUI && pt->mapPurchased)
        drawMinimap(/* back */ true, rtGame, scaledHUDView, resolution, shouldDrawUIAlpha);

    // UI clouds
    gameLoopDisplayCloudBatch(cpuCloudUiDrawableBatch, nonScaledHUDView);
    gameLoopDisplayCloudBatch(cpuCloudHudDrawableBatch, scaledHUDView);

    if (!debugHideUI && pt->mapPurchased)
    {
        drawMinimap(/* back */ false, rtGame, scaledHUDView, resolution, shouldDrawUIAlpha);

        // Jump to minimap position on click
        const auto p = scaledHUDView.screenToWorld(windowSpaceMouseOrFingerPos.toVec2f(), window.getSize().toVec2f());

        if (minimapRect.contains(p) && mBtnDown(sf::Mouse::Button::Left, /* penetrateUI */ true))
        {
            const auto minimapPos = p - minimapRect.position;
            scroll = minimapPos.x * 0.5f * pt->getMapLimit() / minimapRect.size.x - gameView.size.x * 0.25f;
        }
    }

    if (!debugHideUI)
    {
        moneyText.setFillColorAlpha(shouldDrawUIAlpha);
        moneyText.setOutlineColorAlpha(shouldDrawUIAlpha);
        rtGame.draw(moneyText, {.view = scaledHUDView});
    }

    if (!debugHideUI && pt->comboPurchased)
    {
        comboText.setFillColorAlpha(shouldDrawUIAlpha);
        comboText.setOutlineColorAlpha(shouldDrawUIAlpha);
        rtGame.draw(comboText, {.view = scaledHUDView});
    }

    if (!debugHideUI)
    {
        buffText.setFillColorAlpha(shouldDrawUIAlpha);
        buffText.setOutlineColorAlpha(shouldDrawUIAlpha);
        rtGame.draw(buffText, {.view = scaledHUDView});
    }

    //
    // UI and Toasts
    gameLoopDrawImGui(shouldDrawUIAlpha);

    //
    // Draw cats on top of UI
    gameLoopDisplayCloudBatch(cpuTopCloudDrawableBatch, scaledTopGameView);
    drawBatch(cpuTopDrawableBatch, {.view = scaledTopGameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(catTextTopDrawableBatch,
              {.view = scaledTopGameView, .texture = &textureAtlas.getTexture(), .shader = &shader});

    //
    // Purchase unlocked/available effects
    if (shouldDrawUI)
        gameLoopUpdatePurchaseUnlockedEffects(deltaTimeMs);

    // Top-level hud particles
    if (shouldDrawUI)
    {
        hudTopDrawableBatch.clear();
        gameLoopDrawHUDTopParticles();
        drawBatch(hudTopDrawableBatch,
                  {.view = nonScaledHUDView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    }

    //
    // High visibility cursor
    gameLoopDrawCursor(deltaTimeMs, cursorGrow);
    gameLoopDrawCursorComboText(deltaTimeMs, cursorGrow);
    gameLoopDrawCursorComboBar();

    //
    // Splash screen
    if (splashCountdown.value > 0.f)
        drawSplashScreen(rtGame, scaledHUDView, resolution, profile.hudScale);

    //
    // Letter
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

            rtGame.draw(sf::Sprite{.position    = resolution / 2.f / profile.hudScale,
                                   .scale       = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(progress)) /
                                                  profile.hudScale * 2.f,
                                   .origin      = txLetter.getSize().toVec2f() / 2.f,
                                   .textureRect = txLetter.getRect(),
                                   .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(progress) * 255.f))},
                        {.view = scaledHUDView, .texture = &txLetter});
        }

        (void)cdLetterText.updateAndStop(deltaTimeMs);

        const float textProgress = cdLetterText.value > 9000.f   ? remap(cdLetterText.value, 9000.f, 10'000.f, 1.f, 0.f)
                                   : cdLetterText.value < 1000.f ? cdLetterText.value / 1000.f
                                                                 : 1.f;

        rtGame.draw(sf::Sprite{.position    = resolution / 2.f / profile.hudScale,
                               .scale       = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(textProgress)) /
                                              profile.hudScale * 1.45f,
                               .origin      = txLetterText.getSize().toVec2f() / 2.f,
                               .textureRect = txLetterText.getRect(),
                               .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(textProgress) * 255.f))},
                    {.view = scaledHUDView, .texture = &txLetterText});
    }

    //
    // Doll on screen particle border
    if (!pt->dolls.empty())
        gameLoopDrawDollParticleBorder(0.f);

    if (!pt->copyDolls.empty())
        gameLoopDrawDollParticleBorder(180.f);

    //
    // Tips
    gameLoopTips(deltaTimeMs);

    //
    // Reminders
    gameLoopPrestigeAvailableReminder(); // produces notification as well
    gameLoopReminderBuyCombo();          // also handles secret combo purchase achievement
    gameLoopReminderSpendPPs();

    //
    // Update sampler
    gameLoopUpdateDpsSampler(elapsedUs);

    //
    // Notification queue
    if (shouldDrawUI)
        gameLoopUpdateNotificationQueue(deltaTimeMs);

    //
    // Display window
    if (flushBeforeDisplay)
        rtGame.flushGPUCommands();

    if (finishBeforeDisplay)
        rtGame.finishGPUCommands();

    rtGame.display();

    if (flushAfterDisplay)
        rtGame.flushGPUCommands();

    if (finishAfterDisplay)
        rtGame.finishGPUCommands();

    window.clear();

    auto gameViewNoScroll   = gameView;
    gameViewNoScroll.center = getViewCenterWithoutScroll(); // TODO P1: view::withcenter? like vecs

    {
        const float ratio         = resolution.x / 1250.f;
        const float fixedBgScroll = txFixedBg.getSize().toVec2f().x * 0.5f * sf::base::remainder(fixedBgSlide, 3.f);

        window.draw(txFixedBg,
                    {
                        .position    = {0.f, 0.f},
                        .scale       = {ratio, ratio},
                        .textureRect = {{fixedBgScroll - actualScroll / 20.f, 0.f},
                                        {resolution.x / ratio, resolution.y / ratio}},
                        .color       = sf::Color::White,
                    },
                    {.view = nonScaledHUDView});
    }

    window.draw(rtBackgroundProcessed.getTexture(),
                {.textureRect{{0.f, 0.f}, gameViewNoScroll.size}},
                {.view = gameViewNoScroll});

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
        rtGame.flushGPUCommands();

    if (finishBeforeDisplay)
        rtGame.finishGPUCommands();

    {
        SFEX_PROFILE_SCOPE("window.display()");
        window.display();
    }

    if (flushAfterDisplay)
        rtGame.flushGPUCommands();

    if (finishAfterDisplay)
        rtGame.finishGPUCommands();

    //
    // Save last mouse pos
    lastMousePos = mousePos;

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
