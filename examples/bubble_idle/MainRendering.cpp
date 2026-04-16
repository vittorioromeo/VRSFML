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
#include "GameEvent.hpp"
#include "HellPortal.hpp"
#include "HexSession.hpp"
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
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/TrapezoidShapeData.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"

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
#include "SFML/Base/OverloadSet.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToString.hpp"
#include "SFML/Base/Vector.hpp"

#include <cctype>
#include <cstdio>

////////////////////////////////////////////////////////////
void Main::drawCloudFrame(const CloudFrameDrawSettings& settings)
{
    const auto& [time, mins, maxs, xSteps, ySteps, scaleMult, outwardOffsetMult, color, batch] = settings;

    const auto vec2Lerp = [](const sf::Vec2f a, const sf::Vec2f b, const sf::Vec2f t)
    { return sf::Vec2f(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); };

    for (int iX = 0; iX < xSteps; ++iX)
    {
        for (int iY = 0; iY < ySteps; ++iY)
        {
            if (iX != 0 && iY != 0 && iX != xSteps - 1 && iY != ySteps - 1)
                continue;

            const float tX = static_cast<float>(iX) / static_cast<float>(xSteps - 1);
            const float tY = static_cast<float>(iY) / static_cast<float>(ySteps - 1);

            const auto p0 = vec2Lerp(mins, maxs, {tX, tY});

            const float normalX    = iX == 0 ? -1.f : (iX == xSteps - 1 ? 1.f : 0.f);
            const float normalY    = iY == 0 ? -1.f : (iY == ySteps - 1 ? 1.f : 0.f);
            const float cornerMult = (normalX != 0.f && normalY != 0.f) ? 0.70710677f : 1.f;

            const float outwardX = normalX * cornerMult;
            const float outwardY = normalY * cornerMult;
            const float tangentX = -outwardY;
            const float tangentY = outwardX;

            const float noiseSeed = tX * 173.13f + tY * 317.71f;

            const float noise0 = sf::base::sin(noiseSeed * 3.17f + 1.2f) * 0.5f + 0.5f;
            const float noise1 = sf::base::sin(noiseSeed * 5.83f + 4.7f) * 0.5f + 0.5f;
            const float noise2 = sf::base::cos(noiseSeed * 4.11f + 2.3f) * 0.5f + 0.5f;

            const float restOutward = (noise0 * noise1) * 12.f - 2.f;
            const float restTangent = (noise2 - 0.5f) * 5.f;

            const float phase0 = time * (0.45f + noise0 * 0.35f) + noiseSeed * 0.35f;
            const float phase1 = time * (0.90f + noise1 * 0.55f) - noiseSeed * 0.21f;

            const float outwardOffset = (restOutward + sf::base::sin(phase0) * (1.5f + noise0 * 2.5f) +
                                         sf::base::sin(phase1) * (0.5f + noise1 * 1.25f)) *
                                        outwardOffsetMult;

            const float tangentOffset = restTangent +
                                        sf::base::cos(time * (0.6f + noise2 * 0.7f) + noiseSeed * 0.47f) *
                                            (0.5f + noise2 * 2.f) +
                                        sf::base::sin(phase0 * 1.37f + noise1 * 3.f) * 0.75f;

            const sf::Vec2f animatedP{p0.x + outwardX * outwardOffset + tangentX * tangentOffset,
                                      p0.y + outwardY * outwardOffset + tangentY * tangentOffset};

            const float puffScale = (0.58f + noise0 * 0.26f) * scaleMult;

            batch->add(sf::Sprite{
                .position    = animatedP,
                .scale       = {puffScale, puffScale},
                .origin      = txrCloud.size / 2.f,
                .textureRect = txrCloud,
                .color       = color,
            });

            if (((iX + iY) % 3) == 0)
            {
                const float clusterOutward = outwardOffset - (3.f + noise1 * 4.f);
                const float clusterTangent = tangentOffset + (noise2 - 0.5f) * 8.f;

                batch->add(sf::Sprite{
                    .position    = {p0.x + outwardX * clusterOutward + tangentX * clusterTangent,
                                    p0.y + outwardY * clusterOutward + tangentY * clusterTangent},
                    .scale       = {puffScale, puffScale},
                    .origin      = txrCloud.size / 2.f,
                    .textureRect = txrCloud,
                    .color       = color,
                });
            }
        }
    }
}

////////////////////////////////////////////////////////////
void Main::drawMinimap(bool               back,
                       sf::RenderTarget&  rt,
                       const sf::View&    hudView,
                       const sf::Vec2f    resolution,
                       const sf::base::U8 shouldDrawUIAlpha)
{
    constexpr sf::Vec2f minimapPos = {10.f, 10.f};

    const float     minimapScale = profile.minimapScale;
    const float     mapLimit     = pt->getMapLimit();
    const float     hudScale     = profile.hudScale;
    const float     hueMod       = currentBackgroundHue.asDegrees();
    const sf::Vec2f minimapSize  = boundaries / minimapScale;

    const sf::RoundedRectangleShapeData minimapBorder{
        .position         = minimapPos,
        .fillColor        = sf::Color::Transparent,
        .outlineColor     = sf::Color::whiteWithAlpha(static_cast<U8>(shouldDrawUIAlpha * 0.75f)),
        .outlineThickness = -4.f,
        .size             = {mapLimit / minimapScale, minimapSize.y},
        .cornerRadius     = 8.f,
    };

    uiState.minimapRect.position = minimapPos;
    uiState.minimapRect.size     = minimapBorder.size;

    sf::Vec2f offset{-10.f, -10.f};

    if (back)
        drawCloudFrame({
            .time              = shaderTime,
            .mins              = minimapBorder.position - offset - sf::Vec2f{5.f, 5.f},
            .maxs              = minimapBorder.position + minimapBorder.size + offset,
            .xSteps            = 16 + 4 * static_cast<int>(pt->getMapLimitIncreases()),
            .ySteps            = 8,
            .scaleMult         = 1.6f,
            .outwardOffsetMult = 1.f,
            .batch             = &cpuCloudHudDrawableBatch,
        });


    const float visibleLeft  = sf::base::max(0.f, gameView.center.x - gameView.size.x / 2.f);
    const float visibleRight = sf::base::min(mapLimit, gameView.center.x + gameView.size.x / 2.f);

    const sf::RectangleShapeData
        minimapIndicator{.position         = minimapPos.addX(visibleLeft / minimapScale),
                         .fillColor        = sf::Color::Transparent,
                         .outlineColor     = sf::Color::Blue.withRotatedHue(hueMod).withAlpha(shouldDrawUIAlpha),
                         .outlineThickness = 3.f,
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

    const sf::Vec2f backgroundRectSize{static_cast<float>(txBackgroundChunk.getSize().x) * nGameScreens,
                                       static_cast<float>(txBackgroundChunk.getSize().y)};

    if (!back)
    {
        rt.draw(sf::RectangleShapeData{.fillColor = sf::Color::blackWithAlpha(shouldDrawUIAlpha),
                                       .size      = boundaries * hudScale},
                {.view = minimapView});

        rt.draw(txBackgroundChunk,
                {.scale       = {hudScale, hudScale},
                 .textureRect = {{0.f, 0.f}, backgroundRectSize},
                 .color       = hueColor(hueMod, sf::base::min(shouldDrawUIAlpha, static_cast<sf::base::U8>(128u)))},
                {.view = minimapView, .shader = &shader});

        rt.draw(txDrawings,
                {.scale       = {hudScale, hudScale},
                 .textureRect = {{0.f, 0.f}, backgroundRectSize},
                 .color = sf::Color::whiteWithAlpha(sf::base::min(shouldDrawUIAlpha, static_cast<sf::base::U8>(215u)))},
                {.view = minimapView, .shader = &shader});

        if (shouldDrawUIAlpha > 200u)
        {
            minimapDrawableBatch.scale = {hudScale, hudScale};
            rt.draw(minimapDrawableBatch, {.view = minimapView, .texture = &textureAtlas.getTexture(), .shader = &shader});
            minimapDrawableBatch.scale = {1.f, 1.f};
        }


        rt.draw(minimapBorder, {.view = hudView});
    }

    if (0)
    {
        const float frameOffset = 3.f;

        NinePatchRect panel{
            .position    = uiState.minimapRect.position - sf::Vec2f{frameOffset, frameOffset},
            .size        = uiState.minimapRect.size + sf::Vec2f{frameOffset * 2.f, frameOffset * 2.f},
            .textureRect = txFrameTiny.getRect(),
            .borders     = NinePatchBorders::all(18.f),
            .color       = hueColor(hueMod, shouldDrawUIAlpha),
        };

        panel.draw(rt, txFrameTiny, {.view = hudView, .shader = &shader});
    }

    if (!back)
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
                       .color       = sf::Color::whiteWithAlpha(static_cast<U8>(easeInOutSine(progress) * 255.f))},
            {.view = view, .texture = &txLogo});
}


