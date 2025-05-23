#pragma once

#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/Music.hpp"

#include "SFML/Base/FloatMax.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// #define BUBBLEBYTE_NO_AUDIO 1

#include "Achievements.hpp"
#include "Aliases.hpp"
#include "Bubble.hpp"
#include "BubbleType.hpp"
#include "Cat.hpp"
#include "CatConstants.hpp"
#include "CatNames.hpp"
#include "CatType.hpp"
#include "Collision.hpp"
#include "Constants.hpp"
#include "ControlFlow.hpp"
#include "Countdown.hpp"
#include "Doll.hpp"
#include "Easing.hpp"
#include "HellPortal.hpp"
#include "HueColor.hpp"
#include "InputHelper.hpp"
#include "MathUtils.hpp"
#include "MemberGuard.hpp"
#include "Particle.hpp"
#include "ParticleType.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "RNG.hpp"
#include "RNGFast.hpp"
#include "Sampler.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "ShrineConstants.hpp"
#include "ShrineType.hpp"
#include "Sounds.hpp"
#include "Stats.hpp"
#include "SweepAndPrune.hpp"
#include "TextEffectWiggle.hpp"
#include "TextParticle.hpp"
#include "TextShakeEffect.hpp"
#include "Timer.hpp"
#include "Version.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/FloatMax.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Lround.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ThreadPool.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <random>
#include <string>
#include <utility>

#include <cctype>
#include <cmath>
#include <cstdio>
#include <ctime>


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    #include "Steam.hpp"
#endif


////////////////////////////////////////////////////////////<
inline bool debugMode = false;

////////////////////////////////////////////////////////////
inline bool handleBubbleCollision(const float deltaTimeMs, Bubble& iBubble, Bubble& jBubble)
{
    const auto result = handleCollision(deltaTimeMs,
                                        iBubble.position,
                                        jBubble.position,
                                        iBubble.velocity,
                                        jBubble.velocity,
                                        iBubble.radius,
                                        jBubble.radius,
                                        iBubble.type == BubbleType::Bomb ? 5.f : 1.f,
                                        jBubble.type == BubbleType::Bomb ? 5.f : 1.f);

    if (!result.hasValue())
        return false;

    iBubble.position += result->iDisplacement;
    jBubble.position += result->jDisplacement;
    iBubble.velocity += result->iVelocityChange;
    jBubble.velocity += result->jVelocityChange;

    return true;
}


////////////////////////////////////////////////////////////
inline bool handleCatCollision(const float deltaTimeMs, Cat& iCat, Cat& jCat)
{
    const auto
        result = handleCollision(deltaTimeMs, iCat.position, jCat.position, {}, {}, iCat.getRadius(), jCat.getRadius(), 1.f, 1.f);

    if (!result.hasValue())
        return false;

    if (!iCat.isHexedOrCopyHexed())
        iCat.position += result->iDisplacement;

    if (!jCat.isHexedOrCopyHexed())
        jCat.position += result->jDisplacement;

    return true;
}


