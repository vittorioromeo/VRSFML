

#include "Aliases.hpp"
#include "Bubble.hpp"
#include "BubbleIdleMain.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "Collision.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "Doll.hpp"
#include "HellPortal.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "Shrine.hpp"
#include "ShrineType.hpp"

#include "ExampleUtils/ControlFlow.hpp"
#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/HueColor.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/Profiler.hpp"
#include "ExampleUtils/Timer.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Algorithm/AnyOf.hpp"
#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToString.hpp"

#include <climits>
#include <cstdio>

////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCombo(const float                         deltaTimeMs,
                               const bool                          anyBubblePoppedByClicking,
                               const sf::Vec2f                     mousePos,
                               const sf::base::Optional<sf::Vec2f> clickPosition)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    // Mousecat combo
    checkComboEnd(deltaTimeMs, pt->mouseCatCombo, pt->mouseCatComboCountdown);

    // Combo failure countdown for red text effect
    (void)comboState.comboFailCountdown.updateAndStop(deltaTimeMs);

    // Player combo data
    const auto playerLastCombo      = comboState.combo;
    bool       playerJustEndedCombo = false;

    // Player combo failure due to timer end
    if (checkComboEnd(deltaTimeMs, comboState.combo, comboState.comboCountdown))
        playerJustEndedCombo = true;


    // Player combo failure due to missed click
    if (!anyBubblePoppedByClicking && clickPosition.hasValue() && !pt->laserPopEnabled)
    {
        if (comboState.combo > 1)
        {
            playSound(sounds.scratch);
            comboState.comboFailCountdown.value = 250.f;
        }

        comboState.combo                = 0;
        comboState.comboCountdown.value = 0.f;

        playerJustEndedCombo = true;
    }

    if (playerJustEndedCombo)
    {
        if (playerLastCombo > 2)
        {
            comboState.comboAccReward = static_cast<int>(sf::base::pow(static_cast<float>(comboState.comboNOthers), 1.25f));
            comboState.comboAccStarReward = comboState.comboNStars;
        }
        else
        {
            comboState.comboAccReward     = 0;
            comboState.comboAccStarReward = 0;
        }

        comboState.iComboAccReward     = 0;
        comboState.iComboAccStarReward = 0;
        comboState.comboNStars         = 0;
        comboState.comboNOthers        = 0;
    }

    if (profile.accumulatingCombo)
    {
        if (comboState.iComboAccReward < comboState.comboAccReward &&
            comboState.accComboDelay.updateAndLoop(deltaTimeMs, 35.f) == CountdownStatusLoop::Looping)
        {
            ++comboState.iComboAccReward;
            comboState.accComboDelay.value = 35.f;

            if (spawnEarnedCoinParticle(fromWorldToHud(mousePos)))
            {
                earnedCoinParticles.back().startPosition += rngFast.getVec2f({-25.f, -25.f}, {25.f, 25.f});

                const sf::Vec2f viewSize           = getCurrentGameViewSize();
                const sf::Vec2f viewCenter         = getViewCenter();
                sounds.coindelay.settings.position = {viewCenter.x - viewSize.x / 2.f + 25.f,
                                                      viewCenter.y - viewSize.y / 2.f + 25.f};
                sounds.coindelay.settings.pitch    = 0.8f + static_cast<float>(comboState.iComboAccReward) * 0.04f;
                sounds.coindelay.settings.volume   = profile.sfxVolume / 100.f;

                playSound(sounds.coindelay, /* maxOverlap */ 64);
            }
        }

        if (comboState.iComboAccStarReward < comboState.comboAccStarReward &&
            comboState.accComboStarDelay.updateAndLoop(deltaTimeMs, 75.f) == CountdownStatusLoop::Looping)
        {
            ++comboState.iComboAccStarReward;

            sounds.shine3.settings.position = {mousePos.x, mousePos.y};
            sounds.shine3.settings.pitch    = 0.75f + static_cast<float>(comboState.iComboAccStarReward) * 0.075f;
            playSound(sounds.shine3);

            spawnParticle(ParticleData{.position      = mousePos,
                                       .velocity      = {0.f, 0.f},
                                       .scale         = rngFast.getF(0.08f, 0.27f) * 1.f,
                                       .scaleDecay    = 0.f,
                                       .accelerationY = -0.002f,
                                       .opacity       = 1.f,
                                       .opacityDecay  = rngFast.getF(0.00025f, 0.002f),
                                       .rotation      = rngFast.getF(0.f, sf::base::tau),
                                       .torque        = rngFast.getF(-0.002f, 0.002f)},
                          0.f,
                          ParticleType::Star);
        }
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCollisionsBubbleBubble(const float deltaTimeMs)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    auto func = [&](const SizeT bubbleIdxI, const SizeT bubbleIdxJ) SFML_BASE_LAMBDA_ALWAYS_INLINE
    {
        // TODO P2: technically this is a data race
        handleBubbleCollision(deltaTimeMs, pt->bubbles[bubbleIdxI], pt->bubbles[bubbleIdxJ]);
    };

    const sf::base::SizeT nWorkers = threadPool.getWorkerCount();
    sweepAndPrune.forEachUniqueIndexPair(nWorkers, threadPool, func);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCollisionsCatCat(const float deltaTimeMs)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    for (SizeT i = 0u; i < pt->cats.size(); ++i)
        for (SizeT j = i + 1; j < pt->cats.size(); ++j)
        {
            Cat& iCat = pt->cats[i];
            Cat& jCat = pt->cats[j];

            if (isCatBeingDragged(iCat) || isCatBeingDragged(jCat))
                continue;

            const auto applyAstroInspireAndIgnore = [this](Cat& catA, Cat& catB)
            {
                if (!catA.isAstroAndInFlight())
                    return false;

                if (pt->perm.astroCatInspirePurchased && catB.type != CatType::Astro &&
                    detectCollision(catA.position, catB.position, catA.getRadius(), catB.getRadius()))
                {
                    catB.inspiredCountdown.value = pt->getComputedInspirationDuration();

                    pt->achAstrocatInspireByType[asIdx(catB.type)] = true;
                }

                return true;
            };

            if (applyAstroInspireAndIgnore(iCat, jCat))
                continue;

            // NOLINTNEXTLINE(readability-suspicious-call-argument)
            if (applyAstroInspireAndIgnore(jCat, iCat))
                continue;

            handleCatCollision(deltaTimeMs, pt->cats[i], pt->cats[j]);
        }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCollisionsCatShrine(const float deltaTimeMs) const
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    for (Cat& cat : pt->cats)
    {
        if (cat.isAstroAndInFlight())
            continue;

        if (isCatBeingDragged(cat))
            continue;

        for (Shrine& shrine : pt->shrines)
            handleCatShrineCollision(deltaTimeMs, cat, shrine);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCollisionsCatDoll()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto checkCollisionWithDoll = [&](Doll& d, auto collectFn)
    {
        for (const Cat& cat : pt->cats)
        {
            if (!cat.isAstroAndInFlight())
                continue;

            if (pt->perm.witchCatBuffOrbitalDolls && d.isActive() && !d.tcDeath.hasValue() &&
                detectCollision(cat.position, d.position, cat.getRadius(), d.getRadius()))
            {
                collectFn(d);
            }
        }
    };

    for (Doll& doll : pt->dolls)
        checkCollisionWithDoll(doll, [&](Doll& d) { collectDoll(d); });

    for (Doll& copyDoll : pt->copyDolls)
        checkCollisionWithDoll(copyDoll, [&](Doll& d) { collectCopyDoll(d); });
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCollisionsBubbleHellPortal()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!frameProcThisFrame)
        return;

    const float hellPortalRadius        = pt->getComputedRangeByCatType(CatType::Devil) * 1.25f;
    const float hellPortalRadiusSquared = hellPortalRadius * hellPortalRadius;

    for (const HellPortal& hellPortal : pt->hellPortals)
    {
        Cat* linkedCat = hellPortal.catIdx < pt->cats.size() ? &pt->cats[hellPortal.catIdx] : nullptr;

        forEachBubbleInRadiusSquared(hellPortal.position,
                                     hellPortalRadiusSquared,
                                     [&](Bubble& bubble)
        {
            const MoneyType reward = computeFinalReward(/* bubble     */ bubble,
                                                        /* multiplier */ 50.f,
                                                        /* comboMult  */ 1.f,
                                                        /* popperCat  */ linkedCat);

            statHellPortalRevenue(reward);

            popWithRewardAndReplaceBubble({
                .reward          = reward,
                .bubble          = bubble,
                .xCombo          = 1,
                .popSoundOverlap = rngFast.getF(0.f, 1.f) > 0.75f,
                .popperCat       = linkedCat,
                .multiPop        = false,
            });

            if (linkedCat != nullptr)
                linkedCat->textStatusShakeEffect.bump(rngFast, 1.5f);

            return ControlFlow::Continue;
        });
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateScreenShake(const float deltaTimeMs)
{
    if (screenShakeTimer > 0.f)
    {
        screenShakeTimer -= deltaTimeMs;
        screenShakeTimer = sf::base::max(0.f, screenShakeTimer);
    }

    const bool anyShrineDying = sf::base::anyOf(pt->shrines.begin(), pt->shrines.end(), [](const Shrine& shrine) {
        return shrine.tcDeath.hasValue();
    });

    if (!anyShrineDying && screenShakeTimer <= 0.f && screenShakeAmount > 0.f)
    {
        screenShakeAmount -= deltaTimeMs * 0.05f;
        screenShakeAmount = sf::base::max(0.f, screenShakeAmount);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateParticlesAndTextParticles(const float deltaTimeMs)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto resolution = getResolution();

    const auto updateParticleLike = [&](auto& particleLikeVec)
    {
        for (auto& p : particleLikeVec)
        {
            p.velocity.y += p.accelerationY * deltaTimeMs;
            p.position += p.velocity * deltaTimeMs;

            p.rotation += p.torque * deltaTimeMs;

            p.opacity = sf::base::clamp(p.opacity - p.opacityDecay * deltaTimeMs, 0.f, 1.f);
            p.scale   = sf::base::max(p.scale - p.scaleDecay * deltaTimeMs, 0.f);
        }

        sf::base::vectorEraseIf(particleLikeVec, [](const auto& particleLike) { return particleLike.opacity <= 0.f; });
    };

    updateParticleLike(particles);
    updateParticleLike(spentCoinParticles);
    updateParticleLike(hudTopParticles);
    updateParticleLike(hudBottomParticles);
    updateParticleLike(textParticles);

    sf::base::vectorEraseIf(spentCoinParticles,
                            [&](const auto& p)
    {
        return p.type == ParticleType::Coin &&
               (p.position.x > (gameView.viewport.size.x * resolution.x) || p.position.x < 0.f);
    });

    for (auto& earnedCoinParticle : earnedCoinParticles)
        (void)earnedCoinParticle.progress.updateForwardAndStop(deltaTimeMs * 0.0015f);

    sf::base::vectorEraseIf(earnedCoinParticles, [&](const auto& p) { return p.progress.isDoneForward(); });
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateSounds(const float deltaTimeMs, const sf::Vec2f mousePos)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

#ifndef BUBBLEBYTE_NO_AUDIO
    const float volumeMult = profile.playAudioInBackground || window.hasFocus() ? 1.f : 0.f;

    listener.position = {sf::base::clamp(mousePos.x, 0.f, pt->getMapLimit()),
                         sf::base::clamp(mousePos.y, 0.f, boundaries.y),
                         0.f};

    listener.volume = profile.masterVolume / 100.f * volumeMult;

    (void)playbackDevice.applyListener(listener);

    auto& optCurrentMusic = getCurrentBGMBuffer();
    auto& optNextMusic    = getNextBGMBuffer();

    if (!bgmTransition.isDone())
    {
        SFML_BASE_ASSERT(optNextMusic.hasValue());

        const auto processMusic = [&](sf::base::Optional<BGMBuffer>& optMusic, const float transitionMult)
        {
            if (!optMusic.hasValue())
                return;

            optMusic->music.setPosition(listener.position);
            optMusic->music.setVolume(profile.musicVolume / 100.f * volumeMult * transitionMult);

            if (soundManager.countPlayingPooled(sounds.prestige) > 0u)
                optMusic->music.setVolume(0.f);
        };

        processMusic(optCurrentMusic, bgmTransition.getInvProgress(1000.f));
        processMusic(optNextMusic, bgmTransition.getProgress(1000.f));

        if (bgmTransition.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
        {
            optCurrentMusic.reset();
            ++currentBGMBufferIdx;
        }
    }
    else
    {
        const auto processMusic = [&](sf::base::Optional<BGMBuffer>& optMusic)
        {
            if (!optMusic.hasValue())
                return;

            optMusic->music.setPosition(listener.position);
            optMusic->music.setVolume(profile.musicVolume / 100.f * volumeMult);

            if (soundManager.countPlayingPooled(sounds.prestige) > 0u)
                optMusic->music.setVolume(0.f);
        };

        processMusic(optCurrentMusic);
        processMusic(optNextMusic);
    }
#else
    (void)deltaTimeMs;
    (void)mousePos;
#endif
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateTimePlayed(const sf::base::I64 elapsedUs)
{
    playedUsAccumulator += elapsedUs;

    while (playedUsAccumulator > 1'000'000)
    {
        playedUsAccumulator -= 1'000'000;
        statSecondsPlayed();
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateAutosave(const sf::base::I64 elapsedUs)
{
    if (inSpeedrunPlaythrough())
        return;

    autosaveUsAccumulator += elapsedUs;

    if (autosaveUsAccumulator >= 180'000'000) // 3 min
    {
        autosaveUsAccumulator = 0;
        sf::cOut() << "Autosaving...\n";
        saveMainPlaythroughToFile();
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateAndDrawFixedMenuBackground(const float deltaTimeMs, const sf::base::I64 elapsedUs)
{
    fixedBgSlideAccumulator += elapsedUs;

    if (fixedBgSlideAccumulator > 60'000'000) // change slide every 60s
    {
        fixedBgSlideAccumulator = 0;

        fixedBgSlideTarget += 1.f;

        if (fixedBgSlideTarget >= 3.f)
            fixedBgSlideTarget = 0.f;
    }

    fixedBgSlide = exponentialApproach(fixedBgSlide, fixedBgSlideTarget, deltaTimeMs, 1000.f);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateAndDrawBackground(const float deltaTimeMs, const sf::View& gameBackgroundView)
{
    static float backgroundScroll = 0.f;
    backgroundScroll += deltaTimeMs * 0.01f;

    rtBackground.clear(outlineHueColor);

    const auto getAlpha = [&](const float mult)
    { return static_cast<sf::base::U8>(profile.backgroundOpacity / 100.f * mult); };

    ////////////////////////////////////////////////////////////
    const sf::Texture* const chunkTx[] = {
        &txBackgroundChunk,            // Normal
        &txBackgroundChunkDesaturated, // Voodoo
        &txBackgroundChunk,            // Magic
        &txBackgroundChunkDesaturated, // Clicking
        &txBackgroundChunk,            // Automation
        &txBackgroundChunk,            // Repulsion
        &txBackgroundChunkDesaturated, // Attraction
        &txBackgroundChunk,            // Camouflage
        &txBackgroundChunk,            // Victory
    };

    static_assert(sf::base::getArraySize(chunkTx) == nShrineTypes + 1u);

    ////////////////////////////////////////////////////////////
    const sf::Texture* const detailTx[] = {
        &txClouds,          // Normal
        &txBgSwamp,         // Voodoo
        &txBgObservatory,   // Magic
        &txBgAimTraining,   // Clicking
        &txBgFactory,       // Automation
        &txBgWindTunnel,    // Repulsion
        &txBgMagnetosphere, // Attraction
        &txBgAuditorium,    // Camouflage
        &txClouds,          // Victory
    };

    static_assert(sf::base::getArraySize(detailTx) == nShrineTypes + 1u);

    ////////////////////////////////////////////////////////////
    const auto      idx = profile.selectedBackground;
    const sf::Vec2f chunkScale{1.f, 1.f};
    const sf::Vec2f detailScale{1.f, 1.f};
    const sf::Vec2f chunkTextureRectSize  = gameBackgroundView.size.componentWiseDiv(chunkScale);
    const sf::Vec2f detailTextureRectSize = gameBackgroundView.size.componentWiseDiv(detailScale);

    targetBackgroundHue  = sf::radians(sf::degrees(backgroundHues[idx]).asRadians()).wrapUnsigned();
    currentBackgroundHue = currentBackgroundHue.rotatedTowards(targetBackgroundHue, deltaTimeMs * 0.01f).wrapUnsigned();
    outlineHueColor      = colorBlueOutline.withRotatedHue(currentBackgroundHue.asDegrees());

    rtBackground.draw(*chunkTx[idx],
                      {
                          .scale = chunkScale,
                          .textureRect = {{playerInputState.actualScroll + backgroundScroll * 0.25f, 0.f}, chunkTextureRectSize},
                          .color = hueColor(currentBackgroundHue.asDegrees(), getAlpha(255.f)),
                      },
                      {.view = gameBackgroundView, .shader = &shader});


    static float firstCloudTimer       = 0.f;
    const float  firstCloudTimerTarget = pt->comboPurchased ? 1.f : 0.f;

    firstCloudTimer = exponentialApproach(firstCloudTimer, firstCloudTimerTarget, deltaTimeMs, 1000.f);

    rtBackground.draw(txClouds,
                      {
                          .scale       = {detailScale.x, detailScale.y},
                          .origin      = {0.f, 0.f},
                          .textureRect = {{playerInputState.actualScroll * 1.5f + backgroundScroll * 1.5f, 0.f},
                                          detailTextureRectSize},
                          .color       = sf::Color::whiteWithAlpha(getAlpha(255.f * easeInOutSine(firstCloudTimer))),
                      },
                      {.view = gameBackgroundView});

    static float firstDrawingTimer       = 0.f;
    const float  firstDrawingTimerTarget = pt->getCatCountByType(CatType::Normal) > 0 ? 1.f : 0.f;

    firstDrawingTimer = exponentialApproach(firstDrawingTimer, firstDrawingTimerTarget, deltaTimeMs, 1000.f);

    if (idx == 0u || profile.alwaysShowDrawings)
        rtBackground.draw(txDrawings,
                          {
                              .textureRect = {{playerInputState.actualScroll * 2.f, 0.f},
                                              txBackgroundChunk.getSize().toVec2f() * 2.f},
                              .color = sf::Color::whiteWithAlpha(getAlpha(200.f * easeInOutSine(firstDrawingTimer))),
                          },
                          {.view = gameBackgroundView});


    if (idx != 0u)
        rtBackground.draw(*detailTx[idx],
                          {
                              .scale       = detailScale,
                              .textureRect = {{playerInputState.actualScroll * 2.f + backgroundScroll * 0.5f, 0.f},
                                              detailTextureRectSize},
                              .color       = sf::Color::whiteWithAlpha(getAlpha(190.f)),
                          },
                          {.view = gameBackgroundView});

    rtBackground.display();
    updateProcessedBackground();
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateMoneyText(const float deltaTimeMs, const float yBelowMinimap)
{
    moneyText.setString("$" + sf::base::String(toStringWithSeparators(pt->money + spentMoney)));

    moneyText.setOutlineColor(outlineHueColor);
    moneyText.scale  = {0.5f, 0.5f};
    moneyText.origin = moneyText.getLocalBounds().size / 2.f;

    moneyText.setGlobalTopLeft({10.f, 70.f});
    moneyTextShakeEffect.update(deltaTimeMs);
    moneyTextShakeEffect.applyToText(moneyText);
    moneyText.scale *= 0.5f;

    moneyText.position.y = yBelowMinimap + 25.f;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateSpentMoneyEffect(const float deltaTimeMs)
{
    if (spentMoney == 0u || spentMoneyTimer.updateForwardAndLoop(deltaTimeMs * 0.08f) == TimerStatusLoop::Running)
        return;

    if (profile.showCoinParticles)
    {
        playSound(sounds.coin, /* maxOverlap */ 64);

        spawnSpentCoinParticle(
            {.position      = moneyText.getGlobalCenterRight().addY(rngFast.getF(-12.f, 12.f)),
             .velocity      = sf::Vec2f{3.f, 0.f},
             .scale         = 0.35f,
             .scaleDecay    = 0.f,
             .accelerationY = 0.f,
             .opacity       = 0.f,
             .opacityDecay  = -0.015f,
             .rotation      = rngFast.getF(0.f, sf::base::tau),
             .torque        = 0.f});
    }

    if (spentMoney > 5u)
    {
        const auto spentMoneyAsFloat = static_cast<float>(spentMoney);
        spentMoney -= static_cast<MoneyType>(sf::base::max(1.f, sf::base::ceil(spentMoneyAsFloat / 10.f)));
    }
    else
    {
        --spentMoney;
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateComboText(const float deltaTimeMs, const float yBelowMinimap)
{
    if (!pt->comboPurchased)
        return;

    comboState.comboText.setString("x" + sf::base::toString(comboState.combo + 1));
    comboState.comboText.setOutlineColor(outlineHueColor);

    comboState.comboTextShakeEffect.update(deltaTimeMs);
    comboState.comboTextShakeEffect.applyToText(comboState.comboText);
    comboState.comboText.scale *= 0.5f;

    comboState.comboText.position.y = yBelowMinimap + 45.f;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateBuffText()
{
    const char* devilBuffName = (isDevilcatHellsingedActive()) ? "Portal Storm (Scales With Bomb Spawn Chance)"
                                                               : "Explosive Downpour (Bomb Spawn Chance)";

    const char* const buffNames[] = {
        "Midas Paws (x5 Cat Reward)",          // Normal
        "Shooting Stars (Star Spawn Chance) ", // Uni
        devilBuffName,                         // Devil
        "Endless Flight (Looping Astrocats)",  // Astro

        "N/A",                                         // Witch
        "Mana Overload (x3.5 Mana Regen)",             // Wizard
        "Click Fever (x10 Click Reward)",              // Mouse
        "Global Maintenance (x2 Faster Cooldown)",     // Engi
        "Bubble Hurricane (x2 Bubble Count + Wind)",   // Repulso
        "Demonic Attraction (Magnetic Bombs/Portals)", // Attracto
        "N/A",                                         // Copy
        "N/A",                                         // Duck
    };

    static_assert(sf::base::getArraySize(buffNames) == nCatTypes);

    char  buffStrBuffer[1024]{};
    SizeT writeIdx = 0u;

    const SizeT nDollsToClick = sf::base::countIf(pt->dolls.begin(), pt->dolls.end(), [](const Doll& doll) {
        return !doll.tcDeath.hasValue();
    });

    const SizeT nCopyDollsToClick = sf::base::countIf(pt->copyDolls.begin(), pt->copyDolls.end(), [](const Doll& doll) {
        return !doll.tcDeath.hasValue();
    });

    if (nDollsToClick > 0u)
        writeIdx += static_cast<SizeT>(
            std::snprintf(buffStrBuffer + writeIdx, sizeof(buffStrBuffer) - writeIdx, "Dolls to collect: %zu\n", nDollsToClick));

    if (nCopyDollsToClick > 0u)
        writeIdx += static_cast<SizeT>(
            std::snprintf(buffStrBuffer + writeIdx,
                          sizeof(buffStrBuffer) - writeIdx,
                          "Dolls (copy) to collect: %zu\n",
                          nCopyDollsToClick));

    if (pt->mewltiplierAuraTimer > 0.f)
        writeIdx += static_cast<SizeT>(
            std::snprintf(buffStrBuffer + writeIdx,
                          sizeof(buffStrBuffer) - writeIdx,
                          "Mewltiplier Aura (x%.1f Any Reward): %.2fs\n",
                          static_cast<double>(pt->psvMewltiplierMult.currentValue()),
                          static_cast<double>(pt->mewltiplierAuraTimer / 1000.f)));

    if (pt->stasisFieldTimer > 0.f)
        writeIdx += static_cast<SizeT>(
            std::snprintf(buffStrBuffer + writeIdx,
                          sizeof(buffStrBuffer) - writeIdx,
                          "Stasis Field (Bubbles Stuck In Time): %.2fs\n",
                          static_cast<double>(pt->stasisFieldTimer / 1000.f)));

    for (SizeT i = 0u; i < nCatTypes; ++i)
    {
        const float buffTime = pt->buffCountdownsPerType[i].value;

        if (buffTime == 0.f)
            continue;

        writeIdx += static_cast<SizeT>(
            std::snprintf(buffStrBuffer + writeIdx,
                          sizeof(buffStrBuffer) - writeIdx,
                          "%s: %.2fs\n",
                          buffNames[i],
                          static_cast<double>(buffTime / 1000.f)));
    }

    comboState.buffText.setString(buffStrBuffer);
    comboState.buffText.setOutlineColor(outlineHueColor);

    comboState.buffText.position.y = comboState.comboText.getGlobalBottomLeft().y + 10.f;
    comboState.buffText.scale      = {0.5f, 0.5f};
}


////////////////////////////////////////////////////////////
void Main::gameLoopPrestigeAvailableReminder()
{
    if (!wasPrestigeAvailableLastFrame && pt->canBuyNextPrestige())
    {
        pushNotification("Prestige available!", "Purchase through the \"Prestige\" menu!");

        if (pt->psvBubbleValue.nPurchases == 0u)
            doTip("You can now prestige for the first time!");
    }

    wasPrestigeAvailableLastFrame = pt->canBuyNextPrestige();
}


////////////////////////////////////////////////////////////
void Main::gameLoopReminderBuyCombo()
{
    if (pt->comboPurchased || inPrestigeTransition)
        return;

    const auto handPoppedBubbles = pt->statsSession.getTotalNBubblesHandPopped();

    if (handPoppedBubbles >= 25u && buyReminder == 0)
    {
        buyReminder = 1;
        doTip("Remember to buy the combo upgrade!", /* maxPrestigeLevel */ UINT_MAX);
    }
    else if (handPoppedBubbles >= 50u && buyReminder == 1)
    {
        buyReminder = 2;
        doTip("You should really buy the upgrade now!", /* maxPrestigeLevel */ UINT_MAX);
    }
    else if (handPoppedBubbles >= 100u && buyReminder == 2)
    {
        buyReminder = 3;
        doTip("What are you trying to prove...?", /* maxPrestigeLevel */ UINT_MAX);
    }
    else if (handPoppedBubbles >= 200u && buyReminder == 3)
    {
        buyReminder = 4;
        doTip("There is no achievement for doing this!", /* maxPrestigeLevel */ UINT_MAX);
    }
    else if (handPoppedBubbles >= 300u && buyReminder == 4)
    {
        buyReminder = 5;
        doTip("Fine, have it your way!\nHere's your dumb achievement!\nAnd now buy the upgrade!",
              /* maxPrestigeLevel */ UINT_MAX);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopReminderSpendPPs()
{
    if (!pt->spendPPTipShown && pt->psvBubbleValue.nPurchases == 1u && pt->prestigePoints > 0u && pt->money > 500u)
    {
        pt->spendPPTipShown = true;
        doTip("Remember that you have some\nprestige points to spend!");
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateDpsSampler(const sf::base::I64 elapsedUs)
{
    moneyGainedUsAccumulator += elapsedUs;
    while (moneyGainedUsAccumulator >= 1'000'000)
    {
        moneyGainedUsAccumulator -= 1'000'000;

        samplerMoneyPerSecond.record(static_cast<float>(moneyGainedLastSecond));
        moneyGainedLastSecond = 0u;

        statHighestDPS(static_cast<sf::base::U64>(samplerMoneyPerSecond.getAverageAs<double>()));
    }
}