void Main::gameLoopDrawBubbles()
{
    const auto getBubbleHue = [&](const Bubble& bubble)
    {
        if (bubble.type == BubbleType::Bomb)
            return 0.f;

        if (bubble.type == BubbleType::Star)
            return bubble.hueMod;

        if (bubble.type == BubbleType::Nova)
            return bubble.hueMod * 2.f;

        if (bubble.type == BubbleType::Combo)
        {
            // Slowly rotating gold-ish hue.
            constexpr float goldBaseHue = 50.f;
            return goldBaseHue + sf::base::sin(bubble.hueMod * 0.001f) * 8.f;
        }

        SFML_BASE_ASSERT(bubble.type == BubbleType::Normal);

        constexpr float hueRange = 75.f;

        const bool beingRepelledOrAttracted = !bubble.attractedCountdown.isDone() || !bubble.repelledCountdown.isDone();

        const float magnetHueMod = (beingRepelledOrAttracted ? 180.f : 0.f);

        return sf::base::remainder(static_cast<float>(bubble.hueSeed) * 2.f - hueRange / 2.f, hueRange) + magnetHueMod;
    };

    const sf::Rect2f bubbleRects[]{txrBubble, txrBubbleStar, txrBomb, txrBubbleNova, txrBubbleGlass};
    static_assert(sf::base::getArraySize(bubbleRects) == nBubbleTypes);

    sf::CPUDrawableBatch* const batchToUseByType[]{&bubbleDrawableBatch,
                                                   &starBubbleDrawableBatch,
                                                   &bombBubbleDrawableBatch,
                                                   &starBubbleDrawableBatch,
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

        // Combo bubble shake: after the first click, wobble more and more as the timer runs out.
        sf::Vec2f shakeOffset = {};
        if (bubble.type == BubbleType::Combo && bubble.comboClickCount > 0u && bubble.comboTimerMs > 0.f)
        {
            const float maxMs   = gameConstants.events.invincibleBubble.comboTimerMaxMs;
            const float frac    = sf::base::clamp(bubble.comboTimerMs / (maxMs <= 0.f ? 1.f : maxMs), 0.f, 1.f);
            const float urgency = 1.f - frac;
            const float amp     = urgency * urgency * 8.f; // quadratic ramp, peaks ~8px right before pop
            shakeOffset         = {rngFast.getF(-amp, amp), rngFast.getF(-amp, amp)};
        }

        batchToUseByType[asIdx(bubble.type)]->add(sf::Sprite{
            .position    = bubble.position + shakeOffset,
            .scale       = {bubble.radius * scaleMult, bubble.radius * scaleMult},
            .origin      = rect.size / 2.f,
            .rotation    = sf::radians(bubble.rotation),
            .textureRect = rect,
            .color       = hueColor(getBubbleHue(bubble), 255u),
        });

        // Visible marker for the invincible Combo bubble so the player can
        // track it. TODO P2: replace with a dedicated sprite/glow.
        if (bubble.type == BubbleType::Combo)
        {
            // Combo timer ring: only after the first click. Full ring at full
            // timer, depleting clockwise as the timer runs out.
            if (bubble.comboClickCount > 0u && bubble.comboTimerMs > 0.f)
            {
                const float maxMs = gameConstants.events.invincibleBubble.comboTimerMaxMs;
                const float frac  = sf::base::clamp(bubble.comboTimerMs / (maxMs <= 0.f ? 1.f : maxMs), 0.f, 1.f);

                constexpr float thickness = 3.f;
                const float     outerR    = bubble.radius;

                batchToUseByType[asIdx(bubble.type)]->add(sf::RingPieSliceShapeData{
                    .position    = bubble.position + shakeOffset,
                    .origin      = {outerR, outerR},
                    .textureRect = txrWhiteDot,
                    .fillColor   = sf::Color::whiteWithAlpha(64u),
                    .outerRadius = outerR,
                    .innerRadius = outerR - thickness,
                    .startAngle  = sf::degrees(-90.f),
                    .sweepAngle  = sf::radians(sf::base::tau * frac),
                    .pointCount  = 32u,
                });
            }
        }
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

    if (comboState.combo <= 1 && profile.cursorTrailMode == 0 /* combo mode */)
        return;

    const sf::Vec2f mousePosDiff    = playerInputState.lastMousePos - mousePos;
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
        &txrMMNormal, // Warden -- TODO: dedicated minimap icon (reusing Normal)
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

    const auto addDollsToMinimap = [&](const sf::base::Vector<HexSession>& sessions, const float hueMod)
    {
        for (const HexSession& session : sessions)
            for (const Doll& doll : session.dolls)
                minimapDrawableBatch.add(
                    sf::Sprite{.position    = doll.position,
                               .scale       = {0.5f, 0.5f},
                               .origin      = txrDollNormal.size / 2.f,
                               .rotation    = sf::radians(0.f),
                               .textureRect = txrDollNormal,
                               .color       = hueColor(doll.hue + hueMod, 255u)});
    };

    addDollsToMinimap(pt->hexSessions, 0.f);
    addDollsToMinimap(pt->copyHexSessions, 180.f);
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
        &txrCat,       // Normal
        uniCatTxr,     // Uni
        devilCatTxr,   // Devil
        &txrAstroCat,  // Astro
        &txrWardenCat, // Warden (composite -- guardhouse drawn around it)

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
        &txrCatPaw,       // Normal
        &txrUniCatPaw,    // Uni
        devilCatPawTxr,   // Devil
        &txrWhiteDot,     // Astro
        &txrWardencatPaw, // Warden (paw resting on the guardhouse windowsill)

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
        &txrCatTail,      // Warden -- TODO: dedicated tail texture (reusing Normal)

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
    for (Cat& cat : pt->cats)
        gameLoopDrawCat(cat, deltaTimeMs, mousePos, catTxrsByType, catPawTxrsByType, catTailTxrsByType);
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

namespace
{
struct CatTextureTables
{
    const sf::Rect2f* const (&catTxrsByType)[nCatTypes];
    const sf::Rect2f* const (&catPawTxrsByType)[nCatTypes];
    const sf::Rect2f* const (&catTailTxrsByType)[nCatTypes];
};

struct CatDrawContext
{
    Main&                          main;
    Cat&                           cat;
    sf::CPUDrawableBatch&          batchToUse;
    sf::CPUDrawableBatch&          textBatchToUse;
    sf::CPUDrawableBatch&          spriteBatchToUse;
    sf::CPUDrawableBatch&          cloudBatchToUse;
    Cat*                           witchCat;
    Cat*                           copyCat;
    const sf::Rect2f&              catTxr;
    const sf::Rect2f&              catPawTxr;
    const sf::Rect2f&              catTailTxr;
    sf::Vec2f                      catTailOffset;
    sf::Vec2f                      catEyeOffset;
    float                          deltaTimeMs;
    sf::Vec2f                      mousePos;
    bool                           drawHexedWithShader;
    bool                           beingDragged;
    sf::base::Optional<sf::Rect2f> dragRect;
    bool                           insideDragRect;
    bool                           hovered;
    bool                           shouldDisplayRangeCircle;
    U8                             rangeInnerAlpha;
    float                          maxCooldown;
    float                          cooldownDiff;
    float                          catRotation;
    float                          bodyRotationExtra; // tail-wobble + bonk pendulum etc. (added on top of catRotation)
    float                          range;
    U8                             alpha;
    sf::Color                      catColor;
    U8                             circleAlpha;
    sf::Color                      circleColor;
    sf::Color                      circleOutlineColor;
    sf::Color                      textOutlineColor;
    bool                           shouldDrawCatRange;
    float                          catScaleMult;
    sf::Vec2f                      catScale;
    sf::Vec2f                      catAnchor;
    sf::Vec2f                      visualCatAnchor;
    sf::Vec2f                      pushDown;
    sf::Color                      attachmentHue;
    float                          hexedEffectStrength;

    [[nodiscard]] bool isCopyCatWithType(const CatType copiedType) const
    {
        return cat.type == CatType::Copy && main.pt->copycatCopiedCatType == copiedType;
    }

    [[nodiscard]] sf::Angle bodyRotation() const
    {
        return sf::radians(catRotation + bodyRotationExtra);
    }

    [[nodiscard]] sf::Vec2f anchorOffset(const sf::Vec2f offset) const
    {
        return visualCatAnchor + (offset / 2.f * 0.2f * catScaleMult).rotatedBy(bodyRotation());
    }
};

[[nodiscard]] float advanceCatCloudTime(Main& main, Cat& cat, const float deltaTimeMs)
{
    static float cloudTime = 0.f;

    if (!main.pt->cats.empty() && &cat == &main.pt->cats.front())
        cloudTime += deltaTimeMs * 0.4f;

    return cloudTime;
}

void applyWitchAnimation(CatDrawContext& ctx, float& wobblePhase, Cat& witch)
{
    if (&ctx.cat == &witch)
    {
        if (witch.cooldown.value >= 10'000.f)
        {
            if (wobblePhase > 0.f)
                wobblePhase -= ctx.deltaTimeMs * 0.005f;

            wobblePhase = sf::base::max(wobblePhase, 0.f);
        }
        else
        {
            const float frequency = remap(sf::base::min(witch.cooldown.value, 10'000.f), 0.f, 10'000.f, 0.1f, 0.05f);

            wobblePhase += frequency * ctx.deltaTimeMs * 0.05f;
            wobblePhase = sf::base::remainder(wobblePhase, sf::base::tau);
        }
    }

    if (ctx.main.isCatPerformingRitual(witch, ctx.cat))
    {
        const float amplitude = remap(sf::base::min(witch.cooldown.value, 10'000.f), 0.f, 10'000.f, 0.5f, 0.f);
        ctx.catRotation       = sf::base::sin(wobblePhase) * amplitude;
    }
}

[[nodiscard]] CatDrawContext makeCatDrawContext(
    Main&                   main,
    Cat&                    cat,
    const float             deltaTimeMs,
    const sf::Vec2f         mousePos,
    const CatTextureTables& textureTables)
{
    auto& batchToUse = main.playerInputState.catToPlace == &cat ? main.cpuTopDrawableBatch : main.cpuDrawableBatch;
    auto& textBatchToUse = main.playerInputState.catToPlace == &cat ? main.catTextTopDrawableBatch : main.catTextDrawableBatch;

    const bool drawHexedWithShader = cat.isHexedOrCopyHexed() &&
                                     main.hexedCatDrawCommands.size() < Main::maxHexedCatRenderTextures;
    auto&      spriteBatchToUse    = drawHexedWithShader ? main.tempDrawableBatch : batchToUse;
    auto& cloudBatchToUse = main.playerInputState.catToPlace == &cat ? main.cpuTopCloudDrawableBatch : main.cpuCloudDrawableBatch;

    CatDrawContext ctx{
        .main                     = main,
        .cat                      = cat,
        .batchToUse               = batchToUse,
        .textBatchToUse           = textBatchToUse,
        .spriteBatchToUse         = spriteBatchToUse,
        .cloudBatchToUse          = cloudBatchToUse,
        .witchCat                 = main.getWitchCat(),
        .copyCat                  = main.getCopyCat(),
        .catTxr                   = *textureTables.catTxrsByType[asIdx(cat.type)],
        .catPawTxr                = *textureTables.catPawTxrsByType[asIdx(
            cat.type == CatType::Copy && main.pt->copycatCopiedCatType == CatType::Mouse ? CatType::Mouse : cat.type)],
        .catTailTxr               = *textureTables.catTailTxrsByType[asIdx(cat.type)],
        .catTailOffset            = main.gameConstants.catTailOffsetsByType[asIdx(cat.type)],
        .catEyeOffset             = main.gameConstants.catEyeOffsetsByType[asIdx(cat.type)],
        .deltaTimeMs              = deltaTimeMs,
        .mousePos                 = mousePos,
        .drawHexedWithShader      = drawHexedWithShader,
        .beingDragged             = main.isCatBeingDragged(cat),
        .dragRect                 = main.getAoEDragRect(mousePos),
        .insideDragRect           = false,
        .hovered                  = false,
        .shouldDisplayRangeCircle = false,
        .rangeInnerAlpha          = 0u,
        .maxCooldown              = main.getComputedCooldownByCatTypeOrCopyCat(cat.type),
        .cooldownDiff             = cat.cooldown.value,
        .catRotation              = 0.f,
        .bodyRotationExtra        = 0.f,
        .range                    = main.getComputedRangeByCatTypeOrCopyCat(cat.type),
        .alpha                    = 255u,
        .catColor                 = sf::Color::White,
        .circleAlpha              = 0u,
        .circleColor              = sf::Color::White,
        .circleOutlineColor       = sf::Color::White,
        .textOutlineColor         = sf::Color::White,
        .shouldDrawCatRange       = false,
        .catScaleMult             = easeOutElastic(cat.spawnEffectTimer.value),
        .catScale                 = {},
        .catAnchor                = {},
        .visualCatAnchor          = {},
        .pushDown                 = {},
        .attachmentHue            = sf::Color::White,
        .hexedEffectStrength      = cat.isHexedOrCopyHexed() ? cat.getHexedTimer()->remap(0.f, 1.f) : 0.f,
    };

    ctx.insideDragRect           = ctx.dragRect.hasValue() && ctx.dragRect->contains(cat.position);
    ctx.hovered                  = (mousePos - cat.position).lengthSquared() <= cat.getRadiusSquared();
    ctx.shouldDisplayRangeCircle = !ctx.beingDragged && !cat.isAstroAndInFlight() && ctx.hovered &&
                                   !main.mBtnDown(main.getLMB(), /* penetrateUI */ true);
    ctx.rangeInnerAlpha          = ctx.shouldDisplayRangeCircle ? 75u : 0u;

    if (cat.type == CatType::Astro)
    {
        if (cat.astroState.hasValue() && cat.isCloseToStartX())
            ctx.catRotation = remap(sf::base::fabs(cat.position.x - cat.astroState->startX), 0.f, 400.f, 0.f, 0.523599f);
        else if (ctx.cooldownDiff < 1000.f)
            ctx.catRotation = remap(ctx.cooldownDiff, 0.f, 1000.f, 0.523599f, 0.f);
        else if (cat.astroState.hasValue())
            ctx.catRotation = 0.523599f;
    }

    const float mult = remap(cat.dragTime, 0.f, 1000.f, 0.f, 1.f);
    ctx.catRotation += (-0.22f + sf::base::sin(cat.wobbleRadians) * 0.12f) * mult;

    if (ctx.witchCat != nullptr)
        applyWitchAnimation(ctx, main.witchcatWobblePhase, *ctx.witchCat);

    if (ctx.copyCat != nullptr && main.pt->copycatCopiedCatType == CatType::Witch)
        applyWitchAnimation(ctx, main.copyWitchcatWobblePhase, *ctx.copyCat);

    if (cat.type == CatType::Wizard)
        ctx.catRotation += main.wizardcatSpin.value + main.wizardcatAbsorptionRotation;

    // Velocity-based body tilt while dragged + transient wake-wobble poke.
    ctx.catRotation += cat.dragTiltRadians;
    ctx.catRotation += cat.napWakeWobble;

    // Extra body-level rotation that every body-attached sprite (including
    // attachments positioned via `anchorOffset`) must follow: tail-like
    // wobble for the Warden and the pendulum reaction when this cat was
    // just bonked. Decoupled from `catRotation` so logic that needs the
    // "base" rotation (drag, witch animation, etc.) stays unaffected.
    ctx.bodyRotationExtra = 0.f;

    if (cat.type == CatType::Warden)
        ctx.bodyRotationExtra += sf::base::sin(cat.wobbleRadians) * main.gameConstants.wardenCatBodyWobbleRadians;

    if (cat.bonkImpactMs > 0.f)
    {
        constexpr float bonkImpactDurationMs = 500.f;
        constexpr float bonkImpactAmplitude  = 0.28f;
        constexpr float bonkImpactFreqHz     = 4.f;

        const float elapsedS = (bonkImpactDurationMs - cat.bonkImpactMs) * 0.001f;
        const float decay    = cat.bonkImpactMs / bonkImpactDurationMs;

        ctx.bodyRotationExtra += sf::base::sin(elapsedS * bonkImpactFreqHz * sf::base::tau) * bonkImpactAmplitude * decay;
    }

    ctx.alpha    = ctx.insideDragRect ? static_cast<U8>(128u) : static_cast<U8>(255u);
    ctx.catColor = hueColor(cat.hue, ctx.alpha);
    ctx.circleAlpha = cat.cooldown.value < 0.f ? static_cast<U8>(0u)
                                               : static_cast<U8>(255.f - (cat.cooldown.value / ctx.maxCooldown * 225.f));
    ctx.circleColor        = CatConstants::colors[asIdx(cat.type)].withRotatedHue(cat.hue).withLightness(0.75f);
    ctx.circleOutlineColor = ctx.circleColor.withAlpha(ctx.rangeInnerAlpha == 0u ? ctx.circleAlpha : 255u);
    ctx.textOutlineColor   = ctx.circleColor.withLightness(0.25f);
    ctx.shouldDrawCatRange = main.profile.showCatRange && !main.inPrestigeTransition &&
                             (!main.profile.showRangesOnlyOnHover || ctx.shouldDisplayRangeCircle);

    ctx.catScale             = sf::Vec2f{0.2f, 0.2f} * ctx.catScaleMult;
    ctx.catAnchor            = ctx.beingDragged ? cat.position : cat.getDrawPosition(main.profile.enableCatBobbing);
    const auto catDrawOffset = main.gameConstants.catDrawOffsetsByType[asIdx(cat.type)];
    ctx.visualCatAnchor = ctx.catAnchor + (catDrawOffset / 2.f * 0.2f * ctx.catScaleMult).rotatedBy(ctx.bodyRotation());

    ctx.pushDown      = {0.f, ctx.beingDragged ? main.gameConstants.catAttachmentDraggedOffsetY : 0.f};
    ctx.attachmentHue = hueColor(main.gameConstants.catHueByType[asIdx(cat.type)] + cat.hue, ctx.alpha);

    return ctx;
}

void drawCatRange(const CatDrawContext& ctx)
{
    if (!ctx.shouldDrawCatRange)
        return;

    ctx.batchToUse.add(sf::CircleShapeData{
        .position           = Main::getCatRangeCenter(ctx.cat),
        .origin             = {ctx.range, ctx.range},
        .outlineTextureRect = ctx.main.txrWhiteDot,
        .fillColor          = (ctx.circleOutlineColor.withAlpha(ctx.rangeInnerAlpha)),
        .outlineColor       = ctx.circleOutlineColor,
        .outlineThickness   = ctx.main.profile.catRangeOutlineThickness,
        .radius             = ctx.range,
        .pointCount         = static_cast<unsigned int>(ctx.range / 3.f),
    });
}

void drawCatClouds(const CatDrawContext& ctx, const float cloudTime)
{
    const auto& [cloudPositionOffset, cloudXExtentMult] = ctx.main.gameConstants.cloudModifiers[asIdx(ctx.cat.type)];

    const int   cloudCircleCount = sf::base::max(ctx.main.gameConstants.catCloudCircleCount, 3);
    const float cloudScaleBase   = sf::base::max(ctx.catScaleMult, 0.35f) * ctx.main.gameConstants.catCloudScale;

    float cloudMult = easeInOutBack(remap(ctx.cat.dragTime, 0.f, 1000.f, 1.f, 0.f));

    if (ctx.cat.type == CatType::Astro)
    {
        if (ctx.cat.astroState.hasValue() && ctx.cat.isCloseToStartX())
        {
            cloudMult *= easeInOutBack(
                remap(sf::base::fabs(ctx.cat.position.x - ctx.cat.astroState->startX), 0.f, 400.f, 1.f, 0.f));
        }
        else if (ctx.cooldownDiff < 1000.f)
        {
            cloudMult *= easeInOutBack(remap(ctx.cooldownDiff, 0.f, 1000.f, 0.f, 1.f));
        }
    }

    const float cloudScale       = cloudScaleBase * cloudMult;
    const float cloudTimeSeconds = cloudTime * 0.001f;
    const float catCloudPhase    = ctx.cat.position.x * 0.0215f + ctx.cat.position.y * 0.0135f +
                                   static_cast<float>(asIdx(ctx.cat.type)) * 0.7f;

    const float cloudYOffset = ctx.main.gameConstants.catCloudBaseYOffset +
                               ctx.main.gameConstants.catCloudExtraYOffset * (1.f - cloudMult) +
                               (ctx.beingDragged ? ctx.main.gameConstants.catCloudDraggedOffset : 0.f);

    const sf::Vec2f cloudBasePos = ctx.visualCatAnchor.addY(ctx.catTxr.size.y / 2.f * ctx.catScale.y) +
                                   sf::Vec2f{0.f, cloudYOffset * cloudScale} + cloudPositionOffset;

    for (int cloudCircleIndex = 0; cloudCircleIndex < cloudCircleCount; ++cloudCircleIndex)
    {
        const float normalizedIndex = static_cast<float>(cloudCircleIndex) / static_cast<float>(cloudCircleCount - 1);
        const float centeredIndex   = normalizedIndex * 2.f - 1.f;
        const float lobeWeight      = 1.f - sf::base::fabs(centeredIndex);
        const float phase           = cloudTimeSeconds * (1.35f + normalizedIndex * 0.2f) + catCloudPhase +
                                      static_cast<float>(cloudCircleIndex) * 0.85f;

        const float xOffset = centeredIndex * ctx.main.gameConstants.catCloudXExtent * cloudScale * cloudXExtentMult +
                              sf::base::sin(phase) * (ctx.main.gameConstants.catCloudWobbleX + lobeWeight * 1.5f) * cloudScale;

        const float yOffset = -lobeWeight * ctx.main.gameConstants.catCloudLobeLift * cloudScale +
                              sf::base::cos(phase * 1.4f) * (ctx.main.gameConstants.catCloudWobbleY + lobeWeight) * cloudScale;

        const float radius = (ctx.main.gameConstants.catCloudRadiusBase +
                              lobeWeight * ctx.main.gameConstants.catCloudRadiusLobe +
                              sf::base::sin(phase * 1.15f) * ctx.main.gameConstants.catCloudRadiusWobble) *
                             cloudScale;

        ctx.cloudBatchToUse.add(sf::Sprite{
            .position    = cloudBasePos + sf::Vec2f{xOffset, yOffset},
            .scale       = {radius / (ctx.main.txrCloud.size.x / 2.f), radius / (ctx.main.txrCloud.size.y / 2.f)},
            .origin      = ctx.main.txrCloud.size / 2.f,
            .textureRect = ctx.main.txrCloud,
        });
    }
}

void drawCatVisuals(const CatDrawContext& ctx)
{
    const auto addCatSprite = [&](const sf::Sprite& sprite) { ctx.spriteBatchToUse.add(sprite); };

    const float tailRotationMult = ctx.cat.type == CatType::Uni ? 0.4f : 1.f;

    const auto tailWiggleRotation = sf::radians(
        ctx.catRotation + ctx.bodyRotationExtra +
        ((ctx.beingDragged ? -0.2f : 0.f) +
         sf::base::sin(ctx.cat.wobbleRadians) * (ctx.beingDragged ? 0.125f : 0.075f) * tailRotationMult));

    const auto tailWiggleRotationInvertedDragged = sf::radians(
        ctx.catRotation + ctx.bodyRotationExtra +
        ((ctx.beingDragged ? 0.2f : 0.f) +
         sf::base::sin(ctx.cat.wobbleRadians) * (ctx.beingDragged ? 0.125f : 0.075f) * tailRotationMult));

    if (ctx.cat.type == CatType::Devil)
    {
        addCatSprite(
            sf::Sprite{.position = ctx.anchorOffset(
                           ctx.catTailOffset + ctx.main.gameConstants.devilBackTail.positionOffset + ctx.pushDown * 2.f),
                       .scale       = ctx.catScale * 1.25f,
                       .origin      = ctx.main.gameConstants.devilBackTail.origin,
                       .rotation    = tailWiggleRotationInvertedDragged,
                       .textureRect = ctx.catTailTxr,
                       .color       = ctx.catColor});
    }

    if (ctx.cat.type == CatType::Normal && ctx.main.pt->perm.geniusCatsPurchased)
    {
        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.brainJarOffset),
                                .scale       = ctx.catScale,
                                .origin      = ctx.main.txrBrainBack.size / 2.f,
                                .rotation    = ctx.bodyRotation(),
                                .textureRect = ctx.main.txrBrainBack,
                                .color       = ctx.catColor});
    }

    if (ctx.cat.type == CatType::Uni)
    {
        const auto wingRotation = sf::radians(
            ctx.catRotation + (ctx.beingDragged ? -0.2f : 0.f) +
            sf::base::cos(ctx.cat.wobbleRadians) * (ctx.beingDragged ? 0.125f : 0.075f) * 0.75f);

        addCatSprite(
            sf::Sprite{.position = ctx.anchorOffset(ctx.main.gameConstants.uniWingsOffset),
                       .scale    = ctx.catScale * 1.25f,
                       .origin = ctx.main.txrUniCatWings.size / 2.f - ctx.main.gameConstants.uniWingsOriginOffsetFromCenter,
                       .rotation    = wingRotation,
                       .textureRect = ctx.main.txrUniCatWings,
                       .color       = hueColor(ctx.cat.hue + 180.f, 180u)});
    }

    if (ctx.cat.type == CatType::Devil)
    {
        addCatSprite(
            sf::Sprite{.position = ctx.visualCatAnchor + ctx.main.gameConstants.devilBookOffset,
                       .scale    = ctx.catScale * 1.55f,
                       .origin   = ctx.main.txrDevilCat3Book.size / 2.f,
                       .rotation = ctx.bodyRotation(),
                       .textureRect = ctx.main.isDevilcatHellsingedActive() ? ctx.main.txrDevilCat2Book : ctx.main.txrDevilCat3Book,
                       .color = hueColor(sf::base::remainder(ctx.cat.hue * 2.f - 15.f + static_cast<float>(ctx.cat.nameIdx) * 25.f,
                                                             60.f) -
                                             30.f,
                                         255u)});
    }

    if (ctx.cat.type == CatType::Devil)
    {
        addCatSprite(
            sf::Sprite{.position = ctx.cat.pawPosition + (ctx.beingDragged ? ctx.main.gameConstants.devilPawDraggedOffset
                                                                           : ctx.main.gameConstants.devilPawIdleOffset),
                       .scale       = ctx.catScale * 1.25f,
                       .origin      = ctx.catPawTxr.size / 2.f,
                       .rotation    = ctx.cat.pawRotation + sf::degrees(35.f),
                       .textureRect = ctx.catPawTxr,
                       .color       = ctx.catColor.withAlpha(static_cast<U8>(ctx.cat.pawOpacity))});
    }

    // Wardencat is a composite: guardhouse_back goes BEHIND the cat body,
    // guardhouse_front later OCCLUDES the lower half, and the paw sits on top.
    // The guardhouse parts are drawn un-rotated (pure position + bobbing) so
    // only the cat visibly rocks.
    if (ctx.cat.type == CatType::Warden)
    {
        addCatSprite(sf::Sprite{
            .position    = ctx.anchorOffset(ctx.main.gameConstants.wardenGuardhouseBackOffset),
            .scale       = ctx.catScale,
            .origin      = ctx.main.txrGuardhouseBack.size / 2.f,
            .rotation    = sf::radians(0.f),
            .textureRect = ctx.main.txrGuardhouseBack,
            .color       = ctx.catColor,
        });
    }

    // The body-attached rotation (including tail wobble and bonk pendulum)
    // was computed once in `makeCatDrawContext` and promoted onto the context,
    // so attachments positioned via `anchorOffset` + sprites using
    // `ctx.bodyRotation()` stay in sync as a single rigid body.
    const auto bodyRotation = ctx.bodyRotation();

    addCatSprite(
        sf::Sprite{.position = ctx.cat.type == CatType::Warden ? ctx.anchorOffset(ctx.main.gameConstants.wardenCatBodyOffset)
                                                               : ctx.visualCatAnchor,
                   .scale       = ctx.catScale,
                   .origin      = ctx.catTxr.size / 2.f,
                   .rotation    = bodyRotation,
                   .textureRect = ctx.catTxr,
                   .color       = ctx.catColor});

    if (ctx.main.gameConstants.debugDrawCatCenterMarker)
        ctx.batchToUse.add(sf::CircleShapeData{
            .position   = ctx.catAnchor,
            .origin     = {4.f, 4.f},
            .fillColor  = sf::Color{255u, 0u, 0u, ctx.alpha},
            .radius     = 4.f,
            .pointCount = 16u,
        });

    if (ctx.main.gameConstants.debugDrawCatBodyBounds)
        ctx.batchToUse.add(sf::RectangleShapeData{
            .position           = ctx.catAnchor,
            .scale              = ctx.catScale,
            .origin             = ctx.catTxr.size / 2.f,
            .rotation           = bodyRotation.wrapUnsigned(),
            .outlineTextureRect = ctx.main.txrWhiteDot,
            .fillColor          = sf::Color::Transparent,
            .outlineColor       = sf::Color{255u, 0u, 0u, ctx.alpha},
            .outlineThickness   = 4.f,
            .size               = ctx.catTxr.size,
        });

    if (ctx.cat.type == CatType::Duck)
    {
        addCatSprite(
            sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.duckFlag.positionOffset + ctx.pushDown),
                       .scale       = ctx.catScale,
                       .origin      = ctx.main.gameConstants.duckFlag.origin,
                       .rotation    = tailWiggleRotation,
                       .textureRect = ctx.main.txrDuckFlag,
                       .color       = ctx.catColor});
        return;
    }

    if (ctx.cat.type == CatType::Normal && ctx.main.pt->perm.smartCatsPurchased)
    {
        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.smartHatOffset),
                                .scale       = ctx.catScale,
                                .origin      = ctx.main.txrSmartCatHat.size / 2.f,
                                .rotation    = bodyRotation,
                                .textureRect = ctx.main.txrSmartCatHat,
                                .color       = ctx.catColor});
    }

    if (ctx.cat.flapCountdown.isDone() && ctx.cat.flapAnimCountdown.isDone())
    {
        if (ctx.main.rngFast.getI(0, 100) > 92)
            ctx.cat.flapCountdown.value = 75.f;
        else
            ctx.cat.flapCountdown.value = ctx.main.rngFast.getF(4500.f, 12'500.f);
    }

    if (ctx.cat.flapCountdown.updateAndStop(ctx.deltaTimeMs) == CountdownStatusStop::JustFinished)
        ctx.cat.flapAnimCountdown.value = 75.f * Main::nEarRects;

    (void)ctx.cat.flapAnimCountdown.updateAndStop(ctx.deltaTimeMs);

    if (ctx.cat.type == CatType::Normal)
    {
        addCatSprite(
            sf::Sprite{.position = ctx.anchorOffset(ctx.catEyeOffset + ctx.main.gameConstants.earFlapOffset),
                       .scale    = ctx.catScale,
                       .origin   = ctx.main.txrCatEars0.size / 2.f,
                       .rotation = bodyRotation,
                       .textureRect = *ctx.main.earRects[static_cast<unsigned int>(ctx.cat.flapAnimCountdown.value / 75.f) %
                                                         Main::nEarRects],
                       .color = ctx.attachmentHue});
    }

    const auto yawnRectIdx = static_cast<unsigned int>(ctx.cat.yawnAnimCountdown.value / 75.f) % Main::nYawnRects;

    if (ctx.cat.type != CatType::Devil && ctx.cat.type != CatType::Wizard && ctx.cat.type != CatType::Mouse &&
        ctx.cat.type != CatType::Engi)
    {
        if (ctx.cat.yawnCountdown.isDone() && ctx.cat.yawnAnimCountdown.isDone())
            ctx.cat.yawnCountdown.value = ctx.main.rngFast.getF(7500.f, 20'000.f);

        if (ctx.cat.blinkAnimCountdown.isDone() &&
            ctx.cat.yawnCountdown.updateAndStop(ctx.deltaTimeMs) == CountdownStatusStop::JustFinished)
            ctx.cat.yawnAnimCountdown.value = 75.f * Main::nYawnRects;

        (void)ctx.cat.yawnAnimCountdown.updateAndStop(ctx.deltaTimeMs);

        const sf::Vec2f yawnOrigin = ctx.main.txrCatYawn0.size / 2.f +
                                     (ctx.cat.type == CatType::Warden ? ctx.main.gameConstants.wardenCatYawnOriginOffset
                                                                      : sf::Vec2f{0.f, 0.f});

        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.catEyeOffset + ctx.main.gameConstants.yawnOffset),
                                .scale       = ctx.catScale,
                                .origin      = yawnOrigin,
                                .rotation    = bodyRotation,
                                .textureRect = *ctx.main.catYawnRects[yawnRectIdx],
                                .color       = ctx.attachmentHue});
    }
    else
    {
        ctx.cat.yawnCountdown.value = ctx.cat.yawnAnimCountdown.value = 0.f;
    }

    if (ctx.cat.type == CatType::Normal && ctx.main.pt->perm.smartCatsPurchased)
    {
        addCatSprite(
            sf::Sprite{.position = ctx.anchorOffset(ctx.main.gameConstants.smartDiploma.positionOffset + ctx.pushDown),
                       .scale    = ctx.catScale,
                       .origin   = ctx.main.gameConstants.smartDiploma.origin,
                       .rotation = tailWiggleRotation,
                       .textureRect = ctx.main.txrSmartCatDiploma,
                       .color       = ctx.catColor});
    }
    else if (ctx.cat.type == CatType::Astro && ctx.main.pt->perm.astroCatInspirePurchased)
    {
        addCatSprite(
            sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.astroFlag.positionOffset + ctx.pushDown),
                       .scale       = ctx.catScale,
                       .origin      = ctx.main.gameConstants.astroFlag.origin,
                       .rotation    = tailWiggleRotation,
                       .textureRect = ctx.main.txrAstroCatFlag,
                       .color       = ctx.catColor});
    }
    else if (ctx.cat.type == CatType::Engi || ctx.isCopyCatWithType(CatType::Engi))
    {
        addCatSprite(
            sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.engiWrench.positionOffset + ctx.pushDown),
                       .scale       = ctx.catScale,
                       .origin      = ctx.main.gameConstants.engiWrench.origin,
                       .rotation    = tailWiggleRotation,
                       .textureRect = ctx.main.txrEngiCatWrench,
                       .color       = ctx.catColor});
    }
    else if (ctx.cat.type == CatType::Attracto || ctx.isCopyCatWithType(CatType::Attracto))
    {
        addCatSprite(
            sf::Sprite{.position = ctx.anchorOffset(ctx.main.gameConstants.attractoMagnet.positionOffset + ctx.pushDown),
                       .scale       = ctx.catScale,
                       .origin      = ctx.main.gameConstants.attractoMagnet.origin,
                       .rotation    = tailWiggleRotation,
                       .textureRect = ctx.main.txrAttractoCatMagnet,
                       .color       = ctx.catColor});
    }

    if (ctx.cat.type != CatType::Devil && ctx.cat.type != CatType::Warden)
    {
        const auto originOffset = ctx.cat.type == CatType::Uni ? ctx.main.gameConstants.uniTailOriginOffset
                                                               : sf::Vec2f{0.f, 0.f};
        const auto offset = ctx.cat.type == CatType::Uni ? ctx.main.gameConstants.uniTailExtraOffset : sf::Vec2f{0.f, 0.f};

        addCatSprite(
            sf::Sprite{.position = ctx.anchorOffset(
                           ctx.catTailOffset + ctx.main.gameConstants.tail.positionOffset + offset + originOffset),
                       .scale       = ctx.catScale,
                       .origin      = originOffset + ctx.main.gameConstants.tail.origin,
                       .rotation    = tailWiggleRotation,
                       .textureRect = ctx.catTailTxr,
                       .color       = ctx.catColor});
    }

    if (ctx.cat.type == CatType::Mouse || ctx.isCopyCatWithType(CatType::Mouse))
    {
        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.mouseProp.positionOffset),
                                .scale       = ctx.catScale,
                                .origin      = ctx.main.gameConstants.mouseProp.origin,
                                .rotation    = tailWiggleRotationInvertedDragged,
                                .textureRect = ctx.main.txrMouseCatMouse,
                                .color       = ctx.catColor});
    }

    const auto& eyelidArray = (ctx.cat.type == CatType::Mouse || ctx.cat.type == CatType::Attracto ||
                               ctx.cat.type == CatType::Copy)
                                  ? ctx.main.grayEyeLidRects
                              : (ctx.cat.type == CatType::Engi ||
                                 (ctx.cat.type == CatType::Devil && ctx.main.isDevilcatHellsingedActive()))
                                  ? ctx.main.darkEyeLidRects
                              : (ctx.cat.type == CatType::Astro || ctx.cat.type == CatType::Uni)
                                  ? ctx.main.whiteEyeLidRects
                                  : ctx.main.eyeLidRects;

    if (ctx.cat.blinkCountdown.isDone() && ctx.cat.blinkAnimCountdown.isDone())
    {
        if (ctx.main.rngFast.getI(0, 100) > 90)
            ctx.cat.blinkCountdown.value = 75.f;
        else
            ctx.cat.blinkCountdown.value = ctx.main.rngFast.getF(1000.f, 4000.f);
    }

    if (ctx.cat.blinkCountdown.updateAndStop(ctx.deltaTimeMs) == CountdownStatusStop::JustFinished)
        ctx.cat.blinkAnimCountdown.value = 75.f * Main::nEyeLidRects;

    (void)ctx.cat.blinkAnimCountdown.updateAndStop(ctx.deltaTimeMs);

    const sf::Vec2f eyelidOrigin = ctx.main.txrCatEyeLid0.size / 2.f +
                                   (ctx.cat.type == CatType::Warden ? ctx.main.gameConstants.wardenCatEyelidOriginOffset
                                                                    : sf::Vec2f{0.f, 0.f});

    if ((ctx.witchCat != nullptr && ctx.main.isCatPerformingRitual(*ctx.witchCat, ctx.cat)) ||
        (ctx.copyCat != nullptr && ctx.main.pt->copycatCopiedCatType == CatType::Witch &&
         ctx.main.isCatPerformingRitual(*ctx.copyCat, ctx.cat)) ||
        ctx.cat.isNapping())
    {
        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.catEyeOffset + ctx.main.gameConstants.eyelidOffset),
                                .scale       = ctx.catScale,
                                .origin      = eyelidOrigin,
                                .rotation    = bodyRotation,
                                .textureRect = *eyelidArray[2],
                                .color       = ctx.attachmentHue});
    }
    else if (!ctx.cat.yawnAnimCountdown.isDone())
    {
        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.catEyeOffset + ctx.main.gameConstants.eyelidOffset),
                                .scale       = ctx.catScale,
                                .origin      = eyelidOrigin,
                                .rotation    = bodyRotation,
                                .textureRect = *eyelidArray[static_cast<unsigned int>(
                                    remap(static_cast<float>(yawnRectIdx), 0.f, 13.f, 0.f, 7.f))],
                                .color       = ctx.attachmentHue});
    }
    else if (!ctx.cat.blinkAnimCountdown.isDone())
    {
        addCatSprite(
            sf::Sprite{.position    = ctx.anchorOffset(ctx.catEyeOffset + ctx.main.gameConstants.eyelidOffset),
                       .scale       = ctx.catScale,
                       .origin      = eyelidOrigin,
                       .rotation    = bodyRotation,
                       .textureRect = *eyelidArray[static_cast<unsigned int>(ctx.cat.blinkAnimCountdown.value / 75.f) %
                                                   Main::nEyeLidRects],
                       .color       = ctx.attachmentHue});
    }

    if (ctx.cat.type == CatType::Normal && ctx.main.pt->perm.geniusCatsPurchased)
    {
        addCatSprite(sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.brainJarOffset),
                                .scale       = ctx.catScale,
                                .origin      = ctx.main.txrBrainFront.size / 2.f,
                                .rotation    = bodyRotation,
                                .textureRect = ctx.main.txrBrainFront,
                                .color       = ctx.catColor});
    }

    // Wardencat: front of the guardhouse occludes the cat's lower half. Goes
    // over the body + eyes/yawn/eyelids, but still UNDER the paw so the paw
    // reads as resting on the windowsill.
    if (ctx.cat.type == CatType::Warden)
    {
        addCatSprite(sf::Sprite{
            .position    = ctx.anchorOffset(ctx.main.gameConstants.wardenGuardhouseFrontOffset),
            .scale       = ctx.catScale,
            .origin      = ctx.main.txrGuardhouseFront.size / 2.f,
            .rotation    = sf::radians(0.f),
            .textureRect = ctx.main.txrGuardhouseFront,
            .color       = ctx.catColor,
        });
    }

    // Warden's paw is part of the guardhouse composite and must always be
    // visible (not hidden by hex / idle fade). Its scale is tunable so the
    // art lines up with the windowsill regardless of the shared cat scale.
    const bool drawPaw = ctx.cat.type == CatType::Warden ||
                         (!ctx.cat.isHexedOrCopyHexed() && ctx.cat.type != CatType::Devil);

    if (drawPaw)
    {
        const bool isWarden = ctx.cat.type == CatType::Warden;
        const auto pawScale = isWarden ? ctx.catScale * ctx.main.gameConstants.wardenCatPawScale : ctx.catScale * 0.85f;
        const auto pawAlpha = isWarden ? static_cast<U8>(255u) : static_cast<U8>(ctx.cat.pawOpacity);

        // Warden paw state machine:
        //  - Travel:  lerps start pose → `cat.pawPosition` over pawBonkTravelMs
        //  - Hold:    stays at `cat.pawPosition`
        //  - Return:  eases the snapshotted hold pose → idle windowsill spot
        //  - Idle:    sits at the tunable windowsill offset
        const sf::Vec2f wardenIdlePos = isWarden ? ctx.anchorOffset(ctx.main.gameConstants.wardenCatPawOffset)
                                                 : sf::Vec2f{0.f, 0.f};

        sf::Vec2f pawPosition = isWarden ? (ctx.cat.pawHoldMs > 0.f ? ctx.cat.pawPosition : wardenIdlePos)
                                         : ctx.cat.pawPosition +
                                               (ctx.beingDragged ? ctx.main.gameConstants.regularPawDraggedOffset
                                                                 : ctx.main.gameConstants.regularPawIdleOffset);

        auto pawRotate = ctx.cat.type == CatType::Mouse ? sf::radians(-0.6f) : ctx.cat.pawRotation;

        if (isWarden && ctx.cat.pawBonkTravelMs > 0.f && ctx.cat.pawBonkTravelDurationMs > 0.f)
        {
            const float t = easeInOutCubic(
                sf::base::clamp(1.f - (ctx.cat.pawBonkTravelMs / ctx.cat.pawBonkTravelDurationMs), 0.f, 1.f));

            pawPosition = ctx.cat.pawBonkStartPos + (ctx.cat.pawPosition - ctx.cat.pawBonkStartPos) * t;
            pawRotate   = ctx.cat.pawBonkStartRotation.rotatedTowards(ctx.cat.pawRotation, t);
        }
        else if (isWarden && ctx.cat.pawBonkReturnMs > 0.f && ctx.cat.pawBonkReturnDurationMs > 0.f)
        {
            // Return-phase target is recomputed every frame so the paw tracks
            // the cat's bobbing position while it eases back.
            const float t = easeInOutCubic(
                sf::base::clamp(1.f - (ctx.cat.pawBonkReturnMs / ctx.cat.pawBonkReturnDurationMs), 0.f, 1.f));

            pawPosition = ctx.cat.pawBonkReturnStartPos + (wardenIdlePos - ctx.cat.pawBonkReturnStartPos) * t;
            pawRotate   = ctx.cat.pawBonkReturnStartRotation.rotatedTowards(sf::degrees(-45.f), t);
        }

        addCatSprite(sf::Sprite{.position    = pawPosition,
                                .scale       = pawScale,
                                .origin      = ctx.catPawTxr.size / 2.f,
                                .rotation    = pawRotate,
                                .textureRect = ctx.catPawTxr,
                                .color       = ctx.catColor.withAlpha(pawAlpha)});
    }

    if (ctx.cat.type == CatType::Copy)
    {
        if (ctx.main.copycatMaskAnim.isDone() &&
            ctx.main.copycatMaskAnimCd.updateAndStop(ctx.deltaTimeMs) == CountdownStatusStop::AlreadyFinished)
            ctx.main.copycatMaskAnim.value = 3000.f;

        if (ctx.main.copycatMaskAnimCd.isDone() &&
            ctx.main.copycatMaskAnim.updateAndStop(ctx.deltaTimeMs) == CountdownStatusStop::JustFinished)
            ctx.main.copycatMaskAnimCd.value = 4000.f;

        const float foo = easeInOutBack(ctx.main.copycatMaskAnim.getProgressBounced(3000.f)) * 0.5f;

        const auto* txrMaskToUse = [&]() -> const sf::Rect2f*
        {
            if (ctx.main.pt->copycatCopiedCatType == CatType::Witch)
                return &ctx.main.txrCCMaskWitch;

            if (ctx.main.pt->copycatCopiedCatType == CatType::Wizard)
                return &ctx.main.txrCCMaskWizard;

            if (ctx.main.pt->copycatCopiedCatType == CatType::Mouse)
                return &ctx.main.txrCCMaskMouse;

            if (ctx.main.pt->copycatCopiedCatType == CatType::Engi)
                return &ctx.main.txrCCMaskEngi;

            if (ctx.main.pt->copycatCopiedCatType == CatType::Repulso)
                return &ctx.main.txrCCMaskRepulso;

            if (ctx.main.pt->copycatCopiedCatType == CatType::Attracto)
                return &ctx.main.txrCCMaskAttracto;

            return nullptr;
        }();

        if (txrMaskToUse != nullptr)
            addCatSprite(
                sf::Sprite{.position    = ctx.anchorOffset(ctx.main.gameConstants.copyMaskOffset),
                           .scale       = ctx.catScale * remap(foo, 0.f, 0.5f, 1.f, 0.75f),
                           .origin      = {ctx.main.gameConstants.copyMaskOrigin.x,
                                           ctx.main.gameConstants.copyMaskOrigin.y * remap(foo, 0.f, 0.5f, 1.f, 1.25f)},
                           .rotation    = sf::radians(ctx.catRotation + foo),
                           .textureRect = *txrMaskToUse,
                           .color       = ctx.catColor});
    }
}

