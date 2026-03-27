#pragma once

#include "CatConstants.hpp"


// #define BUBBLEBYTE_NO_AUDIO 1
#include "Achievements.hpp"
#include "Aliases.hpp"
#include "Bubble.hpp"
#include "BubbleType.hpp"
#include "Cat.hpp"
#include "CatNames.hpp"
#include "CatType.hpp"
#include "Collision.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "Doll.hpp"
#include "ExactArray.hpp"
#include "IconsFontAwesome6.h"
#include "InputHelper.hpp"
#include "MemberGuard.hpp"
#include "Particle.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "RNGSeedType.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "ShrineType.hpp"
#include "Sounds.hpp"
#include "Stats.hpp"
#include "SweepAndPrune.hpp"
#include "TextEffectWiggle.hpp"
#include "TextParticle.hpp"
#include "TextShakeEffect.hpp"
#include "Version.hpp"

#include "ExampleUtils/ControlFlow.hpp"
#include "ExampleUtils/HueColor.hpp"
#include "ExampleUtils/LoadedSound.hpp"
#include "ExampleUtils/MathUtils.hpp"
#include "ExampleUtils/RNGFast.hpp"
#include "ExampleUtils/Sampler.hpp"
#include "ExampleUtils/SoundManager.hpp"
#include "ExampleUtils/Timer.hpp"

// clang-format off
#define SFEX_PROFILER_ENABLED
// clang-format on

#include "SFML/ImGui/ImGuiContext.hpp"
#include "SFML/ImGui/IncludeImGui.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawTextureSettings.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextUtils.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Array.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/FloatMax.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ThreadPool.hpp"
#include "SFML/Base/Vector.hpp"

#include <cstdarg>
#include <cstdio>
#include <ctime>


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    #include "Steam.hpp"
#endif