////////////////////////////////////////////////////////////
inline bool handleCatShrineCollision(const float deltaTimeMs, Cat& cat, Shrine& shrine)
{
    const auto result = handleCollision(deltaTimeMs, cat.position, shrine.position, {}, {}, cat.getRadius(), 64.f, 1.f, 1.f);

    if (!result.hasValue())
        return false;

    cat.position += result->iDisplacement;
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline Bubble makeRandomBubble(Playthrough& pt, RNG& rng, const float mapLimit, const float maxY)
{
    return {
        .position = rng.getVec2f({mapLimit, maxY}),
        .velocity = rng.getVec2f({-0.1f, -0.1f}, {0.1f, 0.1f}),

        .radius = rng.getF(0.07f, 0.16f) * 256.f *
                  remap(static_cast<float>(pt.psvBubbleCount.nPurchases), 0.f, 30.f, 1.1f, 0.8f),
        .rotation = 0.f,
        .hueMod   = 0.f,

        .repelledCountdown  = {},
        .attractedCountdown = {},

        .type = BubbleType::Normal,
    };
}


////////////////////////////////////////////////////////////
inline constexpr auto playerComboDecay      = 0.95f;
inline constexpr auto playerComboDecayLaser = 0.995f;
inline constexpr auto mouseCatComboDecay    = 0.995f; // higher decay for mousecat (higher reward)

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr float getComboValueMult(const int n, const float decay)
{
    return (1.f - sf::base::pow(decay, static_cast<float>(n))) / (1.f - decay);
}

////////////////////////////////////////////////////////////
inline void drawMinimap(
    sf::Shader&             shader,
    const float             minimapScale,
    const float             mapLimit,
    const sf::View&         gameView,
    const sf::View&         hudView,
    sf::RenderTarget&       rt,
    const sf::Texture&      txBackgroundChunk,
    const sf::Texture&      txDrawings,
    sf::CPUDrawableBatch&   batch,
    const sf::TextureAtlas& textureAtlas,
    const sf::Vec2f         resolution,
    const float             hudScale,
    const float             hueMod,
    const sf::base::U8      shouldDrawUIAlpha,
    sf::FloatRect&          minimapRect)
{
    //
    // Screen position of minimap's top-left corner
    constexpr sf::Vec2f minimapPos = {15.f, 15.f};

    //
    // Size of full map in minimap space
    const sf::Vec2f minimapSize = boundaries / minimapScale;

    //
    // White border around minimap
    const sf::RectangleShapeData minimapBorder{.position         = minimapPos,
                                               .fillColor        = sf::Color::Transparent,
                                               .outlineColor     = sf::Color::whiteMask(shouldDrawUIAlpha),
                                               .outlineThickness = 2.f,
                                               .size             = {mapLimit / minimapScale, minimapSize.y}};

    minimapRect.position = minimapPos;
    minimapRect.size     = minimapBorder.size;

    //
    // Blue rectangle showing current visible area
    const sf::RectangleShapeData
        minimapIndicator{.position     = minimapPos.addX((gameView.center.x - gameScreenSize.x / 2.f) / minimapScale),
                         .fillColor    = sf::Color::Transparent,
                         .outlineColor = sf::Color::Blue.withRotatedHue(hueMod).withAlpha(shouldDrawUIAlpha),
                         .outlineThickness = 2.f,
                         .size             = {gameScreenSize / minimapScale}};

    //
    // Convert minimap dimensions to normalized `[0, 1]` range for scissor rectangle
    const float progressRatio = sf::base::clamp(mapLimit / boundaries.x, 0.f, 1.f);

    auto minimapScaledPosition = minimapPos.componentWiseDiv(resolution / hudScale);

    const auto minimapScaledSize = minimapSize.componentWiseDiv(resolution / hudScale)
                                       .clampX(0.f, (1.f - minimapScaledPosition.x) / progressRatio)
                                       .clampY(0.f, 1.f - minimapScaledPosition.y);

    const sf::FloatRect preClampScissorRect{minimapScaledPosition, // Scissor rectangle position (normalized)
                                            {
                                                progressRatio * minimapScaledSize.x, // Only show accessible width
                                                minimapScaledSize.y                  // Full height
                                            }};

    const auto clampedScissorRect = sf::View::ScissorRect::fromRectClamped(preClampScissorRect);

    //
    // Special view that renders the world scaled down for minimap
    const sf::View minimapView                                                  //
        {.center  = (resolution * 0.5f - minimapPos * hudScale) * minimapScale, // Offset center to align minimap
         .size    = resolution * minimapScale,                                  // Zoom out to show scaled-down world
         .scissor = clampedScissorRect};

    //
    // Draw minimap contents
    rt.setView(minimapView); // Use minimap projection
    rt.draw(sf::RectangleShapeData{.fillColor = sf::Color::blackMask(shouldDrawUIAlpha), .size = boundaries * hudScale});

    // The background has a repeating texture, and it's one ninth of the whole map
    const sf::Vec2f backgroundRectSize{static_cast<float>(txBackgroundChunk.getSize().x) * nGameScreens,
                                       static_cast<float>(txBackgroundChunk.getSize().y)};

    rt.draw(txBackgroundChunk,
            {.scale       = {hudScale, hudScale},
             .textureRect = {{0.f, 0.f}, backgroundRectSize},
             .color       = hueColor(hueMod, sf::base::min(shouldDrawUIAlpha, static_cast<sf::base::U8>(128u)))},
            {.shader = &shader}); // Draw world background

    rt.draw(txDrawings,
            {.scale       = {hudScale, hudScale},
             .textureRect = {{0.f, 0.f}, backgroundRectSize},
             .color       = sf::Color::whiteMask(sf::base::min(shouldDrawUIAlpha, static_cast<sf::base::U8>(215u)))},
            {.shader = &shader}); // Draw drawings

    if (shouldDrawUIAlpha > 200u)
    {
        batch.scale = {hudScale, hudScale};
        rt.draw(batch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        batch.scale = {1.f, 1.f};
    }

    //
    // Switch back to HUD view and draw overlay elements
    rt.setView(hudView);
    rt.draw(minimapBorder);    // Draw border frame
    rt.draw(minimapIndicator); // Draw current view indicator
}


////////////////////////////////////////////////////////////
inline void drawSplashScreen(sf::RenderTarget&        rt,
                             const sf::Texture&       txLogo,
                             const TargetedCountdown& splashCountdown,
                             const sf::Vec2f          resolution,
                             const float              hudScale)
{
    const auto progress = easeInOutCubic(splashCountdown.getProgressBounced());

    rt.draw(sf::Sprite{.position    = resolution / 2.f / hudScale,
                       .scale       = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutCubic(progress)) / hudScale,
                       .origin      = txLogo.getSize().toVec2f() / 2.f,
                       .textureRect = txLogo.getRect(),
                       .color       = sf::Color::whiteMask(static_cast<U8>(easeInOutSine(progress) * 255.f))},
            {.texture = &txLogo});
}

////////////////////////////////////////////////////////////
/// Main struct
///
////////////////////////////////////////////////////////////
struct Main
{
////////////////////////////////////////////////////////////
// Audio context and playback device
#ifndef BUBBLEBYTE_NO_AUDIO
    sf::AudioContext   audioContext{sf::AudioContext::create().value()};
    sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};
#endif

    ////////////////////////////////////////////////////////////
    // Graphics context
    sf::GraphicsContext graphicsContext{sf::GraphicsContext::create().value()};

    ////////////////////////////////////////////////////////////
    // Shader with hue support and bubble effects
    sf::Shader shader{[]
    {
        auto result = sf::Shader::loadFromFile(
                          {.vertexPath = "resources/shader.vert", .fragmentPath = "resources/shader.frag"})
                          .value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    sf::Shader::UniformLocation suBackgroundTexture = shader.getUniformLocation("u_backgroundTexture").value();
    sf::Shader::UniformLocation suTime              = shader.getUniformLocation("u_time").value();
    sf::Shader::UniformLocation suResolution        = shader.getUniformLocation("u_resolution").value();
    sf::Shader::UniformLocation suBubbleEffect      = shader.getUniformLocation("u_bubbleEffect").value();

    sf::Shader::UniformLocation suIridescenceStrength = shader.getUniformLocation("u_iridescenceStrength").value();
    sf::Shader::UniformLocation suEdgeFactorMin       = shader.getUniformLocation("u_edgeFactorMin").value();
    sf::Shader::UniformLocation suEdgeFactorMax       = shader.getUniformLocation("u_edgeFactorMax").value();
    sf::Shader::UniformLocation suEdgeFactorStrength  = shader.getUniformLocation("u_edgeFactorStrength").value();
    sf::Shader::UniformLocation suDistorsionStrength  = shader.getUniformLocation("u_distorsionStrength").value();

    sf::Shader::UniformLocation suSubTexOrigin = shader.getUniformLocation("u_subTexOrigin").value();
    sf::Shader::UniformLocation suSubTexSize   = shader.getUniformLocation("u_subTexSize").value();

    sf::Shader::UniformLocation suBubbleLightness = shader.getUniformLocation("u_bubbleLightness").value();
    sf::Shader::UniformLocation suLensDistortion  = shader.getUniformLocation("u_lensDistortion").value();

    float shaderTime = 0.f;

    ////////////////////////////////////////////////////////////
    // Shader with post-processing effects
    sf::Shader shaderPostProcess{[]
    {
        // TODO P2: (lib) add support for `#include` in shaders
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/postprocess.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    sf::Shader::UniformLocation suPPVibrance   = shaderPostProcess.getUniformLocation("u_vibrance").value();
    sf::Shader::UniformLocation suPPSaturation = shaderPostProcess.getUniformLocation("u_saturation").value();
    sf::Shader::UniformLocation suPPLightness  = shaderPostProcess.getUniformLocation("u_lightness").value();
    sf::Shader::UniformLocation suPPSharpness  = shaderPostProcess.getUniformLocation("u_sharpness").value();

    ////////////////////////////////////////////////////////////
    // Context settings
    const unsigned int        aaLevel = sf::base::min(16u, sf::RenderTexture::getMaximumAntiAliasingLevel());
    const sf::ContextSettings contextSettings{.antiAliasingLevel = aaLevel};

    ///////////////////////////////////////////////////////////
    // Profile (stores settings)
    Profile profile{[&]
    {
        Profile out;

        if (sf::Path{"userdata/profile.json"}.exists())
        {
            loadProfileFromFile(out);
            sf::cOut() << "Loaded profile from file on startup\n";
        }

        return out;
    }()};

    MEMBER_SCOPE_GUARD(Main, {
        sf::cOut() << "Saving profile to file on exit\n";
        saveProfileToFile(self.profile);
    });

    ////////////////////////////////////////////////////////////
    // SFML fonts
    sf::Font fontMouldyCheese{sf::Font::openFromFile("resources/mouldycheese.ttf").value()};

    ////////////////////////////////////////////////////////////
    // Render window
    sf::RenderWindow window{makeWindow()};
    float            dpiScalingFactor = 1.f;

    bool loadingGuard{[&]
    {
        window.clear(sf::Color::Black);

        sf::TextData loadingTextData{.position         = window.getSize() / 2.f,
                                     .string           = "Loading...",
                                     .characterSize    = 48u,
                                     .fillColor        = sf::Color::White,
                                     .outlineColor     = colorBlueOutline,
                                     .outlineThickness = 2.f};

        loadingTextData.origin = sf::TextUtils::precomputeTextLocalBounds(fontMouldyCheese, loadingTextData).size / 2.f;
        window.draw(fontMouldyCheese, loadingTextData);

        window.display();
        return true;
    }()};

    ////////////////////////////////////////////////////////////
    // ImGui context
    sf::ImGuiContext imGuiContext{/* loadDefaultFont */ false};

    ////////////////////////////////////////////////////////////
    // Exiting status
    bool escWasPressed = false;
    bool mustExit      = false;

    ////////////////////////////////////////////////////////////
    // Texture atlas
    sf::TextureAtlas textureAtlas{sf::Texture::create({4096u, 4096u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // SFML fonts
    sf::Font fontSuperBakery{sf::Font::openFromFile("resources/superbakery.ttf", &textureAtlas).value()};

    ////////////////////////////////////////////////////////////
    // ImGui fonts
    ImFont* fontImGuiSuperBakery{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/superbakery.ttf", 26.f)};
    ImFont* fontImGuiMouldyCheese{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/mouldycheese.ttf", 26.f)};

    ////////////////////////////////////////////////////////////
    // Music
    static inline constexpr const char* bgmPathNormal = "resources/hibiscus.mp3";
    static inline constexpr const char* bgmPathWitch  = "resources/bgmwitch.mp3";
    static inline constexpr const char* bgmPathWizard = "resources/bgmwizard.mp3";
    static inline constexpr const char* bgmPathMouse  = "resources/bgmmouse.mp3";
    static inline constexpr const char* bgmPathEngi   = "resources/bgmengi.mp3";

    const char* lastPlayedMusic = bgmPathNormal; // to avoid restarting the same song

    sf::base::SizeT currentBGMBufferIdx = 0u; // which one of the two buffers is "current"
    Countdown       bgmTransition;            // fade in/out timer

    struct BGMBuffer
    {
        sf::MusicReader musicReader;
        sf::Music       music;

        explicit BGMBuffer(sf::PlaybackDevice& playbackDevice, sf::MusicReader&& theMusicSource) :
        musicReader{SFML_BASE_MOVE(theMusicSource)},
        music{playbackDevice, musicReader}
        {
        }
    };

    sf::base::Array<sf::base::Optional<BGMBuffer>, 2u> bgmBuffers{sf::base::nullOpt, sf::base::nullOpt}; // for smooth fade

    ////////////////////////////////////////////////////////////
    // Sound management
    Sounds       sounds{/* volumeMult */ 1.f};
    sf::Listener listener;

    ////////////////////////////////////////////////////////////
    // Delayed actions
    struct DelayedAction
    {
        Countdown                            delayCountdown;
        sf::base::FixedFunction<void(), 128> action;
    };

    sf::base::Vector<DelayedAction> delayedActions;

    ////////////////////////////////////////////////////////////
    // Background and ImGui render textures
    sf::RenderTexture rtBackground{
        sf::RenderTexture::create(gameScreenSize.toVec2u(), {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value()};
    sf::RenderTexture rtImGui{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value()};

    ////////////////////////////////////////////////////////////
    // Game render texture (before post-processing)
    sf::RenderTexture rtGame{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value()};

    ////////////////////////////////////////////////////////////
    // Textures (not in atlas)
    static inline constexpr sf::TextureLoadSettings bgSettings{.smooth = true, .wrapMode = sf::TextureWrapMode::Repeat};

    sf::Texture txLogo{sf::Texture::loadFromFile("resources/logo.png", {.smooth = true}).value()};
    sf::Texture txFixedBg{
        sf::Texture::loadFromFile("resources/fixedbg.png", {.smooth = true, .wrapMode = sf::TextureWrapMode::MirroredRepeat})
            .value()};
    sf::Texture txBackgroundChunk{sf::Texture::loadFromFile("resources/backgroundchunk.png", bgSettings).value()};
    sf::Texture txBackgroundChunkDesaturated{
        sf::Texture::loadFromFile("resources/backgroundchunkdesaturated.png", bgSettings).value()};
    sf::Texture txClouds{sf::Texture::loadFromFile("resources/clouds.png", bgSettings).value()};
    sf::Texture txBgSwamp{sf::Texture::loadFromFile("resources/bgswamp.png", bgSettings).value()};
    sf::Texture txBgObservatory{sf::Texture::loadFromFile("resources/bgobservatory.png", bgSettings).value()};
    sf::Texture txBgAimTraining{sf::Texture::loadFromFile("resources/bgaimtraining.png", bgSettings).value()};
    sf::Texture txBgFactory{sf::Texture::loadFromFile("resources/bgfactory.png", bgSettings).value()};
    sf::Texture txBgWindTunnel{sf::Texture::loadFromFile("resources/bgwindtunnel.png", bgSettings).value()};
    sf::Texture txBgMagnetosphere{sf::Texture::loadFromFile("resources/bgmagnetosphere.png", bgSettings).value()};
    sf::Texture txBgAuditorium{sf::Texture::loadFromFile("resources/bgauditorium.png", bgSettings).value()};
    sf::Texture txDrawings{sf::Texture::loadFromFile("resources/drawings.png", {.smooth = true}).value()};
    sf::Texture txTipBg{sf::Texture::loadFromFile("resources/tipbg.png", {.smooth = true}).value()};
    sf::Texture txTipByte{sf::Texture::loadFromFile("resources/tipbyte.png", {.smooth = true}).value()};
    sf::Texture txCursor{sf::Texture::loadFromFile("resources/cursor.png", {.smooth = true}).value()};
    sf::Texture txCursorMultipop{sf::Texture::loadFromFile("resources/cursormultipop.png", {.smooth = true}).value()};
    sf::Texture txCursorLaser{sf::Texture::loadFromFile("resources/cursorlaser.png", {.smooth = true}).value()};
    sf::Texture txCursorGrab{sf::Texture::loadFromFile("resources/cursorgrab.png", {.smooth = true}).value()};
    sf::Texture txArrow{sf::Texture::loadFromFile("resources/arrow.png", {.smooth = true}).value()};
    sf::Texture txUnlock{sf::Texture::loadFromFile("resources/unlock.png", {.smooth = true}).value()};
    sf::Texture txPurchasable{sf::Texture::loadFromFile("resources/purchasable.png", {.smooth = true}).value()};
    sf::Texture txLetter{sf::Texture::loadFromFile("resources/letter.png", {.smooth = true}).value()};
    sf::Texture txLetterText{sf::Texture::loadFromFile("resources/lettertext.png", {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // UI texture atlas
    sf::TextureAtlas uiTextureAtlas{sf::Texture::create({2048u, 1024u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // Quick toolbar icons
    sf::FloatRect txrIconVolume{addImgResourceToUIAtlas("iconvolumeon.png")};
    sf::FloatRect txrIconBGM{addImgResourceToUIAtlas("iconmusicon.png")};
    sf::FloatRect txrIconBg{addImgResourceToUIAtlas("iconbg.png")};
    sf::FloatRect txrIconCfg{addImgResourceToUIAtlas("iconcfg.png")};
    sf::FloatRect txrIconCopyCat{addImgResourceToUIAtlas("iconcopycat.png")};

    ////////////////////////////////////////////////////////////
    // Shop menu separator textures
    sf::FloatRect txrMenuSeparator0{addImgResourceToUIAtlas("menuseparator0.png")};
    sf::FloatRect txrMenuSeparator1{addImgResourceToUIAtlas("menuseparator1.png")};
    sf::FloatRect txrMenuSeparator2{addImgResourceToUIAtlas("menuseparator2.png")};
    sf::FloatRect txrMenuSeparator3{addImgResourceToUIAtlas("menuseparator3.png")};
    sf::FloatRect txrMenuSeparator4{addImgResourceToUIAtlas("menuseparator4.png")};
    sf::FloatRect txrMenuSeparator5{addImgResourceToUIAtlas("menuseparator5.png")};
    sf::FloatRect txrMenuSeparator6{addImgResourceToUIAtlas("menuseparator6.png")};
    sf::FloatRect txrMenuSeparator7{addImgResourceToUIAtlas("menuseparator7.png")};
    sf::FloatRect txrMenuSeparator8{addImgResourceToUIAtlas("menuseparator8.png")};

    ////////////////////////////////////////////////////////////
    // Prestige menu separator textures
    sf::FloatRect txrPrestigeSeparator0{addImgResourceToUIAtlas("prestigeseparator0.png")};
    sf::FloatRect txrPrestigeSeparator1{addImgResourceToUIAtlas("prestigeseparator1.png")};
    sf::FloatRect txrPrestigeSeparator2{addImgResourceToUIAtlas("prestigeseparator2.png")};
    sf::FloatRect txrPrestigeSeparator3{addImgResourceToUIAtlas("prestigeseparator3.png")};
    sf::FloatRect txrPrestigeSeparator4{addImgResourceToUIAtlas("prestigeseparator4.png")};
    sf::FloatRect txrPrestigeSeparator5{addImgResourceToUIAtlas("prestigeseparator5.png")};
    sf::FloatRect txrPrestigeSeparator6{addImgResourceToUIAtlas("prestigeseparator6.png")};
    sf::FloatRect txrPrestigeSeparator7{addImgResourceToUIAtlas("prestigeseparator7.png")};
    sf::FloatRect txrPrestigeSeparator8{addImgResourceToUIAtlas("prestigeseparator8.png")};
    sf::FloatRect txrPrestigeSeparator9{addImgResourceToUIAtlas("prestigeseparator9.png")};
    sf::FloatRect txrPrestigeSeparator10{addImgResourceToUIAtlas("prestigeseparator10.png")};
    sf::FloatRect txrPrestigeSeparator11{addImgResourceToUIAtlas("prestigeseparator11.png")};
    sf::FloatRect txrPrestigeSeparator12{addImgResourceToUIAtlas("prestigeseparator12.png")};
    sf::FloatRect txrPrestigeSeparator13{addImgResourceToUIAtlas("prestigeseparator13.png")};
    sf::FloatRect txrPrestigeSeparator14{addImgResourceToUIAtlas("prestigeseparator14.png")};
    sf::FloatRect txrPrestigeSeparator15{addImgResourceToUIAtlas("prestigeseparator15.png")};

    ////////////////////////////////////////////////////////////
    // Magic menu separator textures
    sf::FloatRect txrMagicSeparator0{addImgResourceToUIAtlas("magicseparator0.png")};
    sf::FloatRect txrMagicSeparator1{addImgResourceToUIAtlas("magicseparator1.png")};
    sf::FloatRect txrMagicSeparator2{addImgResourceToUIAtlas("magicseparator2.png")};
    sf::FloatRect txrMagicSeparator3{addImgResourceToUIAtlas("magicseparator3.png")};

    ////////////////////////////////////////////////////////////
    // Background hues
    static inline constexpr EXACT_ARRAY(
        float,
        backgroundHues,
        nShrineTypes + 1u,
        {
            0.f,    // Normal
            -120.f, // Voodoo
            47.f,   // Magic
            -15.f,  // Clicking
            180.f,  // Automation
            121.f,  // Repulsion
            -45.f,  // Attraction
            -155.f, // Camouflage
            -80.f,  // Victory
        });

    ////////////////////////////////////////////////////////////
    // Texture atlas rects
    sf::FloatRect txrWhiteDot{textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value()};
    sf::FloatRect txrBubble{addImgResourceToAtlas("bubble2.png")};
    sf::FloatRect txrBubbleStar{addImgResourceToAtlas("bubble3.png")};
    sf::FloatRect txrBubbleNova{addImgResourceToAtlas("bubble4.png")};
    sf::FloatRect txrCat{addImgResourceToAtlas("cat.png")};
    sf::FloatRect txrUniCat{addImgResourceToAtlas("unicat3.png")};
    sf::FloatRect txrUniCat2{addImgResourceToAtlas("unicat2.png")};
    sf::FloatRect txrUniCatWings{addImgResourceToAtlas("unicatwings.png")};
    sf::FloatRect txrDevilCat2{addImgResourceToAtlas("devilcat2.png")};
    sf::FloatRect txrDevilCat3{addImgResourceToAtlas("devilcat3.png")};
    sf::FloatRect txrDevilCat3Arm{addImgResourceToAtlas("devilcat3arm.png")};
    sf::FloatRect txrDevilCat3Book{addImgResourceToAtlas("devilcat3book.png")};
    sf::FloatRect txrDevilCat3Tail{addImgResourceToAtlas("devilcat3tail.png")};
    sf::FloatRect txrDevilCat2Book{addImgResourceToAtlas("devilcat2book.png")};
    sf::FloatRect txrCatPaw{addImgResourceToAtlas("catpaw.png")};
    sf::FloatRect txrCatTail{addImgResourceToAtlas("cattail.png")};
    sf::FloatRect txrSmartCatHat{addImgResourceToAtlas("smartcathat.png")};
    sf::FloatRect txrSmartCatDiploma{addImgResourceToAtlas("smartcatdiploma.png")};
    sf::FloatRect txrBrainBack{addImgResourceToAtlas("brainback.png")};
    sf::FloatRect txrBrainFront{addImgResourceToAtlas("brainfront.png")};
    sf::FloatRect txrUniCatTail{addImgResourceToAtlas("unicattail.png")};
    sf::FloatRect txrUniCat2Tail{addImgResourceToAtlas("unicat2tail.png")};
    sf::FloatRect txrDevilCatTail2{addImgResourceToAtlas("devilcattail2.png")};
    sf::FloatRect txrAstroCatTail{addImgResourceToAtlas("astrocattail.png")};
    sf::FloatRect txrAstroCatFlag{addImgResourceToAtlas("astrocatflag.png")};
    sf::FloatRect txrWitchCatTail{addImgResourceToAtlas("witchcattail.png")};
    sf::FloatRect txrWizardCatTail{addImgResourceToAtlas("wizardcattail.png")};
    sf::FloatRect txrMouseCatTail{addImgResourceToAtlas("mousecattail.png")};
    sf::FloatRect txrMouseCatMouse{addImgResourceToAtlas("mousecatmouse.png")};
    sf::FloatRect txrEngiCatTail{addImgResourceToAtlas("engicattail.png")};
    sf::FloatRect txrEngiCatWrench{addImgResourceToAtlas("engicatwrench.png")};
    sf::FloatRect txrRepulsoCatTail{addImgResourceToAtlas("repulsocattail.png")};
    sf::FloatRect txrAttractoCatTail{addImgResourceToAtlas("attractocattail.png")};
    sf::FloatRect txrCopyCatTail{addImgResourceToAtlas("copycattail.png")};
    sf::FloatRect txrAttractoCatMagnet{addImgResourceToAtlas("attractocatmagnet.png")};
    sf::FloatRect txrUniCatPaw{addImgResourceToAtlas("unicatpaw.png")};
    sf::FloatRect txrDevilCatPaw{addImgResourceToAtlas("devilcatpaw.png")};
    sf::FloatRect txrDevilCatPaw2{addImgResourceToAtlas("devilcatpaw2.png")};
    sf::FloatRect txrParticle{addImgResourceToAtlas("particle.png")};
    sf::FloatRect txrStarParticle{addImgResourceToAtlas("starparticle.png")};
    sf::FloatRect txrFireParticle{addImgResourceToAtlas("fireparticle.png")};
    sf::FloatRect txrFireParticle2{addImgResourceToAtlas("fireparticle2.png")};
    sf::FloatRect txrSmokeParticle{addImgResourceToAtlas("smokeparticle.png")};
    sf::FloatRect txrExplosionParticle{addImgResourceToAtlas("explosionparticle.png")};
    sf::FloatRect txrTrailParticle{addImgResourceToAtlas("trailparticle.png")};
    sf::FloatRect txrHexParticle{addImgResourceToAtlas("hexparticle.png")};
    sf::FloatRect txrShrineParticle{addImgResourceToAtlas("shrineparticle.png")};
    sf::FloatRect txrCogParticle{addImgResourceToAtlas("cogparticle.png")};
    sf::FloatRect txrWitchCat{addImgResourceToAtlas("witchcat.png")};
    sf::FloatRect txrWitchCatPaw{addImgResourceToAtlas("witchcatpaw.png")};
    sf::FloatRect txrAstroCat{addImgResourceToAtlas("astromeow.png")};
    sf::FloatRect txrBomb{addImgResourceToAtlas("bomb.png")};
    sf::FloatRect txrShrine{addImgResourceToAtlas("shrine.png")};
    sf::FloatRect txrWizardCat{addImgResourceToAtlas("wizardcat.png")};
    sf::FloatRect txrWizardCatPaw{addImgResourceToAtlas("wizardcatpaw.png")};
    sf::FloatRect txrMouseCat{addImgResourceToAtlas("mousecat.png")};
    sf::FloatRect txrMouseCatPaw{addImgResourceToAtlas("mousecatpaw.png")};
    sf::FloatRect txrEngiCat{addImgResourceToAtlas("engicat.png")};
    sf::FloatRect txrEngiCatPaw{addImgResourceToAtlas("engicatpaw.png")};
    sf::FloatRect txrRepulsoCat{addImgResourceToAtlas("repulsocat.png")};
    sf::FloatRect txrRepulsoCatPaw{addImgResourceToAtlas("repulsocatpaw.png")};
    sf::FloatRect txrAttractoCat{addImgResourceToAtlas("attractocat.png")};
    sf::FloatRect txrCopyCat{addImgResourceToAtlas("copycat.png")};
    sf::FloatRect txrDuckCat{addImgResourceToAtlas("duck.png")};
    sf::FloatRect txrDuckFlag{addImgResourceToAtlas("duckflag.png")};
    sf::FloatRect txrAttractoCatPaw{addImgResourceToAtlas("attractocatpaw.png")};
    sf::FloatRect txrCopyCatPaw{addImgResourceToAtlas("copycatpaw.png")};
    sf::FloatRect txrDollNormal{addImgResourceToAtlas("dollnormal.png")};
    sf::FloatRect txrDollUni{addImgResourceToAtlas("dolluni.png")};
    sf::FloatRect txrDollDevil{addImgResourceToAtlas("dolldevil.png")};
    sf::FloatRect txrDollAstro{addImgResourceToAtlas("dollastro.png")};
    sf::FloatRect txrDollWizard{addImgResourceToAtlas("dollwizard.png")};
    sf::FloatRect txrDollMouse{addImgResourceToAtlas("dollmouse.png")};
    sf::FloatRect txrDollEngi{addImgResourceToAtlas("dollengi.png")};
    sf::FloatRect txrDollRepulso{addImgResourceToAtlas("dollrepulso.png")};
    sf::FloatRect txrDollAttracto{addImgResourceToAtlas("dollattracto.png")};
    sf::FloatRect txrCoin{addImgResourceToAtlas("bytecoin.png")};
    sf::FloatRect txrCatSoul{addImgResourceToAtlas("catsoul.png")};
    sf::FloatRect txrHellPortal{addImgResourceToAtlas("hellportal.png")};
    sf::FloatRect txrCatEyeLid0{addImgResourceToAtlas("cateyelid0.png")};
    sf::FloatRect txrCatEyeLid1{addImgResourceToAtlas("cateyelid1.png")};
    sf::FloatRect txrCatEyeLid2{addImgResourceToAtlas("cateyelid2.png")};
    sf::FloatRect txrCatWhiteEyeLid0{addImgResourceToAtlas("catwhiteeyelid0.png")};
    sf::FloatRect txrCatWhiteEyeLid1{addImgResourceToAtlas("catwhiteeyelid1.png")};
    sf::FloatRect txrCatWhiteEyeLid2{addImgResourceToAtlas("catwhiteeyelid2.png")};
    sf::FloatRect txrCatDarkEyeLid0{addImgResourceToAtlas("catdarkeyelid0.png")};
    sf::FloatRect txrCatDarkEyeLid1{addImgResourceToAtlas("catdarkeyelid1.png")};
    sf::FloatRect txrCatDarkEyeLid2{addImgResourceToAtlas("catdarkeyelid2.png")};
    sf::FloatRect txrCatGrayEyeLid0{addImgResourceToAtlas("catgrayeyelid0.png")};
    sf::FloatRect txrCatGrayEyeLid1{addImgResourceToAtlas("catgrayeyelid1.png")};
    sf::FloatRect txrCatGrayEyeLid2{addImgResourceToAtlas("catgrayeyelid2.png")};
    sf::FloatRect txrCatEars0{addImgResourceToAtlas("catears0.png")};
    sf::FloatRect txrCatEars1{addImgResourceToAtlas("catears1.png")};
    sf::FloatRect txrCatEars2{addImgResourceToAtlas("catears2.png")};
    sf::FloatRect txrCatYawn0{addImgResourceToAtlas("catyawn0.png")};
    sf::FloatRect txrCatYawn1{addImgResourceToAtlas("catyawn1.png")};
    sf::FloatRect txrCatYawn2{addImgResourceToAtlas("catyawn2.png")};
    sf::FloatRect txrCatYawn3{addImgResourceToAtlas("catyawn3.png")};
    sf::FloatRect txrCatYawn4{addImgResourceToAtlas("catyawn4.png")};
    sf::FloatRect txrCCMaskWitch{addImgResourceToAtlas("ccmaskwitch.png")};
    sf::FloatRect txrCCMaskWizard{addImgResourceToAtlas("ccmaskwizard.png")};
    sf::FloatRect txrCCMaskMouse{addImgResourceToAtlas("ccmaskmouse.png")};
    sf::FloatRect txrCCMaskEngi{addImgResourceToAtlas("ccmaskengi.png")};
    sf::FloatRect txrCCMaskRepulso{addImgResourceToAtlas("ccmaskrepulso.png")};
    sf::FloatRect txrCCMaskAttracto{addImgResourceToAtlas("ccmaskattracto.png")};
    sf::FloatRect txrMMNormal{addImgResourceToAtlas("mmcatnormal.png")};
    sf::FloatRect txrMMUni{addImgResourceToAtlas("mmcatuni.png")};
    sf::FloatRect txrMMDevil{addImgResourceToAtlas("mmcatdevil.png")};
    sf::FloatRect txrMMAstro{addImgResourceToAtlas("mmcatastro.png")};
    sf::FloatRect txrMMWitch{addImgResourceToAtlas("mmcatwitch.png")};
    sf::FloatRect txrMMWizard{addImgResourceToAtlas("mmcatwizard.png")};
    sf::FloatRect txrMMMouse{addImgResourceToAtlas("mmcatmouse.png")};
    sf::FloatRect txrMMEngi{addImgResourceToAtlas("mmcatengi.png")};
    sf::FloatRect txrMMRepulso{addImgResourceToAtlas("mmcatrepulso.png")};
    sf::FloatRect txrMMAttracto{addImgResourceToAtlas("mmcatattracto.png")};
    sf::FloatRect txrMMCopy{addImgResourceToAtlas("mmcatcopy.png")};
    sf::FloatRect txrMMDuck{addImgResourceToAtlas("mmduck.png")};
    sf::FloatRect txrMMShrine{addImgResourceToAtlas("mmshrine.png")};

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking
    const sf::FloatRect* eyeLidRects[8]{
        &txrCatEyeLid2,
        &txrCatEyeLid1,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid0,
        &txrCatEyeLid1,
        &txrCatEyeLid2,
    };

    static inline constexpr auto nEyeLidRects = sf::base::getArraySize(&Main::eyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (white)
    const sf::FloatRect* whiteEyeLidRects[8]{
        &txrCatWhiteEyeLid2,
        &txrCatWhiteEyeLid1,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid0,
        &txrCatWhiteEyeLid1,
        &txrCatWhiteEyeLid2,
    };

    static inline constexpr auto nWhiteEyeLidRects = sf::base::getArraySize(&Main::whiteEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (dark)
    const sf::FloatRect* darkEyeLidRects[8]{
        &txrCatDarkEyeLid2,
        &txrCatDarkEyeLid1,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid0,
        &txrCatDarkEyeLid1,
        &txrCatDarkEyeLid2,
    };

    static inline constexpr auto nDarkEyeLidRects = sf::base::getArraySize(&Main::darkEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking (gray)
    const sf::FloatRect* grayEyeLidRects[8]{
        &txrCatGrayEyeLid2,
        &txrCatGrayEyeLid1,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid0,
        &txrCatGrayEyeLid1,
        &txrCatGrayEyeLid2,
    };

    static inline constexpr auto nGrayEyeLidRects = sf::base::getArraySize(&Main::grayEyeLidRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: ear flapping
    const sf::FloatRect* earRects[8]{
        &txrCatEars0,
        &txrCatEars1,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars2,
        &txrCatEars1,
        &txrCatEars0,
    };

    static inline constexpr auto nEarRects = sf::base::getArraySize(&Main::earRects);

    ////////////////////////////////////////////////////////////
    // Cat animation rects: yawning
    const sf::FloatRect* catYawnRects[14]{
        &txrCatYawn0,
        &txrCatYawn1,
        &txrCatYawn2,
        &txrCatYawn3,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn4,
        &txrCatYawn3,
        &txrCatYawn2,
        &txrCatYawn1,
        &txrCatYawn0,
    };

    static inline constexpr auto nYawnRects = sf::base::getArraySize(&Main::catYawnRects);

    ///////////////////////////////////////////////////////////
    const sf::FloatRect particleRects[nParticleTypes] = {
        txrParticle,
        txrStarParticle,
        txrFireParticle,
        txrHexParticle,
        txrShrineParticle,
        txrMouseCatPaw,
        txrCogParticle,
        txrCoin,
        txrCatSoul,
        txrFireParticle2,
        txrSmokeParticle,
        txrExplosionParticle,
        txrTrailParticle,
    };

    ////////////////////////////////////////////////////////////
    // Playthrough (game state)
    Playthrough ptMain;
    Playthrough ptSpeedrun;
    MEMBER_SCOPE_GUARD(Main, {
        sf::cOut() << "Saving playthrough to file on exit\n";
        self.saveMainPlaythroughToFile();
    });

    ////////////////////////////////////////////////////////////
    // Currently active playthrough (game state)
    Playthrough* pt = &ptMain;

    ////////////////////////////////////////////////////////////
    // Prestige availability tracking
    bool wasPrestigeAvailableLastFrame = false;

    ////////////////////////////////////////////////////////////
    // Buy combo reminder secret achievement
    int buyReminder = 0;

    ////////////////////////////////////////////////////////////
    // Wasted effort ritual secret achievement
    bool wastedEffort = false;

    ////////////////////////////////////////////////////////////
    // Witchcat animation
    float witchcatWobblePhase{0.f};
    float copyWitchcatWobblePhase{0.f};

    ////////////////////////////////////////////////////////////
    // Wizardcat spin
    Countdown wizardcatSpin;
    float     wizardcatAbsorptionRotation{0.f};

    ////////////////////////////////////////////////////////////
    // Copycat state
    Countdown copycatMaskAnimCd;
    Countdown copycatMaskAnim;

    ////////////////////////////////////////////////////////////
    // Frametime-independent astro/portal proc
    Countdown frameProcCd;
    bool      frameProcThisFrame{false};

    ////////////////////////////////////////////////////////////
    // HUD money text
    sf::Text        moneyText{fontSuperBakery,
                              {.position         = {15.f, 70.f},
                               .string           = "$0",
                               .characterSize    = 64u,
                               .fillColor        = sf::Color::White,
                               .outlineColor     = colorBlueOutline,
                               .outlineThickness = 4.f}};
    TextShakeEffect moneyTextShakeEffect;

    ////////////////////////////////////////////////////////////
    // HUD combo text
    sf::Text        comboText{fontSuperBakery,
                              {.position         = moneyText.position.addY(35.f),
                               .string           = "x1",
                               .characterSize    = 48u,
                               .fillColor        = sf::Color::White,
                               .outlineColor     = colorBlueOutline,
                               .outlineThickness = 3.f}};
    TextShakeEffect comboTextShakeEffect;

    ////////////////////////////////////////////////////////////
    // HUD buff text
    sf::Text buffText{fontSuperBakery,
                      {.position         = comboText.position.addY(35.f),
                       .string           = "",
                       .characterSize    = 48u,
                       .fillColor        = sf::Color::White,
                       .outlineColor     = colorBlueOutline,
                       .outlineThickness = 3.f}};

    ////////////////////////////////////////////////////////////
    // HUD demo text
    sf::Text demoText{fontSuperBakery,
                      {.position         = {},
                       .string           = "DEMO VERSION",
                       .characterSize    = 48u,
                       .fillColor        = sf::Color::White,
                       .outlineColor     = colorBlueOutline,
                       .outlineThickness = 3.f}};

    ////////////////////////////////////////////////////////////
    // Spatial partitioning
    SweepAndPrune sweepAndPrune;

    ////////////////////////////////////////////////////////////
    // Particles
    sf::base::Vector<Particle>     particles;          // World space
    sf::base::Vector<TextParticle> textParticles;      // World space
    sf::base::Vector<Particle>     spentCoinParticles; // HUD space
    sf::base::Vector<Particle>     hudBottomParticles; // HUD space, drawn below ImGui
    sf::base::Vector<Particle>     hudTopParticles;    // HUD space, drawn on top of ImGui

    struct EarnedCoinParticle
    {
        sf::Vec2f startPosition;
        Timer     progress;
    };

    sf::base::Vector<EarnedCoinParticle> earnedCoinParticles; // HUD space

    ////////////////////////////////////////////////////////////
    // Random number generation
    RNG     rng{/* seed */ std::random_device{}()};
    RNGFast rngFast; // very fast, low-quality, but good enough for VFXs

    ////////////////////////////////////////////////////////////
    // Cat names
    sf::base::Vector<sf::base::Vector<sf::base::StringView>> shuffledCatNamesPerType = makeShuffledCatNames(rng);

    ////////////////////////////////////////////////////////////
    // Prestige transition
    bool inPrestigeTransition{false};

    ////////////////////////////////////////////////////////////
    // Timers for transitions
    TargetedCountdown bubbleSpawnTimer{.startingValue = 3.f};
    TargetedCountdown catRemoveTimer{.startingValue = 100.f};

    ////////////////////////////////////////////////////////////
    // Combo state
    int       combo{0u};
    Countdown comboCountdown;
    int       laserCursorCombo{0};

    ////////////////////////////////////////////////////////////
    // Accumulating combo effect and cursor combo text
    int       comboNStars{0};        // Number of stars clicked in current combo
    int       comboNOthers{0};       // Number of non-stars clicked in current combo
    int       comboAccReward{0};     // Accumulated combo reward effect for non-stars
    int       comboAccStarReward{0}; // Accumulated combo reward effect for stars
    Countdown comboFailCountdown;    // Countdown for combo failure effect (red text)

    Countdown accComboDelay;      // Combo reward coin spawns rate
    int       iComboAccReward{0}; // Index of spawned coin in combo reward (used for pitch)

    Countdown accComboStarDelay;      // Combo reward star spawns rate
    int       iComboAccStarReward{0}; // Index of spawned star in combo reward (used for pitch)

    sf::Text cursorComboText{fontMouldyCheese, {.origin = {0.f, 0.f}, .characterSize = 48u, .outlineThickness = 4.f}};

    ////////////////////////////////////////////////////////////
    // Clock and accumulator for played time
    sf::Clock     playedClock;
    sf::base::I64 playedUsAccumulator{0};
    sf::base::I64 autosaveUsAccumulator{0};
    sf::base::I64 fixedBgSlideAccumulator{0}; // for menu background slide

    ////////////////////////////////////////////////////////////
    // FPS and delta time clocks
    sf::Clock fpsClock;
    sf::Clock deltaClock;

    ////////////////////////////////////////////////////////////
    // Batches for drawing
    sf::CPUDrawableBatch bubbleDrawableBatch;
    sf::CPUDrawableBatch starBubbleDrawableBatch;
    sf::CPUDrawableBatch bombBubbleDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatch;
    sf::CPUDrawableBatch minimapDrawableBatch;
    sf::CPUDrawableBatch catTextDrawableBatch;
    sf::CPUDrawableBatch hudDrawableBatch;
    sf::CPUDrawableBatch hudTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch hudBottomDrawableBatch;  // drawn below ImGui
    sf::CPUDrawableBatch cpuTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch catTextTopDrawableBatch; // drawn on top of ImGui

    ////////////////////////////////////////////////////////////
    // Scrolling state
    sf::base::Optional<sf::Vec2f> dragPosition;
    float                         scroll{0.f};
    float                         actualScroll{0.f};

    ////////////////////////////////////////////////////////////
    // Screen shake effect state
    float screenShakeAmount{0.f};
    float screenShakeTimer{0.f};

    ////////////////////////////////////////////////////////////
    // Cached culling boundaries
    struct CullingBoundaries
    {
        float left;
        float right;
        float top;
        float bottom;

        ////////////////////////////////////////////////////////////
        [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool isInside(const sf::Vec2f point) const noexcept
        {
            return (point.x >= left) && (point.x <= right) && (point.y >= top) && (point.y <= bottom);
        }
    };

    CullingBoundaries hudCullingBoundaries{};
    CullingBoundaries particleCullingBoundaries{};
    CullingBoundaries bubbleCullingBoundaries{};

    ////////////////////////////////////////////////////////////
    // Last frame mouse position (world space)
    sf::Vec2f lastMousePos;

    ////////////////////////////////////////////////////////////
    // Cat dragging state
    float                         catDragPressDuration{0.f};
    sf::base::Optional<sf::Vec2f> catDragOrigin;
    sf::base::Vector<Cat*>        draggedCats;
    Cat*                          catToPlace{nullptr};

    ////////////////////////////////////////////////////////////
    // Touch state
    sf::base::Vector<sf::base::Optional<sf::Vec2f>> fingerPositions;

    ////////////////////////////////////////////////////////////
    // Splash screen state
    TargetedCountdown splashCountdown{.startingValue = 2500.f};

    ////////////////////////////////////////////////////////////
    // Tip state
    OptionalTargetedCountdown tipTCByte;
    OptionalTargetedCountdown tipTCBackground;
    OptionalTargetedCountdown tipTCBytePreEnd;
    OptionalTargetedCountdown tipTCByteEnd;
    OptionalTargetedCountdown tipTCBackgroundEnd;
    Countdown                 tipCountdownChar;
    std::string               tipString;
    TextEffectWiggle          tipStringWiggle{0.00175f, 4.f};
    sf::base::SizeT           tipCharIdx{0u};

    ////////////////////////////////////////////////////////////
    // Text buffers
    sf::Text textNameBuffer{fontSuperBakery,
                            {.characterSize    = 48u,
                             .fillColor        = sf::Color::White,
                             .outlineColor     = colorBlueOutline,
                             .outlineThickness = 3.f}};
    sf::Text textStatusBuffer{fontSuperBakery,
                              {.characterSize    = 32u,
                               .fillColor        = sf::Color::White,
                               .outlineColor     = colorBlueOutline,
                               .outlineThickness = 2.f}};
    sf::Text textMoneyBuffer{fontSuperBakery,
                             {.characterSize    = 24u,
                              .fillColor        = sf::Color::White,
                              .outlineColor     = colorBlueOutline,
                              .outlineThickness = 1.5f}};

    ////////////////////////////////////////////////////////////
    // Spent money count-down effect
    MoneyType spentMoney{0u};
    Timer     spentMoneyTimer{.value = 0.f};

    ////////////////////////////////////////////////////////////
    // Thread pool
    sf::base::ThreadPool threadPool{getTPWorkerCount()};

    ////////////////////////////////////////////////////////////
    // Cached views
    sf::View gameView;
    sf::View scaledTopGameView;
    sf::View gameBackgroundView;
    sf::View nonScaledHUDView;
    sf::View scaledHUDView;

    ////////////////////////////////////////////////////////////
    // Scroll arrow hint
    Countdown scrollArrowCountdown;

    ////////////////////////////////////////////////////////////
    // $ps sampler
    MoneyType     moneyGainedLastSecond{0u};
    Sampler       samplerMoneyPerSecond{/* capacity */ 60u};
    sf::base::I64 moneyGainedUsAccumulator{0};

    ////////////////////////////////////////////////////////////
    // Bomb-cat tracker for money earned
    ankerl::unordered_dense::map<sf::base::SizeT, sf::base::SizeT> bombIdxToCatIdx;

    ////////////////////////////////////////////////////////////
    // Notification queue
    struct NotificationData
    {
        const char* title;
        std::string content;
    };

    sf::base::Vector<NotificationData> notificationQueue;
    TargetedCountdown                  notificationCountdown{.startingValue = 750.f};

    ////////////////////////////////////////////////////////////
    // FPS counter
    float fps{0.f};

    ////////////////////////////////////////////////////////////
    // Purchase unlocked/available effects
    struct PurchaseUnlockedEffect
    {
        std::string widgetLabel;
        Countdown   countdown;
        Countdown   arrowCountdown;
        float       hue;
        int         type;
    };

    sf::base::Vector<PurchaseUnlockedEffect>        purchaseUnlockedEffects;
    ankerl::unordered_dense::map<std::string, bool> btnWasDisabled;

    ////////////////////////////////////////////////////////////
    // Debug stuff
    bool debugHideUI = false;

    ////////////////////////////////////////////////////////////
    // Portal storm buff countdown
    Countdown portalStormTimer;

#ifdef BUBBLEBYTE_USE_STEAMWORKS
    ////////////////////////////////////////////////////////////
    // Steam manager
    hg::Steam::SteamManager& steamMgr;
#endif

    bool onSteamDeck{false};

    ////////////////////////////////////////////////////////////
    // Background hue changing based on shrine
    sf::Angle currentBackgroundHue;
    sf::Angle targetBackgroundHue;
    sf::Color outlineHueColor{colorBlueOutline};

    ////////////////////////////////////////////////////////////
    // Cached unique cats
    Cat* cachedWitchCat{nullptr};
    Cat* cachedWizardCat{nullptr};
    Cat* cachedMouseCat{nullptr};
    Cat* cachedEngiCat{nullptr};
    Cat* cachedRepulsoCat{nullptr};
    Cat* cachedAttractoCat{nullptr};
    Cat* cachedCopyCat{nullptr};

    ////////////////////////////////////////////////////////////
    // Victory state
    OptionalTargetedCountdown victoryTC;
    Countdown                 cdLetterAppear;
    Countdown                 cdLetterText;

    ////////////////////////////////////////////////////////////
    // Minimap navigation
    sf::FloatRect minimapRect;

    ////////////////////////////////////////////////////////////
    // Input management
    InputHelper inputHelper;

    ////////////////////////////////////////////////////////////
    // Logging
    sf::OutFileStream logFile{"bubblebyte.log", sf::FileOpenMode::out | sf::FileOpenMode::app};

    ////////////////////////////////////////////////////////////
    // Achievement progress tracking
    struct AchievementProgress
    {
        sf::base::SizeT value;
        sf::base::SizeT threshold;
    };

    sf::base::Array<sf::base::Optional<AchievementProgress>, sf::base::getArraySize(achievementData)> achievementProgress{};

    ////////////////////////////////////////////////////////////
    // PP purchase undo
    sf::base::Vector<sf::base::FixedFunction<void(), 128>> undoPPPurchase;
    Countdown                                              undoPPPurchaseTimer;

    ////////////////////////////////////////////////////////////
    void saveMainPlaythroughToFile()
    {
        if constexpr (isDemoVersion)
        {
            if (ptMain.fullVersion)
            {
                sf::cOut() << "Cannot save non-demo playthrough in demo version!\n";
                return;
            }
        }

        ptMain.fullVersion = !isDemoVersion;
        savePlaythroughToFile(ptMain, "userdata/playthrough.json");
    }

    ////////////////////////////////////////////////////////////
    void log(const char* format, ...)
    {
        if (!logFile)
            return;

        const std::time_t currentTime = std::time(nullptr);
        const auto        localTime   = *std::localtime(&currentTime);

        char timeBuffer[100]; // Buffer for the timestamp string
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);

        char messageBuffer[1024];

        va_list args{};
        va_start(args, format);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
#pragma GCC diagnostic pop
        va_end(args);

        logFile << timeBuffer << " - " << messageBuffer << '\n';
        logFile.flush();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool inSpeedrunPlaythrough() const
    {
        return pt == &ptSpeedrun;
    }

    ////////////////////////////////////////////////////////////
    void addMoney(const MoneyType reward)
    {
        pt->money += reward;
        moneyGainedLastSecond += reward;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::base::Vector<sf::base::Vector<sf::base::StringView>> makeShuffledCatNames(RNG& rng)
    {
        sf::base::Vector<sf::base::Vector<sf::base::StringView>> result(nCatTypes);

        for (SizeT i = 0u; i < nCatTypes; ++i)
            result[i] = getShuffledCatNames(static_cast<CatType>(i), rng.getEngine());

        return result;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getTPWorkerCount()
    {
        const auto numThreads = static_cast<unsigned int>(sf::base::ThreadPool::getHardwareWorkerCount());
        return (numThreads == 0u) ? 3u : numThreads - 1u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SizeT getNextCatNameIdx(const CatType catType)
    {
        return pt->nextCatNamePerType[asIdx(catType)]++ % shuffledCatNamesPerType[asIdx(catType)].size();
    }

    ////////////////////////////////////////////////////////////
    Particle& implEmplaceParticle(const sf::Vec2f    position,
                                  const ParticleType particleType,
                                  const float        scaleMult,
                                  const float        speedMult,
                                  const float        opacity = 1.f)
    {
        return particles.emplaceBack(
            ParticleData{
                .position      = position,
                .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}) * speedMult,
                .scale         = rngFast.getF(0.08f, 0.27f) * scaleMult,
                .scaleDecay    = 0.f,
                .accelerationY = 0.002f,
                .opacity       = opacity,
                .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                .rotation      = rngFast.getF(0.f, sf::base::tau),
                .torque        = rngFast.getF(-0.002f, 0.002f),
            },
            sf::base::U8{0u},
            particleType);
    }

    ////////////////////////////////////////////////////////////
    bool spawnSpentCoinParticle(const ParticleData& particleData)
    {
        if (!profile.showParticles || !hudCullingBoundaries.isInside(particleData.position))
            return false;

        spentCoinParticles.emplaceBack(particleData, sf::base::U8{0u}, ParticleType::Coin);
        return true;
    }

    ////////////////////////////////////////////////////////////
    void spawnHUDTopParticle(const ParticleData& particleData, const float hue, const ParticleType particleType)
    {
        if (!profile.showParticles || !hudCullingBoundaries.isInside(particleData.position))
            return;

        hudTopParticles.emplaceBack(particleData, hueToByte(hue), particleType);
    }

    ////////////////////////////////////////////////////////////
    void spawnHUDBottomParticle(const ParticleData& particleData, const float hue, const ParticleType particleType)
    {
        if (!profile.showParticles || !hudCullingBoundaries.isInside(particleData.position))
            return;

        hudBottomParticles.emplaceBack(particleData, hueToByte(hue), particleType);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool spawnEarnedCoinParticle(const sf::Vec2f startPosition)
    {
        if (!profile.showParticles || !profile.showCoinParticles || !hudCullingBoundaries.isInside(startPosition))
            return false;

        earnedCoinParticles.emplaceBack(startPosition);
        return true;
    }

    ////////////////////////////////////////////////////////////
    void spawnParticle(const ParticleData& particleData, const float hue, const ParticleType particleType)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(particleData.position))
            return;

        // TODO P2: consider optimizing this pattern by just returning the emplaced particle and having the caller set the data
        particles.emplaceBack(particleData, hueToByte(hue), particleType);
    }

    ////////////////////////////////////////////////////////////
    void spawnParticles(const SizeT n, const sf::Vec2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
            implEmplaceParticle(position, args...);
    }

    ////////////////////////////////////////////////////////////
    void spawnParticlesWithHue(const float hue, const SizeT n, const sf::Vec2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
            implEmplaceParticle(position, args...).hueByte = hueToByte(hue);
    }

    ////////////////////////////////////////////////////////////
    void spawnParticlesNoGravity(const SizeT n, const sf::Vec2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
            implEmplaceParticle(position, args...).accelerationY = 0.f;
    }

    ////////////////////////////////////////////////////////////
    void spawnParticlesWithHueNoGravity(const float hue, const SizeT n, const sf::Vec2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
        {
            auto& p         = implEmplaceParticle(position, args...);
            p.accelerationY = 0.f;
            p.hueByte       = hueToByte(hue);
        }
    }

    ////////////////////////////////////////////////////////////
    void withAllStats(auto&& func)
    {
        func(profile.statsLifetime);
        func(pt->statsTotal);
        func(pt->statsSession);
    }

    ////////////////////////////////////////////////////////////
    void statBubblePopped(const BubbleType bubbleType, const bool byHand, const MoneyType reward)
    {
        withAllStats([&](Stats& stats)
        {
            stats.nBubblesPoppedByType[asIdx(bubbleType)] += 1u;
            stats.revenueByType[asIdx(bubbleType)] += reward;
        });

        if (byHand)
        {
            withAllStats([&](Stats& stats)
            {
                stats.nBubblesHandPoppedByType[asIdx(bubbleType)] += 1u;
                stats.revenueHandByType[asIdx(bubbleType)] += reward;
            });
        }
    }

    ////////////////////////////////////////////////////////////
    void statExplosionRevenue(const MoneyType reward)
    {
        withAllStats([&](Stats& stats) { stats.explosionRevenue += reward; });
    }

    ////////////////////////////////////////////////////////////
    void statFlightRevenue(const MoneyType reward)
    {
        withAllStats([&](Stats& stats) { stats.flightRevenue += reward; });
    }

    ////////////////////////////////////////////////////////////
    void statHellPortalRevenue(const MoneyType reward)
    {
        withAllStats([&](Stats& stats) { stats.hellPortalRevenue += reward; });
    }

    ////////////////////////////////////////////////////////////
    void statSecondsPlayed()
    {
        withAllStats([&](Stats& stats) { stats.secondsPlayed += 1u; });
    }

    ////////////////////////////////////////////////////////////
    void statHighestStarBubblePopCombo(const sf::base::U64 comboValue)
    {
        withAllStats([&](Stats& stats)
        { stats.highestStarBubblePopCombo = sf::base::max(stats.highestStarBubblePopCombo, comboValue); });
    }

    ////////////////////////////////////////////////////////////
    void statHighestNovaBubblePopCombo(const sf::base::U64 comboValue)
    {
        withAllStats([&](Stats& stats)
        { stats.highestNovaBubblePopCombo = sf::base::max(stats.highestNovaBubblePopCombo, comboValue); });
    }

    ////////////////////////////////////////////////////////////
    void statAbsorbedStarBubble()
    {
        withAllStats([&](Stats& stats) { stats.nAbsorbedStarBubbles += 1u; });
    }

    ////////////////////////////////////////////////////////////
    void statSpellCast(const SizeT spellIndex)
    {
        withAllStats([&](Stats& stats) { stats.nSpellCasts[spellIndex] += 1u; });
    }

    ////////////////////////////////////////////////////////////
    void statMaintenance(const SizeT nCatsHit)
    {
        withAllStats([&](Stats& stats) { stats.nMaintenances += nCatsHit; });
    }

    ////////////////////////////////////////////////////////////
    void statDisguise()
    {
        withAllStats([&](Stats& stats) { stats.nDisguises += 1u; });
    }

    ////////////////////////////////////////////////////////////
    void statDollCollected()
    {
        withAllStats([&](Stats& stats) { stats.nWitchcatDollsCollected += 1u; });
    }

    ////////////////////////////////////////////////////////////
    void statRitual(const CatType catType)
    {
        withAllStats([&](Stats& stats) { stats.nWitchcatRitualsPerCatType[asIdx(catType)] += 1u; });
    }

    ////////////////////////////////////////////////////////////
    void statHighestSimultaneousMaintenances(const sf::base::U64 value)
    {
        withAllStats([&](Stats& stats)
        { stats.highestSimultaneousMaintenances = sf::base::max(stats.highestSimultaneousMaintenances, value); });
    }

    ////////////////////////////////////////////////////////////
    void statHighestDPS(const sf::base::U64 value)
    {
        withAllStats([&](Stats& stats) { stats.highestDPS = sf::base::max(stats.highestDPS, value); });
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool keyDown(const sf::Keyboard::Key key) const
    {
        return inputHelper.isKeyDown(key);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool mBtnDown(const sf::Mouse::Button button, const bool penetrateUI) const
    {
        if (ImGui::GetIO().WantCaptureMouse && !penetrateUI)
            return false;

        return inputHelper.isMouseButtonDown(button);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgResourceToAtlas(const sf::Path& path)
    {
        return textureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgResourceToUIAtlas(const sf::Path& path)
    {
        return uiTextureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    void playSound(const Sounds::LoadedSound& ls, const sf::base::SizeT maxOverlap = 255u)
    {
#ifndef BUBBLEBYTE_NO_AUDIO
        sounds.playPooled(playbackDevice, ls, maxOverlap);
#else
        (void)ls;
        (void)maxOverlap;
#endif
    }

    ////////////////////////////////////////////////////////////
    void forEachBubbleInRadiusSquared(const sf::Vec2f center, const float radiusSq, auto&& func)
    {
        for (Bubble& bubble : pt->bubbles)
            if ((bubble.position - center).lengthSquared() <= radiusSq)
                if (func(bubble) == ControlFlow::Break)
                    break;
    }

    ////////////////////////////////////////////////////////////
    void forEachBubbleInRadius(const sf::Vec2f center, const float radius, auto&& func)
    {
        forEachBubbleInRadiusSquared(center, radius * radius, func);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Bubble* pickRandomBubbleInRadiusMatching(const sf::Vec2f center, const float radius, auto&& predicate)
    {
        const float radiusSq = radius * radius;

        SizeT   count    = 0u;
        Bubble* selected = nullptr;

        for (Bubble& bubble : pt->bubbles)
            if (predicate(bubble) && (bubble.position - center).lengthSquared() <= radiusSq)
            {
                ++count;

                // Select the current bubble with probability `1/count` (reservoir sampling)
                if (rng.getI<SizeT>(0, count - 1) == 0)
                    selected = &bubble;
            }

        return (count == 0u) ? nullptr : selected;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Bubble* pickRandomBubbleInRadius(const sf::Vec2f center, const float radius)
    {
        return pickRandomBubbleInRadiusMatching(center,
                                                radius,
                                                [](const Bubble&) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { return true; });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getResolution() const
    {
        return window.getSize().toVec2f();
    }

    ////////////////////////////////////////////////////////////
    Cat& spawnCat(const sf::Vec2f pos, const CatType catType, const float hue)
    {
        const auto meowPitch = [&]() -> float
        {
            switch (catType)
            {
                case CatType::Uni:
                    return rng.getF(1.2f, 1.3f);
                case CatType::Devil:
                    return rng.getF(0.7f, 0.8f);
                default:
                    return rng.getF(0.9f, 1.1f);
            }
        }();

        sounds.purrmeow.settings.position = {pos.x, pos.y};
        sounds.purrmeow.settings.pitch    = meowPitch;
        playSound(sounds.purrmeow);

        spawnParticles(32, pos, ParticleType::Star, 0.5f, 0.75f);

        catToPlace = nullptr;

        return pt->cats.emplaceBack(Cat{
            .position    = pos,
            .cooldown    = {.value = getComputedCooldownByCatTypeOrCopyCat(catType)},
            .pawPosition = pos,
            .hue         = hue,
            .nameIdx     = getNextCatNameIdx(catType),
            .type        = catType,
        });
    }

    ////////////////////////////////////////////////////////////
    Cat& spawnCatCentered(const CatType catType, const float hue, const bool placeInHand = true)
    {
        const auto pos = window.mapPixelToCoords((getResolution() / 2.f).toVec2i(), gameView);

        Cat& newCat = spawnCat(pos, catType, hue);

        if (placeInHand)
        {
            catToPlace = &newCat;

            draggedCats.clear();
            draggedCats.pushBack(&newCat);

            newCat.position    = lastMousePos;
            newCat.pawPosition = lastMousePos;
        }

        return newCat;
    }

    ////////////////////////////////////////////////////////////
    Cat& spawnSpecialCat(const sf::Vec2f pos, const CatType catType)
    {
        ++pt->psvPerCatType[static_cast<SizeT>(catType)].nPurchases;
        return spawnCat(pos, catType, /* hue */ 0.f);
    }

    ////////////////////////////////////////////////////////////
    void resetTipState()
    {
        tipTCByte.reset();
        tipTCBackground.reset();
        tipTCBytePreEnd.reset();
        tipTCByteEnd.reset();
        tipTCBackgroundEnd.reset();
        tipCountdownChar.value = 0.f;
        tipString              = "";
        tipCharIdx             = 0u;
    }

    ////////////////////////////////////////////////////////////
    void doTip(const std::string& str, const SizeT maxPrestigeLevel = 0u)
    {
        if (!profile.tipsEnabled || pt->psvBubbleValue.nPurchases > maxPrestigeLevel || inSpeedrunPlaythrough())
            return;

        playSound(sounds.byteMeow, /* maxOverlap */ 1u);

        resetTipState();

        tipTCByte.emplace(TargetedCountdown{.startingValue = 500.f});
        tipTCByte->restart();

        tipString = str + "\t\t\t\t\t";
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isUnicatTranscendenceActive() const
    {
        return pt->perm.unicatTranscendencePurchased && pt->perm.unicatTranscendenceEnabled;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isDevilcatHellsingedActive() const
    {
        return pt->perm.devilcatHellsingedPurchased && pt->perm.devilcatHellsingedEnabled;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getAspectRatioScalingFactor(const sf::Vec2f& originalSize, const sf::Vec2f& windowSize) const
    {
        // Calculate the scale factors for both dimensions
        const float scaleX = windowSize.x / originalSize.x;
        const float scaleY = windowSize.y / originalSize.y;

        // Use the smaller scale factor to maintain aspect ratio
        return std::min(scaleX, scaleY);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledGameView(const sf::Vec2f& originalSize, const sf::Vec2f& windowSize) const
    {
        const float     scale      = getAspectRatioScalingFactor(originalSize, windowSize);
        const sf::Vec2f scaledSize = originalSize * scale;

        return {.center   = originalSize / 2.f,
                .size     = originalSize,
                .viewport = {(windowSize - scaledSize).componentWiseDiv(windowSize * 2.f),
                             scaledSize.componentWiseDiv(windowSize)}};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledTopGameView(const sf::Vec2f& originalSize, const sf::Vec2f& windowSize) const
    {
        const float     scale      = getAspectRatioScalingFactor(originalSize, windowSize);
        const sf::Vec2f scaledSize = originalSize * scale;

        // Compute the full window width in world coordinates.
        float newWidth = windowSize.x / scale;

        sf::View view;
        // Use the new width while keeping the original height.
        view.size = {newWidth, originalSize.y};

        // Align the left edge with that of the normal game view.
        // The left edge is given by (baseCenter.x - originalSize.x / 2).
        sf::Vec2f baseCenter = getViewCenter();
        float     left       = baseCenter.x - originalSize.x / 2.f;
        view.center          = {left + newWidth / 2.f, baseCenter.y};

        // Use the same vertical letterboxing as the regular game view.
        view.viewport = {{0.f, (windowSize.y - scaledSize.y) / (windowSize.y * 2.f)}, {1.f, scaledSize.y / windowSize.y}};

        return view;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View makeScaledHUDView(const sf::Vec2f& resolution, float scale) const
    {
        return {.center = {resolution.x / (2.f * scale), resolution.y / (2.f * scale)}, .size = resolution / scale};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getHUDMousePos() const
    {
        return window.mapPixelToCoords(sf::Mouse::getPosition(window), nonScaledHUDView);
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT BufferIdx = 0u, typename T>
    static const char* toStringWithSeparators(const T value)
    {
        // Thread-local buffer to store the result
        // Size should be 27 (max 20 digits for 64-bit integer + up to 6 separators + null terminator)
        // Using 32 for addinional safety just in case
        static thread_local char strBuffer[32];

        // First, convert to string from right to left
        char* const end = strBuffer + sizeof(strBuffer) - 1;

        char* ptr = end;
        *ptr      = '\0';

        // Handle negative numbers
        const bool isNegative = value < 0;
        T          absValue   = isNegative ? -value : value;

        // Handle zero specially
        if (absValue == 0)
        {
            *--ptr = '0';
            return ptr;
        }

        // Convert digits and add separators
        int digitCount = 0;
        while (absValue > 0)
        {
            if (digitCount > 0 && digitCount % 3 == 0)
                *--ptr = '.';

            *--ptr = '0' + static_cast<char>(absValue % 10);
            absValue /= 10;

            ++digitCount;
        }

        if (isNegative)
            *--ptr = '-';

        return ptr;
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr float uiNormalFontScale    = 0.95f;
    static inline constexpr float uiSubBulletFontScale = 0.75f;
    static inline constexpr float uiToolTipFontScale   = 0.65f;
    static inline constexpr float uiWindowWidth        = 425.f;
    static inline constexpr float uiButtonWidth        = 150.f;
    static inline constexpr float uiTooltipWidth       = uiWindowWidth;

    ////////////////////////////////////////////////////////////
    char         uiBuffer[256]{};
    char         uiLabelBuffer[512]{};
    char         uiTooltipBuffer[1024]{};
    float        uiButtonHueMod = 0.f;
    unsigned int uiWidgetId     = 0u;
    float        lastFontScale  = 1.f;

    ////////////////////////////////////////////////////////////
    ankerl::unordered_dense::map<std::string, float> uiLabelToY;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float uiGetMaxWindowHeight() const;
    void                uiSetFontScale(float scale);
    void                uiMakeButtonLabels(const char* label, const char* xLabelBuffer);
    void                uiPushButtonColors();
    void                uiPopButtonColors();
    void                uiBeginTooltip(float width);
    void                uiEndTooltip();
    void                uiMakeTooltip(bool small = false);
    void                uiMakeShrineOrCatTooltip(sf::Vec2f mousePos);

    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] AnimatedButtonOutcome : sf::base::U8
    {
        None,
        Clicked,
        ClickedWhileDisabled,
    };

    ////////////////////////////////////////////////////////////
    [[nodiscard]] AnimatedButtonOutcome uiAnimatedButton(const char* label, const ImVec2& btnSize, float fontScale, float fontScaleMult);
    [[nodiscard]] bool uiMakeButtonImpl(const char* label, const char* xBuffer);
    [[nodiscard]] bool makePSVButtonEx(const char* label, PurchasableScalingValue& psv, SizeT times, MoneyType cost);
    bool               makePSVButton(const char* label, PurchasableScalingValue& psv);
    [[nodiscard]] bool uiCheckPurchasability(const char* label, bool disabled);
    [[nodiscard]] bool makePurchasableButtonOneTime(const char* label, MoneyType cost, bool& done);
    [[nodiscard]] bool makePurchasablePPButtonOneTime(const char* label, PrestigePointsType prestigePointsCost, bool& done);
    bool makePrestigePurchasablePPButtonPSV(const char* label, PurchasableScalingValue& psv);

    ////////////////////////////////////////////////////////////
    template <typename TCost>
    [[nodiscard]] bool makePSVButtonExByCurrency(
        const char*              label,
        PurchasableScalingValue& psv,
        SizeT                    times,
        TCost                    cost,
        TCost&                   availability,
        const char*              currencyFmt);

    ////////////////////////////////////////////////////////////
    template <typename TCost>
    [[nodiscard]] bool makePurchasableButtonOneTimeByCurrency(
        const char* label,
        bool&       done,
        TCost       cost,
        TCost&      availability,
        const char* currencyFmt);

    ////////////////////////////////////////////////////////////
    void switchToBGM(const sf::base::SizeT index, const bool force)
    {
#ifndef BUBBLEBYTE_NO_AUDIO
        if (!force && lastPlayedMusic == bgmPaths[index])
            return;

        lastPlayedMusic     = bgmPaths[index];
        bgmTransition.value = 1000.f;

        auto& optNextMusic = getNextBGMBuffer();
        optNextMusic.emplace(playbackDevice, sf::MusicReader::openFromFile(bgmPaths[index]).value());

        optNextMusic->music.setVolume(0.f);
        optNextMusic->music.setLooping(true);
        optNextMusic->music.setAttenuation(0.f);
        optNextMusic->music.setSpatializationEnabled(false);
        optNextMusic->music.play();
#else
        (void)index;
        (void)force;
#endif
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr const char* bgmPaths[] = {
        bgmPathNormal, // Normal
        bgmPathWitch,  // Voodoo
        bgmPathWizard, // Magic
        bgmPathMouse,  // Clicking
        bgmPathEngi,   // Automation
        bgmPathNormal, // Repulsion
        bgmPathNormal, // Attraction
        bgmPathNormal, // Camouflage
        bgmPathNormal, // Victory
    };

    ////////////////////////////////////////////////////////////
    sf::base::Optional<BGMBuffer>& getCurrentBGMBuffer()
    {
        return bgmBuffers[currentBGMBufferIdx % 2u];
    }

    ////////////////////////////////////////////////////////////
    sf::base::Optional<BGMBuffer>& getNextBGMBuffer()
    {
        return bgmBuffers[(currentBGMBufferIdx + 1u) % 2u];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getHueByCatType(const CatType catType)
    {
        if (catType == CatType::Normal)
            return rng.getF(-20.f, 20.f);

        if (catType == CatType::Uni)
            return rng.getF(0.f, 360.f);

        if (catType == CatType::Devil)
            return rng.getF(-25.f, 25.f);

        if (catType == CatType::Astro)
            return rng.getF(-20.f, 20.f);

        return 0;
    }

    ////////////////////////////////////////////////////////////
    void beginPrestigeTransition(const PrestigePointsType ppReward)
    {
        playSound(sounds.prestige);

        buyReminder = 0;

        inPrestigeTransition = true;
        scroll               = 0.f;

        resetAllDraggedCats();
        pt->onPrestige(ppReward);

        profile.selectedBackground = 0;
        profile.selectedBGM        = 0;

        updateSelectedBackgroundSelectorIndex();
        updateSelectedBGMSelectorIndex();

        switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ true);
    }

    ////////////////////////////////////////////////////////////
    ImGuiTabItemFlags_ shopSelectOnce       = ImGuiTabItemFlags_SetSelected;
    int                lastUiSelectedTabIdx = 1;

    ////////////////////////////////////////////////////////////
    void uiBeginColumns() const;
    void uiCenteredText(const char* str, float offsetX = 0.f, float offsetY = 0.f);
    void uiCenteredTextColored(sf::Color color, const char* str, float offsetX = 0.f, float offsetY = 0.f);
    [[nodiscard]] sf::Vec2f uiGetWindowPos() const;
    void                    uiDrawExitPopup(float newScalingFactor);
    void                    uiDrawQuickbarCopyCat(sf::Vec2f quickBarPos, Cat& copyCat);
    void                    uiDrawQuickbarBackgroundSelector(sf::Vec2f quickBarPos);
    void                    uiDrawQuickbarBGMSelector(sf::Vec2f quickBarPos);
    void                    uiDrawQuickbarQuickSettings(sf::Vec2f quickBarPos);
    void                    uiDrawQuickbarVolumeControls(sf::Vec2f quickBarPos);
    void                    uiDrawQuickbar();
    void                    uiDraw(sf::Vec2f mousePos);
    void                    uiDpsMeter();
    void                    uiSpeedrunning();
    void                    uiTabBar();
    void                    uiSetUnlockLabelY(sf::base::SizeT unlockId);
    [[nodiscard]] bool      checkUiUnlock(sf::base::SizeT unlockId, bool unlockCondition);
    void uiImageFromAtlas(const sf::FloatRect& txr, const sf::RenderTarget::TextureDrawParams& drawParams);
    void uiImgsep(const sf::FloatRect& txr, const char* sepLabel, bool first = false);
    void uiImgsep2(const sf::FloatRect& txr, const char* sepLabel);
    void uiTabBarShop();
    bool uiCheckbox(const char* label, bool* b);
    bool uiRadio(const char* label, int* i, int value);
    void uiTabBarPrestige();
    void uiTabBarMagic();
    void uiTabBarStats();
    void uiTabBarSettings();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<sf::FloatRect> getAoEDragRect(const sf::Vec2f mousePos) const
    {
        if (!catDragOrigin.hasValue())
            return sf::base::nullOpt;

        return sf::base::makeOptional<sf::FloatRect>(*catDragOrigin, mousePos - *catDragOrigin);
    }

    ////////////////////////////////////////////////////////////
    void resetAllDraggedCats()
    {
        catDragPressDuration = 0.f;
        catDragOrigin.reset();
        draggedCats.clear();
        catToPlace = nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT pickDragPivotCatIndex() const
    {
        SFML_BASE_ASSERT(!draggedCats.empty());

        if (draggedCats.size() <= 2u)
            return 0u;

        // First calculate the centroid
        sf::Vec2f centroid;

        for (const Cat* cat : draggedCats)
            centroid += cat->position;

        centroid /= static_cast<float>(draggedCats.size());

        // Find the position closest to the centroid
        sf::base::SizeT closestIndex       = 0u;
        float           minDistanceSquared = SFML_BASE_FLOAT_MAX;

        for (sf::base::SizeT i = 0u; i < draggedCats.size(); ++i)
        {
            // Calculate squared distance (avoiding square root for performance)
            const float distSquared = (draggedCats[i]->position - centroid).lengthSquared();

            if (minDistanceSquared - distSquared < 64.f)
            {
                minDistanceSquared = distSquared;
                closestIndex       = i;
            }
        }

        return closestIndex;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isCatBeingDragged(const Cat& cat) const
    {
        for (const Cat* c : draggedCats)
            if (c == &cat)
                return true;

        return false;
    }

    ////////////////////////////////////////////////////////////
    void stopDraggingCat(const Cat& cat)
    {
        sf::base::vectorEraseIf(draggedCats, [&](const Cat* c) { return c == &cat; });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isWizardBusy() const
    {
        const Cat* wizardCat = cachedWizardCat;

        if (wizardCat == nullptr)
            return false;

        return pt->absorbingWisdom || wizardCat->cooldown.value != 0.f || wizardCat->isHexedOrCopyHexed() ||
               isCatBeingDragged(*wizardCat);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* findFirstCatByType(const CatType catType) const
    {
        for (Cat& cat : pt->cats)
            if (cat.type == catType)
                return &cat;

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    void addCombo(int& xCombo, Countdown& xComboCountdown) const
    {
        if (xCombo == 0)
        {
            xCombo                = 1;
            xComboCountdown.value = pt->psvComboStartTime.currentValue() * 1000.f;
        }
        else
        {
            xCombo += 1;
            xComboCountdown.value += 150.f - sf::base::clamp(static_cast<float>(xCombo) * 10.f, 0.f, 100.f);
        }
    }

    ////////////////////////////////////////////////////////////
    static bool checkComboEnd(const float deltaTimeMs, int& xCombo, Countdown& xComboCountdown)
    {
        if (xComboCountdown.updateAndStop(deltaTimeMs) != CountdownStatusStop::JustFinished)
            return false;

        xCombo = 0;
        return true;
    }

    ////////////////////////////////////////////////////////////
    void doWizardSpellStarpawConversion(Cat& wizardCat)
    {
        const auto range       = getComputedRangeByCatTypeOrCopyCat(wizardCat.type);
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(wizardCat.type);

        sounds.cast0.settings.position = {wizardCat.position.x, wizardCat.position.y};
        playSound(sounds.cast0);

        spawnParticlesNoGravity(256, wizardCat.position, ParticleType::Star, rngFast.getF(0.25f, 1.25f), rngFast.getF(0.5f, 3.f));

        forEachBubbleInRadius(wizardCat.position,
                              range,
                              [&](Bubble& bubble)
        {
            if (pt->perm.starpawConversionIgnoreBombs && bubble.type != BubbleType::Normal)
                return ControlFlow::Continue;

            if (rng.getF(0.f, 99.f) > pt->psvStarpawPercentage.currentValue())
                return ControlFlow::Continue;

            bubble.type   = pt->perm.starpawNova ? BubbleType::Nova : BubbleType::Star;
            bubble.hueMod = rng.getF(0.f, 360.f);
            bubble.velocity.y -= rng.getF(0.025f, 0.05f);

            spawnParticles(1, bubble.position, ParticleType::Star, 0.5f, 0.35f);

            return ControlFlow::Continue;
        });

        ++wizardCat.hits;
        wizardCat.cooldown.value = maxCooldown * 2.f;
    }

    ////////////////////////////////////////////////////////////
    void doWizardSpellMewltiplierAura(Cat& wizardCat)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(wizardCat.type);

        sounds.cast0.settings.position = {wizardCat.position.x, wizardCat.position.y};
        playSound(sounds.cast0);

        spawnParticlesNoGravity(256, wizardCat.position, ParticleType::Star, rngFast.getF(0.25f, 1.25f), rngFast.getF(0.5f, 3.f));

        ++wizardCat.hits;
        wizardCat.cooldown.value = maxCooldown * 2.f;
    }

    ////////////////////////////////////////////////////////////
    void doWizardSpellDarkUnion(Cat& wizardCat)
    {
        const auto range       = getComputedRangeByCatTypeOrCopyCat(wizardCat.type);
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(wizardCat.type);

        Cat* witchCat = cachedWitchCat;

        const bool castSuccessful = pt->dolls.empty() && witchCat != nullptr &&
                                    (witchCat->position - wizardCat.position).lengthSquared() <= range * range;

        if (castSuccessful)
        {
            sounds.cast0.settings.position = {wizardCat.position.x, wizardCat.position.y};
            playSound(sounds.cast0);

            spawnParticlesNoGravity(256,
                                    wizardCat.position,
                                    ParticleType::Star,
                                    rngFast.getF(0.25f, 1.25f),
                                    rngFast.getF(0.5f, 3.f));

            spawnParticlesNoGravity(256,
                                    witchCat->position,
                                    ParticleType::Star,
                                    rngFast.getF(0.25f, 1.25f),
                                    rngFast.getF(0.5f, 3.f));

            witchCat->cooldown.value -= witchCat->cooldown.value * (pt->psvDarkUnionPercentage.currentValue() / 100.f);
        }
        else
        {
            sounds.failcast.settings.position = {wizardCat.position.x, wizardCat.position.y};
            playSound(sounds.failcast);
        }

        ++wizardCat.hits;
        wizardCat.cooldown.value = maxCooldown * 4.f;
    }

    ////////////////////////////////////////////////////////////
    void doWizardSpellStasisField(Cat& wizardCat)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(wizardCat.type);

        sounds.cast0.settings.position = {wizardCat.position.x, wizardCat.position.y};
        playSound(sounds.cast0);

        spawnParticlesNoGravity(256, wizardCat.position, ParticleType::Star, rngFast.getF(0.25f, 1.25f), rngFast.getF(0.5f, 3.f));

        ++wizardCat.hits;
        wizardCat.cooldown.value = maxCooldown * 2.f;
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr ManaType spellManaCostByIndex[4] = {5u, 20u, 30u, 40u};

    ////////////////////////////////////////////////////////////
    void castSpellByIndex(const sf::base::SizeT index, Cat* wizardCat, Cat* copyCat)
    {
        SFML_BASE_ASSERT(index < 4u);

        const bool copyCatMustCast = copyCat != nullptr && pt->copycatCopiedCatType == CatType::Wizard;

        wizardcatSpin.value = sf::base::tau;
        statSpellCast(index);

        if (index == 0u) // Starpaw Conversion
        {
            doWizardSpellStarpawConversion(*wizardCat);

            if (copyCatMustCast)
                doWizardSpellStarpawConversion(*copyCat);

            return;
        }

        if (index == 1u) // Mewltiplier Aura
        {
            pt->mewltiplierAuraTimer += pt->perm.wizardCatDoubleMewltiplierDuration ? 12'000.f : 6000.f;

            doWizardSpellMewltiplierAura(*wizardCat);

            if (copyCatMustCast)
                doWizardSpellMewltiplierAura(*copyCat);

            return;
        }

        if (index == 2u) // Dark Union
        {
            doWizardSpellDarkUnion(*wizardCat);

            if (copyCatMustCast)
                doWizardSpellDarkUnion(*copyCat);

            return;
        }

        if (index == 3u) // Stasis Field
        {
            pt->stasisFieldTimer += pt->perm.wizardCatDoubleStasisFieldDuration ? 12'000.f : 6000.f;

            doWizardSpellStasisField(*wizardCat);

            if (copyCatMustCast)
                doWizardSpellStasisField(*copyCat);

            return;
        }
    }

    ////////////////////////////////////////////////////////////
    static constexpr auto formatTime(const sf::base::U64 seconds)
    {
        struct Result
        {
            sf::base::U64 h;
            sf::base::U64 m;
            sf::base::U64 s;
        };

        return Result{seconds / 3600u, (seconds / 60u) % 60u, seconds % 60u};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr auto formatSpeedrunTime(const sf::Time time)
    {
        const sf::base::I64 elapsedTime       = time.asMicroseconds();
        const sf::base::U64 totalMicroseconds = (elapsedTime >= 0) ? static_cast<sf::base::U64>(elapsedTime) : 0ULL;

        constexpr sf::base::U64 usPerMs   = 1000ULL;
        constexpr sf::base::U64 usPerSec  = 1000ULL * usPerMs; // 1,000,000
        constexpr sf::base::U64 usPerMin  = 60ULL * usPerSec;  // 60,000,000
        constexpr sf::base::U64 usPerHour = 60ULL * usPerMin;  // 3,600,000,000

        struct Result
        {
            sf::base::U64 hours;
            sf::base::U64 mins;
            sf::base::U64 secs;
            sf::base::U64 millis;
        };

        return Result{totalMicroseconds / usPerHour,
                      (totalMicroseconds % usPerHour) / usPerMin,
                      (totalMicroseconds % usPerMin) / usPerSec,
                      (totalMicroseconds % usPerSec) / usPerMs};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool mustApplyMewltiplierAura(const sf::Vec2f bubblePosition) const
    {
        if (pt->mewltiplierAuraTimer <= 0.f)
            return false;

        const Cat* wizardCat = cachedWizardCat;
        if (wizardCat == nullptr)
            return false;

        const float wizardCatRangeSquared = pt->getComputedSquaredRangeByCatType(CatType::Wizard);

        if ((wizardCat->position - bubblePosition).lengthSquared() <= wizardCatRangeSquared)
            return true;

        const Cat* copyCat = cachedCopyCat;
        if (copyCat == nullptr || pt->copycatCopiedCatType != CatType::Wizard)
            return false;

        if ((copyCat->position - bubblePosition).lengthSquared() <= wizardCatRangeSquared)
            return true;

        return false;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] MoneyType computeFinalReward(const Bubble& bubble,
                                               const float   multiplier,
                                               const float   comboMult,
                                               const Cat*    popperCat) const
    {
        // Determine some information about the reward
        const bool byPlayerClick = popperCat == nullptr;

        const bool popperCatIsMousecat = //
            !byPlayerClick && (popperCat->type == CatType::Mouse ||
                               (popperCat->type == CatType::Copy && pt->copycatCopiedCatType == CatType::Mouse));

        const bool popperCatIsNormal = !byPlayerClick && popperCat->type == CatType::Normal;

        const bool mustApplyHandMult = byPlayerClick || popperCatIsMousecat; // mousecat benefits from click and cat mults
        const bool mustApplyCatMult = !byPlayerClick;

        const bool nearShrineOfClicking = byPlayerClick && ([&]
        {
            for (const Shrine& shrine : pt->shrines)
                if (shrine.type == ShrineType::Clicking && shrine.isInRange(bubble.position))
                    return true;

            return false;
        })();

        // Base reward: bubble value by type multiplied by static multiplier (e.g. x10 for bombs, x20 for astro)
        float result = static_cast<float>(pt->getComputedRewardByBubbleType(bubble.type)) * multiplier;

        // Combo mult: applied for player clicks or mousecat clicks
        result *= comboMult;

        // Wizard spells: mewltiplier aura
        if (mustApplyMewltiplierAura(bubble.position))
            result *= pt->psvMewltiplierMult.currentValue();

        // Global bonus -- mousecat (applies to clicks)
        const bool isMouseBeingCopied = cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Mouse;
        if (mustApplyHandMult && cachedMouseCat != nullptr)
            result *= pt->psvPPMouseCatGlobalBonusMult.currentValue() * (isMouseBeingCopied ? 2.f : 1.f);

        // Global bonus -- engicat (applies to cats)
        const bool isEngiBeingCopied = cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Engi;
        if (mustApplyCatMult && cachedEngiCat != nullptr)
            result *= pt->psvPPEngiCatGlobalBonusMult.currentValue() * (isEngiBeingCopied ? 2.f : 1.f);

        // Shrine of clicking: x5 reward for clicks
        if (mustApplyHandMult && nearShrineOfClicking)
            result *= 5.f;

        // Ritual buff -- normalcat: x5 reward for cats
        if (mustApplyCatMult && pt->buffCountdownsPerType[asIdx(CatType::Normal)].value > 0.f)
            result *= 5.f;

        // Ritual buff -- mousecat: x10 reward for clicks
        if (mustApplyHandMult && pt->buffCountdownsPerType[asIdx(CatType::Mouse)].value > 0.f)
            result *= 10.f;

        // Genius cats: x2 reward for normal cats only
        if (!byPlayerClick && popperCatIsNormal && pt->perm.geniusCatsPurchased)
            result *= 2.f;

        // Repulsocat: x2 reward for repelled bubbles
        if (!bubble.repelledCountdown.isDone())
            result *= 2.f;

        // Attractocat: x2 reward for attracted bubbles
        if (!bubble.attractedCountdown.isDone())
            result *= 2.f;

        return static_cast<MoneyType>(sf::base::ceil(result));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::Vec2u getReasonableWindowSize(const float scalingFactorMult = 1.f)
    {
        constexpr float gameRatio = gameScreenSize.x / gameScreenSize.y;

        const auto fullscreenSize = sf::VideoModeUtils::getDesktopMode().size.toVec2f();

        const float aspectRatio = fullscreenSize.x / fullscreenSize.y;

        const bool isUltrawide = aspectRatio >= 2.f;
        const bool isWide      = aspectRatio >= 1.6f && aspectRatio < 2.f;

        const float scalingFactor = isUltrawide ? 0.9f : isWide ? 0.8f : 0.7f;

        const auto windowSize = fullscreenSize * scalingFactor * scalingFactorMult;

        const auto windowedWidth = windowSize.y * gameRatio + (uiWindowWidth + 35.f);

        return sf::Vec2f{windowedWidth, windowSize.y}.toVec2u();
    }

    ////////////////////////////////////////////////////////////
    struct SelectorEntry
    {
        int         index;
        const char* name;
    };

    ////////////////////////////////////////////////////////////
    [[nodiscard]] int pickSelectedIndex(const sf::base::Vector<SelectorEntry>& entries, const int selectedIndex)
    {
        const auto selectedIndexU = static_cast<sf::base::SizeT>(selectedIndex);
        return selectedIndexU < entries.size() ? entries[selectedIndexU].index : 0;
    }

    ////////////////////////////////////////////////////////////
    void selectBackground(const sf::base::Vector<SelectorEntry>& entries, const int selectedIndex)
    {
        profile.selectedBackground = pickSelectedIndex(entries, selectedIndex);
    }

    ////////////////////////////////////////////////////////////
    void selectBGM(const sf::base::Vector<SelectorEntry>& entries, const int selectedIndex)
    {
        profile.selectedBGM = pickSelectedIndex(entries, selectedIndex);
    }

    ////////////////////////////////////////////////////////////
    void updateSelectedBackgroundSelectorIndex() const
    {
        auto& [entries, selectedIndex] = getBackgroundSelectorData();

        for (sf::base::SizeT i = 0u; i < entries.size(); ++i)
            if (profile.selectedBackground == entries[i].index)
                selectedIndex = static_cast<int>(i);
    }

    ////////////////////////////////////////////////////////////
    void updateSelectedBGMSelectorIndex() const
    {
        auto& [entries, selectedIndex] = getBGMSelectorData();

        for (sf::base::SizeT i = 0u; i < entries.size(); ++i)
            if (profile.selectedBGM == entries[i].index)
                selectedIndex = static_cast<int>(i);
    }

    ////////////////////////////////////////////////////////////
    struct SelectorData
    {
        sf::base::Vector<SelectorEntry> entries;
        int                             selectedIndex = -1;
    };

    ////////////////////////////////////////////////////////////
    SelectorData& getBGMSelectorData() const
    {
        static thread_local SelectorData data;
        data.entries.clear();

        data.entries.emplaceBack(0, "Default");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
            data.entries.emplaceBack(1, "Ritual Circle");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
            data.entries.emplaceBack(2, "The Wise One");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
            data.entries.emplaceBack(3, "Click N Chill");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
            data.entries.emplaceBack(4, "Use More Cat");

        if (data.selectedIndex == -1)
        {
            data.selectedIndex = [&]
            {
                for (sf::base::SizeT i = 0u; i < data.entries.size(); ++i)
                    if (profile.selectedBGM == data.entries[i].index)
                        return static_cast<int>(i);

                return 0;
            }();
        }

        return data;
    }

    ////////////////////////////////////////////////////////////
    SelectorData& getBackgroundSelectorData() const
    {
        static thread_local SelectorData data;
        data.entries.clear();

        data.entries.emplaceBack(0, "Default");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
            data.entries.emplaceBack(1, "Swamp");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
            data.entries.emplaceBack(2, "Observatory");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
            data.entries.emplaceBack(3, "Aim Labs");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
            data.entries.emplaceBack(4, "Factory");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
            data.entries.emplaceBack(5, "Wind Tunnel");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
            data.entries.emplaceBack(6, "Magnetosphere");

        if (pt->perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
            data.entries.emplaceBack(7, "Auditorium");

        if (data.selectedIndex == -1)
        {
            data.selectedIndex = [&]
            {
                for (sf::base::SizeT i = 0u; i < data.entries.size(); ++i)
                    if (profile.selectedBackground == data.entries[i].index)
                        return static_cast<int>(i);

                return 0;
            }();
        }

        return data;
    }

    ////////////////////////////////////////////////////////////
    void forceResetGame(const bool goToShopTab = true)
    {
        sounds.stopPlayingAll(sounds.ritual);
        sounds.stopPlayingAll(sounds.copyritual);

        delayedActions.clear();

        rng.reseed(std::random_device{}());
        shuffledCatNamesPerType = makeShuffledCatNames(rng);

        *pt      = Playthrough{};
        pt->seed = rng.getSeed();

        wasPrestigeAvailableLastFrame = false;
        buyReminder                   = 0u;

        resetAllDraggedCats();
        resetTipState();

        particles.clear();
        textParticles.clear();
        spentCoinParticles.clear();
        hudBottomParticles.clear();
        hudTopParticles.clear();
        earnedCoinParticles.clear();

        inPrestigeTransition = false;

        combo            = 0u;
        laserCursorCombo = 0;

        comboNStars         = 0;
        comboNOthers        = 0;
        comboAccReward      = 0;
        comboAccStarReward  = 0;
        iComboAccReward     = 0;
        iComboAccStarReward = 0;

        scroll = 0.f;

        screenShakeAmount = 0.f;
        screenShakeTimer  = 0.f;

        spentMoney            = 0u;
        moneyGainedLastSecond = 0u;
        samplerMoneyPerSecond.clear();

        bombIdxToCatIdx.clear();
        purchaseUnlockedEffects.clear();
        btnWasDisabled.clear();
        undoPPPurchase.clear();

        if (goToShopTab)
            shopSelectOnce = ImGuiTabItemFlags_SetSelected;

        profile.selectedBackground = 0;
        profile.selectedBGM        = 0;

        updateSelectedBackgroundSelectorIndex();
        updateSelectedBGMSelectorIndex();

        switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ true);
    }

    ////////////////////////////////////////////////////////////
    void forceResetProfile()
    {
        profile = Profile{};
        forceResetGame();
    }

    ////////////////////////////////////////////////////////////
    TextParticle& makeRewardTextParticle(const sf::Vec2f position)
    {
        return textParticles.emplaceBack(TextParticle{
            {.position      = {position.x, position.y - 10.f},
             .velocity      = rngFast.getVec2f({-0.1f, -1.65f}, {0.1f, -1.35f}) * 0.395f,
             .scale         = sf::base::clamp(1.f + 0.1f * static_cast<float>(combo + 1) / 1.75f, 1.f, 3.f) * 0.5f,
             .scaleDecay    = 0.f,
             .accelerationY = 0.0039f,
             .opacity       = 1.f,
             .opacityDecay  = 0.0015f,
             .rotation      = 0.f,
             .torque        = rngFast.getF(-0.002f, 0.002f)}});
    }

    ////////////////////////////////////////////////////////////
    void shrineCollectReward(Shrine& shrine, const MoneyType reward, const Bubble& bubble)
    {
        shrine.collectedReward += reward;
        shrine.textStatusShakeEffect.bump(rngFast, 1.5f);

        spawnParticlesWithHue(wrapHue(shrine.getHue() + 40.f),
                              6,
                              shrine.getDrawPosition(),
                              ParticleType::Fire,
                              rngFast.getF(0.25f, 0.6f),
                              0.75f);

        spawnParticlesWithHue(shrine.getHue(), 6, shrine.getDrawPosition(), ParticleType::Shrine, rngFast.getF(0.6f, 1.f), 0.5f);

        const auto diff = bubble.position - shrine.position;

        spawnParticle({.position      = bubble.position,
                       .velocity      = -diff.normalized() * 0.5f,
                       .scale         = 1.5f,
                       .scaleDecay    = 0.f,
                       .accelerationY = 0.f,
                       .opacity       = 1.f,
                       .opacityDecay  = 0.00135f + (shrine.getRange() - diff.length()) / 22000.f,
                       .rotation      = 0.f,
                       .torque        = 0.f},
                      /* hue */ 0.f,
                      ParticleType::Bubble);
    }

    ////////////////////////////////////////////////////////////
    void doExplosion(Bubble& bubble)
    {
        sounds.explosion.settings.position = {bubble.position.x, bubble.position.y};
        playSound(sounds.explosion);

        spawnParticles(16, bubble.position, ParticleType::Fire, 3.f, 1.f);

        for (sf::base::SizeT iP = 0u; iP < 16u; ++iP)
            spawnParticle(ParticleData{.position      = bubble.position,
                                       .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}) * 0.55f,
                                       .scale         = rngFast.getF(0.08f, 0.27f) * 3.75f,
                                       .scaleDecay    = -0.0005f,
                                       .accelerationY = 0.00015f,
                                       .opacity       = 0.75f,
                                       .opacityDecay  = rngFast.getF(0.001f, 0.002f),
                                       .rotation      = rngFast.getF(0.f, sf::base::tau),
                                       .torque        = rngFast.getF(-0.001f, 0.001f)},
                          0.f,
                          ParticleType::Explosion);

        for (sf::base::SizeT iP = 0u; iP < 8u; ++iP)
            spawnParticle(ParticleData{.position      = bubble.position,
                                       .velocity      = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(-0.15f, 0.05f)},
                                       .scale         = rngFast.getF(0.65f, 1.f) * 1.25f,
                                       .scaleDecay    = -0.0005f,
                                       .accelerationY = -0.00017f,
                                       .opacity       = rngFast.getF(0.5f, 0.75f),
                                       .opacityDecay  = rngFast.getF(0.00035f, 0.00055f),
                                       .rotation      = rngFast.getF(0.f, sf::base::tau),
                                       .torque        = rngFast.getF(-0.002f, 0.002f)},
                          0.f,
                          ParticleType::Smoke);

        // TODO P2: cleanup
        const auto  bubbleIdx  = static_cast<sf::base::SizeT>(&bubble - pt->bubbles.data());
        const auto* bombIdxItr = bombIdxToCatIdx.find(bubbleIdx);

        Cat* catWhoMadeBomb = bombIdxItr != bombIdxToCatIdx.end() ? pt->cats.data() + bombIdxItr->second : nullptr;

        const float explosionRadius = pt->getComputedBombExplosionRadius();

        forEachBubbleInRadius(bubble.position,
                              explosionRadius,
                              [&](Bubble& otherBubble)
        {
            if (otherBubble.type == BubbleType::Bomb)
                return ControlFlow::Continue;

            const MoneyType otherReward = computeFinalReward(/* bubble     */ otherBubble,
                                                             /* multiplier */ 10.f,
                                                             /* comboMult  */ 1.f,
                                                             /* popperCat  */ catWhoMadeBomb);

            statExplosionRevenue(otherReward);

            popWithRewardAndReplaceBubble({
                .reward          = otherReward,
                .bubble          = otherBubble,
                .xCombo          = 1,
                .popSoundOverlap = false,
                .popperCat       = catWhoMadeBomb,
                .multiPop        = false,
            });

            return ControlFlow::Continue;
        });

        if (pt->perm.witchCatBuffFlammableDolls)
        {
            for (Doll& doll : pt->dolls)
                if ((doll.position - bubble.position).length() <= explosionRadius && !doll.tcDeath.hasValue())
                    collectDoll(doll);

            for (Doll& copyDoll : pt->copyDolls)
                if ((copyDoll.position - bubble.position).length() <= explosionRadius && !copyDoll.tcDeath.hasValue())
                    collectCopyDoll(copyDoll);
        }

        if (catWhoMadeBomb != nullptr)
            bombIdxToCatIdx.erase(bombIdxItr);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f fromWorldToHud(const sf::Vec2f point) const
    {
        // From game coordinates to screen coordinates
        const sf::Vec2i screenPos = window.mapCoordsToPixel(point, gameView);

        // From screen coordinates to HUD view coordinates
        return window.mapPixelToCoords(screenPos, scaledHUDView);
    }

    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] BubblePopData
    {
        MoneyType reward;
        Bubble&   bubble;
        int       xCombo;
        bool      popSoundOverlap;
        Cat*      popperCat;
        bool      multiPop;
    };

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isBubbleInStasisField(const Bubble& bubble) const
    {
        if (pt->stasisFieldTimer <= 0.f)
            return false;

        const auto rangeSquared = pt->getComputedSquaredRangeByCatType(CatType::Wizard);

        if (cachedWizardCat != nullptr)
            if ((bubble.position - cachedWizardCat->position).lengthSquared() <= rangeSquared)
                return true;

        if (cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Wizard)
            if ((bubble.position - cachedCopyCat->position).lengthSquared() <= rangeSquared)
                return true;

        return false;
    }

    ////////////////////////////////////////////////////////////
    void popWithRewardAndReplaceBubble(const BubblePopData& data)
    {
        const auto& [reward, bubble, xCombo, popSoundOverlap, popperCat, multiPop] = data;

        const bool byPlayerClick = popperCat == nullptr;

        statBubblePopped(bubble.type, byPlayerClick, reward);

        if (byPlayerClick && bubble.type == BubbleType::Star)
            statHighestStarBubblePopCombo(static_cast<sf::base::U64>(combo));

        if (byPlayerClick && bubble.type == BubbleType::Nova)
            statHighestNovaBubblePopCombo(static_cast<sf::base::U64>(combo));

        Shrine* collectorShrine = nullptr;
        for (Shrine& shrine : pt->shrines)
        {
            if ((bubble.position - shrine.position).lengthSquared() > shrine.getRangeSquared())
                continue;

            collectorShrine = &shrine;
            shrineCollectReward(shrine, reward, bubble);
        }

        const bool      collectedByShrine = collectorShrine != nullptr;
        const sf::Vec2f tpPosition        = collectedByShrine ? collectorShrine->getDrawPosition() : bubble.position;

        if (profile.showTextParticles)
        {
            auto& tp = makeRewardTextParticle(tpPosition);
            std::snprintf(tp.buffer, sizeof(tp.buffer), "+$%llu", reward);
        }

        if (profile.accumulatingCombo && !multiPop && byPlayerClick && pt->comboPurchased && !collectedByShrine)
        {
            if (bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova)
                comboNStars += 1;
            else
                comboNOthers += 1;
        }

        if (profile.showCoinParticles)
        {
            if (!collectedByShrine && profile.showCoinParticles)
                spawnSpentCoinParticle(
                    {.position      = moneyText.getCenterRight() + sf::Vec2f{32.f, rngFast.getF(-12.f, 12.f)},
                     .velocity      = {-0.25f, 0.f},
                     .scale         = 0.25f,
                     .scaleDecay    = 0.f,
                     .accelerationY = 0.f,
                     .opacity       = 0.f,
                     .opacityDecay  = -0.003f,
                     .rotation      = rngFast.getF(0.f, sf::base::tau),
                     .torque        = 0.f});


            const sf::Vec2f hudPos = fromWorldToHud(bubble.position);

            if ((!profile.accumulatingCombo || !pt->comboPurchased || !byPlayerClick) && !collectedByShrine &&
                spawnEarnedCoinParticle(hudPos))
            {
                sounds.coindelay.settings.position = {getViewCenter().x - gameScreenSize.x / 2.f + 25.f,
                                                      getViewCenter().y - gameScreenSize.y / 2.f + 25.f};

                sounds.coindelay.settings.pitch  = 1.f;
                sounds.coindelay.settings.volume = profile.sfxVolume / 100.f * 0.5f;

                playSound(sounds.coindelay, /* maxOverlap */ 64);
            }
        }

        sounds.pop.settings.position = {bubble.position.x, bubble.position.y};
        sounds.pop.settings.pitch    = remap(static_cast<float>(xCombo), 1, 10, 1.f, 2.f);

        playSound(sounds.pop, popSoundOverlap ? 255u : 1u);

        spawnParticles(32, bubble.position, ParticleType::Bubble, 0.5f, 0.5f);
        spawnParticles(8, bubble.position, ParticleType::Bubble, 1.2f, 0.25f);

        if (bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova)
            spawnParticles(16, bubble.position, ParticleType::Star, 0.5f, 0.35f);
        else if (bubble.type == BubbleType::Bomb)
            doExplosion(bubble);

        if (popperCat != nullptr)
        {
            popperCat->moneyEarned += reward;
            popperCat->textMoneyShakeEffect.bump(rngFast, 1.25f);
        }

        if (!collectedByShrine)
        {
            addMoney(reward);
            moneyTextShakeEffect.bump(rngFast, 1.f + static_cast<float>(combo) * 0.1f);
        }

        if (!isBubbleInStasisField(bubble))
        {
            bubble = makeRandomBubble(*pt, rng, pt->getMapLimit(), 0.f);
            bubble.position.y -= bubble.radius;
        }
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDebugModeEnabled() const
    {
        return debugMode;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopCheats() const
    {
        if (!isDebugModeEnabled())
            return;

        if (keyDown(sf::Keyboard::Key::F4))
        {
            pt->comboPurchased = true;
            pt->mapPurchased   = true;
        }
        else if (keyDown(sf::Keyboard::Key::F5))
        {
            pt->money = 1'000'000'000u;
        }
        else if (keyDown(sf::Keyboard::Key::F6))
        {
            pt->money += 15u;
        }
        else if (keyDown(sf::Keyboard::Key::F7))
        {
            pt->prestigePoints += 15u;
        }
    }

    ////////////////////////////////////////////////////////////
    void turnBubbleNormal(Bubble& bubble)
    {
        if (bubble.type == BubbleType::Bomb)
            bombIdxToCatIdx.erase(static_cast<sf::base::SizeT>(&bubble - pt->bubbles.data()));

        bubble.type     = BubbleType::Normal;
        bubble.rotation = 0.f;
        bubble.hueMod   = 0.f;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Mouse::Button getLMB() const
    {
        return profile.invertMouseButtons ? sf::Mouse::Button::Right : sf::Mouse::Button::Left;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Mouse::Button getRMB() const
    {
        return profile.invertMouseButtons ? sf::Mouse::Button::Left : sf::Mouse::Button::Right;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateScrolling(const float deltaTimeMs, const sf::base::Vector<sf::Vec2f>& downFingers)
    {
        // Reset map scrolling
        if (keyDown(sf::Keyboard::Key::LShift) || (downFingers.size() != 2u && !mBtnDown(getRMB(), /* penetrateUI */ true)))
            dragPosition.reset();

        //
        // Scrolling
        scroll = sf::base::clamp(scroll,
                                 0.f,
                                 sf::base::min(pt->getMapLimit() / 2.f - gameScreenSize.x / 2.f,
                                               (boundaries.x - gameScreenSize.x) / 2.f));

        actualScroll = exponentialApproach(actualScroll, scroll, deltaTimeMs, 75.f);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateTransitions(const float deltaTimeMs)
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
                    auto& bubble = pt->bubbles.emplaceBack(makeRandomBubble(*pt, rng, pt->getMapLimit(), boundaries.y));
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
                const auto rightmostIt = std::max_element(pt->cats.begin(),
                                                          pt->cats.end(),
                                                          [](const Cat& a, const Cat& b)
                { return a.position.x < b.position.x; });

                const float targetScroll = (rightmostIt->position.x - gameScreenSize.x / 2.f) / 2.f;
                scroll                   = exponentialApproach(scroll, targetScroll, deltaTimeMs, 15.f);

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

            if (!pt->dolls.empty())
            {
                const auto cPos = pt->dolls.back().position;
                pt->dolls.popBack();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }

            if (!pt->copyDolls.empty())
            {
                const auto cPos = pt->copyDolls.back().position;
                pt->copyDolls.popBack();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }

            if (!pt->hellPortals.empty())
            {
                const auto cPos = pt->hellPortals.back().position;
                pt->hellPortals.popBack();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }
        }

        const bool gameElementsRemoved = pt->cats.empty() && pt->shrines.empty() && pt->dolls.empty() &&
                                         pt->copyDolls.empty() && pt->hellPortals.empty();

        // Reset map extension and scroll, and remove bubbles outside of view
        if (gameElementsRemoved)
        {
            pt->mapPurchased               = false;
            pt->psvMapExtension.nPurchases = 0u;

            scroll = 0.f;

            sf::base::vectorEraseIf(pt->bubbles,
                                    [&](const Bubble& b) { return b.position.x > pt->getMapLimit() + 128.f; });
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
    void gameLoopUpdateBubbles(const float deltaTimeMs)
    {
        constexpr float maxVelocityMagnitude  = 2.f;
        constexpr float windMult[4]           = {0.f, 0.00009f, 0.00018f, 0.0005f};
        constexpr float windStartVelocityY[4] = {0.07f, 0.18f, 0.25f, 0.55f};

        for (Bubble& bubble : pt->bubbles)
        {
            if (bubble.velocity.lengthSquared() > maxVelocityMagnitude * maxVelocityMagnitude)
                bubble.velocity = bubble.velocity.normalized() * maxVelocityMagnitude;

            if (bubble.type == BubbleType::Bomb)
                bubble.rotation += deltaTimeMs * 0.01f;

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
                    turnBubbleNormal(bubble);
                else if (willBeBomb && willBeStar)
                    bubble.type = rng.getF(0.f, 1.f) > 0.5f ? starType : BubbleType::Bomb;
                else if (willBeBomb)
                    bubble.type = BubbleType::Bomb;
                else if (willBeStar)
                    bubble.type = starType;
            }
            else if (bubble.position.y + bubble.radius < -128.f)
            {
                turnBubbleNormal(bubble);
            }

            bubble.velocity.y += 0.00005f * deltaTimeMs;

            (void)bubble.repelledCountdown.updateAndStop(deltaTimeMs);
            (void)bubble.attractedCountdown.updateAndStop(deltaTimeMs);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAttractoBuff(const float deltaTimeMs) const
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
    [[nodiscard]] bool gameLoopUpdateBubbleClick(sf::base::Optional<sf::Vec2f>& clickPosition)
    {
        if (!clickPosition.hasValue())
            return false;

        const auto clickPos = window.mapPixelToCoords(clickPosition->toVec2i(), gameView);

        if (!particleCullingBoundaries.isInside(clickPos))
        {
            clickPosition.reset();
            return false;
        }

        bool anyBubblePoppedByClicking = false;

        forEachBubbleInRadius(clickPos,
                              128.f,
                              [&](Bubble& bubble)
        {
            if ((clickPos - bubble.position).lengthSquared() > bubble.getRadiusSquared())
                return ControlFlow::Continue;

            // Prevent clicks around shrine of automation
            for (Shrine& shrine : pt->shrines)
                if (shrine.type == ShrineType::Automation && shrine.isInRange(clickPos))
                {
                    sounds.failpop.settings.position = {clickPos.x, clickPos.y};
                    playSound(sounds.failpop);

                    return ControlFlow::Break;
                }

            anyBubblePoppedByClicking = true;

            if (pt->comboPurchased)
            {
                if (!pt->laserPopEnabled)
                {
                    addCombo(combo, comboCountdown);
                    comboTextShakeEffect.bump(rngFast, 1.f + static_cast<float>(combo) * 0.2f);
                }
                else
                {
                    ++laserCursorCombo;

                    if (combo == 0 || laserCursorCombo >= 10)
                    {
                        addCombo(combo, comboCountdown);
                        comboTextShakeEffect.bump(rngFast, 0.01f + static_cast<float>(combo) * 0.002f);

                        comboCountdown.value = sf::base::min(comboCountdown.value,
                                                             pt->psvComboStartTime.currentValue() * 100.f);

                        combo = sf::base::min(combo, 998);
                    }
                }
            }
            else
            {
                combo = 1;
            }

            const MoneyType
                reward = computeFinalReward(/* bubble     */ bubble,
                                            /* multiplier */ 1.f,
                                            /* comboMult  */ getComboValueMult(combo, pt->laserPopEnabled ? playerComboDecayLaser : playerComboDecay),
                                            /* popperCat  */ nullptr);

            popWithRewardAndReplaceBubble({
                .reward          = reward,
                .bubble          = bubble,
                .xCombo          = combo,
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
                    if (&otherBubble == &bubble)
                        return ControlFlow::Continue;

                    popWithRewardAndReplaceBubble({
                        .reward          = reward,
                        .bubble          = otherBubble,
                        .xCombo          = combo,
                        .popSoundOverlap = false,
                        .popperCat       = nullptr,
                        .multiPop        = true,
                    });

                    return ControlFlow::Continue;
                });


            return ControlFlow::Break;
        });

        return anyBubblePoppedByClicking;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::Vec2f getCatRangeCenter(const Cat& cat)
    {
        return cat.position + CatConstants::rangeOffsets[asIdx(cat.type)];
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionNormal(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto maxCooldown = pt->getComputedCooldownByCatType(cat.type);
        const auto range       = pt->getComputedRangeByCatType(cat.type);
        const auto [cx, cy]    = getCatRangeCenter(cat);

        const auto normalCatPopBubble = [&](Bubble& bubble)
        {
            cat.pawPosition = bubble.position;
            cat.pawOpacity  = 255.f;
            cat.pawRotation = (bubble.position - cat.position).angle() + sf::degrees(45);

            const float squaredMouseCatRange = pt->getComputedSquaredRangeByCatType(CatType::Mouse);

            const bool inMouseCatRange = cachedMouseCat != nullptr &&
                                         (cachedMouseCat->position - cat.position).lengthSquared() <= squaredMouseCatRange;

            const bool inCopyMouseCatRange = cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Mouse &&
                                             (cachedCopyCat->position - cat.position).lengthSquared() <= squaredMouseCatRange;

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
            return pickRandomBubbleInRadiusMatching({cx, cy},
                                                    range,
                                                    [&](const Bubble& b) { return ((b.type == types) || ...); });
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
    void gameLoopUpdateCatActionUni(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto starBubbleType = isUnicatTranscendenceActive() ? BubbleType::Nova : BubbleType::Star;
        const auto nStarParticles = pt->perm.unicatTranscendenceAOEPurchased ? 1u : 4u;

        const auto transformBubble = [&](Bubble& bToTransform)
        {
            bToTransform.type       = starBubbleType;
            bToTransform.hueMod     = rng.getF(0.f, 360.f);
            bToTransform.velocity.y = rng.getF(-0.1f, -0.05f);

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
            Bubble* b = pickRandomBubbleInRadiusMatching(getCatRangeCenter(cat),
                                                         range,
                                                         [&](const Bubble& bubble)
            { return bubble.type == BubbleType::Normal; });

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
    void gameLoopUpdateCatActionDevil(const float /* deltaTimeMs */, Cat& cat)
    {
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

            bubble.type = BubbleType::Bomb;

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
    void gameLoopUpdateCatActionAstro(const float /* deltaTimeMs */, Cat& cat)
    {
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
    [[nodiscard]] Cat* getHexedCat() const
    {
        for (Cat& cat : pt->cats)
            if (cat.hexedTimer.hasValue())
                return &cat;

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getCopyHexedCat() const
    {
        for (Cat& cat : pt->cats)
            if (cat.hexedCopyTimer.hasValue())
                return &cat;

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyCatHexedOrCopyHexed() const
    {
        return sf::base::anyOf(pt->cats.begin(), pt->cats.end(), [](const Cat& cat) { return cat.isHexedOrCopyHexed(); });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyCatHexed() const
    {
        return sf::base::anyOf(pt->cats.begin(), pt->cats.end(), [](const Cat& cat) { return cat.hexedTimer.hasValue(); });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyCatCopyHexed() const
    {
        return sf::base::anyOf(pt->cats.begin(),
                               pt->cats.end(),
                               [](const Cat& cat) { return cat.hexedCopyTimer.hasValue(); });
    }

    ////////////////////////////////////////////////////////////
    void hexCat(Cat& cat, const bool copy)
    {
        if (isCatBeingDragged(cat))
            stopDraggingCat(cat);

        sounds.soulsteal.settings.position = {cat.position.x, cat.position.y};
        playSound(sounds.soulsteal);

        screenShakeAmount = 3.5f;
        screenShakeTimer  = 600.f;

        (copy ? cat.hexedTimer : cat.hexedCopyTimer).reset();
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

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedCooldownByCatTypeOrCopyCat(const CatType catType) const
    {
        return pt->getComputedCooldownByCatType(catType == CatType::Copy ? pt->copycatCopiedCatType : catType);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedRangeByCatTypeOrCopyCat(const CatType catType) const
    {
        return pt->getComputedRangeByCatType(catType == CatType::Copy ? pt->copycatCopiedCatType : catType);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionWitchImpl(const float /* deltaTimeMs */, Cat& cat, sf::base::Vector<Doll>& dollsToUse)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
        const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

        SizeT otherCatCount = 0u;
        Cat*  selected      = nullptr;

        for (Cat& otherCat : pt->cats)
        {
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

            ++otherCatCount;

            // Select the current cat with probability `1/count` (reservoir sampling)
            if (rng.getI<SizeT>(0, otherCatCount - 1) == 0)
                selected = &otherCat;
        }

        if (otherCatCount > 0)
        {
            SFML_BASE_ASSERT(selected != nullptr);

            hexCat(*selected, /* copy */ &dollsToUse == &pt->copyDolls);

            float buffPower = pt->psvPPWitchCatBuffDuration.currentValue();

            if (pt->perm.witchCatBuffPowerScalesWithNCats)
                buffPower += sf::base::ceil(sf::base::pow(static_cast<float>(otherCatCount), 0.9f)) * 0.5f;

            if (pt->perm.witchCatBuffPowerScalesWithMapSize)
            {
                const float nMapExtensions = (pt->mapPurchased ? 1.f : 0.f) +
                                             static_cast<float>(pt->psvMapExtension.nPurchases);

                buffPower += static_cast<float>(nMapExtensions) * 0.75f;
            }

            const auto nDollsToSpawn = sf::base::max(SizeT{2u},
                                                     static_cast<SizeT>(
                                                         buffPower * (pt->perm.witchCatBuffFewerDolls ? 1.f : 2.f) / 4.f));

            SFML_BASE_ASSERT(dollsToUse.empty());

            statRitual(selected->type);

            const auto isPositionFarFromOtherDolls = [&](const sf::Vec2f& position) -> bool
            {
                for (const Doll& d : dollsToUse)
                    if ((d.position - position).lengthSquared() < (256.f * 256.f))
                        return false;

                return true;
            };

            const auto isOnTopOfAnyCat = [&](const sf::Vec2f& position) -> bool
            {
                for (const Cat& c : pt->cats)
                    if ((c.position - position).lengthSquared() < c.getRadiusSquared())
                        return true;

                return false;
            };

            const auto isOnTopOfAnyShrine = [&](const sf::Vec2f& position) -> bool
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
                        isPositionFarFromOtherDolls(candidate) && !isOnTopOfAnyCat(candidate) &&
                        !isOnTopOfAnyShrine(candidate))
                        return candidate;

                return rndDollPosition();
            };

            for (SizeT i = 0u; i < nDollsToSpawn; ++i)
            {
                auto& d = dollsToUse.emplaceBack(
                    Doll{.position      = pickDollPosition(),
                         .wobbleRadians = rng.getF(0.f, sf::base::tau),
                         .buffPower     = buffPower,
                         .catType       = selected->type == CatType::Copy ? pt->copycatCopiedCatType : selected->type,
                         .tcActivation  = {.startingValue = rng.getF(300.f, 600.f) * static_cast<float>(i + 1)},
                         .tcDeath       = {}});

                d.tcActivation.restart();
            }

            const bool copy = &dollsToUse == &pt->copyDolls;
            spawnParticlesWithHue(copy ? 180.f : 0.f, 128, selected->position, ParticleType::Hex, 0.5f, 0.35f);

            cat.textStatusShakeEffect.bump(rngFast, 1.5f);
            cat.hits += 1u;

            if (!pt->dollTipShown)
            {
                pt->dollTipShown = true;
                doTip("Click on all the dolls to\nreceive a powerful timed buff!\nYou might need to scroll...");
            }
        }
        else
        {
            wastedEffort = true;
        }

        cat.cooldown.value = maxCooldown;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionWitch(const float deltaTimeMs, Cat& cat)
    {
        SFML_BASE_ASSERT(!anyCatHexed());
        gameLoopUpdateCatActionWitchImpl(deltaTimeMs, cat, pt->dolls);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionWizard(const float deltaTimeMs, Cat& cat)
    {
        if (!pt->absorbingWisdom)
            return;

        const auto maxCooldown  = getComputedCooldownByCatTypeOrCopyCat(cat.type);
        const auto range        = getComputedRangeByCatTypeOrCopyCat(cat.type);
        const auto [cx, cy]     = getCatRangeCenter(cat);
        const auto drawPosition = cat.getDrawPosition(profile.enableCatBobbing);

        Bubble* starBubble = nullptr;

        const auto findRotatedStarBubble = [&](Bubble& bubble)
        {
            if ((bubble.type != BubbleType::Star && bubble.type != BubbleType::Nova) || bubble.rotation == 0.f)
                return ControlFlow::Continue;

            starBubble = &bubble;
            return ControlFlow::Break;
        };

        forEachBubbleInRadius({cx, cy}, range, findRotatedStarBubble);

        if (starBubble == nullptr)
            starBubble = pickRandomBubbleInRadiusMatching({cx, cy},
                                                          range,
                                                          [&](Bubble& bubble)
            { return bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova; });

        if (starBubble == nullptr)
            return;

        Bubble& bubble = *starBubble;

        cat.pawPosition = bubble.position;
        cat.pawOpacity  = 255.f;
        cat.pawRotation = (bubble.position - cat.position).angle() + sf::degrees(45);

        bubble.rotation += deltaTimeMs * 0.025f;
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
            turnBubbleNormal(bubble);

            cat.cooldown.value = maxCooldown;

            statAbsorbedStarBubble();
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionMouse(const float /* deltaTimeMs */, Cat& cat)
    {
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
    void gameLoopUpdateCatActionEngi(const float /* deltaTimeMs */, Cat& cat)
    {
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
    void gameLoopUpdateCatActionRepulso(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
        const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

        if (pt->repulsoCatConverterEnabled && !pt->repulsoCatIgnoreBubbles.normal)
        {
            Bubble* b = pickRandomBubbleInRadiusMatching(cat.position,
                                                         range,
                                                         [&](Bubble& bubble)
            { return bubble.type != BubbleType::Star && bubble.type != BubbleType::Nova; });

            if (b != nullptr && rng.getF(0.f, 100.f) < pt->psvPPRepulsoCatConverterChance.currentValue())
            {
                b->type   = pt->perm.repulsoCatNovaConverterPurchased ? BubbleType::Nova : BubbleType::Star;
                b->hueMod = rng.getF(0.f, 360.f);
                spawnParticles(2, b->position, ParticleType::Star, 0.5f, 0.35f);

                cat.textStatusShakeEffect.bump(rngFast, 1.5f);
                ++cat.hits;
            }
        }

        cat.cooldown.value = maxCooldown;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionAttracto(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
        const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

        // TODO P1: ? maybe absorb all bubbles in range and give a reward based on the number of bubbles absorbed

        cat.cooldown.value = maxCooldown;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionCopy(const float deltaTimeMs, Cat& cat)
    {
        if (pt->copycatCopiedCatType == CatType::Witch)
        {
            SFML_BASE_ASSERT(!anyCatCopyHexed());
            gameLoopUpdateCatActionWitchImpl(deltaTimeMs, cat, pt->copyDolls);
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
    void gameLoopUpdateCatActionDuck(const float deltaTimeMs, Cat& cat)
    {
        (void)deltaTimeMs;
        (void)cat;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getWindRepulsionMult() const
    {
        constexpr float mults[4] = {1.f, 5.f, 10.f, 15.f};
        return mults[pt->windStrength];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getWindAttractionMult() const
    {
        constexpr float mults[4] = {1.f, 1.5f, 3.f, 4.5f};
        return mults[pt->windStrength];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] auto makeMagnetAction(
        const sf::Vec2f    position,
        const CatType      catType,
        const float        deltaTimeMs,
        auto               countdownPm,
        const float        countdownTime,
        const float        strengthMult,
        const float        direction,
        BubbleIgnoreFlags& ignoreFlags)
    {
        return [this, &ignoreFlags, deltaTimeMs, position, catType, countdownPm, countdownTime, strengthMult, direction](
                   Bubble& bubble)
        {
            if (ignoreFlags.normal && bubble.type == BubbleType::Normal)
                return ControlFlow::Continue;

            if (ignoreFlags.star && (bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova))
                return ControlFlow::Continue;

            if (ignoreFlags.bomb && bubble.type == BubbleType::Bomb)
                return ControlFlow::Continue;

            const auto bcDiff   = (position - bubble.position);
            const auto strength = (getComputedRangeByCatTypeOrCopyCat(catType) - bcDiff.length()) * 0.000017f;
            bubble.velocity += (bcDiff.normalized() * strength * strengthMult) * direction * deltaTimeMs;

            (bubble.*countdownPm).value = sf::base::max((bubble.*countdownPm).value, countdownTime);
            return ControlFlow::Continue;
        };
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActions(const float deltaTimeMs)
    {
        (void)wizardcatSpin.updateAndStop(deltaTimeMs * 0.015f);

        for (Cat& cat : pt->cats)
        {
            // Keep cat in boundaries
            const float catRadius = cat.getRadius();

            // Keep cats away from shrine of clicking
            // Buff cats in shrine of automation
            for (Shrine& shrine : pt->shrines)
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
                    if (shrine.isInRange(cat.position) && cachedWitchCat == nullptr && !anyCatHexedOrCopyHexed() &&
                        !cat.isHexedOrCopyHexed())
                    {
                        hexCat(cat, /* copy */ false);
                    }
                }
            }

            const bool allowOOBCat = &cat == catToPlace || cat.astroState.hasValue() ||
                                     (draggedCats.size() > 1u && isCatBeingDragged(cat));

            if (!allowOOBCat)
                cat.position = cat.position.componentWiseClamp({catRadius, catRadius},
                                                               {pt->getMapLimit() - catRadius, boundaries.y - catRadius});

            const auto maxCooldown  = pt->getComputedCooldownByCatType(cat.type);
            const auto range        = pt->getComputedRangeByCatType(cat.type);
            const auto rangeSquared = range * range;

            const auto drawPosition = cat.getDrawPosition(profile.enableCatBobbing);

            auto diff = cat.pawPosition - drawPosition - sf::Vec2f{-25.f, 25.f};
            cat.pawPosition -= diff * 0.01f * deltaTimeMs;
            cat.pawRotation = cat.pawRotation.rotatedTowards(sf::degrees(-45.f), deltaTimeMs * 0.005f);

            if (isCatBeingDragged(cat) && (cat.pawPosition - drawPosition).length() > 16.f)
                cat.pawPosition = drawPosition + (cat.pawPosition - drawPosition).normalized() * 16.f;

            if (cat.cooldown.value == 0.f && cat.pawOpacity > 10.f)
                cat.pawOpacity -= 0.5f * deltaTimeMs;

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
                    sounds.stopPlayingAll(soundRitual);

                    if (sounds.countPlayingPooled(soundRitualEnd) == 0u)
                    {
                        soundRitualEnd.settings.position = {cat.position.x, cat.position.y};

                        if (profile.playWitchRitualSounds)
                            playSound(soundRitualEnd);
                    }
                }

                if (cat.cooldown.value < 10'000.f)
                {
                    if (cat.cooldown.value > 100.f && sounds.countPlayingPooled(soundRitual) == 0u)
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
                            spawnParticle({.position = otherCat.getDrawPosition(profile.enableCatBobbing) +
                                                       sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
                                           .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                                           .scale         = rngFast.getF(0.08f, 0.27f) * 0.5f,
                                           .scaleDecay    = 0.f,
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
                    sounds.stopPlayingAll(sounds.ritual);
                }
            };

            if (cat.type == CatType::Witch && !anyCatHexed())
                doWitchBehavior(/* hueMod */ 0.f, sounds.ritual, sounds.ritualend);

            if (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && !anyCatCopyHexed())
                doWitchBehavior(/* hueMod */ 180.f, sounds.copyritual, sounds.copyritualend);

            if (cat.hexedTimer.hasValue() || (cat.type == CatType::Witch && (anyCatHexed() || !pt->dolls.empty())))
            {
                spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
                               .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.5f,
                               .scaleDecay    = 0.f,
                               .accelerationY = -0.0017f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00035f, 0.0025f),
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ wrapHue(rngFast.getF(-50.f, 50.f)),
                              ParticleType::Hex);

                continue;
            }

            if (cat.hexedCopyTimer.hasValue() || (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch &&
                                                  (anyCatCopyHexed() || !pt->copyDolls.empty())))
            {
                spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
                               .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.5f,
                               .scaleDecay    = 0.f,
                               .accelerationY = -0.0017f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00035f, 0.0025f),
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
                spawnParticle({.position   = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius},
                               .velocity   = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale      = rngFast.getF(0.08f, 0.27f) * 0.2f,
                               .scaleDecay = 0.f,
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
                               .velocity      = rngFast.getVec2f({-0.025f, -0.015f}, {0.025f, 0.015f}),
                               .scale         = rngFast.getF(0.08f, 0.27f) * 0.15f,
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
                    spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius + 15.f, +catRadius - 5.f),
                                                                        catRadius - 20.f},
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

                    spawnParticles(1, drawPosition + sf::Vec2f{56.f, 45.f}, ParticleType::Fire, 1.5f, 0.25f, 0.65f);

                    if (rngFast.getI(0, 10) > 5)
                        spawnParticle(ParticleData{.position   = drawPosition + sf::Vec2f{56.f, 45.f},
                                                   .velocity   = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(0.f, 0.1f)},
                                                   .scale      = rngFast.getF(0.75f, 1.f) * 0.45f,
                                                   .scaleDecay = -0.00025f,
                                                   .accelerationY = -0.00017f,
                                                   .opacity       = 0.7f,
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
                    spawnParticle({.position = drawPosition + sf::Vec2f{rngFast.getF(-catRadius, +catRadius), catRadius},
                                   .velocity      = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                                   .scale         = rngFast.getF(0.08f, 0.27f) * 0.2f,
                                   .scaleDecay    = 0.f,
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
                        spawnParticle({.position = otherCat.getDrawPosition(profile.enableCatBobbing) +
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

            if (cat.type == CatType::Attracto ||
                (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Attracto))
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
    [[nodiscard]] bool isCatDraggable(const Cat& cat) const
    {
        if (cat.isAstroAndInFlight())
            return false;

        if (cat.isHexedOrCopyHexed())
            return false;

        if (cat.type == CatType::Witch && anyCatHexed())
            return false;

        if (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && anyCatCopyHexed())
            return false;

        if (cat.type == CatType::Witch && cat.cooldown.value <= 10'000.f)
            return false;

        if (cat.type == CatType::Copy && pt->copycatCopiedCatType == CatType::Witch && cat.cooldown.value <= 10'000.f)
            return false;

        return true;
    }

    [[nodiscard]] bool isAOESelecting() const
    {
        return (keyDown(sf::Keyboard::Key::LShift) || keyDown(sf::Keyboard::Key::LControl)) &&
               mBtnDown(getLMB(), /* penetrateUI */ true);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatDragging(const float deltaTimeMs, const SizeT countFingersDown, const sf::Vec2f mousePos)
    {
        if (inPrestigeTransition)
        {
            resetAllDraggedCats();
            return;
        }

        // Automatically scroll when dragging cats near the edge of the screen
        if ((!draggedCats.empty() || isAOESelecting()) && catToPlace == nullptr)
        {
            constexpr float offset = 48.f;

            if (mousePos.x < particleCullingBoundaries.left + offset * 1.f)
                scroll -= 8.f;
            else if (mousePos.x < particleCullingBoundaries.left + offset * 2.f)
                scroll -= 4.f;
            else if (mousePos.x < particleCullingBoundaries.left + offset * 3.f)
                scroll -= 2.f;

            if (mousePos.x > particleCullingBoundaries.right - offset * 1.f)
                scroll += 8.f;
            else if (mousePos.x > particleCullingBoundaries.right - offset * 2.f)
                scroll += 4.f;
            else if (mousePos.x > particleCullingBoundaries.right - offset * 3.f)
                scroll += 2.f;
        }

        if (isAOESelecting())
        {
            if (!catDragOrigin.hasValue())
                catDragOrigin.emplace(mousePos);
        }
        else if (catDragOrigin.hasValue())
        {
            const auto dragRect = getAoEDragRect(mousePos).value();
            catDragOrigin.reset();
            draggedCats.clear();

            for (Cat& cat : pt->cats)
            {
                if (!isCatDraggable(cat))
                    continue;

                if (!dragRect.contains(cat.position))
                    continue;

                draggedCats.pushBack(&cat);
            }

            playSound(sounds.grab);
        }
        else
        {
            const bool shouldDropCats = [&]
            {
                if (catToPlace != nullptr)
                    return bubbleCullingBoundaries.isInside(catToPlace->position) &&
                           inputHelper.wasMouseButtonJustPressed(getLMB());

                const bool noMouseButtonNorFinger = !mBtnDown(getLMB(), /* penetrateUI */ true) && countFingersDown != 1;

                if (draggedCats.size() <= 1u)
                    return noMouseButtonNorFinger;

                if (draggedCats.size() > 1u)
                    return noMouseButtonNorFinger && !keyDown(sf::Keyboard::Key::LShift) &&
                           !keyDown(sf::Keyboard::Key::LControl);

                return false;
            }();

            if (shouldDropCats)
            {
                if (!draggedCats.empty())
                    playSound(sounds.drop);

                resetAllDraggedCats();
                return;
            }

            if (!draggedCats.empty())
            {
                const auto pivotCatIdx = pickDragPivotCatIndex();
                Cat&       pivotCat    = *draggedCats[pivotCatIdx];

                static thread_local sf::base::Vector<sf::Vec2f> relativeCatPositions;
                relativeCatPositions.clear();
                relativeCatPositions.reserve(draggedCats.size());

                for (const Cat* cat : draggedCats)
                    relativeCatPositions.pushBack(cat->position - pivotCat.position);

                pivotCat.position = exponentialApproach(pivotCat.position, mousePos + sf::Vec2f{-10.f, 13.f}, deltaTimeMs, 25.f);

                for (sf::base::SizeT i = 0u; i < draggedCats.size(); ++i)
                {
                    if (i == pivotCatIdx)
                        continue;

                    draggedCats[i]->position = pivotCat.position + relativeCatPositions[i];
                }

                return;
            }

            if (ImGui::GetIO().WantCaptureMouse || !particleCullingBoundaries.isInside(mousePos))
            {
                resetAllDraggedCats();
                return;
            }

            Cat* hoveredCat = nullptr;

            // Only check for hover targets during initial press phase
            if (catDragPressDuration <= profile.catDragPressDuration)
                for (Cat& cat : pt->cats)
                {
                    if (!isCatDraggable(cat))
                        continue;

                    if ((mousePos - cat.position).lengthSquared() > cat.getRadiusSquared())
                        continue;

                    hoveredCat = &cat;
                }

            if (hoveredCat)
            {
                catDragPressDuration += deltaTimeMs;

                if (catDragPressDuration >= profile.catDragPressDuration)
                {
                    draggedCats.clear();
                    draggedCats.pushBack(hoveredCat);

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
    [[nodiscard]] static constexpr CatType shrineTypeToCatType(const ShrineType shrineType)
    {
        return static_cast<CatType>(asIdx(shrineType) + 4u);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateShrines(const float deltaTimeMs)
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

                            turnBubbleNormal(bubble);
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
                            delayedActions.emplaceBack(Countdown{.value = 7000.f},
                                                       [this] { playSound(sounds.letterchime); });
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
    void collectDollImpl(Doll& d, const sf::base::Vector<Doll>& dollsToUse)
    {
        SFML_BASE_ASSERT(!d.tcDeath.hasValue());

        const bool copy = &dollsToUse == &pt->copyDolls;

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

        const bool allDollsCollected = sf::base::allOf(dollsToUse.begin(),
                                                       dollsToUse.end(),
                                                       [&](const Doll& otherDoll)
        { return otherDoll.tcDeath.hasValue(); });

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
                                     ? std::pow((buffDurationSoftCap - currentBuff) / buffDurationSoftCap, 0.15f)
                                     : 0.1f;

            pt->buffCountdownsPerType[asIdx(d.catType)].value += buffDuration * factor;

            const auto* hexedCat = copy ? getCopyHexedCat() : getHexedCat();
            SFML_BASE_ASSERT(hexedCat != nullptr);

            spawnParticle({.position = d.getDrawPosition(),
                           .velocity = (hexedCat->getDrawPosition(profile.enableCatBobbing) - d.getDrawPosition()).normalized() *
                                       1.f,
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
    void collectDoll(Doll& d)
    {
        collectDollImpl(d, pt->dolls);
    }

    ////////////////////////////////////////////////////////////
    void collectCopyDoll(Doll& d)
    {
        collectDollImpl(d, pt->copyDolls);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateDollsImpl(const float deltaTimeMs, const sf::Vec2f mousePos, sf::base::Vector<Doll>& dollsToUse, Cat* hexedCat)
    {
        const bool copy = &dollsToUse == &pt->copyDolls;

        if (dollsToUse.empty())
        {
            if (hexedCat != nullptr)
                (copy ? hexedCat->hexedCopyTimer : hexedCat->hexedTimer)->direction = TimerDirection::Backwards;

            return;
        }

        // Can happen during prestige transition
        if (hexedCat == nullptr)
            return;

        for (Doll& d : dollsToUse)
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

                const bool click = (mBtnDown(getLMB(), /* penetrateUI */ false) || fingerPositions[0].hasValue());

                if (click && (mousePos - d.position).lengthSquared() <= d.getRadiusSquared())
                {
                    if (copy)
                        collectCopyDoll(d);
                    else
                        collectDoll(d);
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

        sf::base::vectorEraseIf(dollsToUse, [](const Doll& d) { return d.getDeathProgress() >= 1.f; });
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateDolls(const float deltaTimeMs, const sf::Vec2f mousePos)
    {
        if (cachedWitchCat == nullptr)
            return;

        gameLoopUpdateDollsImpl(deltaTimeMs, mousePos, pt->dolls, getHexedCat());
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCopyDolls(const float deltaTimeMs, const sf::Vec2f mousePos)
    {
        if (cachedCopyCat == nullptr || pt->copycatCopiedCatType != CatType::Witch)
            return;

        gameLoopUpdateDollsImpl(deltaTimeMs, mousePos, pt->copyDolls, getCopyHexedCat());
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateHellPortals(const float deltaTimeMs)
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
                                           rngFast.getRandomDirection() *
                                               rngFast.getF(hellPortalRadius * 0.95f, hellPortalRadius * 1.15f),
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
    void gameLoopUpdateWitchBuffs(const float deltaTimeMs)
    {
        for (Countdown& buffCountdown : pt->buffCountdownsPerType)
            if (buffCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                playSound(sounds.buffoff);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateMana(const float deltaTimeMs)
    {
        if (cachedWizardCat == nullptr)
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

            if (cachedWizardCat != nullptr)
                for (SizeT i = 0u; i < 8u; ++i)
                    spawnParticlesWithHueNoGravity(230.f,
                                                   1,
                                                   rngFast.getPointInCircle(cachedWizardCat->position, wizardRange),
                                                   ParticleType::Star,
                                                   0.15f,
                                                   0.05f);

            if (cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Wizard)
                for (SizeT i = 0u; i < 8u; ++i)
                    spawnParticlesWithHueNoGravity(230.f,
                                                   1,
                                                   rngFast.getPointInCircle(cachedCopyCat->position, wizardRange),
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

            if (cachedWizardCat != nullptr)
                for (SizeT i = 0u; i < 8u; ++i)
                    spawnParticlesWithHueNoGravity(50.f,
                                                   1,
                                                   rngFast.getPointInCircle(cachedWizardCat->position, wizardRange),
                                                   ParticleType::Star,
                                                   0.15f,
                                                   0.05f);

            if (cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Wizard)
                for (SizeT i = 0u; i < 8u; ++i)
                    spawnParticlesWithHueNoGravity(50.f,
                                                   1,
                                                   rngFast.getPointInCircle(cachedCopyCat->position, wizardRange),
                                                   ParticleType::Star,
                                                   0.15f,
                                                   0.05f);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAutocast()
    {
        if (cachedWizardCat == nullptr || !pt->perm.autocastPurchased || pt->perm.autocastIndex == 0u || isWizardBusy())
            return;

        const auto spellIndex = pt->perm.autocastIndex - 1u;

        if (static_cast<sf::base::SizeT>(spellIndex) > pt->psvSpellCount.nPurchases)
            return;

        if (pt->mana >= spellManaCostByIndex[spellIndex])
        {
            pt->mana -= spellManaCostByIndex[spellIndex];
            castSpellByIndex(spellIndex, cachedWizardCat, cachedCopyCat);

            constexpr const char* spellNames[4] = {
                "Starpaw Conversion",
                "Mewltiplier Aura",
                "Dark Union",
                "Stasis Field",
            };

            pushNotification("Autocast", "Spell \"%s\" casted!", spellNames[spellIndex]);
        }
    }

    ////////////////////////////////////////////////////////////
    void pushNotification(const char* title, const char* format, const auto&... args)
    {
        if (!profile.enableNotifications)
            return;

        char fmtBuffer[1024]{};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wformat-security"
        std::snprintf(fmtBuffer, sizeof(fmtBuffer), format, args...);
#pragma GCC diagnostic pop

        notificationQueue.emplaceBack(title, std::string{fmtBuffer});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateMilestones()
    {
        const auto updateMilestone = [&](const char* name, sf::base::U64& milestone)
        {
            const auto oldMilestone = milestone;

            milestone = sf::base::min(milestone, pt->statsTotal.secondsPlayed);

            if (milestone != oldMilestone)
            {
                const auto [h, m, s] = formatTime(milestone);
                pushNotification("Milestone reached!", "'%s' at %lluh %llum %llus", name, h, m, s);
            }
        };

        const auto nCatNormal = pt->getCatCountByType(CatType::Normal);
        const auto nCatUni    = pt->getCatCountByType(CatType::Uni);
        const auto nCatDevil  = pt->getCatCountByType(CatType::Devil);
        const auto nCatAstro  = pt->getCatCountByType(CatType::Astro);

        if (nCatNormal >= 1)
            updateMilestone("1st Cat", pt->milestones.firstCat);

        if (nCatUni >= 1)
            updateMilestone("1st Unicat", pt->milestones.firstUnicat);

        if (nCatDevil >= 1)
            updateMilestone("1st Devilcat", pt->milestones.firstDevilcat);

        if (nCatAstro >= 1)
            updateMilestone("1st Astrocat", pt->milestones.firstAstrocat);

        if (nCatNormal >= 5)
            updateMilestone("5th Cat", pt->milestones.fiveCats);

        if (nCatUni >= 5)
            updateMilestone("5th Unicat", pt->milestones.fiveUnicats);

        if (nCatDevil >= 5)
            updateMilestone("5th Devilcat", pt->milestones.fiveDevilcats);

        if (nCatAstro >= 5)
            updateMilestone("5th Astrocat", pt->milestones.fiveAstrocats);

        if (nCatNormal >= 10)
            updateMilestone("10th Cat", pt->milestones.tenCats);

        if (nCatUni >= 10)
            updateMilestone("10th Unicat", pt->milestones.tenUnicats);

        if (nCatDevil >= 10)
            updateMilestone("10th Devilcat", pt->milestones.tenDevilcats);

        if (nCatAstro >= 10)
            updateMilestone("10th Astrocat", pt->milestones.tenAstrocats);

        if (pt->psvBubbleValue.nPurchases >= 1)
            updateMilestone("Prestige Level 2", pt->milestones.prestigeLevel2);

        if (pt->psvBubbleValue.nPurchases >= 2)
            updateMilestone("Prestige Level 3", pt->milestones.prestigeLevel3);

        if (pt->psvBubbleValue.nPurchases >= 3)
            updateMilestone("Prestige Level 4", pt->milestones.prestigeLevel4);

        if (pt->psvBubbleValue.nPurchases >= 4)
            updateMilestone("Prestige Level 5", pt->milestones.prestigeLevel5);

        if (pt->psvBubbleValue.nPurchases >= 5)
            updateMilestone("Prestige Level 6", pt->milestones.prestigeLevel6);

        if (pt->psvBubbleValue.nPurchases >= 9)
            updateMilestone("Prestige Level 10", pt->milestones.prestigeLevel10);

        if (pt->psvBubbleValue.nPurchases >= 14)
            updateMilestone("Prestige Level 15", pt->milestones.prestigeLevel15);

        if (pt->psvBubbleValue.nPurchases >= 19)
            updateMilestone("Prestige Level 20 (MAX)", pt->milestones.prestigeLevel20);

        const auto totalRevenue = pt->statsTotal.getTotalRevenue();

        if (totalRevenue >= 10'000)
            updateMilestone("$10.000 Revenue", pt->milestones.revenue10000);

        if (totalRevenue >= 100'000)
            updateMilestone("$100.000 Revenue", pt->milestones.revenue100000);

        if (totalRevenue >= 1'000'000)
            updateMilestone("$1.000.000 Revenue", pt->milestones.revenue1000000);

        if (totalRevenue >= 10'000'000)
            updateMilestone("$10.000.000 Revenue", pt->milestones.revenue10000000);

        if (totalRevenue >= 100'000'000)
            updateMilestone("$100.000.000 Revenue", pt->milestones.revenue100000000);

        if (totalRevenue >= 1'000'000'000)
            updateMilestone("$1.000.000.000 Revenue", pt->milestones.revenue1000000000);

        for (SizeT i = 0u; i < pt->nShrinesCompleted; ++i)
        {
            const char* shrineName = i >= pt->getMapLimitIncreases() ? "Shrine Of ???" : shrineNames[i];
            updateMilestone(shrineName, pt->milestones.shrineCompletions[i]);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateSplits()
    {
        if (!inSpeedrunPlaythrough() || !pt->speedrunStartTime.hasValue())
            return;

        const auto updateSplit = [&](const char* name, sf::base::U64& split)
        {
            if (split == 0u)
                return;

            const auto oldSplit    = split;
            const auto splitTimeUs = (sf::Clock::now() - pt->speedrunStartTime.value()).asMicroseconds();

            split = sf::base::min(split, static_cast<sf::base::U64>(splitTimeUs));

            if (split != oldSplit)
            {
                const auto [hours, mins, secs, millis] = formatSpeedrunTime(sf::Time{splitTimeUs});
                pushNotification("Split reached!", "'%s' at %02llu:%02llu:%02llu:%03llu", name, hours, mins, secs, millis);
            }
        };

        if (pt->psvBubbleValue.nPurchases >= 1)
            updateSplit("Prestige Lv.2", pt->speedrunSplits.prestigeLevel2);

        if (pt->psvBubbleValue.nPurchases >= 2)
            updateSplit("Prestige Lv.3", pt->speedrunSplits.prestigeLevel3);

        if (pt->psvBubbleValue.nPurchases >= 3)
            updateSplit("Prestige Lv.4", pt->speedrunSplits.prestigeLevel4);

        if (pt->psvBubbleValue.nPurchases >= 4)
            updateSplit("Prestige Lv.5", pt->speedrunSplits.prestigeLevel5);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAchievements()
    {
        [[maybe_unused]] static bool mustGetFromSteam = true; // sync achievements from Steam only once

        SizeT nextId = 0u;

        const auto unlockIf = [&](const bool condition)
        {
            const auto achievementId = nextId++;

#if defined(BUBBLEBYTE_USE_STEAMWORKS) && !defined(BUBBLEBYTE_DEMO)
            if (steamMgr.isInitialized())
            {
                if (condition)
                    steamMgr.unlockAchievement(achievementId);

                if (!profile.unlockedAchievements[achievementId] && mustGetFromSteam)
                {
                    if (steamMgr.isAchievementUnlocked(achievementId))
                        profile.unlockedAchievements[achievementId] = true;
                }
            }
#endif

            if (profile.unlockedAchievements[achievementId] || !condition)
                return;

            profile.unlockedAchievements[achievementId] = true;

            pushNotification("Achievement unlocked!",
                             "\"%s\"\n- %s",
                             achievementData[achievementId].name,
                             achievementData[achievementId].description);
        };

        const auto unlockIfPrestige = [&](const bool condition)
        {
            if (undoPPPurchaseTimer.value > 0.f) // don't unlock in undo grace period
            {
                ++nextId;
                return;
            }

            unlockIf(condition);
        };

        const auto unlockIfGtEq = [&](const auto& value, const auto& threshold)
        {
            SFML_BASE_ASSERT(value >= 0);
            SFML_BASE_ASSERT(threshold >= 0);

            unlockIf(static_cast<sf::base::SizeT>(value) >= static_cast<sf::base::SizeT>(threshold));

            achievementProgress[nextId - 1u].emplace(static_cast<sf::base::SizeT>(value),
                                                     static_cast<sf::base::SizeT>(threshold));
        };

        const auto unlockIfGtEqPrestige = [&](const auto& value, const auto& threshold)
        {
            if (undoPPPurchaseTimer.value > 0.f) // don't unlock in undo grace period
            {
                ++nextId;
                return;
            }

            unlockIfGtEq(value, threshold);
        };

        const auto bubblesHandPopped = profile.statsLifetime.getTotalNBubblesHandPopped();
        const auto bubblesCatPopped  = profile.statsLifetime.getTotalNBubblesCatPopped();

        unlockIfGtEq(bubblesHandPopped, 1);
        unlockIfGtEq(bubblesHandPopped, 10);
        unlockIfGtEq(bubblesHandPopped, 100);
        unlockIfGtEq(bubblesHandPopped, 1000);
        unlockIfGtEq(bubblesHandPopped, 10'000);
        unlockIfGtEq(bubblesHandPopped, 100'000);
        unlockIfGtEq(bubblesHandPopped, 1'000'000);

        unlockIfGtEq(bubblesCatPopped, 1);
        unlockIfGtEq(bubblesCatPopped, 100);
        unlockIfGtEq(bubblesCatPopped, 1000);
        unlockIfGtEq(bubblesCatPopped, 10'000);
        unlockIfGtEq(bubblesCatPopped, 100'000);
        unlockIfGtEq(bubblesCatPopped, 1'000'000);
        unlockIfGtEq(bubblesCatPopped, 10'000'000);
        unlockIfGtEq(bubblesCatPopped, 100'000'000);

        unlockIf(pt->comboPurchased);

        unlockIfGtEq(pt->psvComboStartTime.nPurchases, 5);
        unlockIfGtEq(pt->psvComboStartTime.nPurchases, 10);
        unlockIfGtEq(pt->psvComboStartTime.nPurchases, 15);
        unlockIfGtEq(pt->psvComboStartTime.nPurchases, 20);

        unlockIf(pt->mapPurchased); //
        unlockIfGtEq(pt->psvMapExtension.nPurchases, 1);
        unlockIfGtEq(pt->psvMapExtension.nPurchases, 3);
        unlockIfGtEq(pt->psvMapExtension.nPurchases, 5);
        unlockIfGtEq(pt->psvMapExtension.nPurchases, 7);

        unlockIfGtEq(pt->psvBubbleCount.nPurchases, 1);
        unlockIfGtEq(pt->psvBubbleCount.nPurchases, 5);
        unlockIfGtEq(pt->psvBubbleCount.nPurchases, 10);
        unlockIfGtEq(pt->psvBubbleCount.nPurchases, 20);
        unlockIfGtEq(pt->psvBubbleCount.nPurchases, 30);

        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 1);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 5);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 10);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 20);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 30);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Normal)].nPurchases, 40);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases, 9);

        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 1);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 5);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 10);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 20);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 30);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Uni)].nPurchases, 40);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases, 9);

        unlockIfPrestige(pt->perm.unicatTranscendencePurchased);
        unlockIfPrestige(pt->perm.unicatTranscendenceAOEPurchased);

        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 1);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 5);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 10);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 20);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 30);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Devil)].nPurchases, 40);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases, 9);

        unlockIfGtEq(pt->psvExplosionRadiusMult.nPurchases, 1);
        unlockIfGtEq(pt->psvExplosionRadiusMult.nPurchases, 5);
        unlockIfGtEq(pt->psvExplosionRadiusMult.nPurchases, 10);

        unlockIfPrestige(pt->perm.devilcatHellsingedPurchased);

        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 1);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 5);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 10);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 20);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 25);
        unlockIfGtEq(pt->psvPerCatType[asIdx(CatType::Astro)].nPurchases, 30);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases, 9);

        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 1);
        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 2);
        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 3);
        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 5);
        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 10);
        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 15);
        unlockIfGtEq(pt->psvBubbleValue.nPurchases, 19);

        unlockIfPrestige(pt->perm.starterPackPurchased);

        unlockIfPrestige(pt->perm.multiPopPurchased);
        unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 2);
        unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 5);
        unlockIfGtEqPrestige(pt->psvPPMultiPopRange.nPurchases, 10);

        unlockIfPrestige(pt->perm.windPurchased);

        unlockIfPrestige(pt->perm.smartCatsPurchased);
        unlockIfPrestige(pt->perm.geniusCatsPurchased);

        unlockIfPrestige(pt->perm.astroCatInspirePurchased);
        unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 4);
        unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 8);
        unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 12);
        unlockIfGtEqPrestige(pt->psvPPInspireDurationMult.nPurchases, 16);

        unlockIfGtEq(combo, 5);
        unlockIfGtEq(combo, 10);
        unlockIfGtEq(combo, 15);
        unlockIfGtEq(combo, 20);
        unlockIfGtEq(combo, 25);

        unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 5);
        unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 10);
        unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 15);
        unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 20);
        unlockIfGtEq(profile.statsLifetime.highestStarBubblePopCombo, 25);

        const auto nStarBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Star);
        const auto nStarBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Star);

        unlockIfGtEq(nStarBubblesPoppedByHand, 1);
        unlockIfGtEq(nStarBubblesPoppedByHand, 100);
        unlockIfGtEq(nStarBubblesPoppedByHand, 1000);
        unlockIfGtEq(nStarBubblesPoppedByHand, 10'000);
        unlockIfGtEq(nStarBubblesPoppedByHand, 100'000);

        unlockIfGtEq(nStarBubblesPoppedByCat, 1);
        unlockIfGtEq(nStarBubblesPoppedByCat, 100);
        unlockIfGtEq(nStarBubblesPoppedByCat, 1000);
        unlockIfGtEq(nStarBubblesPoppedByCat, 10'000);
        unlockIfGtEq(nStarBubblesPoppedByCat, 100'000);
        unlockIfGtEq(nStarBubblesPoppedByCat, 1'000'000);
        unlockIfGtEq(nStarBubblesPoppedByCat, 10'000'000);

        unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 5);
        unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 10);
        unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 15);
        unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 20);
        unlockIfGtEq(profile.statsLifetime.highestNovaBubblePopCombo, 25);

        const auto nNovaBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Nova);
        const auto nNovaBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Nova);

        unlockIfGtEq(nNovaBubblesPoppedByHand, 1);
        unlockIfGtEq(nNovaBubblesPoppedByHand, 100);
        unlockIfGtEq(nNovaBubblesPoppedByHand, 1000);
        unlockIfGtEq(nNovaBubblesPoppedByHand, 10'000);
        unlockIfGtEq(nNovaBubblesPoppedByHand, 100'000);

        unlockIfGtEq(nNovaBubblesPoppedByCat, 1);
        unlockIfGtEq(nNovaBubblesPoppedByCat, 100);
        unlockIfGtEq(nNovaBubblesPoppedByCat, 1000);
        unlockIfGtEq(nNovaBubblesPoppedByCat, 10'000);
        unlockIfGtEq(nNovaBubblesPoppedByCat, 100'000);
        unlockIfGtEq(nNovaBubblesPoppedByCat, 1'000'000);
        unlockIfGtEq(nNovaBubblesPoppedByCat, 10'000'000);

        const auto nBombBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Bomb);
        const auto nBombBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Bomb);

        unlockIfGtEq(nBombBubblesPoppedByHand, 1);
        unlockIfGtEq(nBombBubblesPoppedByHand, 100);
        unlockIfGtEq(nBombBubblesPoppedByHand, 1000);
        unlockIfGtEq(nBombBubblesPoppedByHand, 10'000);

        unlockIfGtEq(nBombBubblesPoppedByCat, 1);
        unlockIfGtEq(nBombBubblesPoppedByCat, 100);
        unlockIfGtEq(nBombBubblesPoppedByCat, 1000);
        unlockIfGtEq(nBombBubblesPoppedByCat, 10'000);
        unlockIfGtEq(nBombBubblesPoppedByCat, 100'000);

        unlockIf(pt->achAstrocatPopBomb);

        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Normal)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Uni)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Devil)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Witch)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Wizard)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Mouse)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Engi)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Repulso)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Attracto)]);
        unlockIf(pt->achAstrocatInspireByType[asIdx(CatType::Copy)]);

        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 1);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 2);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 3);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 4);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 5);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 6);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 7);
        unlockIfGtEq(pt->psvShrineActivation.nPurchases, 8);

        unlockIfGtEq(pt->nShrinesCompleted, 1);
        unlockIfGtEq(pt->nShrinesCompleted, 2);
        unlockIfGtEq(pt->nShrinesCompleted, 3);
        unlockIfGtEq(pt->nShrinesCompleted, 4);
        unlockIfGtEq(pt->nShrinesCompleted, 5);
        unlockIfGtEq(pt->nShrinesCompleted, 6);
        unlockIfGtEq(pt->nShrinesCompleted, 7);
        unlockIfGtEq(pt->nShrinesCompleted, 8);

        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Witch)]);
        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Wizard)]);
        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Mouse)]);
        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Engi)]);
        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Repulso)]);
        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Attracto)]);
        unlockIfPrestige(pt->perm.unsealedByType[asIdx(CatType::Copy)]);

        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Normal)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Uni)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Devil)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Astro)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Wizard)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Mouse)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Engi)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Repulso)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Attracto)], 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Copy)], 1);

        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Normal)], 500);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Uni)], 100);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Devil)], 100);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Astro)], 50);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Wizard)], 10);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Mouse)], 10);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Engi)], 10);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Repulso)], 10);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Attracto)], 10);
        unlockIfGtEq(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Copy)], 10);

        unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 1);
        unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 10);
        unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 100);
        unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 1000);
        unlockIfGtEq(profile.statsLifetime.nWitchcatDollsCollected, 10'000);

        unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 3);
        unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 6);
        unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 9);
        unlockIfGtEqPrestige(pt->psvPPWitchCatBuffDuration.nPurchases, 12);

        unlockIfPrestige(pt->perm.witchCatBuffPowerScalesWithNCats);
        unlockIfPrestige(pt->perm.witchCatBuffPowerScalesWithMapSize);
        unlockIfPrestige(pt->perm.witchCatBuffFewerDolls);
        unlockIfPrestige(pt->perm.witchCatBuffFlammableDolls);
        unlockIfPrestige(pt->perm.witchCatBuffOrbitalDolls);

        unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 6);
        unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 12);
        unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 18);
        unlockIfGtEqPrestige(pt->psvPPUniRitualBuffPercentage.nPurchases, 24);

        unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 6);
        unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 12);
        unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 18);
        unlockIfGtEqPrestige(pt->psvPPDevilRitualBuffPercentage.nPurchases, 24);

        const auto nActiveBuffs = sf::base::countIf(pt->buffCountdownsPerType,
                                                    pt->buffCountdownsPerType + nCatTypes,
                                                    [](const Countdown& c) { return c.value > 0.f; });

        unlockIfGtEq(nActiveBuffs, 2);
        unlockIfGtEq(nActiveBuffs, 3);
        unlockIfGtEq(nActiveBuffs, 4);
        unlockIfGtEq(nActiveBuffs, 5);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases, 9);

        unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 1);
        unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 100);
        unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 1000);
        unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 10'000);
        unlockIfGtEq(profile.statsLifetime.nAbsorbedStarBubbles, 100'000);

        unlockIfGtEq(pt->psvSpellCount.nPurchases, 1);
        unlockIfGtEq(pt->psvSpellCount.nPurchases, 2);
        unlockIfGtEq(pt->psvSpellCount.nPurchases, 3);
        unlockIfGtEq(pt->psvSpellCount.nPurchases, 4);

        unlockIfGtEq(pt->psvStarpawPercentage.nPurchases, 1);
        unlockIfGtEq(pt->psvStarpawPercentage.nPurchases, 4);
        unlockIfGtEq(pt->psvStarpawPercentage.nPurchases, 8);

        unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 1);
        unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 5);
        unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 10);
        unlockIfGtEq(pt->psvMewltiplierMult.nPurchases, 15);

        unlockIfGtEq(pt->psvDarkUnionPercentage.nPurchases, 1);
        unlockIfGtEq(pt->psvDarkUnionPercentage.nPurchases, 4);
        unlockIfGtEq(pt->psvDarkUnionPercentage.nPurchases, 8);

        unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 1);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 10);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 100);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[0], 1000);

        unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 1);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 10);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 100);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[1], 1000);

        unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 1);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 10);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 100);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[2], 1000);

        unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 1);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 10);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 100);
        unlockIfGtEq(profile.statsLifetime.nSpellCasts[3], 1000);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases, 9);

        unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 4);
        unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 8);
        unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 12);
        unlockIfGtEqPrestige(pt->psvPPManaCooldownMult.nPurchases, 16);

        unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 4);
        unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 8);
        unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 12);
        unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 16);
        unlockIfGtEqPrestige(pt->psvPPManaMaxMult.nPurchases, 20);

        unlockIfPrestige(pt->perm.starpawConversionIgnoreBombs);
        unlockIfPrestige(pt->perm.starpawNova);
        unlockIfPrestige(pt->perm.wizardCatDoubleMewltiplierDuration);
        unlockIfPrestige(pt->perm.wizardCatDoubleStasisFieldDuration);

        unlockIfGtEq(pt->mouseCatCombo, 25);
        unlockIfGtEq(pt->mouseCatCombo, 50);
        unlockIfGtEq(pt->mouseCatCombo, 75);
        unlockIfGtEq(pt->mouseCatCombo, 100);
        unlockIfGtEq(pt->mouseCatCombo, 125);
        unlockIfGtEq(pt->mouseCatCombo, 150);
        unlockIfGtEq(pt->mouseCatCombo, 175);
        unlockIfGtEq(pt->mouseCatCombo, 999);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases, 9);

        unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 2);
        unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 6);
        unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 10);
        unlockIfGtEqPrestige(pt->psvPPMouseCatGlobalBonusMult.nPurchases, 14);

        unlockIfGtEq(profile.statsLifetime.nMaintenances, 1);
        unlockIfGtEq(profile.statsLifetime.nMaintenances, 10);
        unlockIfGtEq(profile.statsLifetime.nMaintenances, 100);
        unlockIfGtEq(profile.statsLifetime.nMaintenances, 1000);
        unlockIfGtEq(profile.statsLifetime.nMaintenances, 10'000);
        unlockIfGtEq(profile.statsLifetime.nMaintenances, 100'000);
        unlockIfGtEq(profile.statsLifetime.nMaintenances, 1'000'000);

        unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 3);
        unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 6);
        unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 9);
        unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 12);
        unlockIfGtEq(profile.statsLifetime.highestSimultaneousMaintenances, 15);

        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 1);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 3);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 6);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 9);
        unlockIfGtEq(pt->psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases, 12);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases, 9);

        unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 2);
        unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 6);
        unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 10);
        unlockIfGtEqPrestige(pt->psvPPEngiCatGlobalBonusMult.nPurchases, 14);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases, 9);

        unlockIfPrestige(pt->perm.repulsoCatFilterPurchased);
        unlockIfPrestige(pt->perm.repulsoCatConverterPurchased);
        unlockIfPrestige(pt->perm.repulsoCatNovaConverterPurchased);

        unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 1);
        unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 4);
        unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 8);
        unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 12);
        unlockIfGtEqPrestige(pt->psvPPRepulsoCatConverterChance.nPurchases, 16);

        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 1);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 3);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 6);
        unlockIfGtEq(pt->psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases, 9);

        unlockIfPrestige(pt->perm.attractoCatFilterPurchased);

        unlockIfGtEq(profile.statsLifetime.nDisguises, 1);
        unlockIfGtEq(profile.statsLifetime.nDisguises, 5);
        unlockIfGtEq(profile.statsLifetime.nDisguises, 25);
        unlockIfGtEq(profile.statsLifetime.nDisguises, 100);

        unlockIf(buyReminder >= 5); // Secret
        unlockIf(pt->geniusCatIgnoreBubbles.normal && pt->geniusCatIgnoreBubbles.star &&
                 pt->geniusCatIgnoreBubbles.bomb); // Secret
        unlockIf(wastedEffort);

        const auto minutesToMicroseconds = [](const sf::base::I64 nMinutes) -> sf::base::I64
        { return nMinutes * 60 * 1'000'000; };

        const bool inSpeedrunMode = inSpeedrunPlaythrough();

        unlockIf(inSpeedrunMode);

        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(9));
        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(7));
        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(5));
        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel2 <= minutesToMicroseconds(4));

        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(30));
        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(26));
        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(22));
        unlockIf(inSpeedrunMode && pt->speedrunSplits.prestigeLevel3 <= minutesToMicroseconds(18));

        mustGetFromSteam = false;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawBubbles()
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

        const sf::FloatRect bubbleRects[]{txrBubble, txrBubbleStar, txrBomb, txrBubbleNova};
        static_assert(sf::base::getArraySize(bubbleRects) == nBubbleTypes);

        sf::CPUDrawableBatch* batchToUseByType[]{&bubbleDrawableBatch,
                                                 &starBubbleDrawableBatch,
                                                 &bombBubbleDrawableBatch,
                                                 &starBubbleDrawableBatch};
        static_assert(sf::base::getArraySize(batchToUseByType) == nBubbleTypes);

        for (SizeT i = 0u; i < pt->bubbles.size(); ++i)
        {
            Bubble& bubble = pt->bubbles[i];

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
    void gameLoopDisplayBubblesWithoutShader()
    {
        shader.setUniform(suBubbleEffect, false);

        rtGame.draw(bubbleDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame.draw(starBubbleDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame.draw(bombBubbleDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursorTrail(const sf::Vec2f mousePos)
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
    void gameLoopDrawMinimapIcons()
    {
        minimapDrawableBatch.clear();

        const sf::FloatRect* mmCatTxrs[]{
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
    void gameLoopDisplayBubblesWithShader()
    {
        if (!shader.setUniform(suBackgroundTexture, rtBackground.getTexture()))
        {
            profile.useBubbleShader = false;
            gameLoopDisplayBubblesWithoutShader();
            return;
        }

        shader.setUniform(suTime, shaderTime);
        shader.setUniform(suResolution, getResolution());
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
                                            sf::BlendMode::Factor::One,
                                            sf::BlendMode::Equation::Add);

        const sf::RenderStates bubbleStates{
            .blendMode = bubbleBlend,
            .texture   = &textureAtlas.getTexture(),
            .shader    = &shader,
        };

        shader.setUniform(suBubbleEffect, true);
        shader.setUniform(suSubTexOrigin, txrBubble.position);
        shader.setUniform(suSubTexSize, txrBubble.size);

        rtGame.draw(bubbleDrawableBatch, bubbleStates);

        shader.setUniform(suBubbleLightness, profile.bsBubbleLightness * 1.25f);
        shader.setUniform(suIridescenceStrength, profile.bsIridescenceStrength * 0.01f);
        shader.setUniform(suSubTexOrigin, txrBubbleStar.position);
        shader.setUniform(suSubTexSize, txrBubbleStar.size);

        rtGame.draw(starBubbleDrawableBatch, bubbleStates);

        shader.setUniform(suBubbleEffect, false);

        rtGame.draw(bombBubbleDrawableBatch, bubbleStates);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCats(const sf::Vec2f mousePos, const float deltaTimeMs)
    {
        ////////////////////////////////////////////////////////////
        const sf::FloatRect* const uniCatTxr     = isUnicatTranscendenceActive() ? &txrUniCat2 : &txrUniCat;
        const sf::FloatRect* const uniCatTailTxr = isUnicatTranscendenceActive() ? &txrUniCat2Tail : &txrUniCatTail;

        const sf::FloatRect* const devilCatTxr    = isDevilcatHellsingedActive() ? &txrDevilCat2 : &txrDevilCat3;
        const sf::FloatRect* const devilCatPawTxr = isDevilcatHellsingedActive() ? &txrDevilCatPaw2 : &txrDevilCat3Arm;
        const sf::FloatRect* const devilCatTailTxr = isDevilcatHellsingedActive() ? &txrDevilCatTail2 : &txrDevilCat3Tail;

        ////////////////////////////////////////////////////////////
        const sf::FloatRect* const catTxrsByType[] = {
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
        const sf::FloatRect* const catPawTxrsByType[] = {
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
        const sf::FloatRect* const catTailTxrsByType[] = {
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
        const sf::Vec2f catTailOffsetsByType[] = {
            {0.f, 0.f},      // Normal
            {-35.f, -222.f}, // Uni
            {-8.f, 2.f},     // Devil
            {56.f, -80.f},   // Astro

            {37.f, 165.f}, // Witch
            {0.f, -35.f},  // Wizard
            {0.f, 0.f},    // Mouse
            {2.f, 43.f},   // Engi
            {4.f, -29.f},  // Repulso
            {0.f, 0.f},    // Attracto
            {0.f, 0.f},    // Copy
            {0.f, 0.f},    // Duck
        };

        static_assert(sf::base::getArraySize(catTailOffsetsByType) == nCatTypes);

        ////////////////////////////////////////////////////////////
        const float catHueByType[] = {
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
                            catHueByType);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCat(Cat&            cat,
                         const float     deltaTimeMs,
                         const sf::Vec2f mousePos,
                         const sf::FloatRect* const (&catTxrsByType)[nCatTypes],
                         const sf::FloatRect* const (&catPawTxrsByType)[nCatTypes],
                         const sf::FloatRect* const (&catTailTxrsByType)[nCatTypes],
                         const sf::Vec2f (&catTailOffsetsByType)[nCatTypes],
                         const float (&catHueByType)[nCatTypes])
    {
        auto& batchToUse     = catToPlace == &cat ? cpuTopDrawableBatch : cpuDrawableBatch;
        auto& textBatchToUse = catToPlace == &cat ? catTextTopDrawableBatch : catTextDrawableBatch;

        const sf::FloatRect& catTxr = *catTxrsByType[asIdx(cat.type)];

        if (catToPlace != &cat && !bubbleCullingBoundaries.isInside(cat.position))
            return;

        const auto isCopyCatWithType = [&](const CatType copiedType)
        { return cat.type == CatType::Copy && pt->copycatCopiedCatType == copiedType; };

        const bool beingDragged = isCatBeingDragged(cat);

        const sf::base::Optional<sf::FloatRect> dragRect = getAoEDragRect(mousePos);

        const bool insideDragRect = dragRect.hasValue() && dragRect->contains(cat.position);

        const bool hovered = (mousePos - cat.position).lengthSquared() <= cat.getRadiusSquared();

        const bool shouldDisplayRangeCircle = !beingDragged && !cat.isAstroAndInFlight() && hovered &&
                                              !mBtnDown(getLMB(), /* penetrateUI */ true);

        const U8 rangeInnerAlpha = shouldDisplayRangeCircle ? 75u : 0u;

        const sf::FloatRect& catPawTxr = *catPawTxrsByType[asIdx(isCopyCatWithType(CatType::Mouse) ? CatType::Mouse : cat.type)];
        const sf::FloatRect& catTailTxr    = *catTailTxrsByType[asIdx(cat.type)];
        const sf::Vec2f      catTailOffset = catTailOffsetsByType[asIdx(cat.type)];

        const float maxCooldown  = getComputedCooldownByCatTypeOrCopyCat(cat.type);
        const float cooldownDiff = cat.cooldown.value;

        float catRotation = 0.f;

        const auto doWitchAnimation = [&](float& wobblePhase, Cat& witch)
        {
            if (witch.cooldown.value >= 10'000.f)
            {
                wobblePhase = 0.f;
            }
            else
            {
                const float frequency = remap(witch.cooldown.value, 0.f, 10'000.f, 0.1f, 0.05f);
                wobblePhase += frequency * deltaTimeMs * 0.005f;

                const auto range        = pt->getComputedRangeByCatType(CatType::Witch);
                const auto rangeSquared = range * range;

                const bool catInWitchRange = (witch.position - cat.position).lengthSquared() <= rangeSquared;

                if (&cat == &witch || (pt->perm.witchCatBuffPowerScalesWithNCats && catInWitchRange))
                {
                    const float amplitude = remap(witch.cooldown.value, 0.f, 10'000.f, 0.5f, 0.f);
                    catRotation           = sf::base::sin(wobblePhase) * amplitude;
                }
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
        else if (beingDragged)
        {
            catRotation = -0.22f + sf::base::sin(cat.wobbleRadians) * 0.12f;
        }
        else if (cachedWitchCat != nullptr)
        {
            doWitchAnimation(witchcatWobblePhase, *cachedWitchCat);
        }
        else if (cachedCopyCat != nullptr && pt->copycatCopiedCatType == CatType::Witch)
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

        const auto circleColor = CatConstants::colors[asIdx(cat.type)].withRotatedHue(cat.hue).withLightness(0.75f);
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

        const auto anchorOffset = [&](const sf::Vec2f offset)
        { return catAnchor + (offset / 2.f * 0.2f * catScaleMult).rotatedBy(sf::radians(catRotation)); };

        const float tailRotationMult = cat.type == CatType::Uni ? 0.4f : 1.f;

        const auto tailWiggleRotation = sf::radians(
            catRotation + ((beingDragged ? -0.2f : 0.f) +
                           std::sin(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * tailRotationMult));

        const auto tailWiggleRotationInvertedDragged = sf::radians(
            catRotation + ((beingDragged ? 0.2f : 0.f) +
                           std::sin(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * tailRotationMult));

        const sf::Vec2f pushDown{0.f, beingDragged ? 75.f : 0.f};

        const auto attachmentHue = hueColor(catHueByType[asIdx(cat.type)] + cat.hue, alpha);

        // Devilcat: draw tail behind
        if (cat.type == CatType::Devil)
        {
            batchToUse.add(sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vec2f{905.f, 10.f} + pushDown * 2.f),
                                      .scale    = catScale * 1.25f,
                                      .origin   = {320.f, 32.f},
                                      .rotation = tailWiggleRotationInvertedDragged,
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
                                                  std::cos(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * 0.75f);

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
                           .color = hueColor(sf::base::remainder(cat.hue * 2.f - 15.f + static_cast<float>(cat.nameIdx) * 25.f,
                                                                 60.f) -
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
                    sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vec2f{-131.f, -365.f}),
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

                batchToUse.add(sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vec2f{-221.f, 25.f}),
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
                    sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vec2f{475.f, 240.f} + offset + originOffset),
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
                : (cat.type == CatType::Engi || (cat.type == CatType::Devil && isDevilcatHellsingedActive()))
                    ? darkEyeLidRects
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

            if (!cat.yawnAnimCountdown.isDone())
            {
                batchToUse.add(sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vec2f{-185.f, -185.f}),
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
                    sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vec2f{-185.f, -185.f}),
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
                               .scale       = catScale,
                               .origin      = catPawTxr.size / 2.f,
                               .rotation    = cat.type == CatType::Mouse ? sf::radians(-0.6f) : cat.pawRotation,
                               .textureRect = catPawTxr,
                               .color       = catColor.withAlpha(static_cast<U8>(cat.pawOpacity))});

            //
            // Copycat: mask
            if (cat.type == CatType::Copy)
            {
                if (copycatMaskAnim.isDone() &&
                    copycatMaskAnimCd.updateAndStop(deltaTimeMs) == CountdownStatusStop::AlreadyFinished)
                    copycatMaskAnim.value = 3000.f;

                if (copycatMaskAnimCd.isDone() &&
                    copycatMaskAnim.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                    copycatMaskAnimCd.value = 4000.f;

                const float foo = easeInOutBack(copycatMaskAnim.getProgressBounced(3000.f)) * 0.5f;

                const auto* txrMaskToUse = [&]() -> const sf::FloatRect*
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
            static thread_local std::string catNameBuffer;
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
                static thread_local std::string actionString;
                actionString.clear();

                actionString += std::to_string(cat.hits);
                actionString += " ";
                actionString += actionName;

                if (cat.type == CatType::Mouse || isCopyCatWithType(CatType::Mouse))
                {
                    actionString += " (x";
                    actionString += std::to_string(pt->mouseCatCombo + 1);
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
                    .position = (cat.moneyEarned != 0u ? textMoneyBuffer : textStatusBuffer).getBottomCenter().addY(2.f),
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
    void gameLoopDrawShrines(const sf::Vec2f mousePos)
    {
        Shrine* hoveredShrine = nullptr;

        for (Shrine& shrine : pt->shrines)
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

            const auto circleColor = sf::Color{231u, 198u, 39u}.withRotatedHue(shrine.getHue()).withLightness(0.75f);
            const auto circleOutlineColor = circleColor.withAlpha(rangeInnerAlpha);
            const auto textOutlineColor   = circleColor.withLightness(0.25f);

            cpuDrawableBatch.add(
                sf::Sprite{.position = shrine.getDrawPosition(),
                           .scale    = sf::Vec2f{0.3f, 0.3f} * invDeathProgress +
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
                static thread_local std::string shrineStatus;

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
    void gameLoopDrawDolls(const sf::Vec2f mousePos)
    {
        ////////////////////////////////////////////////////////////
        const sf::FloatRect* dollTxrs[] = {
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
            for (Doll& doll : container)
            {
                const auto& dollTxr = *dollTxrs[asIdx(doll.catType)];

                const float invDeathProgress = 1.f - doll.getDeathProgress();
                const float progress = doll.tcDeath.hasValue() ? invDeathProgress : doll.getActivationProgress();

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
    void gameLoopDrawHellPortals()
    {
        const float hellPortalRadius = pt->getComputedRangeByCatType(CatType::Devil);

        for (HellPortal& hp : pt->hellPortals)
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

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getViewCenter() const
    {
        return {sf::base::clamp(gameScreenSize.x / 2.f + actualScroll * 2.f,
                                gameScreenSize.x / 2.f,
                                boundaries.x - gameScreenSize.x / 2.f),
                gameScreenSize.y / 2.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getViewCenterWithoutScroll() const
    {
        return {gameScreenSize.x / 2.f, gameScreenSize.y / 2.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CullingBoundaries getViewCullingBoundaries(const float offset) const
    {
        const sf::Vec2f viewCenter{getViewCenter()};

        return {viewCenter.x - gameScreenSize.x / 2.f + offset,
                viewCenter.x + gameScreenSize.x / 2.f - offset,
                viewCenter.y - gameScreenSize.y / 2.f + offset,
                viewCenter.y + gameScreenSize.y / 2.f - offset};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline sf::Sprite particleToSprite(const Particle& particle) const
    {
        const auto  opacityAsAlpha = static_cast<sf::base::U8>(particle.opacity * 255.f);
        const auto& textureRect    = particleRects[asIdx(particle.type)];

        return {
            .position    = particle.position,
            .scale       = {particle.scale, particle.scale},
            .origin      = textureRect.size / 2.f,
            .rotation    = sf::radians(particle.rotation),
            .textureRect = textureRect,
            .color       = hueByteColor(particle.hueByte, opacityAsAlpha),
        };
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawParticles()
    {
        if (!profile.showParticles)
            return;

        for (const auto& particle : particles)
        {
            if (!particleCullingBoundaries.isInside(particle.position))
                continue;

            cpuDrawableBatch.add(particleToSprite(particle));
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawHUDParticles()
    {
        if (!profile.showParticles)
            return;

        for (const auto& particle : spentCoinParticles)
            hudDrawableBatch.add(particleToSprite(particle));
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawEarnedCoinParticles()
    {
        if (!profile.showParticles)
            return;

        const auto targetPosition = moneyText.getCenterRight();

        const auto bezier = [](const sf::Vec2f& start, const sf::Vec2f& end, const float t)
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
    void gameLoopDrawHUDTopParticles()
    {
        if (!profile.showParticles)
            return;

        for (const auto& particle : hudTopParticles)
            hudTopDrawableBatch.add(particleToSprite(particle));
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawHUDBottomParticles()
    {
        if (!profile.showParticles)
            return;

        for (const auto& particle : hudBottomParticles)
            hudBottomDrawableBatch.add(particleToSprite(particle));
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawTextParticles()
    {
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
    void gameLoopDrawScrollArrowHint(const float deltaTimeMs)
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

        rtGame.draw(txArrow,
                    {.position = {gameScreenSize.x - 15.f, 15.f + (gameScreenSize.y / 5.f) * 1.f},
                     .origin   = txArrow.getRect().getCenterRight(),
                     .color    = sf::Color::whiteMask(static_cast<U8>(blinkOpacity))});

        rtGame.draw(txArrow,
                    {.position = {gameScreenSize.x - 15.f, gameScreenSize.y - 15.f - (gameScreenSize.y / 5.f) * 1.f},
                     .origin   = txArrow.getRect().getCenterRight(),
                     .color    = sf::Color::whiteMask(static_cast<U8>(blinkOpacity))});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawImGui(sf::base::U8 shouldDrawUIAlpha);

    ////////////////////////////////////////////////////////////
    void gameLoopUpdatePurchaseUnlockedEffects(const float deltaTimeMs)
    {
        const float imguiWidth = uiWindowWidth * profile.uiScale;
        const auto  blinkFn = [](const float value) { return (1 - sf::base::cos(2.f * sf::base::pi * value)) / 2.f; };

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
                             .scale = sf::Vec2f{0.25f, 0.25f} * (profile.uiScale + -0.15f * easeInOutBack(blinkProgress)),
                             .origin = tx.getRect().getCenterRight(),
                             .color  = hueColor(hue + currentBackgroundHue.asDegrees(), arrowAlpha)},
                            {.shader = &shader});
            }
        }

        sf::base::vectorEraseIf(purchaseUnlockedEffects, [](const auto& pue) { return pue.arrowCountdown.isDone(); });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool shouldDrawGrabbingCursor() const
    {
        return !draggedCats.empty() || mBtnDown(getRMB(), /* penetrateUI */ true);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursor(const float deltaTimeMs, const float cursorGrow)
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
                              ((1.f + easeInOutBack(cursorGrow) * std::pow(static_cast<float>(combo), 0.09f)) *
                               dpiScalingFactor),
                     .origin = {5.f, 5.f},
                     .color  = hueColor(profile.cursorHue + currentBackgroundHue.asDegrees(), 255u)},
                    {.shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursorComboText(const float deltaTimeMs, const float cursorGrow)
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
            cursorComboText.setString("x" + std::to_string(combo + 1));

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

        rtGame.draw(cursorComboText, {.shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursorComboBar()
    {
        if (!pt->comboPurchased || !profile.showCursorComboBar || comboCountdown.value == 0.f || shouldDrawGrabbingCursor())
            return;

        const float scaleMult = profile.cursorScale * dpiScalingFactor;

        const auto cursorComboBarPosition = sf::Mouse::getPosition(window).toVec2f() + sf::Vec2f{52.f, 14.f} * scaleMult;

        rtGame.draw(sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackMask(80u),
            .outlineColor       = cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size = {64.f * scaleMult * pt->psvComboStartTime.currentValue() * 1000.f / 700.f, 24.f * scaleMult},
        });

        rtGame.draw(sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackMask(164u),
            .outlineColor       = cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size               = {64.f * scaleMult * comboCountdown.value / 700.f, 24.f * scaleMult},
        });
    }

    ////////////////////////////////////////////////////////////
    // Helper function to convert a view's normalized viewport to pixel bounds relative to a target size (e.g. window size)
    [[nodiscard]] sf::FloatRect getViewportPixelBounds(const sf::View& view, const sf::Vec2f targetSize) const
    {
        return {{view.viewport.position.x * targetSize.x, view.viewport.position.y * targetSize.y},
                {view.viewport.size.x * targetSize.x, view.viewport.size.y * targetSize.y}};
    }

    ////////////////////////////////////////////////////////////
    // Returns a random position along the edges of the provided bounds.
    [[nodiscard]] sf::Vec2f getEdgeSpawnPosition(const sf::FloatRect& bounds, const float thickness)
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
    void gameLoopDrawDollParticleBorder(const float hueMod)
    {
        if (!profile.showDollParticleBorder)
            return;

        for (int i = 0; i < 10; ++i)
        {
            const sf::FloatRect gameViewBounds = getViewportPixelBounds(gameView, getResolution());
            const sf::Vec2f     spawnPos       = getEdgeSpawnPosition(gameViewBounds, 10.f);

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
    void gameLoopTips(const float deltaTimeMs)
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

        const float bgProgress = tipTCByteEnd.hasValue() ? tipTCByteEnd->getInvProgress() : tipTCBackground.getProgress();
        const float byteProgress = tipTCByteEnd.hasValue() ? tipTCByteEnd->getInvProgress() : tipTCByte.getProgress();

        const float tipByteAlpha       = byteProgress * 255.f;
        const float tipBackgroundAlpha = bgProgress * 255.f;

        sf::Sprite tipBackgroundSprite{.position = {},
                                       .scale = sf::Vec2f{0.4f, 0.4f} + sf::Vec2f{0.4f, 0.4f} * easeInOutBack(bgProgress),
                                       .origin      = txTipBg.getSize().toVec2f() / 2.f,
                                       .textureRect = txTipBg.getRect(),
                                       .color = sf::Color::whiteMask(static_cast<U8>(tipBackgroundAlpha * 0.85f))};


        SFML_BASE_ASSERT(profile.hudScale > 0.f);

        tipBackgroundSprite.setBottomCenter(
            {getResolution().x / 2.f / profile.hudScale, getResolution().y / profile.hudScale - 50.f});

        rtGame.draw(tipBackgroundSprite, {.texture = &txTipBg});

        sf::Sprite tipByteSprite{.position    = {},
                                 .scale       = sf::Vec2f{0.85f, 0.85f} * easeInOutBack(byteProgress),
                                 .origin      = txTipByte.getSize().toVec2f() / 2.f,
                                 .rotation    = sf::radians(sf::base::tau * easeInOutBack(byteProgress)),
                                 .textureRect = txTipByte.getRect(),
                                 .color       = sf::Color::whiteMask(static_cast<U8>(tipByteAlpha))};

        tipByteSprite.setCenter(tipBackgroundSprite.getCenterRight().addY(-40.f));
        rtGame.draw(tipByteSprite, {.texture = &txTipByte});

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
                tipTCBytePreEnd.emplace(
                    TargetedCountdown{.startingValue = 250.f + static_cast<float>(tipString.size()) * 20.f});
                tipTCBytePreEnd->restart();
            }
        }

        if (tipTCBytePreEnd.hasValue() && tipTCBytePreEnd->updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
        {
            tipTCByteEnd.emplace(TargetedCountdown{.startingValue = 750.f});
            tipTCByteEnd->restart();
        }

        sf::Text tipText{fontSuperBakery,
                         {.position         = {},
                          .scale            = sf::Vec2f{0.5f, 0.5f} * easeInOutBack(byteProgress),
                          .string           = tipString.substr(0, tipCharIdx),
                          .characterSize    = 60u,
                          .fillColor        = sf::Color::whiteMask(static_cast<sf::base::U8>(tipByteAlpha)),
                          .outlineColor     = outlineHueColor.withAlpha(static_cast<sf::base::U8>(tipByteAlpha)),
                          .outlineThickness = 4.f}};

        tipText.setTopLeft(tipBackgroundSprite.getTopLeft() + sf::Vec2f{45.f, 65.f});

        tipStringWiggle.advance(deltaTimeMs);
        tipStringWiggle.apply(tipText);

        rtGame.draw(tipText);

        tipStringWiggle.unapply(tipText);
    }

    ////////////////////////////////////////////////////////////
    void recreateImGuiRenderTexture(const sf::Vec2u newResolution)
    {
        rtImGui = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value();
    }

    ////////////////////////////////////////////////////////////
    void recreateGameRenderTexture(const sf::Vec2u newResolution)
    {
        rtGame = sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2u getNewResolution() const
    {
        return profile.resWidth == sf::Vec2u{} ? getReasonableWindowSize(0.9f) : profile.resWidth;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::RenderWindow makeWindow()
    {
        const sf::Vec2u desktopResolution = sf::VideoModeUtils::getDesktopMode().size;
        const sf::Vec2u newResolution     = getNewResolution();

        const bool takesAllScreen = newResolution == desktopResolution;

        return sf::RenderWindow{{
            .size            = newResolution,
            .title           = "BubbleByte " BUBBLEBYTE_VERSION_STR,
            .fullscreen      = !profile.windowed,
            .resizable       = !takesAllScreen,
            .closable        = !takesAllScreen,
            .hasTitlebar     = !takesAllScreen,
            .vsync           = profile.vsync,
            .frametimeLimit  = sf::base::clamp(profile.frametimeLimit, 60u, 144u),
            .contextSettings = contextSettings,
        }};
    }

    ////////////////////////////////////////////////////////////
    void recreateWindow()
    {
        const sf::Vec2u newResolution = getNewResolution();

        window = makeWindow();

        recreateImGuiRenderTexture(newResolution);
        recreateGameRenderTexture(newResolution);

        dpiScalingFactor = window.getWindowDisplayScale();
    }

    ////////////////////////////////////////////////////////////
    void resizeWindow()
    {
        const sf::Vec2u desktopResolution = sf::VideoModeUtils::getDesktopMode().size;
        const sf::Vec2u newResolution     = getNewResolution();

        const bool takesAllScreen = newResolution == desktopResolution;

        window.setResizable(!takesAllScreen);
        window.setHasTitlebar(!takesAllScreen);
        window.setSize(newResolution);

        if (!takesAllScreen)
            window.setPosition(((desktopResolution - newResolution) / 2u).toVec2i());

        recreateImGuiRenderTexture(newResolution);
        recreateGameRenderTexture(newResolution);

        dpiScalingFactor = window.getWindowDisplayScale();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float gameLoopUpdateCursorGrowthEffect(const float deltaTimeMs, const bool anyBubblePoppedByClicking)
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

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCombo(const float                         deltaTimeMs,
                             const bool                          anyBubblePoppedByClicking,
                             const sf::Vec2f                     mousePos,
                             const sf::base::Optional<sf::Vec2f> clickPosition)
    {
        // Mousecat combo
        checkComboEnd(deltaTimeMs, pt->mouseCatCombo, pt->mouseCatComboCountdown);

        // Combo failure countdown for red text effect
        (void)comboFailCountdown.updateAndStop(deltaTimeMs);

        // Player combo data
        const auto playerLastCombo      = combo;
        bool       playerJustEndedCombo = false;

        // Player combo failure due to timer end
        if (checkComboEnd(deltaTimeMs, combo, comboCountdown))
            playerJustEndedCombo = true;


        // Player combo failure due to missed click
        if (!anyBubblePoppedByClicking && clickPosition.hasValue() && !pt->laserPopEnabled)
        {
            if (combo > 1)
            {
                playSound(sounds.scratch);
                comboFailCountdown.value = 250.f;
            }

            combo                = 0;
            comboCountdown.value = 0.f;

            playerJustEndedCombo = true;
        }

        if (playerJustEndedCombo)
        {
            if (playerLastCombo > 2)
            {
                comboAccReward     = static_cast<int>(std::pow(static_cast<float>(comboNOthers), 1.25f));
                comboAccStarReward = comboNStars;
            }
            else
            {
                comboAccReward     = 0;
                comboAccStarReward = 0;
            }

            iComboAccReward     = 0;
            iComboAccStarReward = 0;
            comboNStars         = 0;
            comboNOthers        = 0;
        }

        if (profile.accumulatingCombo)
        {
            if (iComboAccReward < comboAccReward &&
                accComboDelay.updateAndLoop(deltaTimeMs, 35.f) == CountdownStatusLoop::Looping)
            {
                ++iComboAccReward;
                accComboDelay.value = 35.f;

                if (spawnEarnedCoinParticle(fromWorldToHud(mousePos)))
                {
                    earnedCoinParticles.back().startPosition += rngFast.getVec2f({-25.f, -25.f}, {25.f, 25.f});

                    sounds.coindelay.settings.position = {getViewCenter().x - gameScreenSize.x / 2.f + 25.f,
                                                          getViewCenter().y - gameScreenSize.y / 2.f + 25.f};
                    sounds.coindelay.settings.pitch    = 0.8f + static_cast<float>(iComboAccReward) * 0.04f;
                    sounds.coindelay.settings.volume   = profile.sfxVolume / 100.f;

                    playSound(sounds.coindelay, /* maxOverlap */ 64);
                }
            }

            if (iComboAccStarReward < comboAccStarReward &&
                accComboStarDelay.updateAndLoop(deltaTimeMs, 75.f) == CountdownStatusLoop::Looping)
            {
                ++iComboAccStarReward;

                sounds.shine3.settings.position = {mousePos.x, mousePos.y};
                sounds.shine3.settings.pitch    = 0.75f + static_cast<float>(iComboAccStarReward) * 0.075f;
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
    void gameLoopUpdateCollisionsBubbleBubble(const float deltaTimeMs)
    {
        auto func = [&](const SizeT bubbleIdxI, const SizeT bubbleIdxJ) SFML_BASE_LAMBDA_ALWAYS_INLINE
        {
            // TODO P2: technically this is a data race
            handleBubbleCollision(deltaTimeMs, pt->bubbles[bubbleIdxI], pt->bubbles[bubbleIdxJ]);
        };

        const sf::base::SizeT nWorkers = threadPool.getWorkerCount();
        sweepAndPrune.forEachUniqueIndexPair(nWorkers, threadPool, func);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCollisionsCatCat(const float deltaTimeMs)
    {
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
    void gameLoopUpdateCollisionsCatShrine(const float deltaTimeMs) const
    {
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
    void gameLoopUpdateCollisionsCatDoll()
    {
        const auto checkCollisionWithDoll = [&](Doll& d, auto collectFn)
        {
            for (Cat& cat : pt->cats)
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
    void gameLoopUpdateCollisionsBubbleHellPortal()
    {
        if (!frameProcThisFrame)
            return;

        const float hellPortalRadius        = pt->getComputedRangeByCatType(CatType::Devil) * 1.25f;
        const float hellPortalRadiusSquared = hellPortalRadius * hellPortalRadius;

        for (HellPortal& hellPortal : pt->hellPortals)
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
    void gameLoopUpdateScreenShake(const float deltaTimeMs)
    {
        if (screenShakeTimer > 0.f)
        {
            screenShakeTimer -= deltaTimeMs;
            screenShakeTimer = sf::base::max(0.f, screenShakeTimer);
        }

        const bool anyShrineDying = sf::base::anyOf(pt->shrines.begin(),
                                                    pt->shrines.end(),
                                                    [](const Shrine& shrine) { return shrine.tcDeath.hasValue(); });

        if (!anyShrineDying && screenShakeTimer <= 0.f && screenShakeAmount > 0.f)
        {
            screenShakeAmount -= deltaTimeMs * 0.05f;
            screenShakeAmount = sf::base::max(0.f, screenShakeAmount);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateParticlesAndTextParticles(const float deltaTimeMs)
    {
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
    void gameLoopUpdateSounds(const float deltaTimeMs, const sf::Vec2f mousePos)
    {
#ifndef BUBBLEBYTE_NO_AUDIO
        const float volumeMult = profile.playAudioInBackground || window.hasFocus() ? 1.f : 0.f;

        listener.position = {sf::base::clamp(mousePos.x, 0.f, pt->getMapLimit()),
                             sf::base::clamp(mousePos.y, 0.f, boundaries.y),
                             0.f};

        listener.volume = profile.masterVolume / 100.f * volumeMult;

        (void)playbackDevice.updateListener(listener);

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

                if (sounds.countPlayingPooled(sounds.prestige) > 0u)
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

                if (sounds.countPlayingPooled(sounds.prestige) > 0u)
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
    void gameLoopUpdateTimePlayed(const sf::base::I64 elapsedUs)
    {
        playedUsAccumulator += elapsedUs;

        while (playedUsAccumulator > 1'000'000)
        {
            playedUsAccumulator -= 1'000'000;
            statSecondsPlayed();
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAutosave(const sf::base::I64 elapsedUs)
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
    float fixedBgSlideTarget = 0.f;

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAndDrawFixedMenuBackground(const float deltaTimeMs, const sf::base::I64 elapsedUs)
    {
        const auto resolution = getResolution();

        const float ratio = resolution.x / 1250.f;

        static float fixedBgSlide = 0.f;

        fixedBgSlideAccumulator += elapsedUs;

        if (fixedBgSlideAccumulator > 60'000'000) // change slide every 60s
        {
            fixedBgSlideAccumulator = 0;

            fixedBgSlideTarget += 1.f;

            if (fixedBgSlideTarget >= 3.f)
                fixedBgSlideTarget = 0.f;
        }

        fixedBgSlide = exponentialApproach(fixedBgSlide, fixedBgSlideTarget, deltaTimeMs, 1000.f);

        const float fixedBgX = 2100.f * ratio * 0.5f * sf::base::remainder(fixedBgSlide, 3.f);
        const auto  sz       = txFixedBg.getSize().toVec2f();

        // Result of linear regression and trial-and-error >:3
        const float fixedBgOffsetX = 1648.f * ratio - 3216.62f;

        rtGame.draw(txFixedBg,
                    {
                        .position = {sz.x + resolution.x / 2.f - actualScroll / 20.f - fixedBgX + fixedBgOffsetX, sz.y},
                        .scale    = {ratio, ratio},
                        .origin   = {sz.x / 2.f, sz.y / 1.5f},
                        .textureRect = {{sz.x * -2.f, sz.y * -2.f}, {sz.x * 4.f, sz.y * 4.f}},
                        .color       = hueColor(currentBackgroundHue.asDegrees(), 255u),
                    },
                    {.shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAndDrawBackground(const float deltaTimeMs)
    {
        static float backgroundScroll = 0.f;
        backgroundScroll += deltaTimeMs * 0.01f;

        rtBackground.clear(outlineHueColor);

        rtBackground.setView(gameBackgroundView);
        rtBackground.setWrapMode(sf::TextureWrapMode::Repeat); // TODO P2: (lib) add RenderTextureCreateSettings

        const auto getAlpha = [&](const float mult)
        { return static_cast<sf::base::U8>(profile.backgroundOpacity / 100.f * mult); };

        ////////////////////////////////////////////////////////////
        const sf::Texture* chunkTx[] = {
            &txBackgroundChunk,            // Normal
            &txBackgroundChunk,            // Voodoo
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
        const sf::Texture* detailTx[] = {
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
        const auto idx = profile.selectedBackground;

        targetBackgroundHue = sf::radians(sf::degrees(backgroundHues[idx]).asRadians()).wrapUnsigned();
        currentBackgroundHue = currentBackgroundHue.rotatedTowards(targetBackgroundHue, deltaTimeMs * 0.01f).wrapUnsigned();
        outlineHueColor = colorBlueOutline.withRotatedHue(currentBackgroundHue.asDegrees());

        rtBackground.draw(*chunkTx[idx],
                          {
                              .scale       = {0.5f, 0.5f},
                              .textureRect = {{actualScroll + backgroundScroll * 0.25f, 0.f},
                                              txBackgroundChunk.getSize().toVec2f() * 2.f},
                              .color       = hueColor(currentBackgroundHue.asDegrees(), getAlpha(255.f)),
                          },
                          {.shader = &shader});

        if (idx == 0u || profile.alwaysShowDrawings)
            rtBackground.draw(txDrawings,
                              {
                                  .textureRect = {{actualScroll * 2.f, 0.f}, txBackgroundChunk.getSize().toVec2f() * 2.f},
                                  .color = sf::Color::whiteMask(getAlpha(200.f)),
                              });

        rtBackground.draw(*detailTx[idx],
                          {
                              .scale       = {0.75f, 0.75f},
                              .textureRect = {{actualScroll * 2.f + backgroundScroll * 0.5f, 0.f},
                                              txBackgroundChunk.getSize().toVec2f() * 1.5f},
                              .color       = sf::Color::whiteMask(getAlpha(175.f)),
                          });

        rtBackground.draw(txClouds,
                          {
                              .scale       = {1.25f, 1.25f},
                              .textureRect = {{actualScroll * 4.f + backgroundScroll * 3.f, 0.f},
                                              txBackgroundChunk.getSize().toVec2f()},
                              .color       = sf::Color::whiteMask(getAlpha(128.f)),
                          });

        rtBackground.display();

        auto gameViewNoScroll   = gameView;
        gameViewNoScroll.center = getViewCenterWithoutScroll();

        rtGame.setView(gameViewNoScroll);
        rtGame.draw(rtBackground.getTexture(), {.textureRect{{0.f, 0.f}, gameScreenSize}});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateMoneyText(const float deltaTimeMs, const float yBelowMinimap)
    {
        moneyText.setString("$" + std::string(toStringWithSeparators(pt->money + spentMoney)));

        moneyText.setOutlineColor(outlineHueColor);
        moneyText.scale  = {0.5f, 0.5f};
        moneyText.origin = moneyText.getLocalBounds().size / 2.f;

        moneyText.setTopLeft({15.f, 70.f});
        moneyTextShakeEffect.update(deltaTimeMs);
        moneyTextShakeEffect.applyToText(moneyText);
        moneyText.scale *= 0.5f;

        moneyText.position.y = yBelowMinimap + 30.f;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateSpentMoneyEffect(const float deltaTimeMs)
    {
        if (spentMoney == 0u || spentMoneyTimer.updateForwardAndLoop(deltaTimeMs * 0.08f) == TimerStatusLoop::Running)
            return;

        if (profile.showCoinParticles)
        {
            playSound(sounds.coin, /* maxOverlap */ 64);

            spawnSpentCoinParticle(
                {.position      = moneyText.getCenterRight().addY(rngFast.getF(-12.f, 12.f)),
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
    void gameLoopUpdateComboText(const float deltaTimeMs, const float yBelowMinimap)
    {
        if (!pt->comboPurchased)
            return;

        comboText.setString("x" + std::to_string(combo + 1));
        comboText.setOutlineColor(outlineHueColor);

        comboTextShakeEffect.update(deltaTimeMs);
        comboTextShakeEffect.applyToText(comboText);
        comboText.scale *= 0.5f;

        comboText.position.y = yBelowMinimap + 50.f;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateBuffText()
    {
        const char* devilBuffName = (isDevilcatHellsingedActive()) ? "Portal Storm (Scales With Bomb Spawn Chance)"
                                                                   : "Explosive Downpour (Bomb Spawn Chance)";

        const char* buffNames[] = {
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

        const SizeT nDollsToClick = sf::base::countIf(pt->dolls.begin(),
                                                      pt->dolls.end(),
                                                      [](const Doll& doll) { return !doll.tcDeath.hasValue(); });

        const SizeT nCopyDollsToClick = sf::base::countIf(pt->copyDolls.begin(),
                                                          pt->copyDolls.end(),
                                                          [](const Doll& doll) { return !doll.tcDeath.hasValue(); });

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

        buffText.setString(buffStrBuffer);
        buffText.setOutlineColor(outlineHueColor);

        buffText.position.y = comboText.getBottomLeft().y + 10.f;
        buffText.scale      = {0.5f, 0.5f};
    }

    ////////////////////////////////////////////////////////////
    void gameLoopPrestigeAvailableReminder()
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
    void gameLoopReminderBuyCombo()
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
    void gameLoopReminderSpendPPs()
    {
        if (!pt->spendPPTipShown && pt->psvBubbleValue.nPurchases == 1u && pt->prestigePoints > 0u && pt->money > 500u)
        {
            pt->spendPPTipShown = true;
            doTip("Remember that you have some\nprestige points to spend!");
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateDpsSampler(const sf::base::I64 elapsedUs)
    {
        moneyGainedUsAccumulator += elapsedUs;
        while (moneyGainedUsAccumulator >= 1'000'000)
        {
            moneyGainedUsAccumulator -= 1'000'000;

            samplerMoneyPerSecond.record(static_cast<float>(moneyGainedLastSecond));
            moneyGainedLastSecond = 0u;

            statHighestDPS(static_cast<sf::base::U64>(samplerMoneyPerSecond.getAverage()));
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateNotificationQueue(float deltaTimeMs);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool gameLoop()
    {
        if (mustExit)
            return false;

        //
        // Only draw UI elements if not in prestige transition and splash is done
        const bool shouldDrawUI      = !inPrestigeTransition && splashCountdown.value <= 0.f;
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
                recreateImGuiRenderTexture(e6->size);
                recreateGameRenderTexture(e6->size);

                hudTopParticles.clear();
                hudBottomParticles.clear();
            }
            else if (const auto* e7 = event->getIf<sf::Event::KeyPressed>())
            {
                if (e7->code == sf::Keyboard::Key::Z || e7->code == sf::Keyboard::Key::X ||
                    e7->code == sf::Keyboard::Key::Y)
                    clickPosition.emplace(sf::Mouse::getPosition(window).toVec2f());
            }
            else if (const auto* e8 = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                const float scrollMult = keyDown(sf::Keyboard::Key::LShift) ? 200.f : 100.f;

                if (!ImGui::GetIO().WantCaptureMouse)
                    scroll += e8->delta * scrollMult;
            }
#pragma GCC diagnostic pop
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

            sf::base::vectorEraseIf(pt->cats,
                                    [](const Cat& cat)
            { return cat.type >= CatType::Mouse && cat.type <= CatType::Duck; });
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
                scroll = static_cast<float>(nextScrollScreenIndex) * gameScreenSize.x * 0.5f;
            }
            else if ((inputHelper.wasKeyJustPressed(sf::Keyboard::Key::PageUp) ||
                      inputHelper.wasMouseButtonJustPressed(sf::Mouse::Button::Extra1)) &&
                     currentScrollScreenIndex > 0u)
            {
                const auto nextScrollScreenIndex = sf::base::max(static_cast<sf::base::SizeT>(0u),
                                                                 currentScrollScreenIndex - 1u);

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

        const auto mousePos = window.mapPixelToCoords(windowSpaceMouseOrFingerPos, gameView);

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

        sf::base::vectorEraseIf(delayedActions,
                                [](const auto& delayedAction) { return delayedAction.delayCountdown.isDone(); });

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
        uiDraw(mousePos);

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

        scaledTopGameView                     = createScaledTopGameView(gameScreenSize, resolution);
        scaledTopGameView.viewport.position.x = 0.f;

        {
            const float     scale      = getAspectRatioScalingFactor(gameScreenSize, resolution);
            const sf::Vec2f scaledSize = gameScreenSize * scale;

            gameBackgroundView                     = createScaledGameView(gameScreenSize, scaledSize);
            gameBackgroundView.viewport.position.x = 0.f;
            gameBackgroundView.center              = getViewCenterWithoutScroll() + screenShake;
        }

        //
        // Clear window
        rtGame.clear(outlineHueColor);

        //
        // Underlying menu background
        rtGame.setView(nonScaledHUDView);
        gameLoopUpdateAndDrawFixedMenuBackground(deltaTimeMs, elapsedUs);

        //
        // Game background
        gameLoopUpdateAndDrawBackground(deltaTimeMs);

        //
        // Draw bubbles (separate batch to avoid showing in minimap and for shader support)
        rtGame.setView(gameView);
        bubbleDrawableBatch.clear();
        starBubbleDrawableBatch.clear();
        bombBubbleDrawableBatch.clear();

        if (profile.showBubbles)
            gameLoopDrawBubbles();

        if (profile.useBubbleShader)
            gameLoopDisplayBubblesWithShader();
        else
            gameLoopDisplayBubblesWithoutShader();

        //
        // Combo trail
        gameLoopDrawCursorTrail(mousePos);

        //
        // Draw minimap stuff
        gameLoopDrawMinimapIcons();

        //
        // Draw cats, shrines, dolls, particles, and text particles
        cpuDrawableBatch.clear();
        cpuTopDrawableBatch.clear();
        catTextDrawableBatch.clear();
        catTextTopDrawableBatch.clear();

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
        rtGame.draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame.draw(catTextDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});

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
                                               .size             = dragRect->size});

        //
        // Draw border around gameview
        rtGame.setView(nonScaledHUDView);

        // Bottom-level hud particles
        if (shouldDrawUI)
        {
            hudBottomDrawableBatch.clear();
            gameLoopDrawHUDBottomParticles();
            rtGame.draw(hudBottomDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        }

        rtGame.draw(sf::RectangleShapeData{.position         = gameView.viewport.position.componentWiseMul(resolution),
                                           .fillColor        = sf::Color::Transparent,
                                           .outlineColor     = outlineHueColor,
                                           .outlineThickness = 4.f,
                                           .size             = gameView.viewport.size.componentWiseMul(resolution)});

        rtGame.setView(scaledHUDView);

        if (shouldDrawUI)
        {
            hudDrawableBatch.clear();

            if (!debugHideUI)
                gameLoopDrawHUDParticles();

            gameLoopDrawEarnedCoinParticles();
            rtGame.draw(hudDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        }

        //
        // Y coordinate below minimap to position money, combo, and buff texts
        const float yBelowMinimap = pt->mapPurchased ? (boundaries.y / profile.minimapScale) + 12.f : 0.f;

        //
        // Demo text (TODO P0: cleanup)
        if constexpr (isDemoVersion)
        {
            const float xStartOverlay = getAspectRatioScalingFactor(gameScreenSize, getResolution()) *
                                        gameScreenSize.x / profile.hudScale;

            demoText.setTopRight({xStartOverlay - 15.f, 15.f});
            demoText.setOutlineColor(outlineHueColor);
            rtGame.draw(demoText);

            sf::TextData demoInfoTextData{.position         = {},
                                          .string           = "",
                                          .characterSize    = 24u,
                                          .fillColor        = sf::Color::White,
                                          .outlineColor     = outlineHueColor,
                                          .outlineThickness = 2.f};

            const float lineSpacing = fontSuperBakery.getLineSpacing(demoInfoTextData.characterSize);

            sf::base::StringView lines[3] = {"Only one prestige and two shrines",
                                             "Full version available on Steam",
                                             "Your progress will carry over!"};

            for (sf::base::SizeT i = 0u; i < 3u; ++i)
            {
                demoInfoTextData.string = lines[i].data();
                demoInfoTextData.origin
                    .x = sf::TextUtils::precomputeTextLocalBounds(fontSuperBakery, demoInfoTextData).size.x;
                demoInfoTextData.position = demoText.getBottomRight().addY(10.f + (static_cast<float>(i) * lineSpacing));

                rtGame.draw(fontSuperBakery, demoInfoTextData);
            }
        }

        //
        // Money text & spent money effect
        gameLoopUpdateMoneyText(deltaTimeMs, yBelowMinimap);
        gameLoopUpdateSpentMoneyEffect(deltaTimeMs); // handles both text smoothly doing down and particles
        if (!debugHideUI)
        {
            moneyText.setFillColorAlpha(shouldDrawUIAlpha);
            moneyText.setOutlineColorAlpha(shouldDrawUIAlpha);
            rtGame.draw(moneyText);
        }

        //
        // Combo text
        gameLoopUpdateComboText(deltaTimeMs, yBelowMinimap);
        if (!debugHideUI && pt->comboPurchased)
        {
            comboText.setFillColorAlpha(shouldDrawUIAlpha);
            comboText.setOutlineColorAlpha(shouldDrawUIAlpha);
            rtGame.draw(comboText);
        }

        //
        // Portal storm buff
        if (isDevilcatHellsingedActive() && pt->buffCountdownsPerType[asIdx(CatType::Devil)].value > 0.f)
        {
            if (portalStormTimer.updateAndLoop(deltaTimeMs, 10.f) == CountdownStatusLoop::Looping &&
                rng.getF(0.f, 100.f) <= pt->psvPPDevilRitualBuffPercentage.currentValue())
            {
                const float offset = 64.f;
                const auto portalPos = rng.getVec2f({offset, offset}, {pt->getMapLimit() - offset, boundaries.y - offset});

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
        if (!debugHideUI)
        {
            buffText.setFillColorAlpha(shouldDrawUIAlpha);
            buffText.setOutlineColorAlpha(shouldDrawUIAlpha);
            rtGame.draw(buffText);
        }

        //
        // Combo bar
        if (shouldDrawUI && !debugHideUI)
            rtGame.draw(sf::RectangleShapeData{.position  = {comboText.getCenterRight().x + 3.f, yBelowMinimap + 56.f},
                                               .fillColor = sf::Color{255, 255, 255, 75},
                                               .size      = {100.f * comboCountdown.value / 700.f, 20.f}});

        //
        // Minimap
        if (!debugHideUI && pt->mapPurchased)
        {
            drawMinimap(shader,
                        profile.minimapScale,
                        pt->getMapLimit(),
                        gameView,
                        scaledHUDView,
                        rtGame,
                        txBackgroundChunk,
                        txDrawings,
                        minimapDrawableBatch,
                        textureAtlas,
                        resolution,
                        profile.hudScale,
                        currentBackgroundHue.asDegrees(),
                        shouldDrawUIAlpha,
                        minimapRect);

            // Jump to minimap position on click
            const auto p = window.mapPixelToCoords(windowSpaceMouseOrFingerPos, scaledHUDView);
            if (minimapRect.contains(p) && mBtnDown(sf::Mouse::Button::Left, /* penetrateUI */ true))
            {
                const auto minimapPos = p - minimapRect.position;
                scroll = minimapPos.x * 0.5f * pt->getMapLimit() / minimapRect.size.x - gameScreenSize.x * 0.25f;
            }
        }

        //
        // UI and Toasts
        gameLoopDrawImGui(shouldDrawUIAlpha);

        //
        // Draw cats on top of UI
        rtGame.setView(scaledTopGameView);
        rtGame.draw(cpuTopDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame.draw(catTextTopDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});

        //
        // Purchase unlocked/available effects
        rtGame.setView(nonScaledHUDView);

        if (shouldDrawUI)
            gameLoopUpdatePurchaseUnlockedEffects(deltaTimeMs);

        // Top-level hud particles
        if (shouldDrawUI)
        {
            hudTopDrawableBatch.clear();
            gameLoopDrawHUDTopParticles();
            rtGame.draw(hudTopDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        }

        //
        // High visibility cursor
        rtGame.setView(nonScaledHUDView);
        gameLoopDrawCursor(deltaTimeMs, cursorGrow);
        gameLoopDrawCursorComboText(deltaTimeMs, cursorGrow);
        gameLoopDrawCursorComboBar();

        //
        // Splash screen
        rtGame.setView(scaledHUDView);
        if (splashCountdown.value > 0.f)
            drawSplashScreen(rtGame, txLogo, splashCountdown, resolution, profile.hudScale);

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

                rtGame.draw(sf::Sprite{.position = resolution / 2.f / profile.hudScale,
                                       .scale    = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(progress)) /
                                                profile.hudScale * 2.f,
                                       .origin      = txLetter.getSize().toVec2f() / 2.f,
                                       .textureRect = txLetter.getRect(),
                                       .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(progress) * 255.f))},
                            {.texture = &txLetter});
            }

            (void)cdLetterText.updateAndStop(deltaTimeMs);

            const float textProgress = cdLetterText.value > 9000.f ? remap(cdLetterText.value, 9000.f, 10'000.f, 1.f, 0.f)
                                       : cdLetterText.value < 1000.f ? cdLetterText.value / 1000.f
                                                                     : 1.f;

            rtGame.draw(sf::Sprite{.position = resolution / 2.f / profile.hudScale,
                                   .scale    = sf::Vec2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(textProgress)) /
                                            profile.hudScale * 1.45f,
                                   .origin      = txLetterText.getSize().toVec2f() / 2.f,
                                   .textureRect = txLetterText.getRect(),
                                   .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(textProgress) * 255.f))},
                        {.texture = &txLetterText});
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
        rtGame.display();

        shaderPostProcess.setUniform(suPPVibrance, profile.ppSVibrance);
        shaderPostProcess.setUniform(suPPSaturation, profile.ppSSaturation);
        shaderPostProcess.setUniform(suPPLightness, profile.ppSLightness);
        shaderPostProcess.setUniform(suPPSharpness, profile.ppSSharpness);

        window.setView({window.getSize().toVec2f() / 2.f, window.getSize().toVec2f()});

        window.clear();
        window.draw(rtGame.getTexture(), {.shader = &shaderPostProcess});
        window.display();

        //
        // Save last mouse pos
        lastMousePos = mousePos;

        return true;
    }

    ////////////////////////////////////////////////////////////
    void loadPlaythroughFromFileAndReseed()
    {
        const sf::base::StringView loadMessage = loadPlaythroughFromFile(ptMain, "userdata/playthrough.json");

        if (!loadMessage.empty())
            pushNotification("Playthrough loading info", "%s", loadMessage.data());

        rng.reseed(pt->seed);
        shuffledCatNamesPerType = makeShuffledCatNames(rng);
    }

////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    Main(hg::Steam::SteamManager& xSteamMgr) : steamMgr(xSteamMgr), onSteamDeck(steamMgr.isOnSteamDeck())
#else
    Main() : onSteamDeck(false)
#endif
    {
        sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

        if (onSteamDeck)
        {
            // borderless windowed
            profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
            profile.windowed = true;

            profile.uiScale = 1.25f;
        }

        //
        // Playthrough
        if (sf::Path{"userdata/playthrough.json"}.exists())
        {
            loadPlaythroughFromFileAndReseed();
            sf::cOut() << "Loaded playthrough from file on startup\n";
        }
        else
        {
            pt->seed = rng.getSeed();
        }

        //
        // Reserve memory
        particles.reserve(512);
        spentCoinParticles.reserve(512);
        textParticles.reserve(256);
        pt->bubbles.reserve(32'768);
        pt->cats.reserve(512);

        //
        // Touch state
        fingerPositions.resize(10);
    }

    ////////////////////////////////////////////////////////////
    void run()
    {
        //
        // Startup (splash screen and meow)
        splashCountdown.restart();
        playSound(sounds.byteMeow);

        //
        //
        // Background music
        auto& [entries, selectedIndex] = getBGMSelectorData();
        selectBGM(entries, selectedIndex);
        switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ true);

        //
        // Game loop
        playedClock.start();

        while (true)
            if (!gameLoop())
                return;
    }
};

#pragma GCC diagnostic pop
