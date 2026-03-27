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
#include "Particle.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "Shrine.hpp"
#include "ShrineConstants.hpp"
#include "ShrineType.hpp"
#include "Version.hpp"

#include "ExampleUtils/Easing.hpp"
#include "ExampleUtils/HueColor.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/NinePatchRect.hpp"
#include "ExampleUtils/NinePatchUtils.hpp"
#include "ExampleUtils/Profiler.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Fabs.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToString.hpp"

#include <cctype>
#include <cstdio>

////////////////////////////////////////////////////////////
void Main::drawMinimap(sf::RenderTarget& rt, const sf::View& hudView, const sf::Vec2f resolution, const sf::base::U8 shouldDrawUIAlpha)
{
    constexpr sf::Vec2f minimapPos = {15.f, 15.f};

    const float     minimapScale = profile.minimapScale;
    const float     mapLimit     = pt->getMapLimit();
    const float     hudScale     = profile.hudScale;
    const float     hueMod       = currentBackgroundHue.asDegrees();
    const sf::Vec2f minimapSize  = boundaries / minimapScale;

    const sf::RectangleShapeData minimapBorder{.position         = minimapPos,
                                               .fillColor        = sf::Color::Transparent,
                                               .outlineColor     = sf::Color::whiteMask(shouldDrawUIAlpha),
                                               .outlineThickness = 2.f,
                                               .size             = {mapLimit / minimapScale, minimapSize.y}};

    minimapRect.position = minimapPos;
    minimapRect.size     = minimapBorder.size;

    const float visibleLeft  = sf::base::max(0.f, gameView.center.x - gameView.size.x / 2.f);
    const float visibleRight = sf::base::min(mapLimit, gameView.center.x + gameView.size.x / 2.f);

    const sf::RectangleShapeData
        minimapIndicator{.position         = minimapPos.addX(visibleLeft / minimapScale),
                         .fillColor        = sf::Color::Transparent,
                         .outlineColor     = sf::Color::Blue.withRotatedHue(hueMod).withAlpha(shouldDrawUIAlpha),
                         .outlineThickness = 2.f,
                         .size = sf::Vec2f{sf::base::max(0.f, visibleRight - visibleLeft), gameView.size.y} / minimapScale};

    const float progressRatio = sf::base::clamp(mapLimit / boundaries.x, 0.f, 1.f);

    auto minimapScaledPosition = minimapPos.componentWiseDiv(resolution / hudScale);

    const auto minimapScaledSize = minimapSize.componentWiseDiv(resolution / hudScale)
                                       .clampX(0.f, (1.f - minimapScaledPosition.x) / progressRatio)
                                       .clampY(0.f, 1.f - minimapScaledPosition.y);

    const sf::Rect2f preClampScissorRect{minimapScaledPosition, {progressRatio * minimapScaledSize.x, minimapScaledSize.y}};

    const auto clampedScissorRect = sf::View::ScissorRect::fromRectClamped(preClampScissorRect);

    const sf::View minimapView{.center  = (resolution * 0.5f - minimapPos * hudScale) * minimapScale,
                               .size    = resolution * minimapScale,
                               .scissor = clampedScissorRect};

    rt.draw(sf::RectangleShapeData{.fillColor = sf::Color::blackMask(shouldDrawUIAlpha), .size = boundaries * hudScale},
            {.view = minimapView});

    const sf::Vec2f backgroundRectSize{static_cast<float>(txBackgroundChunk.getSize().x) * nGameScreens,
                                       static_cast<float>(txBackgroundChunk.getSize().y)};

    rt.draw(txBackgroundChunk,
            {.scale       = {hudScale, hudScale},
             .textureRect = {{0.f, 0.f}, backgroundRectSize},
             .color       = hueColor(hueMod, sf::base::min(shouldDrawUIAlpha, static_cast<sf::base::U8>(128u)))},
            {.view = minimapView, .shader = &shader});

    rt.draw(txDrawings,
            {.scale       = {hudScale, hudScale},
             .textureRect = {{0.f, 0.f}, backgroundRectSize},
             .color       = sf::Color::whiteMask(sf::base::min(shouldDrawUIAlpha, static_cast<sf::base::U8>(215u)))},
            {.view = minimapView, .shader = &shader});

    if (shouldDrawUIAlpha > 200u)
    {
        minimapDrawableBatch.scale = {hudScale, hudScale};
        rt.draw(minimapDrawableBatch, {.view = minimapView, .texture = &textureAtlas.getTexture(), .shader = &shader});
        minimapDrawableBatch.scale = {1.f, 1.f};
    }

    rt.draw(minimapBorder, {.view = hudView});

    {
        const float offset = 3.f;

        NinePatchRect panel{
            .position    = minimapRect.position - sf::Vec2f{offset, offset},
            .size        = minimapRect.size + sf::Vec2f{offset * 2.f, offset * 2.f},
            .textureRect = txFrameTiny.getRect(),
            .borders     = NinePatchBorders::all(18.f),
            .color       = hueColor(hueMod, shouldDrawUIAlpha),
        };

        panel.draw(rt, txFrameTiny, {.view = hudView, .shader = &shader});
    }

    rt.draw(minimapIndicator, {.view = hudView});
}


////////////////////////////////////////////////////////////
void Main::drawSplashScreen(sf::RenderTarget& rt, const sf::View& view, const sf::Vec2f resolution, const float hudScale) const
{
    const auto progress = easeInOutCubic(splashCountdown.getProgressBounced());

    rt.draw(sf::Sprite{.position    = resolution / 2.f / hudScale,
                       .scale       = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutCubic(progress)) / hudScale,
                       .origin      = txLogo.getSize().toVec2f() / 2.f,
                       .textureRect = txLogo.getRect(),
                       .color       = sf::Color::whiteMask(static_cast<U8>(easeInOutSine(progress) * 255.f))},
            {.view = view, .texture = &txLogo});
}