////////////////////////////////////////////////////////////<
extern bool debugMode;

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
[[nodiscard, gnu::always_inline]] inline Bubble makeRandomBubble(Playthrough& pt, RNGFast& rng, const float mapLimit, const float maxY)
{
    return {
        .position = rng.getVec2f({mapLimit, maxY}),
        .velocity = rng.getVec2f({-0.1f, -0.1f}, {0.1f, 0.1f}),

        .radius   = rng.getF(0.07f, 0.16f) * 256.f *
                    remap(static_cast<float>(pt.psvBubbleCount.nPurchases), 0.f, 30.f, 1.1f, 0.8f),
        .rotation = 0.f,
        .torque   = 0.f,
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
    sf::Shader::UniformLocation suBackgroundOrigin  = shader.getUniformLocation("u_backgroundOrigin").value();
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
    sf::Shader::UniformLocation suPPBlur       = shaderPostProcess.getUniformLocation("u_blur").value();

    ////////////////////////////////////////////////////////////
    // Shader for fluffy cat cloud rendering
    sf::Shader shaderClouds{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/clouds.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    sf::Shader::UniformLocation suCloudTime       = shaderClouds.getUniformLocation("u_time").value();
    sf::Shader::UniformLocation suCloudResolution = shaderClouds.getUniformLocation("u_resolution").value();

    ////////////////////////////////////////////////////////////
    // Context settings
    const unsigned int aaLevel = sf::base::min(16u, sf::RenderTexture::getMaximumAntiAliasingLevel());

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
    [[nodiscard]] sf::RenderWindow makeWindow();
    sf::RenderWindow               window{makeWindow()};
    float                          dpiScalingFactor = 1.f;

    ////////////////////////////////////////////////////////////
    // Views


    ////////////////////////////////////////////////////////////
    void refreshWindowAutoBatchModeFromProfile() // TODO P1: check if this solves flickering
    {
        window.setAutoBatchMode(profile.autobatchMode == 0   ? sf::RenderTarget::AutoBatchMode::Disabled
                                : profile.autobatchMode == 1 ? sf::RenderTarget::AutoBatchMode::CPUStorage
                                                             : sf::RenderTarget::AutoBatchMode::GPUStorage);
    }

    ////////////////////////////////////////////////////////////
    bool flushAfterEveryBatch  = false; // TODO P1: check if this solves flickering
    bool finishAfterEveryBatch = false; // TODO P1: check if this solves flickering
    bool flushBeforeDisplay    = false; // TODO P1: check if this solves flickering
    bool finishBeforeDisplay   = false; // TODO P1: check if this solves flickering
    bool flushAfterDisplay     = false; // TODO P1: check if this solves flickering
    bool finishAfterDisplay    = false; // TODO P1: check if this solves flickering

    ////////////////////////////////////////////////////////////
    bool loadingGuard{[&]
    {
        refreshWindowAutoBatchModeFromProfile();
        window.clear(sf::Color::Black);

        sf::TextData loadingTextData{.position         = window.getSize().toVec2f() / 2.f,
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
    sf::TextureAtlas textureAtlas{sf::Texture::create({6000u, 4096u}, {.smooth = true}).value()}; // TODO P0: make smaller

    ////////////////////////////////////////////////////////////
    // SFML fonts
    sf::Font fontSuperBakery{sf::Font::openFromFile("resources/superbakery.ttf", &textureAtlas).value()};

    ////////////////////////////////////////////////////////////
    // ImGui fonts
    ImFont* fontImGuiMouldyCheese{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/mouldycheese.ttf", 26.f)};
    ImFont* fontImGuiSuperBakery{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/superbakery.ttf", 26.f)};
    ImFont* fontImGuiFA{[]
    {
        static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};

        ImFontConfig iconConfig;
        iconConfig.MergeMode        = true;
        iconConfig.PixelSnapH       = true;
        iconConfig.GlyphMinAdvanceX = 18.0f; // Helps keep icons square
        iconConfig.GlyphOffset.y    = -1.0f; // Moves icons down 2 pixels

        // 3. Load FontAwesome into the SAME font object
        auto* res = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/fa-solid-900.ttf", 16.0f, &iconConfig, iconRanges);
        SFML_BASE_ASSERT(res != nullptr);

        ImGui::GetIO().Fonts->Build();
        return res;
    }()};

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
    Sounds sounds{/* volumeMult */ 1.f};

    SoundManager soundManager;
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
        sf::RenderTexture::create(gameScreenSize.toVec2u(),
                                  {.antiAliasingLevel = aaLevel, .smooth = true, .wrapMode = sf::TextureWrapMode::Repeat})
            .value()};

    sf::RenderTexture rtBackgroundProcessed{
        sf::RenderTexture::create(gameScreenSize.toVec2u(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()};

    sf::RenderTexture rtImGui{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()};

    sf::RenderTexture rtCloudMask{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()};

    sf::RenderTexture rtCloudProcessed{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // Game render texture (before post-processing)
    sf::RenderTexture rtGame{
        sf::RenderTexture::create(window.getSize(), {.antiAliasingLevel = aaLevel, .smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // Textures (not in atlas)
    static inline constexpr sf::TextureLoadSettings bgSettings{.smooth = true, .wrapMode = sf::TextureWrapMode::Repeat};

    sf::Texture txLogo{sf::Texture::loadFromFile("resources/logo.png", {.smooth = true}).value()};
    sf::Texture txFixedBg{
        sf::Texture::loadFromFile("resources/fixedbg.png", {.smooth = true, .wrapMode = sf::TextureWrapMode::MirroredRepeat})
            .value()};
    sf::Texture txBackgroundChunk{sf::Texture::loadFromFile("resources/bgtest.png", bgSettings).value()};
    sf::Texture txBackgroundChunkDesaturated{
        sf::Texture::loadFromFile("resources/backgroundchunkdesaturated.png", bgSettings).value()};
    sf::Texture txClouds{sf::Texture::loadFromFile("resources/clouds.png", bgSettings).value()};
    sf::Texture txTintedClouds{sf::Texture::loadFromFile("resources/tintedclouds.png", bgSettings).value()};
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
    sf::Texture txFrame{sf::Texture::loadFromFile("resources/frame.png", {.smooth = true}).value()};
    sf::Texture txFrameTiny{sf::Texture::loadFromFile("resources/frametiny.png", {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // UI texture atlas
    sf::TextureAtlas uiTextureAtlas{sf::Texture::create({2048u, 1024u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    // Quick toolbar icons
    sf::Rect2f txrIconVolume{addImgResourceToUIAtlas("iconvolumeon.png")};
    sf::Rect2f txrIconBGM{addImgResourceToUIAtlas("iconmusicon.png")};
    sf::Rect2f txrIconBg{addImgResourceToUIAtlas("iconbg.png")};
    sf::Rect2f txrIconCfg{addImgResourceToUIAtlas("iconcfg.png")};
    sf::Rect2f txrIconCopyCat{addImgResourceToUIAtlas("iconcopycat.png")};

    ////////////////////////////////////////////////////////////
    // Shop menu separator textures
    sf::Rect2f txrMenuSeparator0{addImgResourceToUIAtlas("menuseparator0.png")};
    sf::Rect2f txrMenuSeparator1{addImgResourceToUIAtlas("menuseparator1.png")};
    sf::Rect2f txrMenuSeparator2{addImgResourceToUIAtlas("menuseparator2.png")};
    sf::Rect2f txrMenuSeparator3{addImgResourceToUIAtlas("menuseparator3.png")};
    sf::Rect2f txrMenuSeparator4{addImgResourceToUIAtlas("menuseparator4.png")};
    sf::Rect2f txrMenuSeparator5{addImgResourceToUIAtlas("menuseparator5.png")};
    sf::Rect2f txrMenuSeparator6{addImgResourceToUIAtlas("menuseparator6.png")};
    sf::Rect2f txrMenuSeparator7{addImgResourceToUIAtlas("menuseparator7.png")};
    sf::Rect2f txrMenuSeparator8{addImgResourceToUIAtlas("menuseparator8.png")};

    ////////////////////////////////////////////////////////////
    // Prestige menu separator textures
    sf::Rect2f txrPrestigeSeparator0{addImgResourceToUIAtlas("prestigeseparator0.png")};
    sf::Rect2f txrPrestigeSeparator1{addImgResourceToUIAtlas("prestigeseparator1.png")};
    sf::Rect2f txrPrestigeSeparator2{addImgResourceToUIAtlas("prestigeseparator2.png")};
    sf::Rect2f txrPrestigeSeparator3{addImgResourceToUIAtlas("prestigeseparator3.png")};
    sf::Rect2f txrPrestigeSeparator4{addImgResourceToUIAtlas("prestigeseparator4.png")};
    sf::Rect2f txrPrestigeSeparator5{addImgResourceToUIAtlas("prestigeseparator5.png")};
    sf::Rect2f txrPrestigeSeparator6{addImgResourceToUIAtlas("prestigeseparator6.png")};
    sf::Rect2f txrPrestigeSeparator7{addImgResourceToUIAtlas("prestigeseparator7.png")};
    sf::Rect2f txrPrestigeSeparator8{addImgResourceToUIAtlas("prestigeseparator8.png")};
    sf::Rect2f txrPrestigeSeparator9{addImgResourceToUIAtlas("prestigeseparator9.png")};
    sf::Rect2f txrPrestigeSeparator10{addImgResourceToUIAtlas("prestigeseparator10.png")};
    sf::Rect2f txrPrestigeSeparator11{addImgResourceToUIAtlas("prestigeseparator11.png")};
    sf::Rect2f txrPrestigeSeparator12{addImgResourceToUIAtlas("prestigeseparator12.png")};
    sf::Rect2f txrPrestigeSeparator13{addImgResourceToUIAtlas("prestigeseparator13.png")};
    sf::Rect2f txrPrestigeSeparator14{addImgResourceToUIAtlas("prestigeseparator14.png")};
    sf::Rect2f txrPrestigeSeparator15{addImgResourceToUIAtlas("prestigeseparator15.png")};

    ////////////////////////////////////////////////////////////
    // Magic menu separator textures
    sf::Rect2f txrMagicSeparator0{addImgResourceToUIAtlas("magicseparator0.png")};
    sf::Rect2f txrMagicSeparator1{addImgResourceToUIAtlas("magicseparator1.png")};
    sf::Rect2f txrMagicSeparator2{addImgResourceToUIAtlas("magicseparator2.png")};
    sf::Rect2f txrMagicSeparator3{addImgResourceToUIAtlas("magicseparator3.png")};

    ////////////////////////////////////////////////////////////
    // Background hues
    static inline constexpr EXACT_ARRAY(
        float,
        backgroundHues,
        nShrineTypes + 1u,
        {
            0.f,    // Normal
            -140.f, // Voodoo
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
    sf::Rect2f txrWhiteDot{textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value()};
    sf::Rect2f txrBubble{addImgResourceToAtlas("bubble2.png")};
    sf::Rect2f txrBubbleStar{addImgResourceToAtlas("bubble3.png")};
    sf::Rect2f txrBubbleNova{addImgResourceToAtlas("bubble4.png")};
    sf::Rect2f txrCat{addImgResourceToAtlas("cat.png")};
    sf::Rect2f txrUniCat{addImgResourceToAtlas("unicat3.png")};
    sf::Rect2f txrUniCat2{addImgResourceToAtlas("unicat2.png")};
    sf::Rect2f txrUniCatWings{addImgResourceToAtlas("unicatwings.png")};
    sf::Rect2f txrDevilCat2{addImgResourceToAtlas("devilcat2.png")};
    sf::Rect2f txrDevilCat3{addImgResourceToAtlas("devilcat3.png")};
    sf::Rect2f txrDevilCat3Arm{addImgResourceToAtlas("devilcat3arm.png")};
    sf::Rect2f txrDevilCat3Book{addImgResourceToAtlas("devilcat3book.png")};
    sf::Rect2f txrDevilCat3Tail{addImgResourceToAtlas("devilcat3tail.png")};
    sf::Rect2f txrDevilCat2Book{addImgResourceToAtlas("devilcat2book.png")};
    sf::Rect2f txrCatPaw{addImgResourceToAtlas("catpaw.png")};
    sf::Rect2f txrCatTail{addImgResourceToAtlas("cattail.png")};
    sf::Rect2f txrSmartCatHat{addImgResourceToAtlas("smartcathat.png")};
    sf::Rect2f txrSmartCatDiploma{addImgResourceToAtlas("smartcatdiploma.png")};
    sf::Rect2f txrBrainBack{addImgResourceToAtlas("brainback.png")};
    sf::Rect2f txrBrainFront{addImgResourceToAtlas("brainfront.png")};
    sf::Rect2f txrUniCatTail{addImgResourceToAtlas("unicattail.png")};
    sf::Rect2f txrUniCat2Tail{addImgResourceToAtlas("unicat2tail.png")};
    sf::Rect2f txrDevilCatTail2{addImgResourceToAtlas("devilcattail2.png")};
    sf::Rect2f txrAstroCatTail{addImgResourceToAtlas("astrocattail.png")};
    sf::Rect2f txrAstroCatFlag{addImgResourceToAtlas("astrocatflag.png")};
    sf::Rect2f txrWitchCatTail{addImgResourceToAtlas("witchcattail.png")};
    sf::Rect2f txrWizardCatTail{addImgResourceToAtlas("wizardcattail.png")};
    sf::Rect2f txrMouseCatTail{addImgResourceToAtlas("mousecattail.png")};
    sf::Rect2f txrMouseCatMouse{addImgResourceToAtlas("mousecatmouse.png")};
    sf::Rect2f txrEngiCatTail{addImgResourceToAtlas("engicattail.png")};
    sf::Rect2f txrEngiCatWrench{addImgResourceToAtlas("engicatwrench.png")};
    sf::Rect2f txrRepulsoCatTail{addImgResourceToAtlas("repulsocattail.png")};
    sf::Rect2f txrAttractoCatTail{addImgResourceToAtlas("attractocattail.png")};
    sf::Rect2f txrCopyCatTail{addImgResourceToAtlas("copycattail.png")};
    sf::Rect2f txrAttractoCatMagnet{addImgResourceToAtlas("attractocatmagnet.png")};
    sf::Rect2f txrUniCatPaw{addImgResourceToAtlas("unicatpaw.png")};
    sf::Rect2f txrDevilCatPaw{addImgResourceToAtlas("devilcatpaw.png")};
    sf::Rect2f txrDevilCatPaw2{addImgResourceToAtlas("devilcatpaw2.png")};
    sf::Rect2f txrParticle{addImgResourceToAtlas("particle.png")};
    sf::Rect2f txrStarParticle{addImgResourceToAtlas("starparticle.png")};
    sf::Rect2f txrFireParticle{addImgResourceToAtlas("fireparticle.png")};
    sf::Rect2f txrFireParticle2{addImgResourceToAtlas("fireparticle2.png")};
    sf::Rect2f txrSmokeParticle{addImgResourceToAtlas("smokeparticle.png")};
    sf::Rect2f txrExplosionParticle{addImgResourceToAtlas("explosionparticle.png")};
    sf::Rect2f txrTrailParticle{addImgResourceToAtlas("trailparticle.png")};
    sf::Rect2f txrHexParticle{addImgResourceToAtlas("hexparticle.png")};
    sf::Rect2f txrShrineParticle{addImgResourceToAtlas("shrineparticle.png")};
    sf::Rect2f txrCogParticle{addImgResourceToAtlas("cogparticle.png")};
    sf::Rect2f txrWitchCat{addImgResourceToAtlas("witchcat.png")};
    sf::Rect2f txrWitchCatPaw{addImgResourceToAtlas("witchcatpaw.png")};
    sf::Rect2f txrAstroCat{addImgResourceToAtlas("astromeow.png")};
    sf::Rect2f txrBomb{addImgResourceToAtlas("bomb.png")};
    sf::Rect2f txrShrine{addImgResourceToAtlas("shrine.png")};
    sf::Rect2f txrWizardCat{addImgResourceToAtlas("wizardcat.png")};
    sf::Rect2f txrWizardCatPaw{addImgResourceToAtlas("wizardcatpaw.png")};
    sf::Rect2f txrMouseCat{addImgResourceToAtlas("mousecat.png")};
    sf::Rect2f txrMouseCatPaw{addImgResourceToAtlas("mousecatpaw.png")};
    sf::Rect2f txrEngiCat{addImgResourceToAtlas("engicat.png")};
    sf::Rect2f txrEngiCatPaw{addImgResourceToAtlas("engicatpaw.png")};
    sf::Rect2f txrRepulsoCat{addImgResourceToAtlas("repulsocat.png")};
    sf::Rect2f txrRepulsoCatPaw{addImgResourceToAtlas("repulsocatpaw.png")};
    sf::Rect2f txrAttractoCat{addImgResourceToAtlas("attractocat.png")};
    sf::Rect2f txrCopyCat{addImgResourceToAtlas("copycat.png")};
    sf::Rect2f txrDuckCat{addImgResourceToAtlas("duck.png")};
    sf::Rect2f txrDuckFlag{addImgResourceToAtlas("duckflag.png")};
    sf::Rect2f txrAttractoCatPaw{addImgResourceToAtlas("attractocatpaw.png")};
    sf::Rect2f txrCopyCatPaw{addImgResourceToAtlas("copycatpaw.png")};
    sf::Rect2f txrDollNormal{addImgResourceToAtlas("dollnormal.png")};
    sf::Rect2f txrDollUni{addImgResourceToAtlas("dolluni.png")};
    sf::Rect2f txrDollDevil{addImgResourceToAtlas("dolldevil.png")};
    sf::Rect2f txrDollAstro{addImgResourceToAtlas("dollastro.png")};
    sf::Rect2f txrDollWizard{addImgResourceToAtlas("dollwizard.png")};
    sf::Rect2f txrDollMouse{addImgResourceToAtlas("dollmouse.png")};
    sf::Rect2f txrDollEngi{addImgResourceToAtlas("dollengi.png")};
    sf::Rect2f txrDollRepulso{addImgResourceToAtlas("dollrepulso.png")};
    sf::Rect2f txrDollAttracto{addImgResourceToAtlas("dollattracto.png")};
    sf::Rect2f txrCoin{addImgResourceToAtlas("bytecoin.png")};
    sf::Rect2f txrCatSoul{addImgResourceToAtlas("catsoul.png")};
    sf::Rect2f txrHellPortal{addImgResourceToAtlas("hellportal.png")};
    sf::Rect2f txrCatEyeLid0{addImgResourceToAtlas("cateyelid0.png")};
    sf::Rect2f txrCatEyeLid1{addImgResourceToAtlas("cateyelid1.png")};
    sf::Rect2f txrCatEyeLid2{addImgResourceToAtlas("cateyelid2.png")};
    sf::Rect2f txrCatWhiteEyeLid0{addImgResourceToAtlas("catwhiteeyelid0.png")};
    sf::Rect2f txrCatWhiteEyeLid1{addImgResourceToAtlas("catwhiteeyelid1.png")};
    sf::Rect2f txrCatWhiteEyeLid2{addImgResourceToAtlas("catwhiteeyelid2.png")};
    sf::Rect2f txrCatDarkEyeLid0{addImgResourceToAtlas("catdarkeyelid0.png")};
    sf::Rect2f txrCatDarkEyeLid1{addImgResourceToAtlas("catdarkeyelid1.png")};
    sf::Rect2f txrCatDarkEyeLid2{addImgResourceToAtlas("catdarkeyelid2.png")};
    sf::Rect2f txrCatGrayEyeLid0{addImgResourceToAtlas("catgrayeyelid0.png")};
    sf::Rect2f txrCatGrayEyeLid1{addImgResourceToAtlas("catgrayeyelid1.png")};
    sf::Rect2f txrCatGrayEyeLid2{addImgResourceToAtlas("catgrayeyelid2.png")};
    sf::Rect2f txrCatEars0{addImgResourceToAtlas("catears0.png")};
    sf::Rect2f txrCatEars1{addImgResourceToAtlas("catears1.png")};
    sf::Rect2f txrCatEars2{addImgResourceToAtlas("catears2.png")};
    sf::Rect2f txrCatYawn0{addImgResourceToAtlas("catyawn0.png")};
    sf::Rect2f txrCatYawn1{addImgResourceToAtlas("catyawn1.png")};
    sf::Rect2f txrCatYawn2{addImgResourceToAtlas("catyawn2.png")};
    sf::Rect2f txrCatYawn3{addImgResourceToAtlas("catyawn3.png")};
    sf::Rect2f txrCatYawn4{addImgResourceToAtlas("catyawn4.png")};
    sf::Rect2f txrCCMaskWitch{addImgResourceToAtlas("ccmaskwitch.png")};
    sf::Rect2f txrCCMaskWizard{addImgResourceToAtlas("ccmaskwizard.png")};
    sf::Rect2f txrCCMaskMouse{addImgResourceToAtlas("ccmaskmouse.png")};
    sf::Rect2f txrCCMaskEngi{addImgResourceToAtlas("ccmaskengi.png")};
    sf::Rect2f txrCCMaskRepulso{addImgResourceToAtlas("ccmaskrepulso.png")};
    sf::Rect2f txrCCMaskAttracto{addImgResourceToAtlas("ccmaskattracto.png")};
    sf::Rect2f txrMMNormal{addImgResourceToAtlas("mmcatnormal.png")};
    sf::Rect2f txrMMUni{addImgResourceToAtlas("mmcatuni.png")};
    sf::Rect2f txrMMDevil{addImgResourceToAtlas("mmcatdevil.png")};
    sf::Rect2f txrMMAstro{addImgResourceToAtlas("mmcatastro.png")};
    sf::Rect2f txrMMWitch{addImgResourceToAtlas("mmcatwitch.png")};
    sf::Rect2f txrMMWizard{addImgResourceToAtlas("mmcatwizard.png")};
    sf::Rect2f txrMMMouse{addImgResourceToAtlas("mmcatmouse.png")};
    sf::Rect2f txrMMEngi{addImgResourceToAtlas("mmcatengi.png")};
    sf::Rect2f txrMMRepulso{addImgResourceToAtlas("mmcatrepulso.png")};
    sf::Rect2f txrMMAttracto{addImgResourceToAtlas("mmcatattracto.png")};
    sf::Rect2f txrMMCopy{addImgResourceToAtlas("mmcatcopy.png")};
    sf::Rect2f txrMMDuck{addImgResourceToAtlas("mmduck.png")};
    sf::Rect2f txrMMShrine{addImgResourceToAtlas("mmshrine.png")};
    sf::Rect2f txrCloud{addImgResourceToAtlas("cloud.png")};

    ////////////////////////////////////////////////////////////
    // Cat animation rects: eye blinking
    const sf::Rect2f* eyeLidRects[8]{
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
    const sf::Rect2f* whiteEyeLidRects[8]{
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
    const sf::Rect2f* darkEyeLidRects[8]{
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
    const sf::Rect2f* grayEyeLidRects[8]{
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
    const sf::Rect2f* earRects[8]{
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
    const sf::Rect2f* catYawnRects[14]{
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
    const sf::Rect2f particleRects[nParticleTypes] = {
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
        Timer     progress{};
    };

    sf::base::Vector<EarnedCoinParticle> earnedCoinParticles; // HUD space

    ////////////////////////////////////////////////////////////
    // Random number generation
    RNGSeedType seed{static_cast<RNGSeedType>(sf::Clock::now().asMicroseconds())};
    RNGFast     rng{seed};
    RNGFast     rngFast{seed}; // very fast, low-quality, but good enough for VFXs

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
    float         fixedBgSlideTarget = 0.f;
    float         fixedBgSlide       = 0.f;

    ////////////////////////////////////////////////////////////
    // FPS and delta time clocks
    sf::Clock fpsClock;
    sf::Clock deltaClock;

    ////////////////////////////////////////////////////////////
    // Batches for drawing
    sf::CPUDrawableBatch bubbleDrawableBatch;
    sf::CPUDrawableBatch starBubbleDrawableBatch;
    sf::CPUDrawableBatch bombBubbleDrawableBatch;
    sf::CPUDrawableBatch cpuCloudDrawableBatch;
    sf::CPUDrawableBatch cpuTopCloudDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatchAdditive;
    sf::CPUDrawableBatch minimapDrawableBatch;
    sf::CPUDrawableBatch catTextDrawableBatch;
    sf::CPUDrawableBatch hudDrawableBatch;
    sf::CPUDrawableBatch hudTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch hudBottomDrawableBatch;  // drawn below ImGui
    sf::CPUDrawableBatch cpuTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch catTextTopDrawableBatch; // drawn on top of ImGui

    ////////////////////////////////////////////////////////////
    void drawBatch(const sf::CPUDrawableBatch& batch, const sf::RenderStates& states)
    {
        rtGame.draw(batch, states);

        if (flushAfterEveryBatch)
            rtGame.flushGPUCommands();

        if (finishAfterEveryBatch)
            rtGame.finishGPUCommands();
    }

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
    TargetedCountdown splashCountdown{.startingValue = 1.f}; // TODO P0: revert to 2500

    ////////////////////////////////////////////////////////////
    // Tip state
    OptionalTargetedCountdown tipTCByte;
    OptionalTargetedCountdown tipTCBackground;
    OptionalTargetedCountdown tipTCBytePreEnd;
    OptionalTargetedCountdown tipTCByteEnd;
    OptionalTargetedCountdown tipTCBackgroundEnd;
    Countdown                 tipCountdownChar;
    sf::base::String          tipString;
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
    sf::View gameView{{1.f, 1.f}, {1.f, 1.f}};         // TODO P1: compute on the fly, don't cache...
    sf::View nonScaledHUDView{{1.f, 1.f}, {1.f, 1.f}}; // TODO P1: compute on the fly, don't cache...
    sf::View scaledHUDView{{1.f, 1.f}, {1.f, 1.f}};    // TODO P1: compute on the fly, don't cache...

    ////////////////////////////////////////////////////////////
    // Scroll arrow hint
    Countdown scrollArrowCountdown;

    ////////////////////////////////////////////////////////////
    // $ps sampler
    MoneyType      moneyGainedLastSecond{0u};
    Sampler<float> samplerMoneyPerSecond{/* capacity */ 60u};
    sf::base::I64  moneyGainedUsAccumulator{0};

    ////////////////////////////////////////////////////////////
    // Bomb-cat tracker for money earned
    ankerl::unordered_dense::map<sf::base::SizeT, sf::base::SizeT> bombIdxToCatIdx;

    ////////////////////////////////////////////////////////////
    // Notification queue
    struct NotificationData
    {
        const char*      title;
        sf::base::String content;
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
        sf::base::String widgetLabel;
        Countdown        countdown;
        Countdown        arrowCountdown;
        float            hue;
        int              type;
    };

    sf::base::Vector<PurchaseUnlockedEffect>             purchaseUnlockedEffects;
    ankerl::unordered_dense::map<sf::base::String, bool> btnWasDisabled;

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
    sf::Rect2f minimapRect;

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
    void log(const char* format, ...) // NOLINT(modernize-avoid-variadic-functions)
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
    [[nodiscard]] static sf::base::Vector<sf::base::Vector<sf::base::StringView>> makeShuffledCatNames(RNGFast& rng)
    {
        sf::base::Vector<sf::base::Vector<sf::base::StringView>> result(nCatTypes);

        for (SizeT i = 0u; i < nCatTypes; ++i)
            result[i] = getShuffledCatNames(static_cast<CatType>(i), rng);

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
    [[nodiscard]] sf::Rect2f addImgResourceToAtlas(const sf::Path& path)
    {
        return textureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f addImgResourceToUIAtlas(const sf::Path& path)
    {
        return uiTextureAtlas.add(sf::Image::loadFromFile("resources" / path).value()).value();
    }

    ////////////////////////////////////////////////////////////
    void playSound(const LoadedSound& ls, const sf::base::SizeT maxOverlap = 255u)
    {
#ifndef BUBBLEBYTE_NO_AUDIO
        soundManager.playPooled(playbackDevice, ls, maxOverlap);
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
        return pickRandomBubbleInRadiusMatching(center, radius, [](const Bubble&) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN {
            return true;
        });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getResolution() const
    {
        return window.getSize().toVec2f();
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
    [[nodiscard]] sf::Vec2f getViewCenter() const
    {
        const sf::Vec2f currentViewSize = getCurrentGameViewSize();
        return {clampGameViewCenterX(currentViewSize.x / 2.f + actualScroll * 2.f, currentViewSize.x),
                currentViewSize.y / 2.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getViewCenterWithoutScroll() const
    {
        const sf::Vec2f currentViewSize = getCurrentGameViewSize();
        return {clampGameViewCenterX(currentViewSize.x / 2.f, currentViewSize.x), currentViewSize.y / 2.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CullingBoundaries getViewCullingBoundaries(const float offset) const
    {
        const sf::Vec2f viewSize{getCurrentGameViewSize()};
        const sf::Vec2f viewCenter{getViewCenter()};

        return {viewCenter.x - viewSize.x / 2.f + offset,
                viewCenter.x + viewSize.x / 2.f - offset,
                viewCenter.y - viewSize.y / 2.f + offset,
                viewCenter.y + viewSize.y / 2.f - offset};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::Vec2f getCatRangeCenter(const Cat& cat)
    {
        return cat.position + CatConstants::rangeOffsets[asIdx(cat.type)];
    }
    [[nodiscard]] float                                               getWindRepulsionMult() const;
    [[nodiscard]] float                                               getWindAttractionMult() const;
    [[nodiscard]] static constexpr CatType                            shrineTypeToCatType(ShrineType shrineType);
    [[nodiscard, gnu::always_inline, gnu::flatten]] inline sf::Sprite particleToSprite(const Particle& particle) const
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
    [[nodiscard]] sf::Vec2f getEdgeSpawnPosition(const sf::Rect2f& bounds, float thickness);
    [[nodiscard]] sf::Vec2u getNewResolution() const;

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
        const auto pos = gameView.screenToWorld(getResolution() / 2.f, window.getSize().toVec2f());

        Cat& newCat = spawnCat(pos, catType, hue);

        if (placeInHand)
        {
            catToPlace      = &newCat;
            newCat.dragTime = 1000.f;

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
    void doTip(const sf::base::String& str, const SizeT maxPrestigeLevel = 0u)
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
    [[nodiscard]] float getAspectRatioScalingFactor(const sf::Vec2f originalSize, const sf::Vec2f windowSize) const
    {
        // Calculate the scale factors for both dimensions
        const float scaleX = windowSize.x / originalSize.x;
        const float scaleY = windowSize.y / originalSize.y;

        // Use the smaller scale factor to maintain aspect ratio
        return sf::base::min(scaleX, scaleY);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getCappedGameViewAspectRatio(const sf::Vec2f originalSize, const sf::Vec2f windowSize) const
    {
        const float originalAspect       = originalSize.x / originalSize.y;
        const float windowAspect         = windowSize.x / windowSize.y;
        const float configuredMaxAspect  = sf::base::max(maxGameViewAspectRatio, originalAspect);
        const float unlockedMapAspect    = pt != nullptr ? pt->getMapLimit() / originalSize.y : originalAspect;
        const float clampedMaxViewAspect = sf::base::min(configuredMaxAspect, unlockedMapAspect);

        return sf::base::clamp(windowAspect, originalAspect, clampedMaxViewAspect);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getExpandedGameViewSize(const sf::Vec2f originalSize, const sf::Vec2f windowSize) const
    {
        return {originalSize.y * getCappedGameViewAspectRatio(originalSize, windowSize), originalSize.y};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledGameView(const sf::Vec2f originalSize, const sf::Vec2f windowSize) const
    {
        const float     originalAspect = originalSize.x / originalSize.y;
        const float     windowAspect   = windowSize.x / windowSize.y;
        const float     viewAspect     = getCappedGameViewAspectRatio(originalSize, windowSize);
        const sf::Vec2f expandedSize   = getExpandedGameViewSize(originalSize, windowSize);
        const float     viewportWidth  = viewAspect < windowAspect ? viewAspect / windowAspect : 1.f;
        const float     viewportHeight = windowAspect < originalAspect ? windowAspect / originalAspect : 1.f;

        return {.center   = originalSize / 2.f,
                .size     = expandedSize,
                .viewport = {{0.f, (1.f - viewportHeight) * 0.5f}, {viewportWidth, viewportHeight}}};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledTopGameView(const sf::Vec2f originalSize, const sf::Vec2f windowSize) const
    {
        const sf::View scaledGameView = createScaledGameView(originalSize, windowSize);

        return {.center   = scaledGameView.center,
                .size     = scaledGameView.size.componentWiseDiv(scaledGameView.viewport.size),
                .viewport = {{0.f, 0.f}, {1.f, 1.f}}};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getCurrentGameViewSize() const
    {
        return getExpandedGameViewSize(gameScreenSize, getResolution());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float clampGameViewCenterX(const float desiredCenterX, const float viewWidth) const
    {
        if (viewWidth >= boundaries.x)
            return boundaries.x / 2.f;

        const float halfWidth = viewWidth / 2.f;
        return sf::base::clamp(desiredCenterX, halfWidth, boundaries.x - halfWidth);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View makeScaledHUDView(const sf::Vec2f resolution, float scale) const
    {
        return {.center = {resolution.x / (2.f * scale), resolution.y / (2.f * scale)}, .size = resolution / scale};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getHUDMousePos() const
    {
        return nonScaledHUDView.screenToWorld(sf::Mouse::getPosition(window).toVec2f(), window.getSize().toVec2f());
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
    static inline constexpr float uiNormalFontScale      = 0.95f;
    static inline constexpr float uiSubBulletFontScale   = 0.75f;
    static inline constexpr float uiToolTipFontScale     = 0.65f;
    static inline constexpr float uiWindowWidth          = 425.f;
    static inline constexpr float uiButtonWidth          = 150.f;
    static inline constexpr float uiTooltipWidth         = uiWindowWidth;
    static inline constexpr float maxGameViewAspectRatio = 21.f / 9.f;

    ////////////////////////////////////////////////////////////
    char         uiBuffer[256]{};
    char         uiLabelBuffer[512]{};
    char         uiTooltipBuffer[1024]{};
    float        uiButtonHueMod = 0.f;
    unsigned int uiWidgetId     = 0u;
    float        lastFontScale  = 1.f;

    ////////////////////////////////////////////////////////////
    ankerl::unordered_dense::map<sf::base::String, float> uiLabelToY;

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
    void                    uiImageFromAtlas(const sf::Rect2f& txr, const sf::DrawTextureSettings& drawParams);
    void                    uiImgsep(const sf::Rect2f& txr, const char* sepLabel, bool first = false);
    void                    uiImgsep2(const sf::Rect2f& txr, const char* sepLabel);
    void                    uiTabBarShop();
    bool                    uiCheckbox(const char* label, bool* b);
    bool                    uiRadio(const char* label, int* i, int value);
    void                    uiTabBarPrestige();
    void                    uiTabBarMagic();
    void                    uiTabBarStats();
    void                    uiTabBarSettings();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<sf::Rect2f> getAoEDragRect(const sf::Vec2f mousePos) const
    {
        if (!catDragOrigin.hasValue())
            return sf::base::nullOpt;

        return sf::base::makeOptional<sf::Rect2f>(*catDragOrigin, mousePos - *catDragOrigin);
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
    void turnBubbleInto(Bubble& bubble, const BubbleType newType)
    {
        bubble.type = newType;

        if (newType == BubbleType::Normal)
        {
            if (bubble.type == BubbleType::Bomb)
                bombIdxToCatIdx.erase(static_cast<sf::base::SizeT>(&bubble - pt->bubbles.data()));

            bubble.rotation = 0.f;
            bubble.torque   = 0.f;
            bubble.hueMod   = 0.f;

            return;
        }

        if (newType == BubbleType::Star || newType == BubbleType::Nova)
        {
            bubble.hueMod = rng.getF(0.f, 360.f);
            bubble.torque = 0.001f * rng.getSignF();

            return;
        }

        if (newType == BubbleType::Bomb)
        {
            bubble.torque = 0.004f * rng.getSignF();
            bubble.hueMod = 0.f;

            return;
        }
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


            turnBubbleInto(bubble, pt->perm.starpawNova ? BubbleType::Nova : BubbleType::Star);
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

        return sf::Vec2f{windowedWidth + 12.f, windowSize.y}.toVec2u();
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
    void reseedRNGs(const RNGSeedType newSeed)
    {
        seed    = newSeed;
        rng     = RNGFast{seed};
        rngFast = RNGFast{seed};
    }

    ////////////////////////////////////////////////////////////
    void forceResetGame(const bool goToShopTab = true)
    {
        soundManager.stopPlayingAll(sounds.ritual);
        soundManager.stopPlayingAll(sounds.copyritual);

        delayedActions.clear();

        reseedRNGs(static_cast<RNGSeedType>(sf::Clock::now().asMicroseconds()));

        shuffledCatNamesPerType = makeShuffledCatNames(rng);

        *pt      = Playthrough{};
        pt->seed = seed;

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

        for (sf::base::SizeT iP = 0u; iP < 16u; ++iP)
        {
            spawnParticle(
                ParticleData{
                    .position      = bubble.position,
                    .velocity      = rngFast.getVec2f({-0.75f, -1.75f}, {0.75f, -0.75f}) * 0.3f,
                    .scale         = rngFast.getF(0.08f, 0.27f) * 2.5f,
                    .scaleDecay    = 0.0002f,
                    .accelerationY = 0.002f,
                    .opacity       = 0.65f,
                    .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                    .rotation      = rngFast.getF(0.f, sf::base::tau),
                    .torque        = rngFast.getF(-0.002f, 0.002f) * 5.f,
                },
                0.f,
                ParticleType::Fire);

            spawnParticle(ParticleData{.position = bubble.position,
                                       .velocity = sf::Vec2f::fromAngle(rngFast.getF(0.4f, 0.8f),
                                                                        sf::radians(sf::base::tau / static_cast<float>(16u) *
                                                                                    static_cast<float>(iP))),
                                       .scale         = rngFast.getF(0.08f, 0.27f) * 2.75f,
                                       .scaleDecay    = -0.0025f,
                                       .accelerationY = 0.000001f,
                                       .opacity       = 0.35f,
                                       .opacityDecay  = rngFast.getF(0.001f, 0.002f) * 0.6f,
                                       .rotation      = rngFast.getF(0.f, sf::base::tau),
                                       .torque        = rngFast.getF(-0.001f, 0.001f)},
                          0.f,
                          ParticleType::Explosion);
        }

        for (sf::base::SizeT iP = 0u; iP < 8u; ++iP)
            spawnParticle(ParticleData{.position      = bubble.position,
                                       .velocity      = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(-0.15f, 0.05f)},
                                       .scale         = rngFast.getF(0.65f, 1.f) * 1.25f,
                                       .scaleDecay    = -0.0005f,
                                       .accelerationY = -0.00017f,
                                       .opacity       = rngFast.getF(0.5f, 0.75f) * 0.7f,
                                       .opacityDecay  = rngFast.getF(0.00035f, 0.00055f) * 0.8f,
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
        const sf::Vec2f screenPos = gameView.worldToScreen(point, window.getSize().toVec2f());

        // From screen coordinates to HUD view coordinates
        return scaledHUDView.screenToWorld(screenPos, window.getSize().toVec2f());
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

        const Shrine* collectorShrine = nullptr;
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
                    {.position      = moneyText.getGlobalCenterRight() + sf::Vec2f{32.f, rngFast.getF(-12.f, 12.f)},
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
                const sf::Vec2f viewSize           = getCurrentGameViewSize();
                const sf::Vec2f viewCenter         = getViewCenter();
                sounds.coindelay.settings.position = {viewCenter.x - viewSize.x / 2.f + 25.f,
                                                      viewCenter.y - viewSize.y / 2.f + 25.f};

                sounds.coindelay.settings.pitch  = 1.f;
                sounds.coindelay.settings.volume = profile.sfxVolume / 100.f * 0.5f;

                playSound(sounds.coindelay, /* maxOverlap */ 64);
            }
        }

        sounds.pop.settings.position = {bubble.position.x, bubble.position.y};
        sounds.pop.settings.pitch    = remap(static_cast<float>(xCombo), 1, 10, 1.f, 2.f);

        playSound(sounds.pop, popSoundOverlap ? 64u : 1u);

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
    [[nodiscard]] sf::Mouse::Button getLMB() const
    {
        return profile.invertMouseButtons ? sf::Mouse::Button::Right : sf::Mouse::Button::Left;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Mouse::Button getRMB() const
    {
        return profile.invertMouseButtons ? sf::Mouse::Button::Left : sf::Mouse::Button::Right;
    }
    void               gameLoopUpdateScrolling(float deltaTimeMs, const sf::base::Vector<sf::Vec2f>& downFingers);
    void               gameLoopUpdateTransitions(float deltaTimeMs);
    void               gameLoopUpdateBubbles(float deltaTimeMs);
    void               gameLoopUpdateAttractoBuff(float deltaTimeMs) const;
    [[nodiscard]] bool gameLoopUpdateBubbleClick(sf::base::Optional<sf::Vec2f>& clickPosition);
    void               gameLoopUpdateCatActionNormal(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionUni(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionDevil(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionAstro(float /* deltaTimeMs */, Cat& cat);
    [[nodiscard]] Cat* getHexedCat() const;
    [[nodiscard]] Cat* getCopyHexedCat() const;
    [[nodiscard]] bool anyCatHexedOrCopyHexed() const;
    [[nodiscard]] bool anyCatHexed() const;
    [[nodiscard]] bool anyCatCopyHexed() const;
    void               hexCat(Cat& cat, bool copy);
    void gameLoopUpdateCatActionWitchImpl(float /* deltaTimeMs */, Cat& cat, sf::base::Vector<Doll>& dollsToUse);
    void gameLoopUpdateCatActionWitch(float deltaTimeMs, Cat& cat);
    void gameLoopUpdateCatActionWizard(float deltaTimeMs, Cat& cat);
    void gameLoopUpdateCatActionMouse(float /* deltaTimeMs */, Cat& cat);
    void gameLoopUpdateCatActionEngi(float /* deltaTimeMs */, Cat& cat);
    void gameLoopUpdateCatActionRepulso(float /* deltaTimeMs */, Cat& cat);
    void gameLoopUpdateCatActionAttracto(float /* deltaTimeMs */, Cat& cat);
    void gameLoopUpdateCatActionCopy(float deltaTimeMs, Cat& cat);
    void gameLoopUpdateCatActionDuck(float deltaTimeMs, Cat& cat);
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
    void gameLoopUpdateCatActions(float deltaTimeMs);

    [[nodiscard]] bool isCatDraggable(const Cat& cat) const;
    [[nodiscard]] bool isAOESelecting() const;

    void gameLoopUpdateCatDragging(float deltaTimeMs, SizeT countFingersDown, sf::Vec2f mousePos);
    void gameLoopUpdateShrines(float deltaTimeMs);

    void collectDollImpl(Doll& d, const sf::base::Vector<Doll>& dollsToUse);
    void collectDoll(Doll& d);
    void collectCopyDoll(Doll& d);

    void gameLoopUpdateDollsImpl(float deltaTimeMs, sf::Vec2f mousePos, sf::base::Vector<Doll>& dollsToUse, Cat* hexedCat);
    void gameLoopUpdateDolls(float deltaTimeMs, sf::Vec2f mousePos);
    void gameLoopUpdateCopyDolls(float deltaTimeMs, sf::Vec2f mousePos);
    void gameLoopUpdateHellPortals(float deltaTimeMs);
    void gameLoopUpdateWitchBuffs(float deltaTimeMs);
    void gameLoopUpdateMana(float deltaTimeMs);
    void gameLoopUpdateAutocast();
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

        notificationQueue.emplaceBack(title, sf::base::String{fmtBuffer});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateMilestones();
    void gameLoopUpdateSplits();
    void gameLoopUpdateAchievements();
    void gameLoopDrawBubbles();
    void gameLoopDisplayBubblesWithoutShader();
    void gameLoopDrawCursorTrail(sf::Vec2f mousePos);
    void gameLoopDrawMinimapIcons();
    void gameLoopDisplayBubblesWithShader();
    void gameLoopDrawCats(sf::Vec2f mousePos, float deltaTimeMs);

    [[nodiscard]] bool isCatPerformingRitual(Cat& witch, Cat& cat) const;

    void gameLoopDrawCat(Cat&      cat,
                         float     deltaTimeMs,
                         sf::Vec2f mousePos,
                         const sf::Rect2f* const (&catTxrsByType)[nCatTypes],
                         const sf::Rect2f* const (&catPawTxrsByType)[nCatTypes],
                         const sf::Rect2f* const (&catTailTxrsByType)[nCatTypes],
                         const sf::Vec2f (&catTailOffsetsByType)[nCatTypes],
                         const sf::Vec2f (&catEyeOffsetsByType)[nCatTypes],
                         const float (&catHueByType)[nCatTypes]);

    void                gameLoopDrawShrines(sf::Vec2f mousePos);
    void                gameLoopDrawDolls(sf::Vec2f mousePos);
    void                gameLoopDrawHellPortals();
    void                gameLoopDrawParticles();
    void                gameLoopDrawHUDParticles();
    void                gameLoopDrawEarnedCoinParticles();
    void                gameLoopDrawHUDTopParticles();
    void                gameLoopDrawHUDBottomParticles();
    void                gameLoopDrawTextParticles();
    [[nodiscard]] float getLeftMostUsefulX() const;
    void                gameLoopDrawScrollArrowHint(float deltaTimeMs);
    void                gameLoopDrawImGui(sf::base::U8 shouldDrawUIAlpha);
    void                gameLoopUpdatePurchaseUnlockedEffects(float deltaTimeMs);
    [[nodiscard]] bool  shouldDrawGrabbingCursor() const;
    void                gameLoopDrawCursor(float deltaTimeMs, float cursorGrow);
    void                gameLoopDrawCursorComboText(float deltaTimeMs, float cursorGrow);
    void                gameLoopDrawCursorComboBar();
    void drawMinimap(sf::RenderTarget& rt, const sf::View& hudView, sf::Vec2f resolution, sf::base::U8 shouldDrawUIAlpha);
    void drawSplashScreen(sf::RenderTarget& rt, const sf::View& view, sf::Vec2f resolution, float hudScale) const;
    [[nodiscard]] sf::Rect2f getViewportPixelBounds(const sf::View& view, sf::Vec2f targetSize) const;
    void                     gameLoopDrawDollParticleBorder(float hueMod);
    void                     gameLoopTips(float deltaTimeMs);
    void                     recreateImGuiRenderTexture(sf::Vec2u newResolution);
    void                     recreateBackgroundRenderTexture(sf::Vec2u newResolution);
    void                     recreateGameRenderTexture(sf::Vec2u newResolution);
    void setPostProcessUniforms(float vibrance, float saturation, float lightness, float sharpness, float blur) const;
    void updateProcessedBackground();
    void gameLoopDisplayCloudBatch(const sf::CPUDrawableBatch& batch, const sf::View& view);
    void recreateWindow();
    void resizeWindow();
    [[nodiscard]] float gameLoopUpdateCursorGrowthEffect(float deltaTimeMs, bool anyBubblePoppedByClicking);
    void                gameLoopUpdateCombo(float                         deltaTimeMs,
                                            bool                          anyBubblePoppedByClicking,
                                            sf::Vec2f                     mousePos,
                                            sf::base::Optional<sf::Vec2f> clickPosition);
    void                gameLoopUpdateCollisionsBubbleBubble(float deltaTimeMs);
    void                gameLoopUpdateCollisionsCatCat(float deltaTimeMs);
    void                gameLoopUpdateCollisionsCatShrine(float deltaTimeMs) const;
    void                gameLoopUpdateCollisionsCatDoll();
    void                gameLoopUpdateCollisionsBubbleHellPortal();
    void                gameLoopUpdateScreenShake(float deltaTimeMs);
    void                gameLoopUpdateParticlesAndTextParticles(float deltaTimeMs);
    void                gameLoopUpdateSounds(float deltaTimeMs, sf::Vec2f mousePos);
    void                gameLoopUpdateTimePlayed(sf::base::I64 elapsedUs);
    void                gameLoopUpdateAutosave(sf::base::I64 elapsedUs);
    void                gameLoopUpdateAndDrawFixedMenuBackground(float deltaTimeMs, sf::base::I64 elapsedUs);
    void                gameLoopUpdateAndDrawBackground(float deltaTimeMs, const sf::View& gameBackgroundView);
    void                gameLoopUpdateMoneyText(float deltaTimeMs, float yBelowMinimap);
    void                gameLoopUpdateSpentMoneyEffect(float deltaTimeMs);
    void                gameLoopUpdateComboText(float deltaTimeMs, float yBelowMinimap);
    void                gameLoopUpdateBuffText();
    void                gameLoopPrestigeAvailableReminder();
    void                gameLoopReminderBuyCombo();
    void                gameLoopReminderSpendPPs();
    void                gameLoopUpdateDpsSampler(sf::base::I64 elapsedUs);
    ////////////////////////////////////////////////////////////
    void               gameLoopUpdateNotificationQueue(float deltaTimeMs);
    [[nodiscard]] bool gameLoop();
    void               loadPlaythroughFromFileAndReseed();
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    Main(hg::Steam::SteamManager& xSteamMgr);
#else
    Main();
#endif
    void run();
};