void finalizeCatHexedDraw(const CatDrawContext& ctx)
{
    if (!ctx.drawHexedWithShader || ctx.main.tempDrawableBatch.isEmpty())
        return;

    ctx.main.enqueueHexedCatDrawCommand(ctx.main.tempDrawableBatch,
                                        ctx.visualCatAnchor,
                                        ctx.main.playerInputState.catToPlace == &ctx.cat,
                                        ctx.cat.position.x * 0.013f + ctx.cat.position.y * 0.021f +
                                            static_cast<float>(ctx.cat.nameIdx) * 0.17f,
                                        ctx.hexedEffectStrength);

    ctx.main.tempDrawableBatch.clear();
}

void drawCatText(const CatDrawContext& ctx)
{
    if (!ctx.main.profile.showCatText)
        return;

    static thread_local sf::base::String catNameBuffer;
    catNameBuffer.clear();

    if (ctx.main.pt->perm.smartCatsPurchased && ctx.cat.type == CatType::Normal && ctx.cat.nameIdx % 2u == 0u)
        catNameBuffer += "Dr. ";

    const sf::base::StringView catNameSv = ctx.main.shuffledCatNamesPerType[asIdx(ctx.cat.type)][ctx.cat.nameIdx];
    catNameBuffer.append(catNameSv.data(), catNameSv.size());

    if (ctx.main.pt->perm.smartCatsPurchased && ctx.cat.type == CatType::Normal && ctx.cat.nameIdx % 2u != 0u)
        catNameBuffer += ", PhD";

    const auto textAlpha = ctx.cat.isHexedOrCopyHexed() ? static_cast<U8>(160u) : static_cast<U8>(255u);

    ctx.main.textNameBuffer.setString(catNameBuffer);
    ctx.main.textNameBuffer.position = ctx.cat.position.addY(ctx.main.gameConstants.catNameTextOffsetY);
    ctx.main.textNameBuffer.origin   = ctx.main.textNameBuffer.getLocalBounds().size / 2.f;
    ctx.main.textNameBuffer.scale    = sf::Vec2f{0.5f, 0.5f} * ctx.catScaleMult;
    ctx.main.textNameBuffer.setFillColor(sf::Color::White.withAlpha(textAlpha));
    ctx.main.textNameBuffer.setOutlineColor(ctx.textOutlineColor.withAlpha(textAlpha));
    ctx.textBatchToUse.add(ctx.main.textNameBuffer);

    if (ctx.cat.type != CatType::Repulso && ctx.cat.type != CatType::Attracto && ctx.cat.type != CatType::Duck &&
        !ctx.isCopyCatWithType(CatType::Repulso) && !ctx.isCopyCatWithType(CatType::Attracto))
    {
        const char* actionName = CatConstants::actionNames[asIdx(
            ctx.cat.type == CatType::Copy ? ctx.main.pt->copycatCopiedCatType : ctx.cat.type)];

        if (ctx.cat.type == CatType::Devil && ctx.main.isDevilcatHellsingedActive())
            actionName = "Portals";

        static thread_local sf::base::String actionString;
        actionString.clear();

        actionString += sf::base::toString(ctx.cat.hits);
        actionString += " ";
        actionString += actionName;

        if (ctx.cat.type == CatType::Mouse || ctx.isCopyCatWithType(CatType::Mouse))
        {
            actionString += " (x";
            actionString += sf::base::toString(ctx.main.pt->mouseCatCombo + 1);
            actionString += ")";
        }

        if (ctx.cat.moneyEarned != 0u)
        {
            char moneyFmtBuffer[128]{};
            std::sprintf(moneyFmtBuffer, "$%s", Main::toStringWithSeparators(ctx.cat.moneyEarned));

            actionString += " | ";
            actionString += moneyFmtBuffer;
        }

        ctx.main.textStatusBuffer.setString(actionString);
        ctx.main.textStatusBuffer.position = ctx.cat.position.addY(ctx.main.gameConstants.catStatusTextOffsetY);
        ctx.main.textStatusBuffer.origin   = ctx.main.textStatusBuffer.getLocalBounds().size / 2.f;
        ctx.main.textStatusBuffer.setFillColor(sf::Color::White.withAlpha(textAlpha));
        ctx.main.textStatusBuffer.setOutlineColor(ctx.textOutlineColor.withAlpha(textAlpha));
        ctx.cat.textStatusShakeEffect.applyToText(ctx.main.textStatusBuffer);
        ctx.main.textStatusBuffer.scale *= 0.4f * ctx.catScaleMult;
        ctx.textBatchToUse.add(ctx.main.textStatusBuffer);

        if (ctx.cat.isHexedOrCopyHexed())
            actionString = "\n[hexed]";

        ctx.main.textStatusBuffer.setString(actionString);
        ctx.main.textStatusBuffer.position = ctx.cat.position.addY(ctx.main.gameConstants.catStatusTextOffsetY);
        ctx.main.textStatusBuffer.origin   = ctx.main.textStatusBuffer.getLocalBounds().size / 2.f;
        ctx.main.textStatusBuffer.setFillColor(sf::Color::White.withAlpha(textAlpha));
        ctx.main.textStatusBuffer.setOutlineColor(ctx.textOutlineColor.withAlpha(textAlpha));
        ctx.cat.textStatusShakeEffect.applyToText(ctx.main.textStatusBuffer);
        ctx.main.textStatusBuffer.scale *= 0.4f * ctx.catScaleMult;
        ctx.textBatchToUse.add(ctx.main.textStatusBuffer);

        const bool hideCooldownBar = ctx.main.inPrestigeTransition || ctx.cat.type == CatType::Repulso ||
                                     ctx.cat.type == CatType::Attracto || ctx.cat.type == CatType::Duck ||
                                     ctx.cat.isHexedOrCopyHexed();

        if (!hideCooldownBar)
            ctx.textBatchToUse.add(sf::RoundedRectangleShapeData{
                .position = ctx.main.textStatusBuffer.getGlobalBottomCenter().addY(ctx.main.gameConstants.catCooldownBarOffsetY),
                .scale              = {ctx.catScaleMult, ctx.catScaleMult},
                .origin             = {32.f, 0.f},
                .outlineTextureRect = ctx.main.txrWhiteDot,
                .fillColor          = sf::Color::whiteWithAlpha(128u),
                .outlineColor       = ctx.textOutlineColor,
                .outlineThickness   = 1.f,
                .size               = sf::Vec2f{ctx.cat.cooldown.value / ctx.maxCooldown * 64.f, 3.f}.clampX(2.f, 64.f),
                .cornerRadius       = 1.f,
                .cornerPointCount   = 8u,
            });
    }
}
} // namespace


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCat(Cat&            cat,
                           const float     deltaTimeMs,
                           const sf::Vec2f mousePos,
                           const sf::Rect2f* const (&catTxrsByType)[nCatTypes],
                           const sf::Rect2f* const (&catPawTxrsByType)[nCatTypes],
                           const sf::Rect2f* const (&catTailTxrsByType)[nCatTypes])
{
    const float cloudTime = advanceCatCloudTime(*this, cat, deltaTimeMs);

    if (playerInputState.catToPlace != &cat && !bubbleCullingBoundaries.isInside(cat.position))
        return;

    const CatTextureTables textureTables{catTxrsByType, catPawTxrsByType, catTailTxrsByType};
    CatDrawContext         ctx = makeCatDrawContext(*this, cat, deltaTimeMs, mousePos, textureTables);

    if (ctx.drawHexedWithShader)
        tempDrawableBatch.clear();

    drawCatRange(ctx);
    drawCatClouds(ctx, cloudTime);
    drawCatVisuals(ctx);
    finalizeCatHexedDraw(ctx);
    drawCatText(ctx);
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

        cpuDrawableBatchAfterCats.add(
            sf::Sprite{.position    = shrine.getDrawPosition(),
                       .scale       = sf::Vec2f{0.3f, 0.3f} * invDeathProgress +
                                      sf::Vec2f{1.25f, 1.25f} * shrine.textStatusShakeEffect.grow * 0.015f,
                       .origin      = txrShrine.size / 2.f,
                       .textureRect = txrShrine,
                       .color       = shrineColor});

        const auto range = shrine.getRange();

        const bool shouldDrawShrineRange = !profile.showRangesOnlyOnHover || hoveredShrine == &shrine;

        if (shouldDrawShrineRange)
            cpuDrawableBatchAfterCats.add(sf::CircleShapeData{
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
        cpuDrawableBatchAfterCats.add(textStatusBuffer);

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
            cpuDrawableBatchAfterCats.add(textMoneyBuffer);
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
        &txrDollNormal,   // Warden -- TODO: dedicated doll texture (reusing Normal)
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
    const auto processDolls = [&](const sf::base::Vector<HexSession>& sessions, const float hueMod)
    {
        for (const HexSession& session : sessions)
            for (const Doll& doll : session.dolls)
            {
                const auto& dollTxr = *dollTxrs[asIdx(doll.catType)];

                const float invDeathProgress = 1.f - doll.getDeathProgress();
                const float progress = doll.tcDeath.hasValue() ? invDeathProgress : doll.getActivationProgress();

                auto dollAlpha = static_cast<U8>(remap(progress, 0.f, 1.f, 128.f, 255.f));

                if ((mousePos - doll.position).lengthSquared() <= doll.getRadiusSquared() &&
                    !mBtnDown(getLMB(), /* penetrateUI */ true))
                    dollAlpha = 128.f;

                cpuDrawableBatchAfterCats.add(
                    sf::Sprite{.position    = doll.getDrawPosition(),
                               .scale       = sf::Vec2f{0.22f, 0.22f} * progress,
                               .origin      = dollTxr.size / 2.f,
                               .rotation    = sf::radians(-0.15f + 0.3f * sf::base::sin(doll.wobbleRadians / 2.f)),
                               .textureRect = dollTxr,
                               .color       = hueColor(doll.hue + hueMod, dollAlpha)});
            }
    };

    processDolls(pt->hexSessions, /* hueMod */ 0.f);
    processDolls(pt->copyHexSessions, /* hueMod */ 180.f);
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawEvents()
{
    SFEX_PROFILE_SCOPE_AUTOLABEL();

    if (pt->activeEvents.empty())
        return;

    const auto& bfCfg = gameConstants.events.bubblefall;

    // Matches the attack/hold/release envelope driving the spawn rate, so the
    // light ray visually fades in, holds, and fades out on the same curve.
    const auto computeIntensity = [&](const float remainingMs)
    {
        const float duration    = bfCfg.durationMs <= 0.f ? 1.f : bfCfg.durationMs;
        const float elapsedNorm = sf::base::clamp(1.f - (remainingMs / duration), 0.f, 1.f);

        const float attack  = sf::base::clamp(bfCfg.attackRatio, 0.f, 0.5f);
        const float release = sf::base::clamp(bfCfg.releaseRatio, 0.f, 0.5f);

        if (attack > 0.f && elapsedNorm < attack)
            return easeInOutCubic(sf::base::clamp(elapsedNorm / attack, 0.f, 1.f));

        if (release > 0.f && elapsedNorm > 1.f - release)
            return easeInOutCubic(sf::base::clamp((1.f - elapsedNorm) / release, 0.f, 1.f));

        return 1.f;
    };

    constexpr float thinLineWidth = 4.f;
    constexpr float topWidthRatio = 0.4f; // top edge is 40% of the bottom

    const auto drawRay = sf::base::OverloadSet{
        // No light-ray for the global invincible-bubble event.
        [](const EInvincibleBubble&) {},

        [&](const EBubblefall& e)
    {
        const float intensity = computeIntensity(e.remainingMs);
        if (intensity <= 0.f)
            return;

        const float bottomWidth = thinLineWidth + (e.regionWidth - thinLineWidth) * intensity;
        const float topWidth    = thinLineWidth + (e.regionWidth * topWidthRatio - thinLineWidth) * intensity;
        const auto  alpha       = static_cast<U8>(sf::base::clamp(intensity * 90.f, 0.f, 255.f));
        const float height      = boundaries.y;

        const auto makeLightRay = [&](U8 xAlpha, float xSizeMult)
        {
            const auto [fillSpan, outlineSpan] = cpuDrawableBatchBeforeCats.add(sf::TrapezoidShapeData{
                .position    = {e.regionCenterX, 0.f},
                .origin      = {bottomWidth * xSizeMult * 0.5f, 0.f},
                .textureRect = {},
                .fillColor   = sf::Color::whiteWithAlpha(xAlpha),
                .topWidth    = topWidth * xSizeMult,
                .bottomWidth = bottomWidth * xSizeMult,
                .height      = height,
            });

            fillSpan[3].color.a = 5;
            fillSpan[4].color.a = 5;
        };

        makeLightRay(alpha / 3, 5.f);
        makeLightRay(alpha, 3.f);
    },
    };

    for (const GameEvent& ev : pt->activeEvents)
        ev.linearVisit(drawRay);
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

        cpuDrawableBatchBeforeCats.add(
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
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAdditive,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAdditive,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAdditive,
        &cpuDrawableBatchAfterCats,
        &cpuDrawableBatchAfterCats, // Glass
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
            .color       = sf::Color::whiteWithAlpha(static_cast<U8>(alpha)),
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
        textStatusBuffer.setFillColor(sf::Color::whiteWithAlpha(opacityAsAlpha));
        textStatusBuffer.setOutlineColor(outlineHueColor.withAlpha(opacityAsAlpha));

        cpuDrawableBatchAfterCats.add(textStatusBuffer);
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
    if (uiState.scrollArrowCountdown.value <= 0.f)
        return;

    if (playerInputState.scroll == 0.f)
        (void)uiState.scrollArrowCountdown.updateAndLoop(deltaTimeMs, sf::base::tau * 350.f);
    else
        (void)uiState.scrollArrowCountdown.updateAndStop(deltaTimeMs);

    const float blinkOpacity = easeInOutSine(sf::base::fabs(sf::base::sin(
                                   sf::base::remainder(uiState.scrollArrowCountdown.value / 350.f, sf::base::tau)))) *
                               255.f;

    const float arrowX = getLeftMostUsefulX();

    rtGame.draw(txArrow,
                {.position = {arrowX, 15.f + (gameScreenSize.y / 5.f) * 1.f},
                 .origin   = txArrow.getRect().getCenterRight(),
                 .color    = sf::Color::whiteWithAlpha(static_cast<U8>(blinkOpacity))},
                {.view = gameView});

    rtGame.draw(txArrow,
                {.position = {arrowX, gameScreenSize.y - 15.f - (gameScreenSize.y / 5.f) * 1.f},
                 .origin   = txArrow.getRect().getCenterRight(),
                 .color    = sf::Color::whiteWithAlpha(static_cast<U8>(blinkOpacity))},
                {.view = gameView});
}


////////////////////////////////////////////////////////////
void Main::gameLoopUpdatePurchaseUnlockedEffects(const float deltaTimeMs)
{
    const float imguiWidth = uiWindowWidth * profile.uiScale;
    const auto  blinkFn    = [](const float value) { return (1 - sf::base::cos(2.f * sf::base::pi * value)) / 2.f; };
    const float uiMenuCueX = uiState.uiMenuLastDrawSize.x > 1.f ? uiState.uiMenuLastDrawPos.x : uiGetWindowPos().x;
    const bool  uiMenuFullyOpen = uiState.uiMenuRevealT >= 0.999f;

    for (auto& [widgetLabel, countdown, arrowCountdown, hue, type] : uiState.purchaseUnlockedEffects)
    {
        const float y = uiState.uiLabelToY[widgetLabel];

        if (uiMenuFullyOpen && countdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::Running)
        {
            const float x = remap(countdown.value, 0.f, 1000.f, 0.f, imguiWidth);

            const auto pos = sf::Vec2f{uiMenuCueX + x, y + (14.f + rngFast.getF(-14.f, 14.f)) * profile.uiScale};

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

        const bool arrowVisible = !arrowCountdown.isDone() &&
                                  (!uiMenuFullyOpen ||
                                   arrowCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::Running);

        if (arrowVisible)
        {
            const float blinkProgress = blinkFn(arrowCountdown.getProgressBounced(2000.f));

            const auto arrowAlpha = static_cast<sf::base::U8>(easeInOutCubic(blinkProgress) * 255.f);

            const auto& tx = type == 0 ? txUnlock : txPurchasable;

            rtGame.draw(tx,
                        {.position = {uiMenuCueX, y + 14.f * profile.uiScale},
                         .scale  = sf::Vec2f{0.25f, 0.25f} * (profile.uiScale + -0.15f * easeInOutBack(blinkProgress)),
                         .origin = tx.getRect().getCenterRight(),
                         .color  = hueColor(hue + currentBackgroundHue.asDegrees(), arrowAlpha)},
                        {.view = nonScaledHUDView, .shader = &shader});
        }
    }

    sf::base::vectorEraseIf(uiState.purchaseUnlockedEffects, [](const auto& pue) { return pue.arrowCountdown.isDone(); });
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Main::shouldDrawGrabbingCursor() const
{
    return !playerInputState.draggedCats.empty() || mBtnDown(getRMB(), /* penetrateUI */ true);
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
                 .scale = sf::Vec2f{profile.cursorScale, profile.cursorScale} *
                          ((1.f + easeInOutBack(cursorGrow) * sf::base::pow(static_cast<float>(comboState.combo), 0.09f)) *
                           dpiScalingFactor),
                 .origin = {5.f, 5.f},
                 .color  = hueColor(profile.cursorHue + currentBackgroundHue.asDegrees(), 255u)},
                {.view = nonScaledHUDView, .shader = &shader});
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCursorComboText(const float deltaTimeMs, const float cursorGrow)
{
    if (!pt->comboPurchased || !profile.showCursorComboText || shouldDrawGrabbingCursor())
        return;

    static float alpha = 0.f;

    const float scaleMult = profile.cursorScale * dpiScalingFactor;

    if (comboState.combo >= 1)
        alpha = 255.f;
    else if (alpha > 0.f)
        alpha -= deltaTimeMs * 0.5f;

    const auto alphaU8 = static_cast<U8>(sf::base::clamp(alpha, 0.f, 255.f));

    comboState.cursorComboText.position = sf::Mouse::getPosition(window).toVec2f() + sf::Vec2f{30.f, 48.f} * scaleMult;

    comboState.cursorComboText.setFillColor(sf::Color::blackWithAlpha(alphaU8));
    comboState.cursorComboText.setOutlineColor(
        sf::Color{111u, 170u, 244u, alphaU8}.withRotatedHue(profile.cursorHue + currentBackgroundHue.asDegrees()));

    if (comboState.combo > 0)
        comboState.cursorComboText.setString("x" + sf::base::toString(comboState.combo + 1));

    comboState.comboTextShakeEffect.applyToText(comboState.cursorComboText);

    comboState.cursorComboText.scale *= (static_cast<float>(comboState.combo) * 0.65f) * cursorGrow * 0.3f;
    comboState.cursorComboText.scale += {0.85f, 0.85f};
    comboState.cursorComboText.scale += sf::Vec2f{1.f, 1.f} * comboState.comboFailCountdown.value / 325.f;
    comboState.cursorComboText.scale *= scaleMult;

    const auto minScale = sf::Vec2f{0.25f, 0.25f} + sf::Vec2f{0.25f, 0.25f} * comboState.comboFailCountdown.value / 125.f;

    comboState.cursorComboText.scale = comboState.cursorComboText.scale.componentWiseClamp(minScale, {1.5f, 1.5f});

    if (comboState.comboFailCountdown.value > 0.f)
    {
        comboState.cursorComboText.position += rngFast.getVec2f({-5.f, -5.f}, {5.f, 5.f});
        comboState.cursorComboText.setFillColor(sf::Color::Red.withAlpha(alphaU8));
    }

    rtGame.draw(comboState.cursorComboText, {.view = nonScaledHUDView, .shader = &shader});
}


////////////////////////////////////////////////////////////
void Main::gameLoopDrawCursorComboBar()
{
    if (!pt->comboPurchased || !profile.showCursorComboBar || comboState.comboCountdown.value == 0.f ||
        shouldDrawGrabbingCursor())
        return;

    const float scaleMult = profile.cursorScale * dpiScalingFactor;

    const auto cursorComboBarPosition = sf::Mouse::getPosition(window).toVec2f() + sf::Vec2f{52.f, 14.f} * scaleMult;

    rtGame.draw(
        sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackWithAlpha(80u),
            .outlineColor       = comboState.cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size = {64.f * scaleMult * pt->psvComboStartTime.currentValue() * 1000.f / 700.f, 24.f * scaleMult},
        },
        {.view = nonScaledHUDView});

    rtGame.draw(
        sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackWithAlpha(164u),
            .outlineColor       = comboState.cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size               = {64.f * scaleMult * comboState.comboCountdown.value / 700.f, 24.f * scaleMult},
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
                                   .scale  = sf::Vec2f{0.2f, 0.2f} + sf::Vec2f{0.6f, 0.6f} * easeInOutBack(bgProgress),
                                   .origin = txTipBg.getSize().toVec2f() / 2.f,
                                   .textureRect = txTipBg.getRect(),
                                   .color = sf::Color::whiteWithAlpha(static_cast<U8>(tipBackgroundAlpha * 0.85f))};


    SFML_BASE_ASSERT(profile.hudScale > 0.f);

    tipBackgroundSprite.setGlobalBottomCenter(
        {getResolution().x / 2.f / profile.hudScale, getResolution().y / profile.hudScale - 50.f});

    const sf::Vec2f tipBgTopLeft     = tipBackgroundSprite.getGlobalTopLeft() + sf::Vec2f{40.f, 40.f};
    const sf::Vec2f tipBgBottomRight = tipBackgroundSprite.getGlobalBottomRight() - sf::Vec2f{40.f, 40.f};

    cpuCloudHudDrawableBatch.clear();

    drawCloudFrame({
        .time              = shaderTime,
        .mins              = tipBgTopLeft,
        .maxs              = tipBgBottomRight,
        .xSteps            = 10,
        .ySteps            = 3,
        .scaleMult         = 2.5f,
        .outwardOffsetMult = 1.f,
        .color             = sf::Color::whiteWithAlpha(static_cast<U8>(tipBackgroundAlpha * 0.82f)),
        .batch             = &cpuCloudHudDrawableBatch,
    });

    const sf::Vec2f tipBgSize = tipBgBottomRight - tipBgTopLeft;
    const sf::Vec2f tipMidInset{tipBgSize.x * 0.14f, tipBgSize.y * 0.22f};
    const sf::Vec2f tipInnerInset{tipBgSize.x * 0.27f, tipBgSize.y * 0.36f};

    drawCloudFrame({
        .time              = shaderTime + 1.7f,
        .mins              = tipBgTopLeft + tipMidInset,
        .maxs              = tipBgBottomRight - tipMidInset,
        .xSteps            = 8,
        .ySteps            = 3,
        .scaleMult         = 2.6f,
        .outwardOffsetMult = 0.85f,
        .color             = sf::Color::whiteWithAlpha(static_cast<U8>(tipBackgroundAlpha * 0.62f)),
        .batch             = &cpuCloudHudDrawableBatch,
    });

    drawCloudFrame({
        .time              = shaderTime + 3.4f,
        .mins              = tipBgTopLeft + tipInnerInset,
        .maxs              = tipBgBottomRight - tipInnerInset,
        .xSteps            = 6,
        .ySteps            = 2,
        .scaleMult         = 2.6f,
        .outwardOffsetMult = 0.7f,
        .color             = sf::Color::whiteWithAlpha(static_cast<U8>(tipBackgroundAlpha * 0.46f)),
        .batch             = &cpuCloudHudDrawableBatch,
    });

    gameLoopDisplayCloudBatch(cpuCloudHudDrawableBatch, scaledHUDView);

    sf::Sprite tipByteSprite{.position    = {},
                             .scale       = sf::Vec2f{0.85f, 0.85f} * easeInOutBack(byteProgress),
                             .origin      = txTipByte.getSize().toVec2f() / 2.f,
                             .rotation    = sf::radians(sf::base::tau * easeInOutBack(byteProgress)),
                             .textureRect = txTipByte.getRect(),
                             .color       = sf::Color::whiteWithAlpha(static_cast<U8>(tipByteAlpha))};

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
                         .fillColor     = sf::Color::whiteWithAlpha(static_cast<sf::base::U8>(tipByteAlpha)),
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
void Main::drawActivatedShrineBackgroundEffects(sf::RenderTarget& rt,
                                                const sf::View&   backgroundView,
                                                const sf::Vec2f   activeGameViewCenter) const
{
    constexpr float maxShrineEffectRange = 256.f; // Keep in sync with Shrine::getRange().
    constexpr float hexedCatEffectRange  = Cat::radius * 1.35f;

    const sf::Texture& backgroundTexture    = rtBackgroundProcessed.getTexture();
    const sf::Vec2f    backgroundViewOrigin = backgroundView.center - backgroundView.size / 2.f;
    const sf::Vec2f    activeViewDelta      = activeGameViewCenter - backgroundView.center;

    const auto drawBackgroundEffect =
        [&](const sf::Vec2f center, const float range, const sf::Color tint, const float effectStrength)
    {
        if (range <= 0.f || effectStrength <= 0.f)
            return;

        rt.flush();

        shaderShrineBackground.setUniform(suShrineBgTime, shaderTime);
        shaderShrineBackground.setUniform(suShrineBgViewOrigin, backgroundViewOrigin);
        shaderShrineBackground.setUniform(suShrineBgCenter, center);
        shaderShrineBackground.setUniform(suShrineBgRange, range);
        shaderShrineBackground.setUniform(suShrineBgTintR, static_cast<float>(tint.r) / 255.f);
        shaderShrineBackground.setUniform(suShrineBgTintG, static_cast<float>(tint.g) / 255.f);
        shaderShrineBackground.setUniform(suShrineBgTintB, static_cast<float>(tint.b) / 255.f);
        shaderShrineBackground.setUniform(suShrineBgTintA, static_cast<float>(tint.a) / 255.f);
        shaderShrineBackground.setUniform(suShrineBgDistortionStrength, 1812.f);
        shaderShrineBackground.setUniform(suShrineBgTintStrength, 0.2f);
        shaderShrineBackground.setUniform(suShrineBgEffectStrength, effectStrength);

        rt.draw(backgroundTexture,
                {.textureRect = {{0.f, 0.f}, backgroundView.size}},
                {.view = backgroundView, .texture = &backgroundTexture, .shader = &shaderShrineBackground});
    };

    for (const Shrine& shrine : pt->shrines)
    {
        const bool activationInProgress = shrine.tcActivation.hasValue() && !shrine.isActive();
        const bool shouldRender         = activationInProgress || shrine.isActive();
        if (!shouldRender)
            continue;

        const float activationFade = activationInProgress ? shrine.getActivationProgress() : 1.f;
        const float deathFade      = shrine.tcDeath.hasValue() ? (1.f - shrine.getDeathProgress()) : 1.f;
        const float effectStrength = activationFade * deathFade;
        if (effectStrength <= 0.f)
            continue;

        const float range = activationInProgress ? maxShrineEffectRange * activationFade : shrine.getRange();
        if (range <= 0.f)
            continue;

        const sf::Color tint = sf::Color::fromHSLA({.hue = shrine.getHue() + 40.f, .saturation = 1.f, .lightness = 0.5f});
        const sf::Vec2f backgroundSpaceCenter = shrine.position - activeViewDelta;
        drawBackgroundEffect(backgroundSpaceCenter, range, tint, effectStrength);
    }

    for (const Cat& cat : pt->cats)
    {
        if (!cat.isHexedOrCopyHexed())
            continue;

        const float effectStrength = cat.getHexedTimer()->remap(0.f, 1.f);
        if (effectStrength <= 0.f)
            continue;

        const bool      copyHexed = cat.hexedCopyTimer.hasValue();
        const float     hexHue    = copyHexed ? 180.f : 0.f;
        const sf::Color tint      = sf::Color::fromHSLA({.hue = hexHue + 40.f, .saturation = 1.f, .lightness = 0.5f});
        const sf::Vec2f backgroundSpaceCenter = cat.getDrawPosition(profile.enableCatBobbing) - activeViewDelta;
        drawBackgroundEffect(backgroundSpaceCenter, hexedCatEffectRange, tint, effectStrength);
    }

    // Combo (invincible) bubbles: warm gold halo that intensifies as the
    // player builds the combo, so the bubble visually pulls focus.
    for (const Bubble& bubble : pt->bubbles)
    {
        if (bubble.type != BubbleType::Combo)
            continue;

        // Effect grows from a baseline glow to full intensity as the click
        // count climbs toward the auto-pop cap.
        const auto& cfg            = gameConstants.events.invincibleBubble;
        const float maxClicks      = cfg.maxClicks == 0u ? 1.f : static_cast<float>(cfg.maxClicks);
        const float clicksFrac     = sf::base::clamp(static_cast<float>(bubble.comboClickCount) / maxClicks, 0.f, 1.f);
        const float effectStrength = sf::base::clamp(0.45f + 0.55f * clicksFrac, 0.f, 1.f);

        // The halo extends a bit past the bubble's outer edge.
        const float range = sf::base::max(bubble.radius * 3.5f, 96.f);

        const sf::Color tint = sf::Color::fromHSLA({.hue = 255.f, .saturation = 1.f, .lightness = 0.55f});
        const sf::Vec2f backgroundSpaceCenter = bubble.position - activeViewDelta;
        drawBackgroundEffect(backgroundSpaceCenter, range, tint.withAlpha(64), effectStrength);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::RenderTexture& Main::getHexedCatRenderTexture(const sf::base::SizeT index)
{
    SFML_BASE_ASSERT(index < hexedCatRenderTextures.size());
    return hexedCatRenderTextures[index];
}


////////////////////////////////////////////////////////////
void Main::enqueueHexedCatDrawCommand(const sf::CPUDrawableBatch& batch,
                                      const sf::Vec2f             position,
                                      const bool                  top,
                                      const float                 phaseSeed,
                                      const float                 effectStrength)
{
    SFML_BASE_ASSERT(hexedCatDrawCommands.size() < maxHexedCatRenderTextures);

    auto&           rtHexedCat = hexedCatRenderTextures[hexedCatDrawCommands.size()];
    const sf::Vec2f rtCenter   = rtHexedCat.getSize().toVec2f() * 0.5f;
    auto            toLocal    = sf::Transform::fromPosition(rtCenter - position);

    rtHexedCat.clear(sf::Color::Transparent);
    shader.setUniform(suBubbleEffect, false);
    rtHexedCat.draw(batch, {.transform = toLocal, .texture = &textureAtlas.getTexture(), .shader = &shader});
    rtHexedCat.display();

    hexedCatDrawCommands.pushBack(
        {.renderTextureIndex = hexedCatDrawCommands.size(),
         .position           = position,
         .phaseSeed          = phaseSeed,
         .effectStrength     = effectStrength,
         .top                = top});
}


////////////////////////////////////////////////////////////
void Main::drawHexedCatDrawCommands(const sf::View& view, const bool top)
{
    for (const HexedCatDrawCommand& command : hexedCatDrawCommands)
    {
        if (command.top != top)
            continue;

        auto&              rtHexedCat = getHexedCatRenderTexture(command.renderTextureIndex);
        const sf::Texture& texture    = rtHexedCat.getTexture();

        shaderHexed.setUniform(suHexedTime, shaderTime);
        shaderHexed.setUniform(suHexedSeed, command.phaseSeed);
        shaderHexed.setUniform(suHexedDistortionStrength, blend(0.35f, 1.f, command.effectStrength));
        shaderHexed.setUniform(suHexedShimmerStrength, blend(0.2f, 1.f, command.effectStrength));

        rtGame.draw(texture,
                    {.position    = command.position,
                     .origin      = texture.getSize().toVec2f() / 2.f,
                     .textureRect = texture.getRect(),
                     .color = sf::Color::whiteWithAlpha(static_cast<U8>(blend(255.f, 128.f, command.effectStrength)))},
                    {.view = view, .texture = &texture, .shader = &shaderHexed});
    }
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

    const U8 opacity = (&batch == &cpuCloudUiDrawableBatch || &batch == &cpuCloudHudDrawableBatch)
                           ? 255u
                           : static_cast<U8>(gameConstants.catCloudOpacity * 255.f);

    rtGame.draw(rtCloudProcessed.getTexture(),
                {.color = sf::Color{opacity, opacity, opacity, opacity}}, // because of premultiplication
                {.blendMode = premultipliedAlphaBlend});

    rtGame.flush(); // TODO P0: needed?
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

    dpiScalingFactor = window.getDisplayScale();
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

    dpiScalingFactor = window.getDisplayScale();
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