void Main::gameLoopDrawBubbles()
{
    const auto getBubbleHue = [&](const sf::base::SizeT idx, const Bubble& bubble)
    {
        if (bubble.type == BubbleType::Bomb)
            return 0.f;

        if (bubble.type == BubbleType::Star)
            return bubble.hueMod;

        if (bubble.type == BubbleType::Nova)
            return bubble.hueMod * 2.f;

        SFML_BASE_ASSERT(bubble.type == BubbleType::Normal);

        constexpr float hueRange = 75.f;

        const bool beingRepelledOrAttracted = !bubble.attractedCountdown.isDone() || !bubble.repelledCountdown.isDone();

        const float magnetHueMod = (beingRepelledOrAttracted ? 180.f : 0.f);

        return sf::base::remainder(static_cast<float>(idx) * 2.f - hueRange / 2.f, hueRange) + magnetHueMod;
    };

    const sf::Rect2f bubbleRects[]{txrBubble, txrBubbleStar, txrBomb, txrBubbleNova};
    static_assert(sf::base::getArraySize(bubbleRects) == nBubbleTypes);

    sf::CPUDrawableBatch* const batchToUseByType[]{&bubbleDrawableBatch,
                                                   &starBubbleDrawableBatch,
                                                   &bombBubbleDrawableBatch,
                                                   &starBubbleDrawableBatch};

    static_assert(sf::base::getArraySize(batchToUseByType) == nBubbleTypes);

    for (SizeT i = 0u; i < pt->bubbles.size(); ++i)
    {
        const Bubble& bubble = pt->bubbles[i];

        if (!bubbleCullingBoundaries.isInside(bubble.position))
            continue;

        constexpr float radiusToScale = 1.f / 256.f;
        const float     scaleMult     = radiusToScale * (bubble.type == BubbleType::Bomb ? 1.65f : 1.f);

        const auto& rect = bubbleRects[asIdx(bubble.type)];

        batchToUseByType[asIdx(bubble.type)]->add(sf::Sprite{
            .position    = bubble.position,
            .scale       = {bubble.radius * scaleMult, bubble.radius * scaleMult},
            .origin      = rect.size / 2.f,
            .rotation    = sf::radians(bubble.rotation),
            .textureRect = rect,
            .color       = hueColor(getBubbleHue(i, bubble), 255u),
        });
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopDisplayBubblesWithoutShader()
{
    shader.setUniform(suBubbleEffect, false);

    drawBatch(bubbleDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(starBubbleDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
    drawBatch(bombBubbleDrawableBatch, {.view = gameView, .texture = &textureAtlas.getTexture(), .shader = &shader});
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCursorTrail(const sf::Vec2f mousePos)
{
    if (profile.cursorTrailMode == 2 /* disabled */)
        return;

    if (combo <= 1 && profile.cursorTrailMode == 0 /* combo mode */)
        return;

    const sf::Vec2f mousePosDiff    = lastMousePos - mousePos;
    const float     mousePosDiffLen = mousePosDiff.length();

    if (mousePosDiffLen == 0.f)
        return;

    const float chunks   = mousePosDiffLen / 0.5f;
    const float chunkLen = mousePosDiffLen / chunks;

    const float trailHue = wrapHue(profile.cursorHue + currentBackgroundHue.asDegrees());

    const sf::Vec2f trailStep = mousePosDiff.normalized() * chunkLen;

    const float trailScaleMult = pt->laserPopEnabled ? 1.5f : 1.f;

    for (float i = 0.f; i < chunks; ++i)
        spawnParticle(ParticleData{.position      = mousePos + trailStep * i,
                                   .velocity      = {0.f, 0.f},
                                   .scale         = 0.135f * profile.cursorTrailScale * trailScaleMult,
                                   .scaleDecay    = 0.0002f,
                                   .accelerationY = 0.f,
                                   .opacity       = 0.1f,
                                   .opacityDecay  = 0.0005f,
                                   .rotation      = 0.f,
                                   .torque        = 0.f},
                      trailHue,
                      ParticleType::Trail);
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawMinimapIcons()
{
    minimapDrawableBatch.clear();

    const sf::Rect2f* const mmCatTxrs[]{
        &txrMMNormal,
        &txrMMUni,
        &txrMMDevil,
        &txrMMAstro,
        &txrMMWitch,
        &txrMMWizard,
        &txrMMMouse,
        &txrMMEngi,
        &txrMMRepulso,
        &txrMMAttracto,
        &txrMMCopy,
        &txrMMDuck,
    };

    static_assert(sf::base::getArraySize(mmCatTxrs) == nCatTypes);

    for (const Shrine& shrine : pt->shrines)
    {
        const auto shrineAlpha = static_cast<U8>(remap(shrine.getActivationProgress(), 0.f, 1.f, 128.f, 255.f));

        minimapDrawableBatch.add(
            sf::Sprite{.position    = shrine.position,
                       .scale       = {0.7f, 0.7f},
                       .origin      = txrMMShrine.size / 2.f,
                       .rotation    = sf::radians(0.f),
                       .textureRect = txrMMShrine,
                       .color       = hueColor(shrine.getHue(), shrineAlpha)});
    }

    for (const Cat& cat : pt->cats)
    {
        const auto& catMMTxr = *mmCatTxrs[asIdx(cat.type)];

        minimapDrawableBatch.add(
            sf::Sprite{.position    = cat.position,
                       .scale       = {1.f, 1.f},
                       .origin      = catMMTxr.size / 2.f,
                       .rotation    = sf::radians(0.f),
                       .textureRect = catMMTxr,
                       .color       = hueColor(cat.hue, 255u)});
    }

    for (const Doll& doll : pt->dolls)
    {
        minimapDrawableBatch.add(
            sf::Sprite{.position    = doll.position,
                       .scale       = {0.5f, 0.5f},
                       .origin      = txrDollNormal.size / 2.f,
                       .rotation    = sf::radians(0.f),
                       .textureRect = txrDollNormal,
                       .color       = hueColor(doll.hue, 255u)});
    }

    for (const Doll& doll : pt->copyDolls)
    {
        minimapDrawableBatch.add(
            sf::Sprite{.position    = doll.position,
                       .scale       = {0.5f, 0.5f},
                       .origin      = txrDollNormal.size / 2.f,
                       .rotation    = sf::radians(0.f),
                       .textureRect = txrDollNormal,
                       .color       = hueColor(doll.hue + 180.f, 255u)});
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopDisplayBubblesWithShader()
{
    if (!shader.setUniform(suBackgroundTexture, rtBackgroundProcessed.getTexture()))
    {
        profile.useBubbleShader = false;
        gameLoopDisplayBubblesWithoutShader();
        return;
    }

    shader.setUniform(suTime, shaderTime);
    shader.setUniform(suResolution, rtBackgroundProcessed.getSize().toVec2f());
    shader.setUniform(suBackgroundOrigin, gameView.center - gameView.size / 2.f);
    shader.setUniform(suBubbleEffect, false);

    shader.setUniform(suIridescenceStrength, profile.bsIridescenceStrength);
    shader.setUniform(suEdgeFactorMin, profile.bsEdgeFactorMin);
    shader.setUniform(suEdgeFactorMax, profile.bsEdgeFactorMax);
    shader.setUniform(suEdgeFactorStrength, profile.bsEdgeFactorStrength);
    shader.setUniform(suDistorsionStrength, profile.bsDistortionStrength);

    shader.setUniform(suBubbleLightness, profile.bsBubbleLightness);
    shader.setUniform(suLensDistortion, profile.bsLensDistortion);

    constexpr sf::BlendMode bubbleBlend(sf::BlendMode::Factor::One,
                                        sf::BlendMode::Factor::OneMinusSrcAlpha,
                                        sf::BlendMode::Equation::Add,
                                        sf::BlendMode::Factor::One,
                                        sf::BlendMode::Factor::OneMinusSrcAlpha,
                                        sf::BlendMode::Equation::Add);

    const sf::RenderStates bubbleStates{
        .blendMode = bubbleBlend,
        .view      = gameView,
        .texture   = &textureAtlas.getTexture(),
        .shader    = &shader,
    };

    shader.setUniform(suBubbleEffect, true);
    shader.setUniform(suSubTexOrigin, txrBubble.position);
    shader.setUniform(suSubTexSize, txrBubble.size);

    drawBatch(bubbleDrawableBatch, bubbleStates);

    shader.setUniform(suBubbleLightness, profile.bsBubbleLightness * 1.25f);
    shader.setUniform(suIridescenceStrength, profile.bsIridescenceStrength * 0.01f);
    shader.setUniform(suSubTexOrigin, txrBubbleStar.position);
    shader.setUniform(suSubTexSize, txrBubbleStar.size);

    drawBatch(starBubbleDrawableBatch, bubbleStates);

    shader.setUniform(suBubbleEffect, false);

    drawBatch(bombBubbleDrawableBatch, bubbleStates);
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCats(const sf::Vec2f mousePos, const float deltaTimeMs)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    ////////////////////////////////////////////////////////////
    const sf::Rect2f* const uniCatTxr     = isUnicatTranscendenceActive() ? &txrUniCat2 : &txrUniCat;
    const sf::Rect2f* const uniCatTailTxr = isUnicatTranscendenceActive() ? &txrUniCat2Tail : &txrUniCatTail;

    const sf::Rect2f* const devilCatTxr     = isDevilcatHellsingedActive() ? &txrDevilCat2 : &txrDevilCat3;
    const sf::Rect2f* const devilCatPawTxr  = isDevilcatHellsingedActive() ? &txrDevilCatPaw2 : &txrDevilCat3Arm;
    const sf::Rect2f* const devilCatTailTxr = isDevilcatHellsingedActive() ? &txrDevilCatTail2 : &txrDevilCat3Tail;

    ////////////////////////////////////////////////////////////
    const sf::Rect2f* const catTxrsByType[] = {
        &txrCat,      // Normal
        uniCatTxr,    // Uni
        devilCatTxr,  // Devil
        &txrAstroCat, // Astro

        &txrWitchCat,    // Witch
        &txrWizardCat,   // Wizard
        &txrMouseCat,    // Mouse
        &txrEngiCat,     // Engi
        &txrRepulsoCat,  // Repulso
        &txrAttractoCat, // Attracto
        &txrCopyCat,     // Copy
        &txrDuckCat,     // Duck
    };

    static_assert(sf::base::getArraySize(catTxrsByType) == nCatTypes);

    ////////////////////////////////////////////////////////////
    const sf::Rect2f* const catPawTxrsByType[] = {
        &txrCatPaw,     // Normal
        &txrUniCatPaw,  // Uni
        devilCatPawTxr, // Devil
        &txrWhiteDot,   // Astro

        &txrWitchCatPaw,    // Witch
        &txrWizardCatPaw,   // Wizard
        &txrMouseCatPaw,    // Mouse
        &txrEngiCatPaw,     // Engi
        &txrRepulsoCatPaw,  // Repulso
        &txrAttractoCatPaw, // Attracto
        &txrCopyCatPaw,     // Copy
        &txrWhiteDot,       // Duck
    };

    static_assert(sf::base::getArraySize(catPawTxrsByType) == nCatTypes);

    ////////////////////////////////////////////////////////////
    const sf::Rect2f* const catTailTxrsByType[] = {
        &txrCatTail,      // Normal
        uniCatTailTxr,    // Uni
        devilCatTailTxr,  // Devil
        &txrAstroCatTail, // Astro

        &txrWitchCatTail,    // Witch
        &txrWizardCatTail,   // Wizard
        &txrMouseCatTail,    // Mouse
        &txrEngiCatTail,     // Engi
        &txrRepulsoCatTail,  // Repulso
        &txrAttractoCatTail, // Attracto
        &txrCopyCatTail,     // Copy
        &txrWhiteDot,        // Duck
    };

    static_assert(sf::base::getArraySize(catTailTxrsByType) == nCatTypes);

    ////////////////////////////////////////////////////////////
    constexpr sf::Vec2f catTailOffsetsByType[] = {
        {0.f, 0.f},      // Normal
        {-35.f, -222.f}, // Uni
        {-8.f, 2.f},     // Devil
        {56.f, -80.f},   // Astro

        {37.f, 165.f},  // Witch
        {-90.f, 120.f}, // Wizard
        {0.f, 0.f},     // Mouse
        {2.f, 43.f},    // Engi
        {4.f, -29.f},   // Repulso
        {0.f, 0.f},     // Attracto
        {0.f, 0.f},     // Copy
        {0.f, 0.f},     // Duck
    };

    static_assert(sf::base::getArraySize(catTailOffsetsByType) == nCatTypes);

    ////////////////////////////////////////////////////////////
    constexpr sf::Vec2f catEyeOffsetsByType[] = {
        {0.f, 0.f},      // Normal
        {-35.f, -222.f}, // Uni
        {-8.f, 2.f},     // Devil
        {56.f, -80.f},   // Astro

        {37.f, 165.f}, // Witch
        {-25.f, 65.f}, // Wizard
        {0.f, 0.f},    // Mouse
        {2.f, 43.f},   // Engi
        {4.f, -29.f},  // Repulso
        {0.f, 0.f},    // Attracto
        {0.f, 0.f},    // Copy
        {0.f, 0.f},    // Duck
    };

    static_assert(sf::base::getArraySize(catEyeOffsetsByType) == nCatTypes);

    ////////////////////////////////////////////////////////////
    constexpr float catHueByType[] = {
        0.f,   // Normal
        160.f, // Uni
        -25.f, // Devil
        0.f,   // Astro

        80.f,   // Witch
        -135.f, // Wizard
        0.f,    // Mouse
        -10.f,  // Engi
        -40.f,  // Repulso
        0.f,    // Attracto
        0.f,    // Copy
        0.f,    // Duck
    };

    static_assert(sf::base::getArraySize(catHueByType) == nCatTypes);

    ////////////////////////////////////////////////////////////
    for (Cat& cat : pt->cats)
        gameLoopDrawCat(cat,
                        deltaTimeMs,
                        mousePos,
                        catTxrsByType,
                        catPawTxrsByType,
                        catTailTxrsByType,
                        catTailOffsetsByType,
                        catEyeOffsetsByType,
                        catHueByType);
}


[[nodiscard]] bool Main::isCatPerformingRitual(Cat& witch, Cat& cat) const
{
    if (witch.cooldown.value > 10'000.f)
        return false;

    if (&cat == &witch)
        return true;

    if (!pt->perm.witchCatBuffPowerScalesWithNCats)
        return false;

    const auto range        = pt->getComputedRangeByCatType(CatType::Witch);
    const auto rangeSquared = range * range;

    return (witch.position - cat.position).lengthSquared() <= rangeSquared;
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCat(
    Cat&            cat,
    const float     deltaTimeMs,
    const sf::Vec2f mousePos,
    const sf::Rect2f* const (&catTxrsByType)[nCatTypes],
    const sf::Rect2f* const (&catPawTxrsByType)[nCatTypes],
    const sf::Rect2f* const (&catTailTxrsByType)[nCatTypes],
    const sf::Vec2f (&catTailOffsetsByType)[nCatTypes],
    const sf::Vec2f (&catEyeOffsetsByType)[nCatTypes],
    const float (&catHueByType)[nCatTypes])
{
    auto& batchToUse     = catToPlace == &cat ? cpuTopDrawableBatch : cpuDrawableBatch;
    auto& textBatchToUse = catToPlace == &cat ? catTextTopDrawableBatch : catTextDrawableBatch;

    static int   cloudCircleIndex = 0;
    static float cloudTime        = 0.f;

    if (!pt->cats.empty() && &cat == &pt->cats.front())
        cloudTime += deltaTimeMs * 0.4f;

    const sf::Rect2f& catTxr = *catTxrsByType[asIdx(cat.type)];

    if (catToPlace != &cat && !bubbleCullingBoundaries.isInside(cat.position))
        return;

    const auto isCopyCatWithType = [&](const CatType copiedType)
    { return cat.type == CatType::Copy && pt->copycatCopiedCatType == copiedType; };

    const bool beingDragged = isCatBeingDragged(cat);

    const sf::base::Optional<sf::Rect2f> dragRect = getAoEDragRect(mousePos);

    const bool insideDragRect = dragRect.hasValue() && dragRect->contains(cat.position);

    const bool hovered = (mousePos - cat.position).lengthSquared() <= cat.getRadiusSquared();

    const bool shouldDisplayRangeCircle = !beingDragged && !cat.isAstroAndInFlight() && hovered &&
                                          !mBtnDown(getLMB(), /* penetrateUI */ true);

    const U8 rangeInnerAlpha = shouldDisplayRangeCircle ? 75u : 0u;

    const sf::Rect2f& catPawTxr = *catPawTxrsByType[asIdx(isCopyCatWithType(CatType::Mouse) ? CatType::Mouse : cat.type)];
    const sf::Rect2f& catTailTxr    = *catTailTxrsByType[asIdx(cat.type)];
    const sf::Vec2f   catTailOffset = catTailOffsetsByType[asIdx(cat.type)];
    const sf::Vec2f   catEyeOffset  = catEyeOffsetsByType[asIdx(cat.type)];

    const float maxCooldown  = getComputedCooldownByCatTypeOrCopyCat(cat.type);
    const float cooldownDiff = cat.cooldown.value;

    float catRotation = 0.f;

    const auto doWitchAnimation = [&](float& wobblePhase, Cat& witch)
    {
        if (&cat == &witch)
        {
            if (witch.cooldown.value >= 10'000.f)
            {
                if (wobblePhase > 0.f)
                    wobblePhase -= deltaTimeMs * 0.005f;

                wobblePhase = sf::base::max(wobblePhase, 0.f);
            }
            else
            {
                const float frequency = remap(sf::base::min(witch.cooldown.value, 10'000.f), 0.f, 10'000.f, 0.1f, 0.05f);

                wobblePhase += frequency * deltaTimeMs * 0.05f;
                wobblePhase = sf::base::remainder(wobblePhase, sf::base::tau);
            }
        }

        if (isCatPerformingRitual(witch, cat))
        {
            const float amplitude = remap(sf::base::min(witch.cooldown.value, 10'000.f), 0.f, 10'000.f, 0.5f, 0.f);
            catRotation           = sf::base::sin(wobblePhase) * amplitude;
        }
    };

    if (cat.type == CatType::Astro)
    {
        if (cat.astroState.hasValue() && cat.isCloseToStartX())
            catRotation = remap(sf::base::fabs(cat.position.x - cat.astroState->startX), 0.f, 400.f, 0.f, 0.523599f);
        else if (cooldownDiff < 1000.f)
            catRotation = remap(cooldownDiff, 0.f, 1000.f, 0.523599f, 0.f);
        else if (cat.astroState.hasValue())
            catRotation = 0.523599f;
    }
    else if (cat.isHexedOrCopyHexed())
    {
        catRotation = cat.getHexedTimer()->remap(0.f, cat.wobbleRadians);
    }

    const float mult = remap(cat.dragTime, 0.f, 1000.f, 0.f, 1.f);
    catRotation      = (-0.22f + sf::base::sin(cat.wobbleRadians) * 0.12f) * mult;

    if (cachedWitchCat != nullptr)
    {
        doWitchAnimation(witchcatWobblePhase, *cachedWitchCat);
    }

    if (cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Witch)
    {
        doWitchAnimation(copyWitchcatWobblePhase, *cachedCopyCat);
    }

    if (cat.type == CatType::Wizard)
        catRotation += wizardcatSpin.value + wizardcatAbsorptionRotation;

    const auto range = getComputedRangeByCatTypeOrCopyCat(cat.type);

    const auto alpha = cat.isHexedOrCopyHexed() ? static_cast<U8>(cat.getHexedTimer()->remap(255.f, 128.f))
                       : insideDragRect         ? static_cast<U8>(128u)
                                                : static_cast<U8>(255u);

    const auto catColor = hueColor(cat.hue, alpha);

    const auto circleAlpha = cat.cooldown.value < 0.f
                                 ? static_cast<U8>(0u)
                                 : static_cast<U8>(255.f - (cat.cooldown.value / maxCooldown * 225.f));

    const auto circleColor        = CatConstants::colors[asIdx(cat.type)].withRotatedHue(cat.hue).withLightness(0.75f);
    const auto circleOutlineColor = circleColor.withAlpha(rangeInnerAlpha == 0u ? circleAlpha : 255u);
    const auto textOutlineColor   = circleColor.withLightness(0.25f);

    if (profile.showCatRange && !inPrestigeTransition)
        batchToUse.add(sf::CircleShapeData{
            .position           = getCatRangeCenter(cat),
            .origin             = {range, range},
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = (circleOutlineColor.withAlpha(rangeInnerAlpha)),
            .outlineColor       = circleOutlineColor,
            .outlineThickness   = profile.catRangeOutlineThickness,
            .radius             = range,
            .pointCount         = static_cast<unsigned int>(range / 3.f),
        });

    const float catScaleMult = easeOutElastic(cat.spawnEffectTimer.value);
    const auto  catScale     = sf::Vec2f{0.2f, 0.2f} * catScaleMult;

    const auto catAnchor = beingDragged ? cat.position : cat.getDrawPosition(profile.enableCatBobbing);


    auto& cloudBatchToUse = catToPlace == &cat ? cpuTopCloudDrawableBatch : cpuCloudDrawableBatch;

    {
        const int   cloudCircleCount = sf::base::max(profile.catCloudCircleCount, 3);
        const float cloudScaleBase   = sf::base::max(catScaleMult, 0.35f) * profile.catCloudScale;
        const float cloudMult        = easeInOutBack(remap(cat.dragTime, 0.f, 1000.f, 1.f, 0.f));
        const float cloudScale       = cloudScaleBase * cloudMult;

        const float     cloudTimeSeconds = cloudTime * 0.001f;
        const float     catCloudPhase    = cat.position.x * 0.0215f + cat.position.y * 0.0135f +
                                           static_cast<float>(asIdx(cat.type)) * 0.7f;
        const sf::Vec2f cloudBasePos     = catAnchor +
                                           sf::Vec2f{0.f,
                                                     profile.catCloudBaseYOffset * cloudScale +
                                                         (beingDragged ? profile.catCloudDraggedOffset : 0.f)} +
                                           sf::Vec2f{5.f, profile.catCloudExtraYOffset};

        for (cloudCircleIndex = 0; cloudCircleIndex < cloudCircleCount; ++cloudCircleIndex)
        {
            const float normalizedIndex = static_cast<float>(cloudCircleIndex) / static_cast<float>(cloudCircleCount - 1);
            const float centeredIndex = normalizedIndex * 2.f - 1.f;
            const float lobeWeight    = 1.f - sf::base::fabs(centeredIndex);
            const float phase         = cloudTimeSeconds * (1.35f + normalizedIndex * 0.2f) + catCloudPhase +
                                        static_cast<float>(cloudCircleIndex) * 0.85f;

            const float xOffset = centeredIndex * profile.catCloudXExtent * cloudScale +
                                  sf::base::sin(phase) * (profile.catCloudWobbleX + lobeWeight * 1.5f) * cloudScale;
            const float yOffset = -lobeWeight * profile.catCloudLobeLift * cloudScale +
                                  sf::base::cos(phase * 1.4f) * (profile.catCloudWobbleY + lobeWeight) * cloudScale;
            const float radius  = (profile.catCloudRadiusBase + lobeWeight * profile.catCloudRadiusLobe +
                                   sf::base::sin(phase * 1.15f) * profile.catCloudRadiusWobble) *
                                  cloudScale;

            cloudBatchToUse.add(sf::Sprite{
                .position    = cloudBasePos + sf::Vec2f{xOffset, yOffset},
                .scale       = {radius / (txrCloud.size.x / 2.f), radius / (txrCloud.size.y / 2.f)},
                .origin      = txrCloud.size / 2.f,
                .textureRect = txrCloud,
            });
        }
    }


    const auto anchorOffset = [&](const sf::Vec2f offset)
    { return catAnchor + (offset / 2.f * 0.2f * catScaleMult).rotatedBy(sf::radians(catRotation)); };

    const float tailRotationMult = cat.type == CatType::Uni ? 0.4f : 1.f;

    const auto tailWiggleRotation = sf::radians(
        catRotation + ((beingDragged ? -0.2f : 0.f) +
                       sf::base::sin(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * tailRotationMult));

    const auto tailWiggleRotationInvertedDragged = sf::radians(
        catRotation + ((beingDragged ? 0.2f : 0.f) +
                       sf::base::sin(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * tailRotationMult));

    const sf::Vec2f pushDown{0.f, beingDragged ? 75.f : 0.f};

    const auto attachmentHue = hueColor(catHueByType[asIdx(cat.type)] + cat.hue, alpha);

    // Devilcat: draw tail behind
    if (cat.type == CatType::Devil)
    {
        batchToUse.add(sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vec2f{905.f, 10.f} + pushDown * 2.f),
                                  .scale       = catScale * 1.25f,
                                  .origin      = {320.f, 32.f},
                                  .rotation    = tailWiggleRotationInvertedDragged,
                                  .textureRect = catTailTxr,
                                  .color       = catColor});
    }

    //
    // Draw brain jar in the background
    if (cat.type == CatType::Normal && pt->perm.geniusCatsPurchased)
    {
        batchToUse.add(sf::Sprite{.position    = anchorOffset({210.f, -235.f}),
                                  .scale       = catScale,
                                  .origin      = txrBrainBack.size / 2.f,
                                  .rotation    = sf::radians(catRotation),
                                  .textureRect = txrBrainBack,
                                  .color       = catColor});
    }

    //
    // Unicats: wings
    if (cat.type == CatType::Uni)
    {
        const auto wingRotation = sf::radians(catRotation + (beingDragged ? -0.2f : 0.f) +
                                              sf::base::cos(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * 0.75f);

        batchToUse.add(sf::Sprite{.position    = anchorOffset({250.f, -175.f}),
                                  .scale       = catScale * 1.25f,
                                  .origin      = txrUniCatWings.size / 2.f - sf::Vec2f{35.f, 10.f},
                                  .rotation    = wingRotation,
                                  .textureRect = txrUniCatWings,
                                  .color       = hueColor(cat.hue + 180.f, 180u)});
    }

    //
    // Devilcat: draw book
    if (cat.type == CatType::Devil)
    {
        batchToUse.add(
            sf::Sprite{.position    = catAnchor + sf::Vec2f{10.f, 20.f},
                       .scale       = catScale * 1.55f,
                       .origin      = txrDevilCat3Book.size / 2.f,
                       .rotation    = sf::radians(catRotation),
                       .textureRect = isDevilcatHellsingedActive() ? txrDevilCat2Book : txrDevilCat3Book,
                       .color = hueColor(sf::base::remainder(cat.hue * 2.f - 15.f + static_cast<float>(cat.nameIdx) * 25.f, 60.f) -
                                             30.f,
                                         255u)});
    }

    //
    // Devilcat: draw paw behind book
    if (cat.type == CatType::Devil)
    {
        batchToUse.add(
            sf::Sprite{.position    = cat.pawPosition + (beingDragged ? sf::Vec2f{-6.f, 6.f} : sf::Vec2f{4.f, 2.f}),
                       .scale       = catScale * 1.25f,
                       .origin      = catPawTxr.size / 2.f,
                       .rotation    = cat.pawRotation + sf::degrees(35.f),
                       .textureRect = catPawTxr,
                       .color       = catColor.withAlpha(static_cast<U8>(cat.pawOpacity))});
    }

    //
    // Draw cat main shape
    batchToUse.add(sf::Sprite{.position    = catAnchor,
                              .scale       = catScale,
                              .origin      = catTxr.size / 2.f,
                              .rotation    = sf::radians(catRotation),
                              .textureRect = catTxr,
                              .color       = catColor});

    if (cat.type == CatType::Duck)
    {
        batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{335.f, -65.f} + pushDown),
                                  .scale       = catScale,
                                  .origin      = {98.f, 330.f},
                                  .rotation    = tailWiggleRotation,
                                  .textureRect = txrDuckFlag,
                                  .color       = catColor});
    }
    else
    {
        //
        // Draw graudation hat
        if (cat.type == CatType::Normal && pt->perm.smartCatsPurchased)
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset({-150.f, -535.f}),
                                      .scale       = catScale,
                                      .origin      = txrSmartCatHat.size / 2.f,
                                      .rotation    = sf::radians(catRotation),
                                      .textureRect = txrSmartCatHat,
                                      .color       = catColor});
        }

        //
        // Ear flapping animation
        if (cat.flapCountdown.isDone() && cat.flapAnimCountdown.isDone())
        {
            if (rngFast.getI(0, 100) > 92) // Double-flap chance
                cat.flapCountdown.value = 75.f;
            else
                cat.flapCountdown.value = rngFast.getF(4500.f, 12'500.f);
        }

        if (cat.flapCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            cat.flapAnimCountdown.value = 75.f * nEarRects;

        (void)cat.flapAnimCountdown.updateAndStop(deltaTimeMs);

        if (cat.type == CatType::Normal) // TODO P2: implement for other cats as well?
        {
            batchToUse.add(
                sf::Sprite{.position = anchorOffset(catEyeOffset + sf::Vec2f{-131.f, -365.f}),
                           .scale    = catScale,
                           .origin   = txrCatEars0.size / 2.f,
                           .rotation = sf::radians(catRotation),
                           .textureRect = *earRects[static_cast<unsigned int>(cat.flapAnimCountdown.value / 75.f) % nEarRects],
                           .color = attachmentHue});
        }

        //
        // Yawning animation
        const auto yawnRectIdx = static_cast<unsigned int>(cat.yawnAnimCountdown.value / 75.f) % nYawnRects;

        if (cat.type != CatType::Devil && cat.type != CatType::Wizard && cat.type != CatType::Mouse &&
            cat.type != CatType::Engi)
        {
            if (cat.yawnCountdown.isDone() && cat.yawnAnimCountdown.isDone())
                cat.yawnCountdown.value = rngFast.getF(7500.f, 20'000.f);

            if (cat.blinkAnimCountdown.isDone() &&
                cat.yawnCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                cat.yawnAnimCountdown.value = 75.f * nYawnRects;

            (void)cat.yawnAnimCountdown.updateAndStop(deltaTimeMs);

            batchToUse.add(sf::Sprite{.position    = anchorOffset(catEyeOffset + sf::Vec2f{-221.f, 25.f}),
                                      .scale       = catScale,
                                      .origin      = txrCatYawn0.size / 2.f,
                                      .rotation    = sf::radians(catRotation),
                                      .textureRect = *catYawnRects[yawnRectIdx],
                                      .color       = attachmentHue});
        }
        else
        {
            cat.yawnCountdown.value = cat.yawnAnimCountdown.value = 0.f;
        }

        //
        // Draw attachments
        if (cat.type == CatType::Normal && pt->perm.smartCatsPurchased) // Smart cat diploma
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{295.f, 355.f} + pushDown),
                                      .scale       = catScale,
                                      .origin      = {23.f, 150.f},
                                      .rotation    = tailWiggleRotation,
                                      .textureRect = txrSmartCatDiploma,
                                      .color       = catColor});
        }
        else if (cat.type == CatType::Astro && pt->perm.astroCatInspirePurchased) // Astro cat flag
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{395.f, 225.f} + pushDown),
                                      .scale       = catScale,
                                      .origin      = {98.f, 330.f},
                                      .rotation    = tailWiggleRotation,
                                      .textureRect = txrAstroCatFlag,
                                      .color       = catColor});
        }
        else if (cat.type == CatType::Engi ||
                 (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Engi)) // Engi cat wrench
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{295.f, 385.f} + pushDown),
                                      .scale       = catScale,
                                      .origin      = {36.f, 167.f},
                                      .rotation    = tailWiggleRotation,
                                      .textureRect = txrEngiCatWrench,
                                      .color       = catColor});
        }
        else if (cat.type == CatType::Attracto ||
                 (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Attracto)) // Attracto cat magnet
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{190.f, 315.f} + pushDown),
                                      .scale       = catScale,
                                      .origin      = {142.f, 254.f},
                                      .rotation    = tailWiggleRotation,
                                      .textureRect = txrAttractoCatMagnet,
                                      .color       = catColor});
        }


        //
        // Draw cat tail
        if (cat.type != CatType::Devil)
        {
            const auto originOffset = cat.type == CatType::Uni ? sf::Vec2f{250.f, 0.f} : sf::Vec2f{0.f, 0.f};
            const auto offset       = cat.type == CatType::Uni ? sf::Vec2f{-130.f, 405.f} : sf::Vec2f{0.f, 0.f};

            batchToUse.add(
                sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vec2f{475.f, 240.f} + offset + originOffset),
                           .scale       = catScale,
                           .origin      = originOffset + sf::Vec2f{320.f, 32.f},
                           .rotation    = tailWiggleRotation,
                           .textureRect = catTailTxr,
                           .color       = catColor});
        }

        //
        // Mousecat: mouse
        if (cat.type == CatType::Mouse || (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Mouse))
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{-275.f, -15.f}),
                                      .scale       = catScale,
                                      .origin      = {53.f, 77.f},
                                      .rotation    = tailWiggleRotationInvertedDragged,
                                      .textureRect = txrMouseCatMouse,
                                      .color       = catColor});
        }

        //
        // Eye blining animation
        const auto& eyelidArray = //
            (cat.type == CatType::Mouse || cat.type == CatType::Attracto || cat.type == CatType::Copy) ? grayEyeLidRects
            : (cat.type == CatType::Engi || (cat.type == CatType::Devil && isDevilcatHellsingedActive())) ? darkEyeLidRects
            : (cat.type == CatType::Astro || cat.type == CatType::Uni)
                ? whiteEyeLidRects
                : eyeLidRects;

        if (cat.blinkCountdown.isDone() && cat.blinkAnimCountdown.isDone())
        {
            if (rngFast.getI(0, 100) > 90) // Double animation chance
                cat.blinkCountdown.value = 75.f;
            else
                cat.blinkCountdown.value = rngFast.getF(1000.f, 4000.f);
        }

        if (cat.blinkCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            cat.blinkAnimCountdown.value = 75.f * nEyeLidRects;

        (void)cat.blinkAnimCountdown.updateAndStop(deltaTimeMs);

        if ((cachedWitchCat != nullptr && isCatPerformingRitual(*cachedWitchCat, cat)) ||
            (cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Witch &&
             isCatPerformingRitual(*cachedCopyCat, cat)))
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(catEyeOffset + sf::Vec2f{-185.f, -185.f}),
                                      .scale       = catScale,
                                      .origin      = txrCatEyeLid0.size / 2.f,
                                      .rotation    = sf::radians(catRotation),
                                      .textureRect = *eyelidArray[2],
                                      .color       = attachmentHue});
        }
        else if (!cat.yawnAnimCountdown.isDone())
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset(catEyeOffset + sf::Vec2f{-185.f, -185.f}),
                                      .scale       = catScale,
                                      .origin      = txrCatEyeLid0.size / 2.f,
                                      .rotation    = sf::radians(catRotation),
                                      .textureRect = *eyelidArray[static_cast<unsigned int>(
                                          remap(static_cast<float>(yawnRectIdx), 0.f, 13.f, 0.f, 7.f))],
                                      .color       = attachmentHue});
        }
        else if (!cat.blinkAnimCountdown.isDone())
        {
            batchToUse.add(
                sf::Sprite{.position = anchorOffset(catEyeOffset + sf::Vec2f{-185.f, -185.f}),
                           .scale    = catScale,
                           .origin   = txrCatEyeLid0.size / 2.f,
                           .rotation = sf::radians(catRotation),
                           .textureRect = *eyelidArray[static_cast<unsigned int>(cat.blinkAnimCountdown.value / 75.f) % nEyeLidRects],
                           .color = attachmentHue});
        }

        if (cat.type == CatType::Normal && pt->perm.geniusCatsPurchased)
        {
            batchToUse.add(sf::Sprite{.position    = anchorOffset({210.f, -235.f}),
                                      .scale       = catScale,
                                      .origin      = txrBrainFront.size / 2.f,
                                      .rotation    = sf::radians(catRotation),
                                      .textureRect = txrBrainFront,
                                      .color       = catColor});
        }


        if (!cat.isHexedOrCopyHexed() && cat.type != CatType::Devil)
            batchToUse.add(
                sf::Sprite{.position = cat.pawPosition + (beingDragged ? sf::Vec2f{-12.f, 12.f} : sf::Vec2f{0.f, 0.f}),
                           .scale    = catScale * 0.85f,
                           .origin   = catPawTxr.size / 2.f,
                           .rotation = cat.type == CatType::Mouse ? sf::radians(-0.6f) : cat.pawRotation,
                           .textureRect = catPawTxr,
                           .color       = catColor.withAlpha(static_cast<U8>(cat.pawOpacity))});

        //
        // Copycat: mask
        if (cat.type == CatType::Copy)
        {
            if (copycatMaskAnim.isDone() &&
                copycatMaskAnimCd.updateAndStop(deltaTimeMs) == CountdownStatusStop::AlreadyFinished)
                copycatMaskAnim.value = 3000.f;

            if (copycatMaskAnimCd.isDone() && copycatMaskAnim.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                copycatMaskAnimCd.value = 4000.f;

            const float foo = easeInOutBack(copycatMaskAnim.getProgressBounced(3000.f)) * 0.5f;

            const auto* txrMaskToUse = [&]() -> const sf::Rect2f*
            {
                if (pt->copycatCopiedCatType == CatType::Witch)
                    return &txrCCMaskWitch;

                if (pt->copycatCopiedCatType == CatType::Wizard)
                    return &txrCCMaskWizard;

                if (pt->copycatCopiedCatType == CatType::Mouse)
                    return &txrCCMaskMouse;

                if (pt->copycatCopiedCatType == CatType::Engi)
                    return &txrCCMaskEngi;

                if (pt->copycatCopiedCatType == CatType::Repulso)
                    return &txrCCMaskRepulso;

                if (pt->copycatCopiedCatType == CatType::Attracto)
                    return &txrCCMaskAttracto;

                return nullptr;
            }();

            if (txrMaskToUse != nullptr)
                batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vec2f{265.f, 115.f}),
                                          .scale       = catScale * remap(foo, 0.f, 0.5f, 1.f, 0.75f),
                                          .origin      = {353.f, 295.f * remap(foo, 0.f, 0.5f, 1.f, 1.25f)},
                                          .rotation    = sf::radians(catRotation + foo),
                                          .textureRect = *txrMaskToUse,
                                          .color       = catColor});
        }
    }

    if (profile.showCatText)
    {
        // TODO P2: move to member data
        static thread_local sf::base::String catNameBuffer;
        catNameBuffer.clear();

        if (pt->perm.smartCatsPurchased && cat.type == CatType::Normal && cat.nameIdx % 2u == 0u)
            catNameBuffer += "Dr. ";

        const sf::base::StringView catNameSv = shuffledCatNamesPerType[asIdx(cat.type)][cat.nameIdx];
        catNameBuffer.append(catNameSv.data(), catNameSv.size());

        if (pt->perm.smartCatsPurchased && cat.type == CatType::Normal && cat.nameIdx % 2u != 0u)
            catNameBuffer += ", PhD";

        // Name text
        textNameBuffer.setString(catNameBuffer);
        textNameBuffer.position = cat.position.addY(48.f);
        textNameBuffer.origin   = textNameBuffer.getLocalBounds().size / 2.f;
        textNameBuffer.scale    = sf::Vec2f{0.5f, 0.5f} * catScaleMult;
        textNameBuffer.setOutlineColor(textOutlineColor);
        textBatchToUse.add(textNameBuffer);

        // Status text
        if (cat.type != CatType::Repulso && cat.type != CatType::Attracto && cat.type != CatType::Duck &&
            !isCopyCatWithType(CatType::Repulso) && !isCopyCatWithType(CatType::Attracto))
        {
            const char* actionName = CatConstants::actionNames[asIdx(
                cat.type == CatType::Copy ? pt->copycatCopiedCatType : cat.type)];

            if (cat.type == CatType::Devil && isDevilcatHellsingedActive())
                actionName = "Portals";

            // TODO P2: move to member data
            static thread_local sf::base::String actionString;
            actionString.clear();

            actionString += sf::base::toString(cat.hits);
            actionString += " ";
            actionString += actionName;

            if (cat.type == CatType::Mouse || isCopyCatWithType(CatType::Mouse))
            {
                actionString += " (x";
                actionString += sf::base::toString(pt->mouseCatCombo + 1);
                actionString += ")";
            }

            textStatusBuffer.setString(actionString);
            textStatusBuffer.position = cat.position.addY(68.f);
            textStatusBuffer.origin   = textStatusBuffer.getLocalBounds().size / 2.f;
            textStatusBuffer.setFillColor(sf::Color::White);
            textStatusBuffer.setOutlineColor(textOutlineColor);
            cat.textStatusShakeEffect.applyToText(textStatusBuffer);
            textStatusBuffer.scale *= 0.5f * catScaleMult;
            textBatchToUse.add(textStatusBuffer);

            // Money text
            if (cat.moneyEarned != 0u)
            {
                char moneyFmtBuffer[128]{};
                std::sprintf(moneyFmtBuffer, "$%s", toStringWithSeparators(cat.moneyEarned));

                textMoneyBuffer.setString(moneyFmtBuffer);
                textMoneyBuffer.position = cat.position.addY(84.f);
                textMoneyBuffer.origin   = textMoneyBuffer.getLocalBounds().size / 2.f;
                textMoneyBuffer.setOutlineColor(textOutlineColor);
                cat.textMoneyShakeEffect.applyToText(textMoneyBuffer);
                textMoneyBuffer.scale *= 0.5f * catScaleMult;
                textBatchToUse.add(textMoneyBuffer);
            }
        }

        const bool hideCooldownBar = inPrestigeTransition || cat.type == CatType::Repulso ||
                                     cat.type == CatType::Attracto || cat.type == CatType::Duck;

        if (!hideCooldownBar)
            textBatchToUse.add(sf::RoundedRectangleShapeData{
                .position = (cat.moneyEarned != 0u ? textMoneyBuffer : textStatusBuffer).getGlobalBottomCenter().addY(2.f),
                .scale              = {catScaleMult, catScaleMult},
                .origin             = {32.f, 0.f},
                .outlineTextureRect = txrWhiteDot,
                .fillColor          = sf::Color::whiteMask(128u),
                .outlineColor       = textOutlineColor,
                .outlineThickness   = 1.f,
                .size               = sf::Vec2f{cat.cooldown.value / maxCooldown * 64.f, 3.f}.clampX(1.f, 64.f),
                .cornerRadius       = 1.f,
                .cornerPointCount   = 8u,
            });
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawShrines(const sf::Vec2f mousePos)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const Shrine* hoveredShrine = nullptr;

    for (const Shrine& shrine : pt->shrines)
    {
        U8 rangeInnerAlpha = 0u;

        if (hoveredShrine == nullptr && (mousePos - shrine.position).lengthSquared() <= shrine.getRadiusSquared() &&
            !mBtnDown(getLMB(), /* penetrateUI */ true))
        {
            hoveredShrine   = &shrine;
            rangeInnerAlpha = 75u;

            if (!pt->shrineHoverTipShown)
            {
                pt->shrineHoverTipShown = true;

                if (pt->psvBubbleValue.nPurchases == 0u)
                {
                    doTip(
                        "Unique cats are sealed inside shrines!\nShrines absorb money, and can have\ndangerous "
                        "effects, read their tooltip!");
                }
            }
        }

        const float invDeathProgress = 1.f - shrine.getDeathProgress();

        const auto shrineAlpha = static_cast<U8>(remap(shrine.getActivationProgress(), 0.f, 1.f, 128.f, 255.f));
        const auto shrineColor = hueColor(shrine.getHue(), shrineAlpha);

        const auto circleColor        = sf::Color{231u, 198u, 39u}.withRotatedHue(shrine.getHue()).withLightness(0.75f);
        const auto circleOutlineColor = circleColor.withAlpha(rangeInnerAlpha);
        const auto textOutlineColor   = circleColor.withLightness(0.25f);

        cpuDrawableBatch.add(
            sf::Sprite{.position    = shrine.getDrawPosition(),
                       .scale       = sf::Vec2f{0.3f, 0.3f} * invDeathProgress +
                                      sf::Vec2f{1.25f, 1.25f} * shrine.textStatusShakeEffect.grow * 0.015f,
                       .origin      = txrShrine.size / 2.f,
                       .textureRect = txrShrine,
                       .color       = shrineColor});

        const auto range = shrine.getRange();

        cpuDrawableBatch.add(sf::CircleShapeData{
            .position           = shrine.position,
            .origin             = {range, range},
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = circleOutlineColor.withAlpha(rangeInnerAlpha),
            .outlineColor       = circleColor,
            .outlineThickness   = 1.f,
            .radius             = range,
            .pointCount         = 64u,
        });

        textNameBuffer.setString(shrineNames[asIdx(shrine.type)]);
        textNameBuffer.position = shrine.position.addY(48.f);
        textNameBuffer.origin   = textNameBuffer.getLocalBounds().size / 2.f;
        textNameBuffer.scale    = sf::Vec2f{0.5f, 0.5f} * invDeathProgress;
        textNameBuffer.setFillColor(sf::Color::White);
        textNameBuffer.setOutlineColor(textOutlineColor);
        catTextDrawableBatch.add(textNameBuffer);

        if (shrine.isActive())
        {
            // TODO P2: move to member data
            static thread_local sf::base::String shrineStatus;

            shrineStatus = "$";
            shrineStatus += toStringWithSeparators(shrine.collectedReward);
            shrineStatus += " / $";
            shrineStatus += toStringWithSeparators(pt->getComputedRequiredRewardByShrineType(shrine.type));

            textStatusBuffer.setString(shrineStatus);
        }
        else
        {
            textStatusBuffer.setString("Inactive");
        }

        textStatusBuffer.position = shrine.position.addY(68.f);
        textStatusBuffer.origin   = textStatusBuffer.getLocalBounds().size / 2.f;
        textStatusBuffer.setFillColor(sf::Color::White);
        textStatusBuffer.setOutlineColor(textOutlineColor);
        shrine.textStatusShakeEffect.applyToText(textStatusBuffer);
        textStatusBuffer.scale *= invDeathProgress;
        textStatusBuffer.scale *= 0.5f;
        cpuDrawableBatch.add(textStatusBuffer);

        if (pt->psvBubbleValue.nPurchases == 0u)
        {
            if (shrine.isActive())
                textMoneyBuffer.setString("Pop bubbles in shrine range to complete it");
            else
                textMoneyBuffer.setString("Buy \"Activate Next Shrine\" in the shop");

            textMoneyBuffer.position = shrine.position.addY(84.f);
            textMoneyBuffer.origin   = textMoneyBuffer.getLocalBounds().size / 2.f;
            textMoneyBuffer.setFillColor(sf::Color::White);
            textMoneyBuffer.setOutlineColor(textOutlineColor);
            shrine.textStatusShakeEffect.applyToText(textMoneyBuffer);
            textMoneyBuffer.scale *= invDeathProgress;
            textMoneyBuffer.scale *= 0.5f;
            cpuDrawableBatch.add(textMoneyBuffer);
        }
    };
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawDolls(const sf::Vec2f mousePos)
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    ////////////////////////////////////////////////////////////
    const sf::Rect2f* dollTxrs[] = {
        &txrDollNormal,   // Normal
        &txrDollUni,      // Uni
        &txrDollDevil,    // Devil
        &txrDollAstro,    // Astro
        &txrDollNormal,   // Witch (missing, hexing a witchcat is not possible, even with copycat)
        &txrDollWizard,   // Wizard
        &txrDollMouse,    // Mouse
        &txrDollEngi,     // Engi
        &txrDollRepulso,  // Repulso
        &txrDollAttracto, // Attracto
        &txrDollNormal,   // Copy (missing, hexing a copycat hexes the mimicked cat)
        &txrDollNormal,   // Duck (missing, cannot be hexed)
    };

    static_assert(sf::base::getArraySize(dollTxrs) == nCatTypes);

    ////////////////////////////////////////////////////////////
    const auto processDolls = [&](auto& container, const float hueMod)
    {
        for (const Doll& doll : container)
        {
            const auto& dollTxr = *dollTxrs[asIdx(doll.catType)];

            const float invDeathProgress = 1.f - doll.getDeathProgress();
            const float progress         = doll.tcDeath.hasValue() ? invDeathProgress : doll.getActivationProgress();

            auto dollAlpha = static_cast<U8>(remap(progress, 0.f, 1.f, 128.f, 255.f));

            if ((mousePos - doll.position).lengthSquared() <= doll.getRadiusSquared() &&
                !mBtnDown(getLMB(), /* penetrateUI */ true))
                dollAlpha = 128.f;

            cpuDrawableBatch.add(
                sf::Sprite{.position    = doll.getDrawPosition(),
                           .scale       = sf::Vec2f{0.22f, 0.22f} * progress,
                           .origin      = dollTxr.size / 2.f,
                           .rotation    = sf::radians(-0.15f + 0.3f * sf::base::sin(doll.wobbleRadians / 2.f)),
                           .textureRect = dollTxr,
                           .color       = hueColor(doll.hue + hueMod, dollAlpha)});
        }
    };

    processDolls(pt->dolls, /* hueMod */ 0.f);
    processDolls(pt->copyDolls, /* hueMod */ 180.f);
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawHellPortals()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    const float hellPortalRadius = pt->getComputedRangeByCatType(CatType::Devil);

    for (const HellPortal& hp : pt->hellPortals)
    {
        const float scaleMult = //
            (hp.life.value > 1500.f)  ? easeOutBack(remap(hp.life.value, 1500.f, 1750.f, 1.f, 0.f))
            : (hp.life.value < 250.f) ? easeOutBack(remap(hp.life.value, 0.f, 250.f, 0.f, 1.f))
                                      : 1.f;

        cpuDrawableBatch.add(
            sf::Sprite{.position    = hp.getDrawPosition(),
                       .scale       = sf::Vec2f{1.f, 1.f} * scaleMult * hellPortalRadius / 256.f * 1.15f,
                       .origin      = txrHellPortal.size / 2.f,
                       .rotation    = sf::radians(hp.life.value / 200.f),
                       .textureRect = txrHellPortal,
                       .color       = sf::Color::White});
    }
}


