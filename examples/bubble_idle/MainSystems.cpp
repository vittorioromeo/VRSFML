

#include "Aliases.hpp"
#include "Bubble.hpp"
#include "BubbleIdleMain.hpp"
#include "BubbleType.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "Doll.hpp"
#include "HellPortal.hpp"
#include "HexSession.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "Shrine.hpp"
#include "ShrineType.hpp"
#include "Stats.hpp"

#include "ExampleUtils/ControlFlow.hpp"
#include "ExampleUtils/HueColor.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/Profiler.hpp"
#include "ExampleUtils/Timer.hpp"

#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Window/Keyboard.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/Algorithm/AllOf.hpp"
#include "SFML/Base/Algorithm/AnyOf.hpp"
#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Algorithm/MaxElement.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include <utility>

#include <cstdio>

////////////////////////////////////////////////////////////
void Main::gameLoopUpdateScrolling(const float deltaTimeMs, const sf::base::Vector<sf::Vec2f>& downFingers)
{
    // Reset map scrolling
    if (keyDown(sf::Keyboard::Key::LShift) || (downFingers.size() != 2u && !mBtnDown(getRMB(), /* penetrateUI */ true)))
        playerInputState.dragPosition.reset();

    //
    // Scrolling
    playerInputState
        .scroll = sf::base::clamp(playerInputState.scroll,
                                  0.f,
                                  sf::base::max(0.f,
                                                sf::base::min(pt->getMapLimit() / 2.f - getCurrentGameViewSize().x / 2.f,
                                                              (boundaries.x - getCurrentGameViewSize().x) / 2.f)));

    playerInputState.actualScroll = exponentialApproach(playerInputState.actualScroll, playerInputState.scroll, deltaTimeMs, 75.f);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateTransitions(const float deltaTimeMs)
{
    // Compute screen count
    constexpr auto nMaxScreens       = boundaries.x / gameScreenSize.x;
    const auto     nPurchasedScreens = static_cast<SizeT>(pt->getMapLimit() / gameScreenSize.x) + 1u;

    // Compute total target bubble count
    const auto targetBubbleCountPerScreen = static_cast<SizeT>(pt->psvBubbleCount.currentValue() / nMaxScreens);

    auto targetBubbleCount = targetBubbleCountPerScreen * nPurchasedScreens;

    const bool repulsoBuffActive = pt->buffCountdownsPerType[asIdx(CatType::Repulso)].value > 0.f;

    if (repulsoBuffActive)
        targetBubbleCount *= 2u;

    // Helper functions
    const auto playReversePopAt = [this](const sf::Vec2f position)
    {
        // TODO P2: refactor into function for any sound and reuse
        sounds.reversePop.settings.position = {position.x, position.y};
        playSound(sounds.reversePop, /* maxOverlap */ 1u);
    };

    // If we are still displaying the splash screen, exit early
    if (splashCountdown.updateAndStop(deltaTimeMs) != CountdownStatusStop::AlreadyFinished)
        return;

    // Spawn bubbles and shrines during normal gmaeplay
    if (!inPrestigeTransition)
    {
        // Spawn shrines if required
        pt->spawnAllShrinesIfNeeded();

        // Spawn bubbles (or remove extra bubbles via debug menu)
        if (pt->bubbles.size() < targetBubbleCount)
        {
            const SizeT times = (targetBubbleCount - pt->bubbles.size()) > 500u ? 25u : 1u;

            for (SizeT i = 0; i < times; ++i)
            {
                auto& bubble    = pt->bubbles.emplaceBack(makeRandomBubble(*pt, rng, pt->getMapLimit(), boundaries.y));
                const auto bPos = bubble.position;

                if (repulsoBuffActive)
                    bubble.velocity += {0.18f, 0.18f};

                spawnParticles(8, bPos, ParticleType::Bubble, 0.5f, 0.5f);
                playReversePopAt(bPos);
            }
        }
        else if (pt->bubbles.size() > targetBubbleCount)
        {
            const SizeT times = (pt->bubbles.size() - targetBubbleCount) > 500u ? 25u : 1u;

            for (SizeT i = 0; i < times; ++i)
            {
                const auto bPos = pt->bubbles.back().position;
                pt->bubbles.popBack();

                spawnParticles(8, bPos, ParticleType::Bubble, 0.5f, 0.5f);
                playReversePopAt(bPos);
            }
        }

        return;
    }

    SFML_BASE_ASSERT(inPrestigeTransition);

    // Despawn cats, dolls, copydolls, and shrines
    if (catRemoveTimer.updateAndLoop(deltaTimeMs) == CountdownStatusLoop::Looping)
    {
        if (!pt->cats.empty())
        {
            for (auto& cat : pt->cats)
            {
                cat.astroState.reset();
                cat.cooldown.value = 100.f;
            }

            // Find rightmost cat
            auto* const rightmostIt = sf::base::maxElement(pt->cats.begin(), pt->cats.end(), [](const Cat& a, const Cat& b) {
                return a.position.x < b.position.x;
            });

            const float targetScroll = (rightmostIt->position.x - getCurrentGameViewSize().x / 2.f) / 2.f;
            playerInputState.scroll  = exponentialApproach(playerInputState.scroll, targetScroll, deltaTimeMs, 15.f);

            if (rightmostIt != pt->cats.end())
                std::swap(*rightmostIt, pt->cats.back());

            const auto cPos = pt->cats.back().position;
            pt->cats.popBack();

            spawnParticle({.position      = cPos.addY(29.f),
                           .velocity      = {0.f, 0.f},
                           .scale         = 0.2f,
                           .scaleDecay    = 0.f,
                           .accelerationY = -0.00015f,
                           .opacity       = 1.f,
                           .opacityDecay  = 0.0002f,
                           .rotation      = 0.f,
                           .torque        = rngFast.getF(-0.0002f, 0.0002f)},
                          /* hue */ 0.f,
                          ParticleType::CatSoul);

            spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
            playReversePopAt(cPos);
        }

        if (!pt->shrines.empty())
        {
            const auto cPos = pt->shrines.back().position;
            pt->shrines.popBack();

            spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
            playReversePopAt(cPos);
        }

        const auto popOneDollOrSession = [&](sf::base::Vector<HexSession>& sessions)
        {
            while (!sessions.empty() && sessions.back().dolls.empty())
                sessions.popBack();

            if (sessions.empty())
                return;

            auto&      session = sessions.back();
            const auto cPos    = session.dolls.back().position;
            session.dolls.popBack();

            if (session.dolls.empty())
                sessions.popBack();

            spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
            playReversePopAt(cPos);
        };

        popOneDollOrSession(pt->hexSessions);
        popOneDollOrSession(pt->copyHexSessions);

        if (!pt->hellPortals.empty())
        {
            const auto cPos = pt->hellPortals.back().position;
            pt->hellPortals.popBack();

            spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
            playReversePopAt(cPos);
        }
    }

    const bool gameElementsRemoved = pt->cats.empty() && pt->shrines.empty() && pt->hexSessions.empty() &&
                                     pt->copyHexSessions.empty() && pt->hellPortals.empty();

    // Reset map extension and scroll, and remove bubbles outside of view
    if (gameElementsRemoved)
    {
        pt->mapPurchased               = false;
        pt->psvMapExtension.nPurchases = 0u;

        playerInputState.scroll = 0.f;

        sf::base::vectorEraseIf(pt->bubbles, [&](const Bubble& b) { return b.position.x > pt->getMapLimit() + 128.f; });
    }

    // Despawn bubbles after other things
    if (gameElementsRemoved && !pt->bubbles.empty() &&
        bubbleSpawnTimer.updateAndLoop(deltaTimeMs) == CountdownStatusLoop::Looping)
    {
        const SizeT times = pt->bubbles.size() > 500u ? 25u : 1u;

        for (SizeT i = 0; i < times; ++i)
        {
            const auto bPos = pt->bubbles.back().position;
            pt->bubbles.popBack();

            spawnParticles(8, bPos, ParticleType::Bubble, 0.5f, 0.5f);
            playReversePopAt(bPos);
        }
    }

    // End prestige transition
    if (gameElementsRemoved && pt->bubbles.empty())
    {
        pt->statsSession = Stats{};

        inPrestigeTransition = false;
        pt->money            = pt->perm.starterPackPurchased ? 1000u : 0u;

        resetAllDraggedCats();

        spentMoney               = 0u;
        moneyGainedLastSecond    = 0u;
        moneyGainedUsAccumulator = 0u;
        samplerMoneyPerSecond.clear();
        bombIdxToCatIdx.clear();

        splashCountdown.restart();
        playSound(sounds.byteMeow);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateBubbles(const float deltaTimeMs)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    constexpr float maxVelocityMagnitude  = 2.f;
    constexpr float windMult[4]           = {0.f, 0.00009f, 0.00018f, 0.0005f};
    constexpr float windStartVelocityY[4] = {0.07f, 0.18f, 0.25f, 0.55f};

    for (Bubble& bubble : pt->bubbles)
    {
        if (bubble.velocity.lengthSquared() > maxVelocityMagnitude * maxVelocityMagnitude)
            bubble.velocity = bubble.velocity.normalized() * maxVelocityMagnitude;

        bubble.rotation += deltaTimeMs * bubble.torque;

        if (bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova)
            bubble.hueMod += deltaTimeMs * 0.125f;

        float windVelocity = windMult[pt->windStrength] * (bubble.type == BubbleType::Bomb ? 0.01f : 0.9f);

        if (pt->buffCountdownsPerType[asIdx(CatType::Repulso)].value > 0.f)
            windVelocity += 0.00015f;

        if (windVelocity > 0.f)
        {
            bubble.velocity.x += (windVelocity * 0.5f) * deltaTimeMs;
            bubble.velocity.y += windVelocity * deltaTimeMs;
        }

        if (isBubbleInStasisField(bubble))
            bubble.velocity = {0.f, 0.f};

        bubble.position += bubble.velocity * deltaTimeMs;

        // X-axis wraparound
        if (bubble.position.x - bubble.radius > pt->getMapLimit())
            bubble.position.x = -bubble.radius;
        else if (bubble.position.x + bubble.radius < 0.f)
            bubble.position.x = pt->getMapLimit() + bubble.radius;

        // Y-axis below and above screen
        if (bubble.position.y - bubble.radius > boundaries.y)
        {
            bubble.position.x = rng.getF(0.f, pt->getMapLimit());
            bubble.position.y = -bubble.radius * rng.getF(1.f, 2.f);

            bubble.velocity.y = windStartVelocityY[pt->windStrength];

            if (sf::base::fabs(bubble.velocity.x) > 0.04f)
                bubble.velocity.x = 0.04f;

            const bool uniBuffEnabled       = pt->buffCountdownsPerType[asIdx(CatType::Uni)].value > 0.f;
            const bool devilBombBuffEnabled = !isDevilcatHellsingedActive() &&
                                              pt->buffCountdownsPerType[asIdx(CatType::Devil)].value > 0.f;

            const bool willBeStar = uniBuffEnabled &&
                                    rng.getF(0.f, 100.f) <= pt->psvPPUniRitualBuffPercentage.currentValue();
            const bool willBeBomb = devilBombBuffEnabled &&
                                    rng.getF(0.f, 100.f) <= pt->psvPPDevilRitualBuffPercentage.currentValue();

            const auto starType = isUnicatTranscendenceActive() ? BubbleType::Nova : BubbleType::Star;

            if (!willBeStar && !willBeBomb)
                turnBubbleInto(bubble, BubbleType::Normal);
            else if (willBeBomb && willBeStar)
            {
                turnBubbleInto(bubble, rng.getF(0.f, 1.f) > 0.5f ? starType : BubbleType::Bomb);
            }
            else if (willBeBomb)
            {
                turnBubbleInto(bubble, BubbleType::Bomb);
            }
            else if (willBeStar)
            {
                turnBubbleInto(bubble, starType);
            }
        }
        else if (bubble.position.y + bubble.radius < -128.f)
        {
            turnBubbleInto(bubble, BubbleType::Normal);
        }

        bubble.velocity.y += 0.00005f * deltaTimeMs;

        (void)bubble.repelledCountdown.updateAndStop(deltaTimeMs);
        (void)bubble.attractedCountdown.updateAndStop(deltaTimeMs);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateAttractoBuff(const float deltaTimeMs) const
{
    if (pt->buffCountdownsPerType[asIdx(CatType::Attracto)].value <= 0.f)
        return;

    const auto sqAttractoRange = pt->getComputedSquaredRangeByCatType(CatType::Attracto);
    const auto attractoRange   = SFML_BASE_MATH_SQRTF(sqAttractoRange);

    static thread_local sf::base::Vector<Bubble*> bombs;
    bombs.clear();

    for (Bubble& bubble : pt->bubbles)
        if (bubble.type == BubbleType::Bomb)
            bombs.pushBack(&bubble);

    const auto attract = [&](const sf::Vec2f pos, Bubble& bubble)
    {
        const auto diff     = (pos - bubble.position);
        const auto sqLength = diff.lengthSquared();

        if (sqLength > sqAttractoRange)
            return;

        const float length = SFML_BASE_MATH_SQRTF(sqLength);

        const auto strength = (attractoRange - length) * 0.000017f;
        bubble.velocity += (diff / length * strength * getWindAttractionMult()) * 1.f * deltaTimeMs;

        bubble.attractedCountdown.value = sf::base::max(bubble.attractedCountdown.value, 750.f);
    };

    for (Bubble& bubble : pt->bubbles)
    {
        if (bubble.type == BubbleType::Bomb)
            continue;

        for (const HellPortal& hp : pt->hellPortals)
            attract(hp.position, bubble);

        for (const Bubble* bomb : bombs)
            attract(bomb->position, bubble);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::gameLoopUpdateBubbleClick(sf::base::Optional<sf::Vec2f>& clickPosition)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!clickPosition.hasValue())
        return false;

    const auto clickPos = gameView.screenToWorld(*clickPosition, window.getSize().toVec2f());

    if (!particleCullingBoundaries.isInside(clickPos))
    {
        clickPosition.reset();
        return false;
    }

    Bubble* firstClickedBubble           = nullptr;
    bool    firstClickedBubbleByMultiPop = false;

    forEachBubbleInRadius(clickPos,
                          128.f,
                          [&](Bubble& bubble)
    {
        if ((clickPos - bubble.position).lengthSquared() > bubble.getRadiusSquared())
            return ControlFlow::Continue;

        // Prevent clicks around shrine of automation
        for (const Shrine& shrine : pt->shrines)
            if (shrine.type == ShrineType::Automation && shrine.isInRange(clickPos))
            {
                sounds.failpop.settings.position = {clickPos.x, clickPos.y};
                playSound(sounds.failpop);

                return ControlFlow::Break;
            }

        firstClickedBubble = &bubble;
        return ControlFlow::Break;
    });

    if (firstClickedBubble == nullptr && pt->multiPopEnabled && !pt->laserPopEnabled)
        forEachBubbleInRadius(clickPos,
                              pt->psvPPMultiPopRange.currentValue(),
                              [&](Bubble& b)
        {
            firstClickedBubble           = &b;
            firstClickedBubbleByMultiPop = true;

            return ControlFlow::Break;
        });

    if (firstClickedBubble != nullptr)
    {
        if (!firstClickedBubbleByMultiPop && pt->comboPurchased)
        {
            if (!pt->laserPopEnabled)
            {
                addCombo(comboState.combo, comboState.comboCountdown);
                comboState.comboTextShakeEffect.bump(rngFast, 1.f + static_cast<float>(comboState.combo) * 0.2f);
            }
            else
            {
                ++comboState.laserCursorCombo;

                if (comboState.combo == 0 || comboState.laserCursorCombo >= 10)
                {
                    addCombo(comboState.combo, comboState.comboCountdown);
                    comboState.comboTextShakeEffect.bump(rngFast, 0.01f + static_cast<float>(comboState.combo) * 0.002f);

                    comboState.comboCountdown.value = sf::base::min(comboState.comboCountdown.value,
                                                                    pt->psvComboStartTime.currentValue() * 100.f);

                    comboState.combo = sf::base::min(comboState.combo, 998);
                }
            }
        }
        else
        {
            comboState.combo = 1;
        }

        const MoneyType
            reward = computeFinalReward(/* bubble     */ *firstClickedBubble,
                                        /* multiplier */ 1.f,
                                        /* comboMult  */ getComboValueMult(comboState.combo, pt->laserPopEnabled ? playerComboDecayLaser : playerComboDecay),
                                        /* popperCat  */ nullptr);

        popWithRewardAndReplaceBubble({
            .reward          = reward,
            .bubble          = *firstClickedBubble,
            .xCombo          = comboState.combo,
            .popSoundOverlap = true,
            .popperCat       = nullptr,
            .multiPop        = false,
        });

        if (!pt->speedrunStartTime.hasValue())
            pt->speedrunStartTime.emplace(sf::Clock::now());

        if (pt->multiPopEnabled && !pt->laserPopEnabled)
            forEachBubbleInRadius(clickPos,
                                  pt->psvPPMultiPopRange.currentValue(),
                                  [&](Bubble& otherBubble)
            {
                if (&otherBubble == firstClickedBubble)
                    return ControlFlow::Continue;

                popWithRewardAndReplaceBubble({
                    .reward          = reward,
                    .bubble          = otherBubble,
                    .xCombo          = comboState.combo,
                    .popSoundOverlap = false,
                    .popperCat       = nullptr,
                    .multiPop        = true,
                });

                return ControlFlow::Continue;
            });
    }

    return firstClickedBubble != nullptr;
}


void Main::gameLoopUpdateCatActionNormal(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto maxCooldown = pt->getComputedCooldownByCatType(cat.type);
    const auto range       = pt->getComputedRangeByCatType(cat.type);
    const auto [cx, cy]    = getCatRangeCenter(cat);

    const auto normalCatPopBubble = [&](Bubble& bubble)
    {
        cat.pawPosition = bubble.position;
        cat.pawOpacity  = 255.f;
        cat.pawRotation = (bubble.position - cat.position).angle() + sf::degrees(45);

        const float squaredMouseCatRange = pt->getComputedSquaredRangeByCatType(CatType::Mouse);

        const Cat* mouseCat = getMouseCat();
        const Cat* copyCat  = getCopyCat();

        const bool inMouseCatRange = mouseCat != nullptr &&
                                     (mouseCat->position - cat.position).lengthSquared() <= squaredMouseCatRange;

        const bool inCopyMouseCatRange = copyCat != nullptr && pt->copycatCopiedCatType == CatType::Mouse &&
                                         (copyCat->position - cat.position).lengthSquared() <= squaredMouseCatRange;

        const int comboMult = (inMouseCatRange || inCopyMouseCatRange) ? pt->mouseCatCombo : 1;

        const MoneyType reward = computeFinalReward(/* bubble     */ bubble,
                                                    /* multiplier */ 1.f,
                                                    /* comboMult  */ getComboValueMult(comboMult, mouseCatComboDecay),
                                                    /* popperCat  */ &cat);

        popWithRewardAndReplaceBubble({
            .reward          = reward,
            .bubble          = bubble,
            .xCombo          = comboMult,
            .popSoundOverlap = true,
            .popperCat       = &cat,
            .multiPop        = false,
        });

        cat.textStatusShakeEffect.bump(rngFast, 1.5f);
        ++cat.hits;

        cat.cooldown.value = maxCooldown;
    };

    if (!pt->perm.smartCatsPurchased)
    {
        if (Bubble* b = pickRandomBubbleInRadius({cx, cy}, range))
            normalCatPopBubble(*b);

        return;
    }

    const auto pickAny = [&](const auto... types) -> Bubble*
    {
        return pickRandomBubbleInRadiusMatching({cx, cy}, range, [&](const Bubble& b) {
            return (... || (b.type == types));
        });
    };

    if (!pt->perm.geniusCatsPurchased)
    {
        if (Bubble* specialBubble = pickAny(BubbleType::Nova, BubbleType::Star, BubbleType::Bomb))
            normalCatPopBubble(*specialBubble);
        else if (Bubble* b = pickRandomBubbleInRadius({cx, cy}, range))
            normalCatPopBubble(*b);

        return;
    }

    if (Bubble* bBomb = pickAny(BubbleType::Bomb); bBomb != nullptr && !pt->geniusCatIgnoreBubbles.bomb)
        normalCatPopBubble(*bBomb);
    else if (Bubble* bNova = pickAny(BubbleType::Nova); bNova != nullptr && !pt->geniusCatIgnoreBubbles.star)
        normalCatPopBubble(*bNova);
    else if (Bubble* bStar = pickAny(BubbleType::Star); bStar != nullptr && !pt->geniusCatIgnoreBubbles.star)
        normalCatPopBubble(*bStar);
    else if (Bubble* bNormal = pickAny(BubbleType::Normal); bNormal != nullptr && !pt->geniusCatIgnoreBubbles.normal)
        normalCatPopBubble(*bNormal);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionUni(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto starBubbleType = isUnicatTranscendenceActive() ? BubbleType::Nova : BubbleType::Star;
    const auto nStarParticles = pt->perm.unicatTranscendenceAOEPurchased ? 1u : 4u;

    const auto transformBubble = [&](Bubble& bToTransform)
    {
        turnBubbleInto(bToTransform, starBubbleType);
        spawnParticles(nStarParticles, bToTransform.position, ParticleType::Star, 0.5f, 0.35f);

        ++cat.hits;
    };

    const auto maxCooldown = pt->getComputedCooldownByCatType(cat.type);
    const auto range       = pt->getComputedRangeByCatType(cat.type);

    if (pt->perm.unicatTranscendenceAOEPurchased)
    {
        Bubble* firstBubble = nullptr;

        forEachBubbleInRadius(getCatRangeCenter(cat),
                              range,
                              [&](Bubble& bubble)
        {
            if (bubble.type != BubbleType::Normal)
                return ControlFlow::Continue;

            if (firstBubble == nullptr)
                firstBubble = &bubble;

            transformBubble(bubble);
            return ControlFlow::Continue;
        });

        if (firstBubble == nullptr)
            return;

        cat.pawPosition = firstBubble->position;
        cat.pawOpacity  = 255.f;
        cat.pawRotation = (firstBubble->position - cat.position).angle() + sf::degrees(45);

        sounds.shine2.settings.position = {firstBubble->position.x, firstBubble->position.y};
        playSound(sounds.shine2);
    }
    else
    {
        Bubble* b = pickRandomBubbleInRadiusMatching(getCatRangeCenter(cat), range, [&](const Bubble& bubble) {
            return bubble.type == BubbleType::Normal;
        });

        if (b == nullptr)
            return;

        transformBubble(*b);

        cat.pawPosition = b->position;
        cat.pawOpacity  = 255.f;
        cat.pawRotation = (b->position - cat.position).angle() + sf::degrees(45);

        sounds.shine.settings.position = {b->position.x, b->position.y};
        playSound(sounds.shine);
    }

    cat.textStatusShakeEffect.bump(rngFast, 1.5f);
    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionDevil(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto maxCooldown = pt->getComputedCooldownByCatType(cat.type);
    const auto range       = pt->getComputedRangeByCatType(cat.type);

    if (!isDevilcatHellsingedActive())
    {
        Bubble* b = pickRandomBubbleInRadius(getCatRangeCenter(cat), range);
        if (b == nullptr)
            return;

        Bubble& bubble = *b;

        // cat.pawPosition = bubble.position;
        cat.pawOpacity  = 255.f;
        cat.pawRotation = (bubble.position - cat.position).angle() - sf::degrees(45);

        turnBubbleInto(bubble, BubbleType::Bomb);

        const auto bubbleIdx = static_cast<sf::base::SizeT>(&bubble - pt->bubbles.data());
        const auto catIdx    = static_cast<sf::base::SizeT>(&cat - pt->cats.data());

        bombIdxToCatIdx[bubbleIdx] = catIdx;

        bubble.velocity.y += rng.getF(0.1f, 0.2f);
        sounds.makeBomb.settings.position = {bubble.position.x, bubble.position.y};
        playSound(sounds.makeBomb);

        spawnParticles(8, bubble.position, ParticleType::Fire, 1.25f, 0.35f);
    }
    else
    {
        const auto portalPos = getCatRangeCenter(cat);

        pt->hellPortals.pushBack({
            .position = portalPos,
            .life     = Countdown{.value = 1750.f},
            .catIdx   = static_cast<sf::base::SizeT>(&cat - pt->cats.data()),
        });

        sounds.makeBomb.settings.position = {portalPos.x, portalPos.y};
        playSound(sounds.portalon);
    }

    cat.textStatusShakeEffect.bump(rngFast, 1.5f);
    ++cat.hits;

    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionAstro(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto [cx, cy] = getCatRangeCenter(cat);

    if (cat.astroState.hasValue() || pt->disableAstrocatFlight)
        return;

    sounds.launch.settings.position = {cx, cy};
    playSound(sounds.launch);

    ++cat.hits;
    cat.astroState.emplace(/* startX */ cat.position.x, /* velocityX */ 0.f, /* wrapped */ false);
    --cat.position.x;
}


////////////////////////////////////////////////////////////
[[nodiscard]] Cat* Main::getSessionTargetCat(const HexSession& session) const
{
    if (session.catIdx >= pt->cats.size())
        return nullptr;

    Cat& cat = pt->cats[session.catIdx];

    // Guard against index aliasing during prestige transition (cats get
    // swap-to-back popped): a live session must point at an actually hexed cat.
    if (!cat.isHexedOrCopyHexed())
        return nullptr;

    return &cat;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::anyCatHexedOrCopyHexed() const
{
    return sf::base::anyOf(pt->cats.begin(), pt->cats.end(), [](const Cat& cat) { return cat.isHexedOrCopyHexed(); });
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::canHexMore() const
{
    return pt->hexSessions.size() < maxConcurrentHexes;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::canCopyHexMore() const
{
    return pt->copyHexSessions.size() < maxConcurrentHexes;
}


////////////////////////////////////////////////////////////
void Main::hexCat(Cat& cat, const SizeT /* catIdx */, const bool copy)
{
    if (isCatBeingDragged(cat))
        stopDraggingCat(cat);

    sounds.soulsteal.settings.position = {cat.position.x, cat.position.y};
    playSound(sounds.soulsteal);

    screenShakeAmount = 3.5f;
    screenShakeTimer  = 600.f;

    (copy ? cat.hexedCopyTimer : cat.hexedTimer).emplace(BidirectionalTimer{.direction = TimerDirection::Forward});

    cat.wobbleRadians = 0.f;

    spawnParticle({.position      = cat.getDrawPosition(profile.enableCatBobbing).addY(29.f),
                   .velocity      = {0.f, 0.f},
                   .scale         = 0.2f,
                   .scaleDecay    = 0.f,
                   .accelerationY = -0.00015f,
                   .opacity       = 1.f,
                   .opacityDecay  = 0.0002f,
                   .rotation      = 0.f,
                   .torque        = rngFast.getF(-0.0002f, 0.0002f)},
                  /* hue */ 0.f,
                  ParticleType::CatSoul);
}


void Main::gameLoopUpdateCatActionWitchImpl(const float /* deltaTimeMs */,
                                            Cat&                          cat,
                                            sf::base::Vector<HexSession>& sessionsToUse,
                                            const SizeT                   nCatsToHex)
{
    const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
    const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

    const bool copy = &sessionsToUse == &pt->copyHexSessions;

    // Collect all eligible candidates in range.
    sf::base::Vector<SizeT> candidateIndices;

    for (SizeT i = 0u; i < pt->cats.size(); ++i)
    {
        const Cat& otherCat = pt->cats[i];

        if (otherCat.type == CatType::Duck)
            continue;

        if (otherCat.type == CatType::Witch)
            continue;

        if (otherCat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch)
            continue;

        if ((otherCat.position - cat.position).length() > range)
            continue;

        if (otherCat.isHexedOrCopyHexed())
            continue;

        candidateIndices.pushBack(i);
    }

    const SizeT otherCatCount = candidateIndices.size();

    if (otherCatCount == 0u)
    {
        wastedEffort       = true;
        cat.cooldown.value = maxCooldown;
        return;
    }

    // Partial Fisher-Yates shuffle to pick `nToHex` distinct random indices.
    const SizeT nToHex = sf::base::min(nCatsToHex, otherCatCount);

    for (SizeT i = 0u; i < nToHex; ++i)
    {
        const SizeT j = rng.getI<SizeT>(i, otherCatCount - 1);
        if (j != i)
        {
            const SizeT tmp     = candidateIndices[i];
            candidateIndices[i] = candidateIndices[j];
            candidateIndices[j] = tmp;
        }
    }

    float buffPower = pt->psvPPWitchCatBuffDuration.currentValue();

    if (pt->perm.witchCatBuffPowerScalesWithNCats)
        buffPower += sf::base::ceil(sf::base::pow(static_cast<float>(otherCatCount), 0.9f)) * 0.5f;

    if (pt->perm.witchCatBuffPowerScalesWithMapSize)
    {
        const float nMapExtensions = (pt->mapPurchased ? 1.f : 0.f) + static_cast<float>(pt->psvMapExtension.nPurchases);

        buffPower += static_cast<float>(nMapExtensions) * 0.75f;
    }

    const auto nDollsToSpawn = sf::base::max(SizeT{2u},
                                             static_cast<SizeT>(
                                                 buffPower * (pt->perm.witchCatBuffFewerDolls ? 1.f : 2.f) / 4.f));

    const auto isPositionFarFromOtherDolls = [&](const sf::Vec2f position) -> bool
    {
        for (const HexSession& s : sessionsToUse)
            for (const Doll& d : s.dolls)
                if ((d.position - position).lengthSquared() < (256.f * 256.f))
                    return false;

        return true;
    };

    const auto isOnTopOfAnyCat = [&](const sf::Vec2f position) -> bool
    {
        for (const Cat& c : pt->cats)
            if ((c.position - position).lengthSquared() < c.getRadiusSquared())
                return true;

        return false;
    };

    const auto isOnTopOfAnyShrine = [&](const sf::Vec2f position) -> bool
    {
        for (const Shrine& s : pt->shrines)
            if ((s.position - position).lengthSquared() < s.getRadiusSquared())
                return true;

        return false;
    };

    const auto rndDollPosition = [&]
    {
        constexpr float offset = 64.f;
        return rng.getVec2f({offset, offset}, {pt->getMapLimit() - offset - uiWindowWidth, boundaries.y - offset});
    };

    const auto pickDollPosition = [&]
    {
        constexpr unsigned int maxRetries = 16u;

        for (unsigned int retryCount = 0; retryCount < maxRetries; ++retryCount)
            if (const auto candidate = rndDollPosition();
                isPositionFarFromOtherDolls(candidate) && !isOnTopOfAnyCat(candidate) && !isOnTopOfAnyShrine(candidate))
                return candidate;

        return rndDollPosition();
    };

    for (SizeT s = 0u; s < nToHex; ++s)
    {
        const SizeT selectedIdx = candidateIndices[s];
        Cat&        selected    = pt->cats[selectedIdx];

        hexCat(selected, selectedIdx, copy);
        statRitual(selected.type);

        auto& session = sessionsToUse.emplaceBack(HexSession{.catIdx = selectedIdx, .dolls = {}});
        session.dolls.reserve(nDollsToSpawn);

        for (SizeT i = 0u; i < nDollsToSpawn; ++i)
        {
            auto& d = session.dolls.emplaceBack(
                Doll{.position      = pickDollPosition(),
                     .wobbleRadians = rng.getF(0.f, sf::base::tau),
                     .buffPower     = buffPower,
                     .catType       = selected.type == CatType::Copy ? pt->copycatCopiedCatType : selected.type,
                     .tcActivation  = {.startingValue = rng.getF(300.f, 600.f) * static_cast<float>(i + 1)},
                     .tcDeath       = {}});

            d.tcActivation.restart();
        }

        spawnParticlesWithHue(copy ? 180.f : 0.f, 128, selected.position, ParticleType::Hex, 0.5f, 0.35f);
    }

    cat.textStatusShakeEffect.bump(rngFast, 1.5f);
    cat.hits += 1u;

    if (!pt->dollTipShown)
    {
        pt->dollTipShown = true;
        doTip("Click on all the dolls to\nreceive a powerful timed buff!\nYou might need to scroll...");
    }

    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionWitch(const float deltaTimeMs, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    SFML_BASE_ASSERT(canHexMore());
    gameLoopUpdateCatActionWitchImpl(deltaTimeMs, cat, pt->hexSessions, /* nCatsToHex */ 1u); // TODO P1: add PP for more cats to hex at once
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionWizard(const float deltaTimeMs, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!pt->absorbingWisdom)
        return;

    const auto maxCooldown  = getComputedCooldownByCatTypeOrCopyCat(cat.type);
    const auto range        = getComputedRangeByCatTypeOrCopyCat(cat.type);
    const auto [cx, cy]     = getCatRangeCenter(cat);
    const auto drawPosition = cat.getDrawPosition(profile.enableCatBobbing);

    Bubble* starBubble = nullptr;

    const auto findRotatedStarBubble = [&](Bubble& bubble) // TODO P0: change this to something sensible
    {
        if ((bubble.type != BubbleType::Star && bubble.type != BubbleType::Nova) || bubble.rotation == 0.f)
            return ControlFlow::Continue;

        starBubble = &bubble;
        return ControlFlow::Break;
    };

    forEachBubbleInRadius({cx, cy}, range, findRotatedStarBubble);

    if (starBubble == nullptr)
        starBubble = pickRandomBubbleInRadiusMatching({cx, cy}, range, [&](Bubble& bubble) {
            return bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova;
        });

    if (starBubble == nullptr)
        return;

    Bubble& bubble = *starBubble;

    cat.pawPosition = bubble.position;
    cat.pawOpacity  = 255.f;
    cat.pawRotation = (bubble.position - cat.position).angle() + sf::degrees(45);

    bubble.rotation += deltaTimeMs * 0.025f; // TODO P0: change this to something sensible
    spawnParticlesWithHue(230.f, 1, bubble.position, ParticleType::Star, 0.5f, 0.35f);

    if (bubble.rotation >= sf::base::tau)
    {
        const auto wisdomReward = pt->getComputedRewardByBubbleType(bubble.type);

        if (profile.showTextParticles)
        {
            auto& tp = makeRewardTextParticle(drawPosition);
            std::snprintf(tp.buffer, sizeof(tp.buffer), "+%llu WP", wisdomReward);
        }

        sounds.absorb.settings.position = {bubble.position.x, bubble.position.y};
        playSound(sounds.absorb, /* maxOverlap */ 1u);

        spawnParticlesWithHue(230.f, 16, bubble.position, ParticleType::Star, 0.5f, 0.35f);

        pt->wisdom += wisdomReward;
        turnBubbleInto(bubble, BubbleType::Normal);

        cat.cooldown.value = maxCooldown;

        statAbsorbedStarBubble();
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionMouse(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
    const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

    Bubble* b = pickRandomBubbleInRadius(cat.position, range);
    if (b == nullptr)
        return;

    Bubble& bubble = *b;

    cat.pawPosition = bubble.position;
    cat.pawOpacity  = 255.f;

    addCombo(pt->mouseCatCombo, pt->mouseCatComboCountdown);
    pt->mouseCatCombo = sf::base::min(pt->mouseCatCombo, 999); // cap at 999x

    const auto savedBubblePos = bubble.position;

    const MoneyType reward = computeFinalReward(/* bubble     */ bubble,
                                                /* multiplier */ pt->mouseCatCombo == 999 ? 5.f : 1.f,
                                                /* comboMult  */ getComboValueMult(pt->mouseCatCombo, mouseCatComboDecay),
                                                /* popperCat  */ &cat);

    popWithRewardAndReplaceBubble({
        .reward          = reward,
        .bubble          = bubble,
        .xCombo          = pt->mouseCatCombo,
        .popSoundOverlap = true,
        .popperCat       = &cat,
        .multiPop        = false,
    });

    if (pt->multiPopMouseCatEnabled)
        forEachBubbleInRadius(savedBubblePos,
                              pt->psvPPMultiPopRange.currentValue(),
                              [&](Bubble& otherBubble)
        {
            if (&otherBubble == &bubble)
                return ControlFlow::Continue;

            popWithRewardAndReplaceBubble({
                .reward          = reward,
                .bubble          = otherBubble,
                .xCombo          = pt->mouseCatCombo,
                .popSoundOverlap = false,
                .popperCat       = &cat,
                .multiPop        = true,
            });

            return ControlFlow::Continue;
        });

    cat.textStatusShakeEffect.bump(rngFast, 1.5f);
    ++cat.hits;

    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionEngi(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto maxCooldown  = getComputedCooldownByCatTypeOrCopyCat(cat.type);
    const auto range        = getComputedRangeByCatTypeOrCopyCat(cat.type);
    const auto rangeSquared = range * range;

    SizeT nCatsHit = 0u;

    for (Cat& otherCat : pt->cats)
    {
        if (otherCat.type == CatType::Engi)
            continue;

        if ((otherCat.position - cat.position).lengthSquared() > rangeSquared)
            continue;

        ++nCatsHit;

        spawnParticles(8, otherCat.getDrawPosition(profile.enableCatBobbing), ParticleType::Cog, 0.25f, 0.5f);

        sounds.maintenance.settings.position = {otherCat.position.x, otherCat.position.y};
        playSound(sounds.maintenance, /* maxOverlap */ 1u);

        otherCat.boostCountdown.value = 1500.f;
    }

    if (nCatsHit > 0)
    {
        cat.textStatusShakeEffect.bump(rngFast, 1.5f);
        cat.hits += static_cast<sf::base::U32>(nCatsHit);

        statMaintenance(nCatsHit);
        statHighestSimultaneousMaintenances(nCatsHit);
    }

    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionRepulso(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
    const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

    if (pt->repulsoCatConverterEnabled && !pt->repulsoCatIgnoreBubbles.normal)
    {
        Bubble* b = pickRandomBubbleInRadiusMatching(cat.position, range, [&](Bubble& bubble) {
            return bubble.type != BubbleType::Star && bubble.type != BubbleType::Nova;
        });

        if (b != nullptr && rng.getF(0.f, 100.f) < pt->psvPPRepulsoCatConverterChance.currentValue())
        {
            turnBubbleInto(*b, pt->perm.repulsoCatNovaConverterPurchased ? BubbleType::Nova : BubbleType::Star);

            spawnParticles(2, b->position, ParticleType::Star, 0.5f, 0.35f);

            cat.textStatusShakeEffect.bump(rngFast, 1.5f);
            ++cat.hits;
        }
    }

    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionAttracto(const float /* deltaTimeMs */, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);

    // TODO P1: ? maybe absorb all bubbles in range and give a reward based on the number of bubbles absorbed

    cat.cooldown.value = maxCooldown;
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionCopy(const float deltaTimeMs, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (pt->copycatCopiedCatType == CatType::Witch)
    {
        SFML_BASE_ASSERT(canCopyHexMore());
        gameLoopUpdateCatActionWitchImpl(deltaTimeMs, cat, pt->copyHexSessions, /* nCatsToHex */ 1u);
    }
    else if (pt->copycatCopiedCatType == CatType::Wizard)
        gameLoopUpdateCatActionWizard(deltaTimeMs, cat);
    else if (pt->copycatCopiedCatType == CatType::Mouse)
        gameLoopUpdateCatActionMouse(deltaTimeMs, cat);
    else if (pt->copycatCopiedCatType == CatType::Engi)
        gameLoopUpdateCatActionEngi(deltaTimeMs, cat);
    else if (pt->copycatCopiedCatType == CatType::Repulso)
        gameLoopUpdateCatActionRepulso(deltaTimeMs, cat);
    else if (pt->copycatCopiedCatType == CatType::Attracto)
        gameLoopUpdateCatActionAttracto(deltaTimeMs, cat);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActionDuck(const float deltaTimeMs, Cat& cat)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    (void)deltaTimeMs;
    (void)cat;
}


////////////////////////////////////////////////////////////
[[nodiscard]] float Main::getWindRepulsionMult() const
{
    constexpr float mults[4] = {1.f, 5.f, 10.f, 15.f};
    return mults[pt->windStrength];
}


////////////////////////////////////////////////////////////
[[nodiscard]] float Main::getWindAttractionMult() const
{
    constexpr float mults[4] = {1.f, 1.5f, 3.f, 4.5f};
    return mults[pt->windStrength];
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatActions(const float deltaTimeMs)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    (void)wizardcatSpin.updateAndStop(deltaTimeMs * 0.015f);

    for (SizeT catIdx = 0u; catIdx < pt->cats.size(); ++catIdx)
    {
        Cat& cat = pt->cats[catIdx];

        // Keep cat in boundaries
        const float catRadius = cat.getRadius();

        // Keep cats away from shrine of clicking
        // Buff cats in shrine of automation
        for (const Shrine& shrine : pt->shrines)
        {
            if (shrine.type == ShrineType::Clicking && shrine.isActive() && cat.type != CatType::Mouse)
            {
                const auto diff = (shrine.position - cat.position);
                if (diff.lengthSquared() < shrine.getRangeSquared() * 1.35f)
                {
                    const auto strength = (shrine.getRange() * 1.35f - diff.length()) * 0.00125f * deltaTimeMs;
                    cat.position -= diff.normalized() * strength;
                }
            }
            else if (shrine.type == ShrineType::Automation && shrine.isActive())
            {
                if (shrine.isInRange(cat.position))
                    cat.boostCountdown.value = 250.f;
            }
            else if (shrine.type == ShrineType::Voodoo && shrine.isActive())
            {
                if (shrine.isInRange(cat.position) && getWitchCat() == nullptr && !anyCatHexedOrCopyHexed() &&
                    !cat.isHexedOrCopyHexed())
                {
                    hexCat(cat, catIdx, /* copy */ false);
                }
            }
        }

        const bool allowOOBCat = &cat == playerInputState.catToPlace || cat.astroState.hasValue() ||
                                 (playerInputState.draggedCats.size() > 1u && isCatBeingDragged(cat));

        if (!allowOOBCat)
            cat.position = cat.position.componentWiseClamp({catRadius, catRadius},
                                                           {pt->getMapLimit() - catRadius, boundaries.y - catRadius});

        const auto maxCooldown  = pt->getComputedCooldownByCatType(cat.type);
        const auto range        = pt->getComputedRangeByCatType(cat.type);
        const auto rangeSquared = range * range;

        const auto drawPosition = cat.getDrawPosition(profile.enableCatBobbing);

        auto diff = cat.pawPosition - drawPosition - sf::Vec2f{-30.f, 30.f};
        cat.pawPosition -= diff * 0.01f * deltaTimeMs;
        cat.pawRotation = cat.pawRotation.rotatedTowards(sf::degrees(-45.f), deltaTimeMs * 0.005f);

        if (isCatBeingDragged(cat) && (cat.pawPosition - drawPosition).length() > 16.f)
            cat.pawPosition = drawPosition + (cat.pawPosition - drawPosition).normalized() * 16.f;

        if (cat.cooldown.value == 0.f && cat.pawOpacity > 10.f)
        {
            cat.pawOpacity -= 0.5f * deltaTimeMs;
            cat.pawOpacity = sf::base::max(cat.pawOpacity, 0.f);
        }

        // Spawn effect
        const auto seStatus = cat.spawnEffectTimer.updateForwardAndStop(deltaTimeMs * 0.002f);
        if (seStatus == TimerStatusStop::Running)
            continue;

        if (seStatus == TimerStatusStop::JustFinished)
        {
            spawnParticles(4, cat.position, ParticleType::Star, 0.5f, 0.75f);
        }

        cat.update(deltaTimeMs);

        if (cat.isHexedOrCopyHexed())
        {
            const auto res = cat.getHexedTimer()->updateAndStop(deltaTimeMs * 0.001f);

            if (cat.getHexedTimer()->direction == TimerDirection::Backwards && res == TimerStatusStop::JustFinished)
                cat.getHexedTimer().reset();
        }

        const auto doWitchBehavior = [&](const float hueMod, auto& soundRitual, auto& soundRitualEnd)
        {
            if (cat.cooldown.value < 100.f)
            {
                soundManager.stopPlayingAll(soundRitual);

                if (soundManager.countPlayingPooled(soundRitualEnd) == 0u)
                {
                    soundRitualEnd.settings.position = {cat.position.x, cat.position.y};

                    if (profile.playWitchRitualSounds)
                        playSound(soundRitualEnd);
                }
            }

            if (cat.cooldown.value < 10'000.f)
            {
                if (cat.cooldown.value > 100.f && soundManager.countPlayingPooled(soundRitual) == 0u)
                {
                    soundRitual.settings.position = {cat.position.x, cat.position.y};

                    if (profile.playWitchRitualSounds)
                        playSound(soundRitual);
                }

                const float intensity = remap(cat.cooldown.value, 0.f, 10'000.f, 1.f, 0.f);

                for (Cat& otherCat : pt->cats)
                {
                    if (otherCat.isHexedOrCopyHexed())
                        continue;

                    if (&otherCat == &cat)
                        cat.hue = sf::base::sin(
                                      sf::base::remainder(cat.cooldown.value /
                                                              remap(cat.cooldown.value, 0.f, 10'000.f, 15.f, 150.f),
                                                          sf::base::tau)) *
                                  50.f * intensity;

                    const auto diff2 = otherCat.position - cat.position;

                    if (diff2.lengthSquared() > rangeSquared)
                        continue;

                    if (rngFast.getF(0.f, 1.f) < intensity)
                        spawnParticle({.position   = otherCat.getDrawPosition(profile.enableCatBobbing) +
                                                     sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
                                       .velocity   = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                                       .scale      = rngFast.getF(0.08f, 0.27f) * 0.5f,
                                       .scaleDecay = 0.f,
                                       .accelerationY = -0.0017f,
                                       .opacity       = 1.f,
                                       .opacityDecay  = rngFast.getF(0.00035f, 0.0025f),
                                       .rotation      = rngFast.getF(0.f, sf::base::tau),
                                       .torque        = rngFast.getF(-0.002f, 0.002f)},
                                      /* hue */ wrapHue(rngFast.getF(-50.f, 50.f) + hueMod),
                                      ParticleType::Hex);
                }
            }
            else
            {
                soundManager.stopPlayingAll(sounds.ritual);
            }
        };

        if (cat.type == CatType::Witch && canHexMore())
            doWitchBehavior(/* hueMod */ 0.f, sounds.ritual, sounds.ritualend);

        if (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && canCopyHexMore())
            doWitchBehavior(/* hueMod */ 180.f, sounds.copyritual, sounds.copyritualend);

        if (cat.hexedTimer.hasValue() || (cat.type == CatType::Witch && !pt->hexSessions.empty()))
        {
            if (rngFast.getI(0, 10) > 5)
                spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
                               .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.5f,
                               .scaleDecay    = 0.f,
                               .accelerationY = -0.0017f,
                               .opacity       = 0.5f,
                               .opacityDecay  = rngFast.getF(0.00035f, 0.0025f) * 0.6f,
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ wrapHue(rngFast.getF(-50.f, 50.f)),
                              ParticleType::Hex);

            continue;
        }

        if (cat.hexedCopyTimer.hasValue() ||
            (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && !pt->copyHexSessions.empty()))
        {
            if (rngFast.getI(0, 10) > 5)
                spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
                               .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.5f,
                               .scaleDecay    = 0.f,
                               .accelerationY = -0.0017f,
                               .opacity       = 0.5f,
                               .opacityDecay  = rngFast.getF(0.00035f, 0.0025f) * 0.6f,
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ wrapHue(rngFast.getF(-50.f, 50.f) + 180.f),
                              ParticleType::Hex);

            continue;
        }

        if (pt->buffCountdownsPerType[asIdx(CatType::Normal)].value > 0.f && cat.pawOpacity >= 75.f)
        {
            spawnParticle({.position      = cat.pawPosition + rngFast.getVec2f({-12.f, -12.f}, {12.f, 12.f}),
                           .velocity      = rngFast.getVec2f({-0.015f, -0.015f}, {0.015f, 0.015f}),
                           .scale         = rngFast.getF(0.08f, 0.27f) * 0.1f,
                           .scaleDecay    = 0.f,
                           .accelerationY = 0.f,
                           .opacity       = 1.f,
                           .opacityDecay  = rngFast.getF(0.00025f, 0.0015f) * 1.5f,
                           .rotation      = rngFast.getF(0.f, sf::base::tau),
                           .torque        = rngFast.getF(-0.002f, 0.002f)},
                          /* hue */ 0.f,
                          ParticleType::Star);
        }


        const auto [cx, cy] = getCatRangeCenter(cat);

        if (cat.inspiredCountdown.value > 0.f && rngFast.getF(0.f, 1.f) > 0.5f)
        {
            spawnParticle({.position      = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius},
                           .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                           .scale         = rngFast.getF(0.08f, 0.27f) * 0.2f,
                           .scaleDecay    = 0.f,
                           .accelerationY = -0.002f,
                           .opacity       = 1.f,
                           .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                           .rotation      = rngFast.getF(0.f, sf::base::tau),
                           .torque        = rngFast.getF(-0.002f, 0.002f)},
                          /* hue */ 0.f,
                          ParticleType::Star);
        }

        const float globalBoost = pt->buffCountdownsPerType[asIdx(CatType::Engi)].value;
        if ((globalBoost > 0.f || cat.boostCountdown.value > 0.f) && rngFast.getF(0.f, 1.f) > 0.75f)
        {
            spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 25.f},
                           .velocity = rngFast.getVec2f({-0.025f, -0.015f}, {0.025f, 0.015f}),
                           .scale    = rngFast.getF(0.08f, 0.27f) * 0.15f,
                           .scaleDecay    = 0.f,
                           .accelerationY = -0.0015f,
                           .opacity       = 1.f,
                           .opacityDecay  = rngFast.getF(0.00055f, 0.0045f),
                           .rotation      = rngFast.getF(0.f, sf::base::tau),
                           .torque        = rngFast.getF(-0.002f, 0.002f)},
                          /* hue */ 180.f,
                          ParticleType::Cog);
        }

        if (cat.type == CatType::Uni)
            cat.hue += deltaTimeMs * (isUnicatTranscendenceActive() ? 0.25f : 0.1f);

        if (cat.type == CatType::Devil && isDevilcatHellsingedActive())
        {
            if (rngFast.getF(0.f, 1.f) > 0.75f)
            {
                spawnParticle({.position = drawPosition +
                                           sf::Vec2f{rngFast.getF(-catRadius + 15.f, +catRadius - 5.f), catRadius - 20.f},
                               .velocity      = rngFast.getVec2f({-0.025f, -0.015f}, {0.025f, 0.015f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.55f,
                               .scaleDecay    = -0.00025f,
                               .accelerationY = -0.0015f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00055f, 0.0045f),
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ 0.f,
                              ParticleType::Fire2);

                spawnParticle({.position      = drawPosition + sf::Vec2f{-52.f * 0.2f, -85.f * 0.2f},
                               .velocity      = rngFast.getVec2f({-0.025f, -0.015f}, {0.025f, 0.015f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.55f,
                               .scaleDecay    = -0.00025f,
                               .accelerationY = -0.0015f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00055f, 0.0045f) * 2.f,
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ 0.f,
                              ParticleType::Fire2);

                spawnParticle({.position      = drawPosition + sf::Vec2f{-140.f * 0.2f, -90.f * 0.2f},
                               .velocity      = rngFast.getVec2f({-0.025f, -0.015f}, {0.025f, 0.015f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.55f,
                               .scaleDecay    = -0.00025f,
                               .accelerationY = -0.0015f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00055f, 0.0045f) * 2.f,
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ 0.f,
                              ParticleType::Fire2);
            }
        }

        if (cat.type == CatType::Astro && cat.astroState.hasValue())
        {
            auto& [startX, velocityX, particleTimer, wrapped] = *cat.astroState;

            particleTimer += deltaTimeMs;

            if (particleTimer >= 3.f && !cat.isCloseToStartX())
            {
                sounds.rocket.settings.position = {cx, cy};
                playSound(sounds.rocket, /* maxOverlap */ 1u);

                spawnParticles(1, drawPosition + sf::Vec2f{46.f, 55.f}, ParticleType::Fire, 1.6f, 0.25f, 0.65f);

                if (rngFast.getI(0, 10) > 5)
                    spawnParticle(ParticleData{.position      = drawPosition + sf::Vec2f{46.f, 55.f},
                                               .velocity      = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(0.f, 0.1f)},
                                               .scale         = rngFast.getF(0.75f, 1.f) * 0.4f,
                                               .scaleDecay    = -0.00025f,
                                               .accelerationY = -0.00017f,
                                               .opacity       = 0.5f,
                                               .opacityDecay  = rngFast.getF(0.00065f, 0.00075f),
                                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                                  0.f,
                                  ParticleType::Smoke);

                particleTimer = 0.f;
            }

            if (frameProcThisFrame)
                forEachBubbleInRadius({cx, cy},
                                      range,
                                      [&](Bubble& bubble)
                {
                    const MoneyType reward = computeFinalReward(/* bubble     */ bubble,
                                                                /* multiplier */ 20.f,
                                                                /* comboMult  */ 1.f,
                                                                /* popperCat  */ &cat);

                    statFlightRevenue(reward);

                    if (bubble.type == BubbleType::Bomb)
                        pt->achAstrocatPopBomb = true;

                    popWithRewardAndReplaceBubble({
                        .reward          = reward,
                        .bubble          = bubble,
                        .xCombo          = 1,
                        .popSoundOverlap = rngFast.getF(0.f, 1.f) > 0.75f,
                        .popperCat       = &cat,
                        .multiPop        = false,
                    });

                    cat.textStatusShakeEffect.bump(rngFast, 1.5f);

                    return ControlFlow::Continue;
                });

            if (!cat.isCloseToStartX() && velocityX > -5.f)
                velocityX -= 0.00025f * deltaTimeMs;

            if (!cat.isCloseToStartX())
                cat.position.x += velocityX * deltaTimeMs;
            else
                cat.position.x = exponentialApproach(cat.position.x, startX - 10.f, deltaTimeMs, 500.f);

            if (!wrapped && cat.position.x + catRadius < 0.f)
            {
                cat.position.x = pt->getMapLimit() + catRadius;

                if (pt->buffCountdownsPerType[asIdx(CatType::Astro)].value == 0.f) // loop if astro buff active
                    wrapped = true;
            }

            if (wrapped && cat.position.x <= startX)
            {
                cat.astroState.reset();
                cat.position.x     = startX;
                cat.cooldown.value = maxCooldown;
            }

            continue;
        }

        if (cat.type == CatType::Wizard)
        {
            if (pt->absorbingWisdom)
            {
                static float absorbSin = 0.f;
                absorbSin += deltaTimeMs * 0.002f;

                cat.hue = wrapHue(sf::base::sin(sf::base::remainder(absorbSin, sf::base::tau)) * 25.f);

                if (wizardcatAbsorptionRotation < 0.15f)
                    wizardcatAbsorptionRotation += deltaTimeMs * 0.0005f;
            }
            else
            {
                cat.hue = 0.f;

                if (wizardcatAbsorptionRotation > 0.f)
                    wizardcatAbsorptionRotation -= deltaTimeMs * 0.0005f;
            }

            if (isWizardBusy() && rngFast.getF(0.f, 1.f) > 0.5f)
            {
                spawnParticle({.position   = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius},
                               .velocity   = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale      = rngFast.getF(0.08f, 0.27f) * 0.2f,
                               .scaleDecay = 0.f,
                               .accelerationY = -0.002f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ 225.f,
                              ParticleType::Star);
            }
        }

        if (cat.type == CatType::Mouse || (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Mouse))
        {
            for (const Cat& otherCat : pt->cats)
            {
                if (otherCat.type != CatType::Normal)
                    continue;

                if ((otherCat.position - cat.position).lengthSquared() > rangeSquared)
                    continue;

                if (rngFast.getF(0.f, 1.f) > 0.95f)
                    spawnParticle({.position      = otherCat.getDrawPosition(profile.enableCatBobbing) +
                                                    sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 25.f},
                                   .velocity      = rngFast.getVec2f({-0.01f, -0.05f}, {0.01f, 0.05f}),
                                   .scale         = rngFast.getF(0.08f, 0.27f) * 0.4f,
                                   .scaleDecay    = 0.f,
                                   .accelerationY = -0.00015f,
                                   .opacity       = 1.f,
                                   .opacityDecay  = rngFast.getF(0.0003f, 0.002f),
                                   .rotation      = -0.6f,
                                   .torque        = 0.f},
                                  /* hue */ 0.f,
                                  ParticleType::Cursor);
            }
        }

        if (cat.type == CatType::Repulso || (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Repulso))
            forEachBubbleInRadius(cat.position,
                                  pt->getComputedRangeByCatType(CatType::Repulso),
                                  makeMagnetAction(cat.position,
                                                   cat.type,
                                                   deltaTimeMs,
                                                   &Bubble::repelledCountdown,
                                                   1500.f,
                                                   getWindRepulsionMult(),
                                                   -1.f,
                                                   pt->repulsoCatIgnoreBubbles));

        if (cat.type == CatType::Attracto || (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Attracto))
            forEachBubbleInRadius(cat.position,
                                  pt->getComputedRangeByCatType(CatType::Attracto),
                                  makeMagnetAction(cat.position,
                                                   cat.type,
                                                   deltaTimeMs,
                                                   &Bubble::attractedCountdown,
                                                   750.f,
                                                   getWindAttractionMult(),
                                                   1.f,
                                                   pt->attractoCatIgnoreBubbles));

        if (isCatBeingDragged(cat))
            continue;

        const float globalBoostMult = globalBoost > 0.f ? 2.f : 1.f;
        if (!cat.updateCooldown(deltaTimeMs * globalBoostMult))
            continue;

        using FnPtr = void (Main::*)(const float, Cat&);

        const FnPtr fnPtrs[]{
            &Main::gameLoopUpdateCatActionNormal,
            &Main::gameLoopUpdateCatActionUni,
            &Main::gameLoopUpdateCatActionDevil,
            &Main::gameLoopUpdateCatActionAstro,

            &Main::gameLoopUpdateCatActionWitch,
            &Main::gameLoopUpdateCatActionWizard,
            &Main::gameLoopUpdateCatActionMouse,
            &Main::gameLoopUpdateCatActionEngi,
            &Main::gameLoopUpdateCatActionRepulso,
            &Main::gameLoopUpdateCatActionAttracto,
            &Main::gameLoopUpdateCatActionCopy,
            &Main::gameLoopUpdateCatActionDuck,
        };

        static_assert(sf::base::getArraySize(fnPtrs) == nCatTypes);

        (this->*fnPtrs[asIdx(cat.type)])(deltaTimeMs, cat);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::isCatDraggable(const Cat& cat) const
{
    if (cat.isAstroAndInFlight())
        return false;

    if (cat.isHexedOrCopyHexed())
        return false;

    if (cat.type == CatType::Witch && !pt->hexSessions.empty())
        return false;

    if (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && !pt->copyHexSessions.empty())
        return false;

    if (cat.type == CatType::Witch && cat.cooldown.value <= 10'000.f)
        return false;

    if (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && cat.cooldown.value <= 10'000.f)
        return false;

    return true;
}


[[nodiscard]] bool Main::isAOESelecting() const
{
    return (keyDown(sf::Keyboard::Key::LShift) || keyDown(sf::Keyboard::Key::LControl)) &&
           mBtnDown(getLMB(), /* penetrateUI */ true);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCatDragging(const float deltaTimeMs, const SizeT countFingersDown, const sf::Vec2f mousePos)
{
    const bool dragInputHeld = mBtnDown(getLMB(), /* penetrateUI */ true) || countFingersDown == 1u;

    if (inPrestigeTransition)
    {
        resetAllDraggedCats();
        return;
    }

    // Automatically scroll when dragging cats near the edge of the screen
    if ((!playerInputState.draggedCats.empty() || isAOESelecting()) && playerInputState.catToPlace == nullptr)
    {
        constexpr float offset = 48.f;

        if (mousePos.x < particleCullingBoundaries.left + offset * 1.f)
            playerInputState.scroll -= 8.f;
        else if (mousePos.x < particleCullingBoundaries.left + offset * 2.f)
            playerInputState.scroll -= 4.f;
        else if (mousePos.x < particleCullingBoundaries.left + offset * 3.f)
            playerInputState.scroll -= 2.f;

        if (mousePos.x > particleCullingBoundaries.right - offset * 1.f)
            playerInputState.scroll += 8.f;
        else if (mousePos.x > particleCullingBoundaries.right - offset * 2.f)
            playerInputState.scroll += 4.f;
        else if (mousePos.x > particleCullingBoundaries.right - offset * 3.f)
            playerInputState.scroll += 2.f;
    }

    if (isAOESelecting())
    {
        if (!playerInputState.catDragOrigin.hasValue())
            playerInputState.catDragOrigin.emplace(mousePos);
    }
    else if (playerInputState.catDragOrigin.hasValue())
    {
        const auto dragRect = getAoEDragRect(mousePos).value();
        playerInputState.catDragOrigin.reset();
        playerInputState.draggedCats.clear();
        playerInputState.draggedCatsStartedWithTouch        = false;
        playerInputState.draggedCatsStartedFromAOESelection = true;

        for (Cat& cat : pt->cats)
        {
            if (!isCatDraggable(cat))
                continue;

            if (!dragRect.contains(cat.position))
                continue;

            playerInputState.draggedCats.pushBack(&cat);
        }

        playSound(sounds.grab);
    }
    else
    {
        const bool shouldDropCats = [&]
        {
            if (playerInputState.catToPlace != nullptr)
                return bubbleCullingBoundaries.isInside(playerInputState.catToPlace->position) &&
                       inputHelper.wasMouseButtonJustPressed(getLMB());

            if (playerInputState.draggedCats.empty())
                return false;

            if (playerInputState.draggedCatsStartedWithTouch)
                return countFingersDown != 1u;

            if (playerInputState.draggedCatsStartedFromAOESelection)
                return inputHelper.wasMouseButtonJustPressed(getLMB());

            return !dragInputHeld;
        }();

        if (shouldDropCats)
        {
            if (!playerInputState.draggedCats.empty())
                playSound(sounds.drop);

            resetAllDraggedCats();
            return;
        }

        if (!playerInputState.draggedCats.empty())
        {
            const auto pivotCatIdx = pickDragPivotCatIndex();
            Cat&       pivotCat    = *playerInputState.draggedCats[pivotCatIdx];

            static thread_local sf::base::Vector<sf::Vec2f> relativeCatPositions;
            relativeCatPositions.clear();
            relativeCatPositions.reserve(playerInputState.draggedCats.size());

            for (const Cat* cat : playerInputState.draggedCats)
                relativeCatPositions.pushBack(cat->position - pivotCat.position);

            pivotCat.position = exponentialApproach(pivotCat.position, mousePos + sf::Vec2f{-10.f, 13.f}, deltaTimeMs, 25.f);

            for (sf::base::SizeT i = 0u; i < playerInputState.draggedCats.size(); ++i)
            {
                if (i == pivotCatIdx)
                    continue;

                playerInputState.draggedCats[i]->position = pivotCat.position + relativeCatPositions[i];
            }

            return;
        }

        if (ImGui::GetIO().WantCaptureMouse || !particleCullingBoundaries.isInside(mousePos))
        {
            resetAllDraggedCats();
            return;
        }

        if (!dragInputHeld)
        {
            playerInputState.catDragPressDuration = 0.f;
            return;
        }

        Cat* hoveredCat = nullptr;

        // Only check for hover targets during initial press phase
        if (playerInputState.catDragPressDuration <= profile.catDragPressDuration)
            for (Cat& cat : pt->cats)
            {
                if (!isCatDraggable(cat))
                    continue;

                if ((mousePos - cat.position).lengthSquared() > cat.getRadiusSquared())
                    continue;

                hoveredCat = &cat;
            }

        if (hoveredCat == nullptr)
        {
            playerInputState.catDragPressDuration = 0.f;
            return;
        }

        if (hoveredCat)
        {
            playerInputState.catDragPressDuration += deltaTimeMs;

            if (playerInputState.catDragPressDuration >= profile.catDragPressDuration)
            {
                playerInputState.draggedCats.clear();
                playerInputState.draggedCats.pushBack(hoveredCat);
                playerInputState.draggedCatsStartedWithTouch        = countFingersDown == 1u;
                playerInputState.draggedCatsStartedFromAOESelection = false;

                if (hoveredCat->type == CatType::Duck)
                {
                    sounds.quack.settings.position = {hoveredCat->position.x, hoveredCat->position.y};
                    playSound(sounds.quack);
                }
                else
                    playSound(sounds.grab);
            }
        }
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] constexpr CatType Main::shrineTypeToCatType(const ShrineType shrineType)
{
    return static_cast<CatType>(asIdx(shrineType) + 4u);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateShrines(const float deltaTimeMs)
{
    for (SizeT i = 0u; i < pt->psvShrineActivation.nPurchases; ++i)
    {
        for (Shrine& shrine : pt->shrines)
        {
            if (shrine.tcActivation.hasValue() || shrine.type != static_cast<ShrineType>(i))
                continue;

            shrine.tcActivation.emplace(TargetedCountdown{.startingValue = 2000.f});
            shrine.tcActivation->restart();

            sounds.earthquakeFast.settings.position = {shrine.position.x, shrine.position.y};
            playSound(sounds.earthquakeFast);

            screenShakeAmount = 4.5f;
            screenShakeTimer  = 1000.f;
        }
    }

    // Should only be triggered in testing via cheats
    for (SizeT i = pt->psvShrineActivation.nPurchases; i < nShrineTypes; ++i)
        for (Shrine& shrine : pt->shrines)
            if (shrine.type == static_cast<ShrineType>(i))
                shrine.tcActivation.reset();

    for (Shrine& shrine : pt->shrines)
    {
        if (shrine.tcActivation.hasValue())
        {
            const auto cdStatus = shrine.tcActivation->updateAndStop(deltaTimeMs);

            if (cdStatus == CountdownStatusStop::Running)
            {
                spawnParticlesWithHue(wrapHue(shrine.getHue() + 40.f),
                                      static_cast<SizeT>(1 + 12 * (1.f - shrine.tcActivation->getProgress())),
                                      shrine.getDrawPosition() + rngFast.getVec2f({-1.f, -1.f}, {1.f, 1.f}) * 32.f,
                                      ParticleType::Fire,
                                      rngFast.getF(0.25f, 1.f),
                                      0.75f);

                spawnParticlesWithHue(shrine.getHue(),
                                      static_cast<SizeT>(4 + 36 * (1.f - shrine.tcActivation->getProgress())),
                                      shrine.getDrawPosition() + rngFast.getVec2f({-1.f, -1.f}, {1.f, 1.f}) * 32.f,
                                      ParticleType::Shrine,
                                      rngFast.getF(0.35f, 1.2f),
                                      0.5f);
            }
            else if (cdStatus == CountdownStatusStop::JustFinished)
            {
                playSound(sounds.woosh);

                const auto asCatType = asIdx(shrineTypeToCatType(shrine.type));
                if (asCatType >= asIdx(CatType::Count))
                    return;

                if (pt->perm.unsealedByType[asCatType])
                    spawnSpecialCat(shrine.position, static_cast<CatType>(asCatType));
            }
        }

        shrine.update(deltaTimeMs);

        if (!shrine.isActive())
            continue;

        forEachBubbleInRadius(shrine.position,
                              shrine.getRange(),
                              [&](Bubble& bubble)
        {
            const auto diff = (shrine.position - bubble.position);

            // `ShrineType::Voodoo` handled elsewhere
            // `ShrineType::Clicking` handled elsewhere
            // `ShrineType::Automation` handled elsewhere

            if (shrine.type == ShrineType::Magic)
            {
                if (bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova)
                {
                    if (rngFast.getF(0.f, 1.f) > 0.85f)
                        spawnParticlesWithHue(230.f, 1, bubble.position, ParticleType::Star, 0.5f, 0.35f);

                    bubble.rotation += deltaTimeMs * 0.025f;

                    if (bubble.rotation >= sf::base::tau)
                    {
                        sounds.absorb.settings.position = {bubble.position.x, bubble.position.y};
                        playSound(sounds.absorb, /* maxOverlap */ 1u);

                        turnBubbleInto(bubble, BubbleType::Normal);
                    }
                }
            }
            else if (shrine.type == ShrineType::Repulsion)
            {
                const auto strength = (shrine.getRange() - diff.length()) * 0.0000015f * deltaTimeMs;
                bubble.velocity -= diff.normalized() * strength * getWindRepulsionMult();
            }
            else if (shrine.type == ShrineType::Attraction)
            {
                const auto strength = (shrine.getRange() - diff.length()) * 0.0000025f * deltaTimeMs;
                bubble.velocity += diff.normalized() * strength * getWindAttractionMult();
            }
            else if (shrine.type == ShrineType::Camouflage)
            {
                // TODO P1: any effect?
            }
            else if (shrine.type == ShrineType::Victory)
            {
                // TODO P1: any effect?
            }

            return ControlFlow::Continue;
        });

        if (shrine.collectedReward >= pt->getComputedRequiredRewardByShrineType(shrine.type))
        {
            if (!shrine.tcDeath.hasValue())
            {
                shrine.tcDeath.emplace(TargetedCountdown{.startingValue = 5000.f});
                shrine.tcDeath->restart();

                sounds.earthquake.settings.position = {shrine.position.x, shrine.position.y};
                playSound(sounds.earthquake);

                screenShakeAmount = 4.5f;
            }
            else
            {
                const auto cdStatus = shrine.tcDeath->updateAndStop(deltaTimeMs);

                if (cdStatus == CountdownStatusStop::JustFinished)
                {
                    playSound(sounds.woosh);
                    ++pt->nShrinesCompleted;

                    const auto doShrineReward = [&](const CatType catType)
                    {
                        if (findFirstCatByType(catType) == nullptr)
                        {
                            spawnSpecialCat(shrine.position, catType);
                        }
                        else // unsealed
                        {
                            const auto unsealedReward = static_cast<MoneyType>(
                                static_cast<float>(shrine.collectedReward) * 1.5f);
                            addMoney(unsealedReward);

                            sounds.kaching.settings.position = {shrine.position.x, shrine.position.y};
                            playSound(sounds.kaching);

                            if (profile.showTextParticles)
                            {
                                auto& tp = makeRewardTextParticle(shrine.position);
                                std::snprintf(tp.buffer, sizeof(tp.buffer), "+$%llu", unsealedReward);
                            }
                        }
                    };

                    if (shrine.type == ShrineType::Voodoo)
                    {
                        doShrineReward(CatType::Witch);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
                            doTip(
                                "The Witchcat has been unsealed!\nThey perform voodoo rituals on nearby "
                                "cats,\ngiving you powerful timed buffs.");
                    }
                    else if (shrine.type == ShrineType::Magic)
                    {
                        doShrineReward(CatType::Wizard);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
                            doTip(
                                "The Wizardcat has been unsealed!\nThey absorb star bubbles to learn "
                                "spells,\nwhich can be casted on demand.");
                    }
                    else if (shrine.type == ShrineType::Clicking)
                    {
                        doShrineReward(CatType::Mouse);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
                            doTip(
                                "The Mousecat has been unsealed!\nThey combo-click bubbles, buff nearby "
                                "cats,\nand "
                                "provide a global click buff.");
                    }
                    else if (shrine.type == ShrineType::Automation)
                    {
                        doShrineReward(CatType::Engi);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
                            doTip(
                                "The Engicat has been unsealed!\nThey speed-up nearby cats and provide\na "
                                "global "
                                "cat buff.");
                    }
                    else if (shrine.type == ShrineType::Repulsion)
                    {
                        doShrineReward(CatType::Repulso);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
                            doTip(
                                "The Repulsocat has been unsealed!\nNearby bubbles getting pushed away "
                                "from\nthem "
                                "gain a x2 multiplier.");
                    }
                    else if (shrine.type == ShrineType::Attraction)
                    {
                        doShrineReward(CatType::Attracto);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
                            doTip(
                                "The Attractocat has been unsealed!\nNearby bubbles getting attracted to\nthem "
                                "gain a x2 multiplier.");
                    }
                    else if (shrine.type == ShrineType::Camouflage)
                    {
                        doShrineReward(CatType::Copy);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
                            doTip(
                                "The Copycat has been unsealed!\nThey can mimic other unique cats,\ngaining "
                                "their "
                                "powers!");
                    }
                    else if (shrine.type == ShrineType::Victory)
                    {
                        doShrineReward(CatType::Duck);

                        if (!pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Duck)])
                            doTip(
                                "It's... a duck. I am as confused\nas you are! But hey,\nyou won! "
                                "Congratulations!");

                        playSound(sounds.quack);

                        victoryTC.emplace(TargetedCountdown{.startingValue = 6500.f});
                        victoryTC->restart();
                        delayedActions.emplaceBack(Countdown{.value = 7000.f}, [this] { playSound(sounds.letterchime); });
                    }

                    const auto catType = asIdx(shrineTypeToCatType(shrine.type));
                    if (!pt->perm.shrineCompletedOnceByCatType[catType])
                    {
                        pushNotification("New unlocks!", "A new background and BGM have been unlocked!");
                        pt->perm.shrineCompletedOnceByCatType[catType] = true;
                    }

                    profile.selectedBackground = static_cast<int>(shrine.type) + 1;
                    profile.selectedBGM        = static_cast<int>(shrine.type) + 1;

                    updateSelectedBackgroundSelectorIndex();
                    updateSelectedBGMSelectorIndex();

                    switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ false);
                }
                else if (cdStatus == CountdownStatusStop::Running)
                {
                    spawnParticlesWithHue(wrapHue(shrine.getHue() + 40.f),
                                          static_cast<SizeT>(1 + 12 * shrine.getDeathProgress()),
                                          shrine.getDrawPosition() + rngFast.getVec2f({-1.f, -1.f}, {1.f, 1.f}) * 32.f,
                                          ParticleType::Fire,
                                          sf::base::max(0.25f, 1.f - shrine.getDeathProgress()),
                                          0.75f);

                    spawnParticlesWithHue(shrine.getHue(),
                                          static_cast<SizeT>(4 + 36 * shrine.getDeathProgress()),
                                          shrine.getDrawPosition() + rngFast.getVec2f({-1.f, -1.f}, {1.f, 1.f}) * 32.f,
                                          ParticleType::Shrine,
                                          sf::base::max(0.35f, 1.2f - shrine.getDeathProgress()),
                                          0.5f);
                }
            }
        }
    }

    sf::base::vectorEraseIf(pt->shrines, [](const Shrine& shrine) { return shrine.getDeathProgress() >= 1.f; });
}


////////////////////////////////////////////////////////////
void Main::collectDollImpl(Doll& d, HexSession& session, const bool copy)
{
    SFML_BASE_ASSERT(!d.tcDeath.hasValue());

    statDollCollected();

    for (SizeT i = 0u; i < 8u; ++i)
        spawnParticlesWithHue(wrapHue(rngFast.getF(-50.f, 50.f) + (copy ? 180.f : 0.f)),
                              8,
                              d.getDrawPosition(),
                              ParticleType::Hex,
                              0.5f,
                              0.35f);

    screenShakeAmount = 1.5f;
    screenShakeTimer  = 500.f;

    d.tcDeath.emplace(TargetedCountdown{.startingValue = 750.f});
    d.tcDeath->restart();

    const bool allDollsCollected = sf::base::allOf(session.dolls.begin(), session.dolls.end(), [&](const Doll& otherDoll) {
        return otherDoll.tcDeath.hasValue();
    });

    if (allDollsCollected)
    {
        playSound(sounds.buffon);

        constexpr float buffDurationMult[] = {
            1.f, // Normal
            1.f, // Uni
            1.f, // Devil
            1.f, // Astro

            1.f, // Witch
            1.f, // Wizard
            1.f, // Mouse
            1.f, // Engi
            1.f, // Repulso
            1.f, // Attracto
            1.f, // Copy
            1.f, // Duck
        };

        static_assert(sf::base::getArraySize(buffDurationMult) == nCatTypes);

        // Apply some diminishing returns if the same buff type is chosen over and over
        constexpr float buffDurationSoftCap = 60'000.f;
        const float     buffDuration        = d.buffPower * 1000.f;

        const float currentBuff = pt->buffCountdownsPerType[asIdx(d.catType)].value;

        const float factor = (currentBuff < buffDurationSoftCap)
                                 ? sf::base::pow((buffDurationSoftCap - currentBuff) / buffDurationSoftCap, 0.15f)
                                 : 0.1f;

        pt->buffCountdownsPerType[asIdx(d.catType)].value += buffDuration * factor;

        Cat* const hexedCat = getSessionTargetCat(session);
        SFML_BASE_ASSERT(hexedCat != nullptr);

        spawnParticle({.position = d.getDrawPosition(),
                       .velocity = (hexedCat->getDrawPosition(profile.enableCatBobbing) - d.getDrawPosition()).normalized() * 1.f,
                       .scale         = 0.2f,
                       .scaleDecay    = 0.f,
                       .accelerationY = 0.f,
                       .opacity       = 1.f,
                       .opacityDecay  = 0.0015f,
                       .rotation      = 0.f,
                       .torque        = rngFast.getF(-0.0002f, 0.0002f)},
                      /* hue */ 0.f,
                      ParticleType::CatSoul);

        sounds.soulreturn.settings.position = {d.position.x, d.position.y};
        playSound(sounds.soulreturn);
    }
    else
    {
        sounds.hex.settings.position = {d.position.x, d.position.y};
        playSound(sounds.hex);
    }
}


////////////////////////////////////////////////////////////
void Main::collectDoll(Doll& d, HexSession& session)
{
    collectDollImpl(d, session, /* copy */ false);
}


////////////////////////////////////////////////////////////
void Main::collectCopyDoll(Doll& d, HexSession& session)
{
    collectDollImpl(d, session, /* copy */ true);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateDollsImpl(const float                   deltaTimeMs,
                                   const sf::Vec2f               mousePos,
                                   sf::base::Vector<HexSession>& sessionsToUse,
                                   const bool                    copy)
{
    for (HexSession& session : sessionsToUse)
    {
        Cat* const hexedCat = getSessionTargetCat(session);

        if (session.dolls.empty())
        {
            if (hexedCat != nullptr)
                (copy ? hexedCat->hexedCopyTimer : hexedCat->hexedTimer)->direction = TimerDirection::Backwards;

            continue;
        }

        // Can happen during prestige transition
        if (hexedCat == nullptr)
            continue;

        for (Doll& d : session.dolls)
        {
            d.update(deltaTimeMs);

            if (!d.tcActivation.isDone())
            {
                (void)d.tcActivation.updateAndStop(deltaTimeMs);
                continue;
            }

            if (!d.tcDeath.hasValue())
            {
                if (rngFast.getF(0.f, 1.f) > 0.8f)
                    spawnParticle({.position      = d.getDrawPosition() + sf::Vec2f{rngFast.getF(-32.f, +32.f), 32.f},
                                   .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                                   .scale         = rngFast.getF(0.08f, 0.27f) * 0.5f,
                                   .scaleDecay    = 0.f,
                                   .accelerationY = -0.002f,
                                   .opacity       = 1.f,
                                   .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                                   .rotation      = rngFast.getF(0.f, sf::base::tau),
                                   .torque        = rngFast.getF(-0.002f, 0.002f)},
                                  /* hue */ wrapHue(rngFast.getF(-50.f, 50.f) + (copy ? 180.f : 0.f)),
                                  ParticleType::Hex);

                const bool click = (mBtnDown(getLMB(), /* penetrateUI */ false) ||
                                    playerInputState.fingerPositions[0].hasValue());

                if (click && (mousePos - d.position).lengthSquared() <= d.getRadiusSquared())
                {
                    if (copy)
                        collectCopyDoll(d, session);
                    else
                        collectDoll(d, session);
                }
            }
            else
            {
                (void)d.tcDeath->updateAndStop(deltaTimeMs);

                spawnParticlesWithHue(wrapHue(d.hue + (copy ? 180.f : 0.f)),
                                      static_cast<SizeT>(1 + 12 * d.getDeathProgress()),
                                      d.getDrawPosition() + rngFast.getVec2f({-1.f, -1.f}, {1.f, 1.f}) * 32.f,
                                      ParticleType::Hex,
                                      sf::base::max(0.25f, 1.f - d.getDeathProgress()),
                                      0.75f);
            }
        }

        sf::base::vectorEraseIf(session.dolls, [](const Doll& d) { return d.getDeathProgress() >= 1.f; });
    }

    // Drop sessions whose cat has since gone away (e.g. prestige transition).
    sf::base::vectorEraseIf(sessionsToUse, [&](const HexSession& s) { return getSessionTargetCat(s) == nullptr; });
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateDolls(const float deltaTimeMs, const sf::Vec2f mousePos)
{
    if (getWitchCat() == nullptr)
        return;

    gameLoopUpdateDollsImpl(deltaTimeMs, mousePos, pt->hexSessions, /* copy */ false);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateCopyDolls(const float deltaTimeMs, const sf::Vec2f mousePos)
{
    if (getCopyCat() == nullptr || pt->copycatCopiedCatType != CatType::Witch)
        return;

    gameLoopUpdateDollsImpl(deltaTimeMs, mousePos, pt->copyHexSessions, /* copy */ true);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateHellPortals(const float deltaTimeMs)
{
    const float hellPortalRadius = pt->getComputedRangeByCatType(CatType::Devil);

    for (HellPortal& hp : pt->hellPortals)
    {
        if (hp.life.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
        {
            sounds.makeBomb.settings.position = {hp.position.x, hp.position.y};
            playSound(sounds.portaloff);
        }

        for (sf::base::SizeT iP = 0u; iP < 2u; ++iP)
            spawnParticle({.position = hp.getDrawPosition() +
                                       rngFast.getDirVec2f() * rngFast.getF(hellPortalRadius * 0.95f, hellPortalRadius * 1.15f),
                           .velocity      = rngFast.getVec2f({-0.025f, -0.025f}, {0.025f, 0.025f}),
                           .scale         = rngFast.getF(0.08f, 0.27f) * 0.85f,
                           .scaleDecay    = -0.00025f,
                           .accelerationY = 0.f,
                           .opacity       = 1.f,
                           .opacityDecay  = rngFast.getF(0.00155f, 0.0145f),
                           .rotation      = rngFast.getF(0.f, sf::base::tau),
                           .torque        = rngFast.getF(-0.002f, 0.002f)},
                          /* hue */ 0.f,
                          ParticleType::Fire2);
    }

    sf::base::vectorEraseIf(pt->hellPortals, [](const HellPortal& hp) { return hp.life.isDone(); });
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateWitchBuffs(const float deltaTimeMs)
{
    for (Countdown& buffCountdown : pt->buffCountdownsPerType)
        if (buffCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            playSound(sounds.buffoff);
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateMana(const float deltaTimeMs)
{
    if (getWizardCat() == nullptr)
        return;

    //
    // Mana mult buff
    const float manaMult = pt->buffCountdownsPerType[asIdx(CatType::Wizard)].value > 0.f ? 3.5f : 1.f;

    //
    // Mana
    if (pt->mana < pt->getComputedMaxMana())
        pt->manaTimer += deltaTimeMs * manaMult;
    else
        pt->manaTimer = 0.f;

    if (pt->manaTimer >= pt->getComputedManaCooldown())
    {
        pt->manaTimer = 0.f;

        if (pt->mana < pt->getComputedMaxMana())
        {
            pt->mana += 1u;

            if (profile.showFullManaNotification && pt->mana == pt->getComputedMaxMana())
                pushNotification("Mana fully charged!", "The Wizardcat is eager to cast a spell...");
        }
    }

    //
    // Mewltiplier Aura spell
    if (pt->mewltiplierAuraTimer > 0.f)
    {
        pt->mewltiplierAuraTimer -= deltaTimeMs;
        pt->mewltiplierAuraTimer = sf::base::max(pt->mewltiplierAuraTimer, 0.f);

        const float wizardRange = pt->getComputedRangeByCatType(CatType::Wizard);

        if (const Cat* wizardCat = getWizardCat(); wizardCat != nullptr)
            for (SizeT i = 0u; i < 8u; ++i)
                spawnParticlesWithHueNoGravity(230.f,
                                               1,
                                               rngFast.getPointInCircle(wizardCat->position, wizardRange),
                                               ParticleType::Star,
                                               0.15f,
                                               0.05f);

        if (const Cat* copyCat = getCopyCat(); copyCat != nullptr && pt->copycatCopiedCatType == CatType::Wizard)
            for (SizeT i = 0u; i < 8u; ++i)
                spawnParticlesWithHueNoGravity(230.f,
                                               1,
                                               rngFast.getPointInCircle(copyCat->position, wizardRange),
                                               ParticleType::Star,
                                               0.15f,
                                               0.05f);
    }


    //
    // Stasis Field spell
    if (pt->stasisFieldTimer > 0.f)
    {
        pt->stasisFieldTimer -= deltaTimeMs;
        pt->stasisFieldTimer = sf::base::max(pt->stasisFieldTimer, 0.f);

        const float wizardRange = pt->getComputedRangeByCatType(CatType::Wizard);

        if (const Cat* wizardCat = getWizardCat(); wizardCat != nullptr)
            for (SizeT i = 0u; i < 8u; ++i)
                spawnParticlesWithHueNoGravity(50.f,
                                               1,
                                               rngFast.getPointInCircle(wizardCat->position, wizardRange),
                                               ParticleType::Star,
                                               0.15f,
                                               0.05f);

        if (const Cat* copyCat = getCopyCat(); copyCat != nullptr && pt->copycatCopiedCatType == CatType::Wizard)
            for (SizeT i = 0u; i < 8u; ++i)
                spawnParticlesWithHueNoGravity(50.f,
                                               1,
                                               rngFast.getPointInCircle(copyCat->position, wizardRange),
                                               ParticleType::Star,
                                               0.15f,
                                               0.05f);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdateAutocast()
{
    Cat* wizardCat = getWizardCat();
    Cat* copyCat   = getCopyCat();

    if (wizardCat == nullptr || !pt->perm.autocastPurchased || pt->perm.autocastIndex == 0u || isWizardBusy())
        return;

    const auto spellIndex = pt->perm.autocastIndex - 1u;

    if (static_cast<sf::base::SizeT>(spellIndex) > pt->psvSpellCount.nPurchases)
        return;

    if (pt->mana >= spellManaCostByIndex[spellIndex])
    {
        pt->mana -= spellManaCostByIndex[spellIndex];
        castSpellByIndex(spellIndex, wizardCat, copyCat);

        constexpr const char* spellNames[4] = {
            "Starpaw Conversion",
            "Mewltiplier Aura",
            "Dark Union",
            "Stasis Field",
        };

        pushNotification("Autocast", "Spell \"%s\" casted!", spellNames[spellIndex]);
    }
}