void Main::gameLoopDrawParticles()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!profile.showParticles)
        return;

    sf::CPUDrawableBatch* const cpuDrawableBatchToUse[nParticleTypes] = {
        &cpuDrawableBatch,
        &cpuDrawableBatch,
        &cpuDrawableBatchAdditive,
        &cpuDrawableBatch,
        &cpuDrawableBatch,
        &cpuDrawableBatch,
        &cpuDrawableBatch,
        &cpuDrawableBatch,
        &cpuDrawableBatch,
        &cpuDrawableBatchAdditive,
        &cpuDrawableBatch,
        &cpuDrawableBatchAdditive,
        &cpuDrawableBatch,
    };

    for (const auto& particle : particles)
    {
        if (!particleCullingBoundaries.isInside(particle.position))
            continue;

        cpuDrawableBatchToUse[asIdx(particle.type)]->add(particleToSprite(particle));
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawHUDParticles()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!profile.showParticles)
        return;

    for (const auto& particle : spentCoinParticles)
        hudDrawableBatch.add(particleToSprite(particle));
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawEarnedCoinParticles()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!profile.showParticles)
        return;

    const auto targetPosition = moneyText.getGlobalCenterRight();

    const auto bezier = [](const sf::Vec2f start, const sf::Vec2f end, const float t)
    {
        const sf::Vec2f control(start.x, end.y);
        const float     u = 1.f - t;

        return u * u * start + 2.f * u * t * control + t * t * end;
    };

    for (const auto& particle : earnedCoinParticles)
    {
        const auto newPos  = bezier(particle.startPosition, targetPosition, easeInOutSine(particle.progress.value));
        const auto newPos2 = bezier(particle.startPosition, targetPosition, easeInOutBack(particle.progress.value));

        const float opacityScale = sf::base::clamp(particle.progress.value, 0.f, 0.15f) / 0.15f;
        const float alpha        = (128.f + particle.progress.remapEased(easeInQuint, 128.f, 0.f)) * opacityScale;

        hudDrawableBatch.add(sf::Sprite{
            .position    = {blend(newPos2.x, newPos.x, 0.5f), newPos.y},
            .scale       = sf::Vec2f{0.25f, 0.25f} * opacityScale,
            .origin      = txrCoin.size / 2.f,
            .rotation    = sf::radians(particle.progress.remap(0.f, sf::base::tau)),
            .textureRect = txrCoin,
            .color       = sf::Color::whiteMask(static_cast<U8>(alpha)),
        });
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawHUDTopParticles()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!profile.showParticles)
        return;

    for (const auto& particle : hudTopParticles)
        hudTopDrawableBatch.add(particleToSprite(particle));
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawHUDBottomParticles()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!profile.showParticles)
        return;

    for (const auto& particle : hudBottomParticles)
        hudBottomDrawableBatch.add(particleToSprite(particle));
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawTextParticles()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (!profile.showTextParticles)
        return;

    for (const auto& tp : textParticles)
    {
        if (!particleCullingBoundaries.isInside(tp.position))
            continue;

        textStatusBuffer.setString(tp.buffer); // TODO P2: (lib) should find a way to assign directly to buffer

        textStatusBuffer.position = tp.position;
        textStatusBuffer.scale    = {tp.scale, tp.scale};
        textStatusBuffer.rotation = sf::radians(tp.rotation);
        textStatusBuffer.origin   = textStatusBuffer.getLocalBounds().size / 2.f;

        const auto opacityAsAlpha = static_cast<sf::base::U8>(tp.opacity * 255.f);
        textStatusBuffer.setFillColor(sf::Color::whiteMask(opacityAsAlpha));
        textStatusBuffer.setOutlineColor(outlineHueColor.withAlpha(opacityAsAlpha));

        cpuDrawableBatch.add(textStatusBuffer);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] float Main::getLeftMostUsefulX() const
{
    const float rightEdgeX = getViewCenter().x + gameView.size.x / 2.f;

    const sf::Vec2f resolution{getResolution()};

    const sf::Rect2f gameViewBounds{getViewportPixelBounds(gameView, resolution)};

    const float menuOverlapScreenX = sf::base::max(uiGetWindowPos().x, gameViewBounds.position.x);

    const float menuOverlapWorldX = gameView
                                        .screenToWorld({menuOverlapScreenX,
                                                        gameViewBounds.position.y + gameViewBounds.size.y / 2.f},
                                                       resolution)
                                        .x;

    return sf::base::min(rightEdgeX, menuOverlapWorldX);
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawScrollArrowHint(const float deltaTimeMs)
{
    if (scrollArrowCountdown.value <= 0.f)
        return;

    if (scroll == 0.f)
        (void)scrollArrowCountdown.updateAndLoop(deltaTimeMs, sf::base::tau * 350.f);
    else
        (void)scrollArrowCountdown.updateAndStop(deltaTimeMs);

    const float blinkOpacity = easeInOutSine(sf::base::fabs(sf::base::sin(
                                   sf::base::remainder(scrollArrowCountdown.value / 350.f, sf::base::tau)))) *
                               255.f;

    const float arrowX = getLeftMostUsefulX();

    rtGame.draw(txArrow,
                {.position = {arrowX, 15.f + (gameScreenSize.y / 5.f) * 1.f},
                 .origin   = txArrow.getRect().getCenterRight(),
                 .color    = sf::Color::whiteMask(static_cast<U8>(blinkOpacity))},
                {.view = gameView});

    rtGame.draw(txArrow,
                {.position = {arrowX, gameScreenSize.y - 15.f - (gameScreenSize.y / 5.f) * 1.f},
                 .origin   = txArrow.getRect().getCenterRight(),
                 .color    = sf::Color::whiteMask(static_cast<U8>(blinkOpacity))},
                {.view = gameView});
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdatePurchaseUnlockedEffects(const float deltaTimeMs)
{
    const float imguiWidth = uiWindowWidth * profile.uiScale;
    const auto  blinkFn    = [](const float value) { return (1 - sf::base::cos(2.f * sf::base::pi * value)) / 2.f; };

    for (auto& [widgetLabel, countdown, arrowCountdown, hue, type] : purchaseUnlockedEffects)
    {
        const float y = uiLabelToY[widgetLabel];

        if (countdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::Running)
        {
            const float x = remap(countdown.value, 0.f, 1000.f, 0.f, imguiWidth);

            const auto pos = sf::Vec2f{uiGetWindowPos().x + x, y + (14.f + rngFast.getF(-14.f, 14.f)) * profile.uiScale};

            for (sf::base::SizeT i = 0u; i < 2u; ++i)
                spawnHUDTopParticle({.position      = pos,
                                     .velocity      = rngFast.getVec2f({-0.04f, -0.04f}, {0.04f, 0.04f}),
                                     .scale         = rngFast.getF(0.08f, 0.27f) * 0.25f * profile.uiScale,
                                     .scaleDecay    = 0.f,
                                     .accelerationY = 0.f,
                                     .opacity       = 1.f,
                                     .opacityDecay  = rngFast.getF(0.00065f, 0.0055f),
                                     .rotation      = rngFast.getF(0.f, sf::base::tau),
                                     .torque        = rngFast.getF(-0.002f, 0.002f)},
                                    /* hue */ wrapHue(165.f + hue + currentBackgroundHue.asDegrees()),
                                    ParticleType::Star);
        }

        if (arrowCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::Running)
        {
            const float blinkProgress = blinkFn(arrowCountdown.getProgressBounced(2000.f));

            const auto arrowAlpha = static_cast<sf::base::U8>(easeInOutCubic(blinkProgress) * 255.f);

            const auto& tx = type == 0 ? txUnlock : txPurchasable;

            rtGame.draw(tx,
                        {.position = {uiGetWindowPos().x, y + 14.f * profile.uiScale},
                         .scale  = sf::Vec2f{0.25f, 0.25f} * (profile.uiScale + -0.15f * easeInOutBack(blinkProgress)),
                         .origin = tx.getRect().getCenterRight(),
                         .color  = hueColor(hue + currentBackgroundHue.asDegrees(), arrowAlpha)},
                        {.view = nonScaledHUDView, .shader = &shader});
        }
    }

    sf::base::vectorEraseIf(purchaseUnlockedEffects, [](const auto& pue) { return pue.arrowCountdown.isDone(); });
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::shouldDrawGrabbingCursor() const
{
    return !draggedCats.empty() || mBtnDown(getRMB(), /* penetrateUI */ true);
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCursor(const float deltaTimeMs, const float cursorGrow)
{
    const sf::Vec2i windowSpaceMousePos = sf::Mouse::getPosition(window);

    const bool mouseNearWindowEdges = windowSpaceMousePos.x < 4 || windowSpaceMousePos.y < 4 ||
                                      windowSpaceMousePos.x > static_cast<int>(window.getSize().x) - 4 ||
                                      windowSpaceMousePos.y > static_cast<int>(window.getSize().y) - 4;

    window.setMouseCursorVisible(!profile.highVisibilityCursor || mouseNearWindowEdges);

    if (!profile.highVisibilityCursor)
        return;

    if (profile.multicolorCursor)
        profile.cursorHue += deltaTimeMs * 0.5f;

    profile.cursorHue = wrapHue(profile.cursorHue);

    rtGame.draw(shouldDrawGrabbingCursor() ? txCursorGrab
                : pt->laserPopEnabled      ? txCursorLaser
                : pt->multiPopEnabled      ? txCursorMultipop
                                           : txCursor,
                {.position = sf::Mouse::getPosition(window).toVec2f(),
                 .scale    = sf::Vec2f{profile.cursorScale, profile.cursorScale} *
                             ((1.f + easeInOutBack(cursorGrow) * sf::base::pow(static_cast<float>(combo), 0.09f)) *
                              dpiScalingFactor),
                 .origin   = {5.f, 5.f},
                 .color    = hueColor(profile.cursorHue + currentBackgroundHue.asDegrees(), 255u)},
                {.view = nonScaledHUDView, .shader = &shader});
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCursorComboText(const float deltaTimeMs, const float cursorGrow)
{
    if (!pt->comboPurchased || !profile.showCursorComboText || shouldDrawGrabbingCursor())
        return;

    static float alpha = 0.f;

    const float scaleMult = profile.cursorScale * dpiScalingFactor;

    if (combo >= 1)
        alpha = 255.f;
    else if (alpha > 0.f)
        alpha -= deltaTimeMs * 0.5f;

    const auto alphaU8 = static_cast<U8>(sf::base::clamp(alpha, 0.f, 255.f));

    cursorComboText.position = sf::Mouse::getPosition(window).toVec2f() + sf::Vec2f{30.f, 48.f} * scaleMult;

    cursorComboText.setFillColor(sf::Color::blackMask(alphaU8));
    cursorComboText.setOutlineColor(
        sf::Color{111u, 170u, 244u, alphaU8}.withRotatedHue(profile.cursorHue + currentBackgroundHue.asDegrees()));

    if (combo > 0)
        cursorComboText.setString("x" + sf::base::toString(combo + 1));

    comboTextShakeEffect.applyToText(cursorComboText);

    cursorComboText.scale *= (static_cast<float>(combo) * 0.65f) * cursorGrow * 0.3f;
    cursorComboText.scale += {0.85f, 0.85f};
    cursorComboText.scale += sf::Vec2f{1.f, 1.f} * comboFailCountdown.value / 325.f;
    cursorComboText.scale *= scaleMult;

    const auto minScale = sf::Vec2f{0.25f, 0.25f} + sf::Vec2f{0.25f, 0.25f} * comboFailCountdown.value / 125.f;

    cursorComboText.scale = cursorComboText.scale.componentWiseClamp(minScale, {1.5f, 1.5f});

    if (comboFailCountdown.value > 0.f)
    {
        cursorComboText.position += rngFast.getVec2f({-5.f, -5.f}, {5.f, 5.f});
        cursorComboText.setFillColor(sf::Color::Red.withAlpha(alphaU8));
    }

    rtGame.draw(cursorComboText, {.view = nonScaledHUDView, .shader = &shader});
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCursorComboBar()
{
    if (!pt->comboPurchased || !profile.showCursorComboBar || comboCountdown.value == 0.f || shouldDrawGrabbingCursor())
        return;

    const float scaleMult = profile.cursorScale * dpiScalingFactor;

    const auto cursorComboBarPosition = sf::Mouse::getPosition(window).toVec2f() + sf::Vec2f{52.f, 14.f} * scaleMult;

    rtGame.draw(
        sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackMask(80u),
            .outlineColor       = cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size = {64.f * scaleMult * pt->psvComboStartTime.currentValue() * 1000.f / 700.f, 24.f * scaleMult},
        },
        {.view = nonScaledHUDView});

    rtGame.draw(
        sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackMask(164u),
            .outlineColor       = cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size               = {64.f * scaleMult * comboCountdown.value / 700.f, 24.f * scaleMult},
        },
        {.view = nonScaledHUDView});
}


[[nodiscard]] sf::Rect2f Main::getViewportPixelBounds(const sf::View& view, const sf::Vec2f targetSize) const
{
    return {{view.viewport.position.x * targetSize.x, view.viewport.position.y * targetSize.y},
            {view.viewport.size.x * targetSize.x, view.viewport.size.y * targetSize.y}};
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::Vec2f Main::getEdgeSpawnPosition(const sf::Rect2f& bounds, const float thickness)
{
    // Randomly select one of the four edges: 0=top, 1=bottom, 2=left, 3=right.
    const int edge = rngFast.getI<int>(0, 3);

    // Top edge
    if (edge == 0)
        return {bounds.position.x + rngFast.getF(0.f, bounds.size.x), bounds.position.y + rngFast.getF(0.f, thickness)};

    // Bottom edge
    if (edge == 1)
        return {bounds.position.x + rngFast.getF(0.f, bounds.size.x),
                bounds.position.y + bounds.size.y - rngFast.getF(0.f, thickness)};

    // Left edge
    if (edge == 2)
        return {bounds.position.x + rngFast.getF(0.f, thickness), bounds.position.y + rngFast.getF(0.f, bounds.size.y)};

    SFML_BASE_ASSERT(edge == 3);

    // Right edge
    return {bounds.position.x + bounds.size.x - rngFast.getF(0.f, thickness),
            bounds.position.y + rngFast.getF(0.f, bounds.size.y)};
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawDollParticleBorder(const float hueMod)
{
    if (!profile.showDollParticleBorder)
        return;

    for (int i = 0; i < 10; ++i)
    {
        const sf::Rect2f gameViewBounds = getViewportPixelBounds(gameView, getResolution());
        const sf::Vec2f  spawnPos       = getEdgeSpawnPosition(gameViewBounds, 10.f);

        spawnHUDBottomParticle({.position      = spawnPos,
                                .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                                .scale         = rngFast.getF(0.12f, 0.52f) * 0.65f,
                                .scaleDecay    = 0.f,
                                .accelerationY = 0.f,
                                .opacity       = 1.f,
                                .opacityDecay  = rngFast.getF(0.0015f, 0.0025f) * 0.65f,
                                .rotation      = rngFast.getF(0.f, sf::base::tau),
                                .torque        = rngFast.getF(-0.002f, 0.002f)},
                               /* hue */ wrapHue(rngFast.getF(-50.f, 50.f) + hueMod),
                               ParticleType::Hex);
    }
}


////////////////////////////////////////////////////////////
void Main::gameLoopTips(const float deltaTimeMs)
{
    if (!profile.tipsEnabled)
    {
        resetTipState();
        return;
    }

    if (!tipTCByte.hasValue())
        return;

    bool mustSpawnByteParticles = false;
    if (tipTCByte->updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
    {
        mustSpawnByteParticles = true;
        tipTCBackground.emplace(TargetedCountdown{.startingValue = 500.f});
        tipTCBackground->restart();
    }

    if (tipTCBackground.hasValue())
        (void)tipTCBackground->updateAndStop(deltaTimeMs);

    if (tipTCByteEnd.hasValue() && tipTCByteEnd->updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
    {
        resetTipState();
        return;
    }

    const float bgProgress   = tipTCByteEnd.hasValue() ? tipTCByteEnd->getInvProgress() : tipTCBackground.getProgress();
    const float byteProgress = tipTCByteEnd.hasValue() ? tipTCByteEnd->getInvProgress() : tipTCByte.getProgress();

    const float tipByteAlpha       = byteProgress * 255.f;
    const float tipBackgroundAlpha = bgProgress * 255.f;

    sf::Sprite tipBackgroundSprite{.position = {},
                                   .scale  = sf::Vec2f{0.4f, 0.4f} + sf::Vec2f{0.4f, 0.4f} * easeInOutBack(bgProgress),
                                   .origin = txTipBg.getSize().toVec2f() / 2.f,
                                   .textureRect = txTipBg.getRect(),
                                   .color       = sf::Color::whiteMask(static_cast<U8>(tipBackgroundAlpha * 0.85f))};


    SFML_BASE_ASSERT(profile.hudScale > 0.f);

    tipBackgroundSprite.setGlobalBottomCenter(
        {getResolution().x / 2.f / profile.hudScale, getResolution().y / profile.hudScale - 50.f});

    rtGame.draw(tipBackgroundSprite, {.view = scaledHUDView, .texture = &txTipBg});

    sf::Sprite tipByteSprite{.position    = {},
                             .scale       = sf::Vec2f{0.85f, 0.85f} * easeInOutBack(byteProgress),
                             .origin      = txTipByte.getSize().toVec2f() / 2.f,
                             .rotation    = sf::radians(sf::base::tau * easeInOutBack(byteProgress)),
                             .textureRect = txTipByte.getRect(),
                             .color       = sf::Color::whiteMask(static_cast<U8>(tipByteAlpha))};

    tipByteSprite.setGlobalCenter(tipBackgroundSprite.getGlobalCenterRight().addY(-40.f));
    rtGame.draw(tipByteSprite, {.view = scaledHUDView, .texture = &txTipByte});

    if (mustSpawnByteParticles)
    {
        for (SizeT i = 0u; i < 32u; ++i)
            spawnHUDTopParticle({.position      = tipByteSprite.position,
                                 .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.1f}) * 1.5f,
                                 .scale         = rngFast.getF(0.18f, 0.32f) * 1.55f,
                                 .scaleDecay    = 0.f,
                                 .accelerationY = 0.0015f,
                                 .opacity       = 1.f,
                                 .opacityDecay  = rngFast.getF(0.00025f, 0.0015f) * 0.5f,
                                 .rotation      = rngFast.getF(0.f, sf::base::tau),
                                 .torque        = rngFast.getF(-0.002f, 0.002f)},
                                /* hue */ 0.f,
                                ParticleType::Star);
    }

    const auto getCharDelay = [](const char c)
    {
        if (c == '\t')
            return 250.f;

        if (c == '.' || c == ',' || c == '!' || c == '?' || c == ':')
            return 250.f;

        return 30.f;
    };

    const auto getCharSound = [](const char c) { return std::isalnum(c); };

    if (tipTCBackground.hasValue() && tipTCBackground->isDone() && tipCharIdx < tipString.size() &&
        tipCountdownChar.isDone())
        tipCountdownChar.value = tipCharIdx > 0u ? getCharDelay(tipString[tipCharIdx - 1u]) : 50.f;

    if (tipCountdownChar.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
    {
        if (getCharSound(tipString[tipCharIdx]))
        {
            sounds.byteSpeak.settings.pitch = 1.6f;
            playSound(sounds.byteSpeak, /* maxOverlap */ 1u);
        }

        ++tipCharIdx;

        if (tipCharIdx == tipString.size())
        {
            tipTCBytePreEnd.emplace(TargetedCountdown{.startingValue = 250.f + static_cast<float>(tipString.size()) * 20.f});
            tipTCBytePreEnd->restart();
        }
    }

    if (tipTCBytePreEnd.hasValue() && tipTCBytePreEnd->updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
    {
        tipTCByteEnd.emplace(TargetedCountdown{.startingValue = 750.f});
        tipTCByteEnd->restart();
    }

    sf::Text tipText{fontSuperBakery,
                     {
                         .position      = {},
                         .scale         = sf::Vec2f{0.5f, 0.5f} * easeInOutBack(byteProgress),
                         .string        = tipString.toStringView().substrByPosLen(0, tipCharIdx).to<sf::base::String>(),
                         .characterSize = 60u,
                         .fillColor     = sf::Color::whiteMask(static_cast<sf::base::U8>(tipByteAlpha)),
                         .outlineColor  = outlineHueColor.withAlpha(static_cast<sf::base::U8>(tipByteAlpha)),
                         .outlineThickness = 4.f,
                     }};

    tipText.setGlobalTopLeft(tipBackgroundSprite.getGlobalTopLeft() + sf::Vec2f{45.f, 65.f});

    tipStringWiggle.advance(deltaTimeMs);
    tipStringWiggle.apply(tipText);

    rtGame.draw(tipText, {.view = scaledHUDView});

    tipStringWiggle.unapply(tipText);
}


////////////////////////////////////////////////////////////
void Main::recreateImGuiRenderTexture(const sf::Vec2u newResolution)
{
    rtImGui = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .smooth = true}).value();
}


////////////////////////////////////////////////////////////
void Main::recreateBackgroundRenderTexture(const sf::Vec2u newResolution)
{
    rtBackground = sf::RenderTexture::create(newResolution,
                                             {.antiAliasingLevel = aaLevel, .smooth = true, .wrapMode = sf::TextureWrapMode::Repeat})
                       .value();

    rtBackgroundProcessed = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .smooth = true}).value();
}


////////////////////////////////////////////////////////////
void Main::recreateGameRenderTexture(const sf::Vec2u newResolution)
{
    rtCloudMask      = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .smooth = true}).value();
    rtCloudProcessed = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .smooth = true}).value();
    rtGame           = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .smooth = true}).value();
}


////////////////////////////////////////////////////////////
void Main::setPostProcessUniforms(const float vibrance,
                                  const float saturation,
                                  const float lightness,
                                  const float sharpness,
                                  const float blur) const
{
    shaderPostProcess.setUniform(suPPVibrance, vibrance);
    shaderPostProcess.setUniform(suPPSaturation, saturation);
    shaderPostProcess.setUniform(suPPLightness, lightness);
    shaderPostProcess.setUniform(suPPSharpness, sharpness);
    shaderPostProcess.setUniform(suPPBlur, blur);
}


////////////////////////////////////////////////////////////
void Main::updateProcessedBackground()
{
    rtBackgroundProcessed.clear(sf::Color::Transparent);

    setPostProcessUniforms(profile.ppBGVibrance,
                           profile.ppBGSaturation,
                           profile.ppBGLightness,
                           profile.ppBGSharpness,
                           profile.ppBGBlur);

    rtBackgroundProcessed.draw(rtBackground.getTexture(), {.shader = &shaderPostProcess});
    rtBackgroundProcessed.display();
}


////////////////////////////////////////////////////////////
void Main::gameLoopDisplayCloudBatch(const sf::CPUDrawableBatch& batch, const sf::View& view)
{
    if (batch.isEmpty())
        return;

    rtCloudMask.clear(sf::Color::Transparent);
    rtCloudMask.draw(batch, {.view = view, .texture = &textureAtlas.getTexture()});
    rtCloudMask.display();

    shaderClouds.setUniform(suCloudTime, shaderTime);
    shaderClouds.setUniform(suCloudResolution, rtCloudMask.getSize().toVec2f());

    rtCloudProcessed.clear(sf::Color::Transparent);
    rtCloudProcessed.draw(rtCloudMask.getTexture(), {.blendMode = sf::BlendNone, .shader = &shaderClouds});
    rtCloudProcessed.display();

    constexpr sf::BlendMode premultipliedAlphaBlend(sf::BlendMode::Factor::One,
                                                    sf::BlendMode::Factor::OneMinusSrcAlpha,
                                                    sf::BlendMode::Equation::Add,
                                                    sf::BlendMode::Factor::One,
                                                    sf::BlendMode::Factor::OneMinusSrcAlpha,
                                                    sf::BlendMode::Equation::Add);

    rtCloudMask.clear(sf::Color::Transparent);
    rtCloudMask.draw(rtCloudProcessed.getTexture(), {.blendMode = sf::BlendNone});
    rtCloudMask.display();

    const auto opacity = static_cast<U8>(profile.catCloudOpacity * 255.f);

    rtGame.draw(rtCloudMask.getTexture(),
                {.color = sf::Color{opacity, opacity, opacity, opacity}}, // because of premultiplication
                {.blendMode = premultipliedAlphaBlend});
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::Vec2u Main::getNewResolution() const
{
    return profile.resWidth == sf::Vec2u{} ? getReasonableWindowSize(0.9f) : profile.resWidth;
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::RenderWindow Main::makeWindow()
{
    const sf::Vec2u desktopResolution = sf::VideoModeUtils::getDesktopMode().size;
    const sf::Vec2u newResolution     = getNewResolution();

    const bool takesAllScreen = newResolution == desktopResolution;

    return sf::RenderWindow::create(
               {
                   .size           = newResolution,
                   .title          = "BubbleByte " BUBBLEBYTE_VERSION_STR,
                   .fullscreen     = !profile.windowed,
                   .resizable      = !takesAllScreen,
                   .closable       = !takesAllScreen,
                   .hasTitlebar    = !takesAllScreen,
                   .vsync          = profile.vsync,
                   .frametimeLimit = sf::base::clamp(profile.frametimeLimit, 60u, 144u),
               })
        .value();
}


////////////////////////////////////////////////////////////
void Main::recreateWindow()
{
    const sf::Vec2u newResolution = getNewResolution();

    window = makeWindow();
    refreshWindowAutoBatchModeFromProfile();

    recreateBackgroundRenderTexture(getExpandedGameViewSize(gameScreenSize, newResolution.toVec2f()).toVec2u());
    recreateImGuiRenderTexture(newResolution);
    recreateGameRenderTexture(newResolution);

    dpiScalingFactor = window.getWindowDisplayScale();
}


////////////////////////////////////////////////////////////
void Main::resizeWindow()
{
    const sf::Vec2u desktopResolution = sf::VideoModeUtils::getDesktopMode().size;
    const sf::Vec2u newResolution     = getNewResolution();

    const bool takesAllScreen = newResolution == desktopResolution;

    window.setResizable(!takesAllScreen);
    window.setHasTitlebar(!takesAllScreen);
    window.setSize(newResolution);

    if (!takesAllScreen)
        window.setPosition(((desktopResolution - newResolution) / 2u).toVec2i());

    recreateBackgroundRenderTexture(getExpandedGameViewSize(gameScreenSize, newResolution.toVec2f()).toVec2u());
    recreateImGuiRenderTexture(newResolution);
    recreateGameRenderTexture(newResolution);

    dpiScalingFactor = window.getWindowDisplayScale();
}


////////////////////////////////////////////////////////////
[[nodiscard]] float Main::gameLoopUpdateCursorGrowthEffect(const float deltaTimeMs, const bool anyBubblePoppedByClicking)
{
    static float cursorGrow = 0.f;

    if (anyBubblePoppedByClicking)
        cursorGrow = 0.49f;

    if (cursorGrow >= 0.f)
    {
        cursorGrow -= deltaTimeMs * 0.0015f;
        cursorGrow = sf::base::max(cursorGrow, 0.f);
    }

    return cursorGrow;
}
