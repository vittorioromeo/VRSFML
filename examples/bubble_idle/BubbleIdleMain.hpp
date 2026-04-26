#pragma once


// #define BUBBLEBYTE_NO_AUDIO 1
#include "Achievements.hpp"
#include "Aliases.hpp"
#include "Bubble.hpp"
#include "BubbleType.hpp"
#include "Cat.hpp"
#include "CatType.hpp"
#include "ComboState.hpp"
#include "Constants.hpp"
#include "Countdown.hpp"
#include "Doll.hpp"
#include "ExactArray.hpp"
#include "GameConstants.hpp"
#include "HexSession.hpp"
#include "InputHelper.hpp"
#include "MemberGuard.hpp"
#include "NotificationState.hpp"
#include "Particle.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "PlayerInput.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"
#include "PurchasableScalingValue.hpp"
#include "RNGSeedType.hpp"
#include "Serialization.hpp"
#include "Shrine.hpp"
#include "ShrineType.hpp"
#include "Sounds.hpp"
#include "SweepAndPrune.hpp"
#include "TextEffectWiggle.hpp"
#include "TextParticle.hpp"
#include "TextShakeEffect.hpp"
#include "UIState.hpp"

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
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Array.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ThreadPool.hpp"
#include "SFML/Base/Vector.hpp"

#include <cstdio>

#if defined(__GNUC__) || defined(__clang__)
    #define BUBBLE_IDLE_PRINTF_FORMAT(fmtIndex, firstArgIndex) __attribute__((format(printf, fmtIndex, firstArgIndex)))
#else
    #define BUBBLE_IDLE_PRINTF_FORMAT(fmtIndex, firstArgIndex)
#endif


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    #include "Steam.hpp"
#endif


////////////////////////////////////////////////////////////<
extern bool debugMode;


////////////////////////////////////////////////////////////
struct CloudFrameDrawSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    float time;

    sf::Vec2f mins;
    sf::Vec2f maxs;

    int xSteps;
    int ySteps;

    float     scaleMult;
    float     outwardOffsetMult;
    sf::Color color = sf::Color::White;

    sf::CPUDrawableBatch* batch;
};


////////////////////////////////////////////////////////////
bool handleBubbleCollision(float deltaTimeMs, Bubble& iBubble, Bubble& jBubble);


////////////////////////////////////////////////////////////
bool handleCatCollision(float deltaTimeMs, Cat& iCat, Cat& jCat);


////////////////////////////////////////////////////////////
bool handleCatShrineCollision(float deltaTimeMs, Cat& cat, Shrine& shrine);


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

        .type      = BubbleType::Normal,
        .ephemeral = false,
        .hueSeed   = pt.nextBubbleHueSeed++,
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
    sf::AudioContext   audioContext;
    sf::PlaybackDevice playbackDevice;
#endif

    ////////////////////////////////////////////////////////////
    // Graphics context
    sf::GraphicsContext graphicsContext;

    ////////////////////////////////////////////////////////////
    // Shader with hue support and bubble effects
    sf::Shader shader;

    sf::Shader::UniformLocation suBackgroundTexture;
    sf::Shader::UniformLocation suTime;
    sf::Shader::UniformLocation suResolution;
    sf::Shader::UniformLocation suBackgroundOrigin;
    sf::Shader::UniformLocation suBubbleEffect;

    sf::Shader::UniformLocation suIridescenceStrength;
    sf::Shader::UniformLocation suEdgeFactorMin;
    sf::Shader::UniformLocation suEdgeFactorMax;
    sf::Shader::UniformLocation suEdgeFactorStrength;
    sf::Shader::UniformLocation suDistorsionStrength;

    sf::Shader::UniformLocation suSubTexOrigin;
    sf::Shader::UniformLocation suSubTexSize;

    sf::Shader::UniformLocation suBubbleLightness;
    sf::Shader::UniformLocation suLensDistortion;

    sf::Shader::UniformLocation suRimShineStrength;
    sf::Shader::UniformLocation suRimShineFallRate;
    sf::Shader::UniformLocation suRimShineTimeRate;
    sf::Shader::UniformLocation suRimShineArc;

    float shaderTime = 0.f;

    ////////////////////////////////////////////////////////////
    // Shader with post-processing effects
    sf::Shader shaderPostProcess;

    sf::Shader::UniformLocation suPPVibrance;
    sf::Shader::UniformLocation suPPSaturation;
    sf::Shader::UniformLocation suPPLightness;
    sf::Shader::UniformLocation suPPSharpness;
    sf::Shader::UniformLocation suPPBlur;

    ////////////////////////////////////////////////////////////
    // Shader for fluffy cat cloud rendering
    sf::Shader shaderClouds;

    sf::Shader::UniformLocation suCloudTime;
    sf::Shader::UniformLocation suCloudResolution;

    ////////////////////////////////////////////////////////////
    // Shader for hexed cat phasing/distortion
    sf::Shader shaderHexed;

    sf::Shader::UniformLocation suHexedTime;
    sf::Shader::UniformLocation suHexedSeed;
    sf::Shader::UniformLocation suHexedDistortionStrength;
    sf::Shader::UniformLocation suHexedShimmerStrength;

    ////////////////////////////////////////////////////////////
    // Shader for activated shrine background distortion
    sf::Shader shaderShrineBackground;

    sf::Shader::UniformLocation suShrineBgTime;
    sf::Shader::UniformLocation suShrineBgViewOrigin;
    sf::Shader::UniformLocation suShrineBgCenter;
    sf::Shader::UniformLocation suShrineBgRange;
    sf::Shader::UniformLocation suShrineBgTintR;
    sf::Shader::UniformLocation suShrineBgTintG;
    sf::Shader::UniformLocation suShrineBgTintB;
    sf::Shader::UniformLocation suShrineBgTintA;
    sf::Shader::UniformLocation
        suShrineBgDistortionStrength = shaderShrineBackground.getUniformLocation("u_distortionStrength").value();
    sf::Shader::UniformLocation suShrineBgTintStrength;
    sf::Shader::UniformLocation suShrineBgEffectStrength;

    ////////////////////////////////////////////////////////////
    // Context settings
    const unsigned int aaLevel;

    ///////////////////////////////////////////////////////////
    // Profile (stores settings)
    Profile profile;

    ///////////////////////////////////////////////////////////
    // Game constants (loaded once on startup)
    GameConstants gameConstants;

    MEMBER_SCOPE_GUARD(Main, {
        sf::cOut() << "Saving profile to file on exit\n";
        saveProfileToFile(self.profile);
    });

    ////////////////////////////////////////////////////////////
    // SFML fonts
    sf::Font fontMouldyCheese;

    ////////////////////////////////////////////////////////////
    // Render window
    [[nodiscard]] sf::RenderWindow makeWindow();
    sf::RenderWindow               window;
    float                          dpiScalingFactor = 1.f;

    ////////////////////////////////////////////////////////////
    void refreshWindowAutoBatchModeFromProfile(); // TODO P1: check if this solves flickering

    ////////////////////////////////////////////////////////////
    bool flushAfterEveryBatch  = false; // TODO P1: check if this solves flickering
    bool finishAfterEveryBatch = false; // TODO P1: check if this solves flickering
    bool flushBeforeDisplay    = false; // TODO P1: check if this solves flickering
    bool finishBeforeDisplay   = false; // TODO P1: check if this solves flickering
    bool flushAfterDisplay     = false; // TODO P1: check if this solves flickering
    bool finishAfterDisplay    = false; // TODO P1: check if this solves flickering

    ////////////////////////////////////////////////////////////
    bool loadingGuard;

    ////////////////////////////////////////////////////////////
    // ImGui context
    sf::ImGuiContext imGuiContext{/* loadDefaultFont */ false};

    ////////////////////////////////////////////////////////////
    // Exiting status
    PlayerInputState playerInputState;
    bool             mustExit{false};

    ////////////////////////////////////////////////////////////
    // Texture atlas
    sf::TextureAtlas textureAtlas; // TODO P0: make smaller

    ////////////////////////////////////////////////////////////
    // SFML fonts
    sf::Font fontSuperBakery;

    ////////////////////////////////////////////////////////////
    // ImGui fonts
    ImFont* fontImGuiMouldyCheese;
    ImFont* fontImGuiSuperBakery;
    ImFont* fontImGuiFA;

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
    sf::RenderTexture rtBackground;

    sf::RenderTexture rtBackgroundProcessed;

    sf::RenderTexture rtImGui;

    sf::RenderTexture rtCloudMask;

    sf::RenderTexture rtCloudProcessed;

    ////////////////////////////////////////////////////////////
    // Game render texture (before post-processing)
    sf::RenderTexture rtGame;

    ////////////////////////////////////////////////////////////
    // Hexed cat offscreen render textures (one per concurrent hex, for witch and copy-witch combined)
    static inline constexpr sf::Vec2u       hexedCatRenderTextureSize{640u, 640u};
    static inline constexpr sf::base::SizeT maxHexedCatRenderTextures = maxConcurrentHexes * 2u;

    sf::base::Vector<sf::RenderTexture> hexedCatRenderTextures;

    ////////////////////////////////////////////////////////////
    // Textures (not in atlas)
    static inline constexpr sf::TextureLoadSettings bgSettings{.smooth = true, .wrapMode = sf::TextureWrapMode::Repeat};

    sf::Texture txLogo;
    sf::Texture txFixedBg;
    sf::Texture txBackgroundChunk;
    sf::Texture txBackgroundChunkDesaturated;
    sf::Texture txClouds;
    sf::Texture txTintedClouds;
    sf::Texture txBgSwamp;
    sf::Texture txBgObservatory;
    sf::Texture txBgAimTraining;
    sf::Texture txBgFactory;
    sf::Texture txBgWindTunnel;
    sf::Texture txBgMagnetosphere;
    sf::Texture txBgAuditorium;
    sf::Texture txDrawings;
    sf::Texture txTipBg;
    sf::Texture txTipByte;
    sf::Texture txCursor;
    sf::Texture txCursorMultipop;
    sf::Texture txCursorLaser;
    sf::Texture txCursorGrab;
    sf::Texture txArrow;
    sf::Texture txUnlock;
    sf::Texture txPurchasable;
    sf::Texture txLetter;
    sf::Texture txLetterText;
    sf::Texture txFrame;
    sf::Texture txFrameTiny;
    sf::Texture txCloudBtn;
    sf::Texture txCloudBtnSmall;
    sf::Texture txCloudBtnSquare;
    sf::Texture txCloudBtnSquare2;

    ////////////////////////////////////////////////////////////
    // UI texture atlas
    sf::TextureAtlas uiTextureAtlas;

    ////////////////////////////////////////////////////////////
    // Quick toolbar icons
    sf::Rect2f txrIconVolume;
    sf::Rect2f txrIconBGM;
    sf::Rect2f txrIconBg;
    sf::Rect2f txrIconCfg;
    sf::Rect2f txrIconCopyCat;

    ////////////////////////////////////////////////////////////
    // Shop menu separator textures
    sf::Rect2f txrMenuSeparator0;
    sf::Rect2f txrMenuSeparator1;
    sf::Rect2f txrMenuSeparator2;
    sf::Rect2f txrMenuSeparator3;
    sf::Rect2f txrMenuSeparator4;
    sf::Rect2f txrMenuSeparator5;
    sf::Rect2f txrMenuSeparator6;
    sf::Rect2f txrMenuSeparator7;
    sf::Rect2f txrMenuSeparator8;

    ////////////////////////////////////////////////////////////
    // Prestige menu separator textures
    sf::Rect2f txrPrestigeSeparator0;
    sf::Rect2f txrPrestigeSeparator1;
    sf::Rect2f txrPrestigeSeparator2;
    sf::Rect2f txrPrestigeSeparator3;
    sf::Rect2f txrPrestigeSeparator4;
    sf::Rect2f txrPrestigeSeparator5;
    sf::Rect2f txrPrestigeSeparator6;
    sf::Rect2f txrPrestigeSeparator7;
    sf::Rect2f txrPrestigeSeparator8;
    sf::Rect2f txrPrestigeSeparator9;
    sf::Rect2f txrPrestigeSeparator10;
    sf::Rect2f txrPrestigeSeparator11;
    sf::Rect2f txrPrestigeSeparator12;
    sf::Rect2f txrPrestigeSeparator13;
    sf::Rect2f txrPrestigeSeparator14;
    sf::Rect2f txrPrestigeSeparator15;

    ////////////////////////////////////////////////////////////
    // Magic menu separator textures
    sf::Rect2f txrMagicSeparator0;
    sf::Rect2f txrMagicSeparator1;
    sf::Rect2f txrMagicSeparator2;
    sf::Rect2f txrMagicSeparator3;

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
    sf::Rect2f txrWhiteDot;
    sf::Rect2f txrBubble;
    sf::Rect2f txrBubbleStar;
    sf::Rect2f txrBubbleNova;
    sf::Rect2f txrBubbleGlass;
    sf::Rect2f txrCat;

    // Wardencat composite: drawn back-to-front as guardhouse_back, wardencat
    // (with a tail-like body wobble), guardhouse_front, wardencatpaw.
    sf::Rect2f txrGuardhouseBack;
    sf::Rect2f txrWardenCat;
    sf::Rect2f txrGuardhouseFront;
    sf::Rect2f txrWardencatPaw;
    sf::Rect2f txrUniCat;
    sf::Rect2f txrUniCat2;
    sf::Rect2f txrUniCatWings;
    sf::Rect2f txrDevilCat2;
    sf::Rect2f txrDevilCat3;
    sf::Rect2f txrDevilCat3Arm;
    sf::Rect2f txrDevilCat3Book;
    sf::Rect2f txrDevilCat3Tail;
    sf::Rect2f txrDevilCat2Book;
    sf::Rect2f txrCatPaw;
    sf::Rect2f txrCatTail;
    sf::Rect2f txrSmartCatHat;
    sf::Rect2f txrSmartCatDiploma;
    sf::Rect2f txrBrainBack;
    sf::Rect2f txrBrainFront;
    sf::Rect2f txrUniCatTail;
    sf::Rect2f txrUniCat2Tail;
    sf::Rect2f txrDevilCatTail2;
    sf::Rect2f txrAstroCatTail;
    sf::Rect2f txrAstroCatFlag;
    sf::Rect2f txrWitchCatTail;
    sf::Rect2f txrWizardCatTail;
    sf::Rect2f txrMouseCatTail;
    sf::Rect2f txrMouseCatMouse;
    sf::Rect2f txrEngiCatTail;
    sf::Rect2f txrEngiCatWrench;
    sf::Rect2f txrRepulsoCatTail;
    sf::Rect2f txrAttractoCatTail;
    sf::Rect2f txrCopyCatTail;
    sf::Rect2f txrAttractoCatMagnet;
    sf::Rect2f txrUniCatPaw;
    sf::Rect2f txrDevilCatPaw;
    sf::Rect2f txrDevilCatPaw2;
    sf::Rect2f txrParticle;
    sf::Rect2f txrStarParticle;
    sf::Rect2f txrFireParticle;
    sf::Rect2f txrFireParticle2;
    sf::Rect2f txrSmokeParticle;
    sf::Rect2f txrExplosionParticle;
    sf::Rect2f txrTrailParticle;
    sf::Rect2f txrHexParticle;
    sf::Rect2f txrShrineParticle;
    sf::Rect2f txrCogParticle;
    sf::Rect2f txrGlassParticle;
    sf::Rect2f txrWitchCat;
    sf::Rect2f txrWitchCatPaw;
    sf::Rect2f txrAstroCat;
    sf::Rect2f txrBomb;
    sf::Rect2f txrShrine;
    sf::Rect2f txrWizardCat;
    sf::Rect2f txrWizardCatPaw;
    sf::Rect2f txrMouseCat;
    sf::Rect2f txrMouseCatPaw;
    sf::Rect2f txrEngiCat;
    sf::Rect2f txrEngiCatPaw;
    sf::Rect2f txrRepulsoCat;
    sf::Rect2f txrRepulsoCatPaw;
    sf::Rect2f txrAttractoCat;
    sf::Rect2f txrCopyCat;
    sf::Rect2f txrDuckCat;
    sf::Rect2f txrDuckFlag;
    sf::Rect2f txrAttractoCatPaw;
    sf::Rect2f txrCopyCatPaw;
    sf::Rect2f txrDollNormal;
    sf::Rect2f txrDollUni;
    sf::Rect2f txrDollDevil;
    sf::Rect2f txrDollAstro;
    sf::Rect2f txrDollWizard;
    sf::Rect2f txrDollMouse;
    sf::Rect2f txrDollEngi;
    sf::Rect2f txrDollRepulso;
    sf::Rect2f txrDollAttracto;
    sf::Rect2f txrCoin;
    sf::Rect2f txrCatSoul;
    sf::Rect2f txrHellPortal;
    sf::Rect2f txrCatEyeLid0;
    sf::Rect2f txrCatEyeLid1;
    sf::Rect2f txrCatEyeLid2;
    sf::Rect2f txrCatWhiteEyeLid0;
    sf::Rect2f txrCatWhiteEyeLid1;
    sf::Rect2f txrCatWhiteEyeLid2;
    sf::Rect2f txrCatDarkEyeLid0;
    sf::Rect2f txrCatDarkEyeLid1;
    sf::Rect2f txrCatDarkEyeLid2;
    sf::Rect2f txrCatGrayEyeLid0;
    sf::Rect2f txrCatGrayEyeLid1;
    sf::Rect2f txrCatGrayEyeLid2;
    sf::Rect2f txrCatEars0;
    sf::Rect2f txrCatEars1;
    sf::Rect2f txrCatEars2;
    sf::Rect2f txrCatYawn0;
    sf::Rect2f txrCatYawn1;
    sf::Rect2f txrCatYawn2;
    sf::Rect2f txrCatYawn3;
    sf::Rect2f txrCatYawn4;
    sf::Rect2f txrCCMaskWitch;
    sf::Rect2f txrCCMaskWizard;
    sf::Rect2f txrCCMaskMouse;
    sf::Rect2f txrCCMaskEngi;
    sf::Rect2f txrCCMaskRepulso;
    sf::Rect2f txrCCMaskAttracto;
    sf::Rect2f txrMMNormal;
    sf::Rect2f txrMMUni;
    sf::Rect2f txrMMDevil;
    sf::Rect2f txrMMAstro;
    sf::Rect2f txrMMWitch;
    sf::Rect2f txrMMWizard;
    sf::Rect2f txrMMMouse;
    sf::Rect2f txrMMEngi;
    sf::Rect2f txrMMRepulso;
    sf::Rect2f txrMMAttracto;
    sf::Rect2f txrMMCopy;
    sf::Rect2f txrMMDuck;
    sf::Rect2f txrMMShrine;
    sf::Rect2f txrCloud;

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
        txrGlassParticle,
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
                              {.position         = {10.f, 70.f},
                               .string           = "$0",
                               .characterSize    = 64u,
                               .fillColor        = sf::Color::White,
                               .outlineColor     = colorBlueOutline,
                               .outlineThickness = 4.f}};
    TextShakeEffect moneyTextShakeEffect;

    ////////////////////////////////////////////////////////////
    // Combo state
    ComboState comboState{fontSuperBakery, fontMouldyCheese, moneyText.position, colorBlueOutline};

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
    // Combo-bubble payout queue. Each entry holds a swarm of coin particles
    // spawned at the popped bubble. They first burst outward physically (with
    // exponential damping, so they settle to a near-stop), then transition
    // into the rising-pitch collection sequence -- when the collection delay
    // ticks, one settled coin is consumed and a regular `EarnedCoinParticle`
    // is spawned at its position to fly to the money text on the HUD.
    struct [[nodiscard]] BurstingComboCoin
    {
        sf::Vec2f position{};
        sf::Vec2f velocity{};
        bool      collected{false};
    };

    struct [[nodiscard]] PendingComboBubblePayout
    {
        sf::base::Vector<BurstingComboCoin> coins;
        SizeT                               coinsCollected{0u}; // total collected so far (drives pitch)
        Countdown                           settleCountdown{};  // burst → collect transition
        Countdown                           collectDelay{};
    };

    sf::base::Vector<PendingComboBubblePayout> pendingComboBubblePayouts;

    ////////////////////////////////////////////////////////////
    // Random number generation
    RNGSeedType seed;
    RNGFast     rng{seed};
    RNGFast     rngFast{seed}; // very fast, low-quality, but good enough for VFXs

    ////////////////////////////////////////////////////////////
    // Cat names
    sf::base::Vector<sf::base::Vector<sf::base::StringView>> shuffledCatNamesPerType;

    ////////////////////////////////////////////////////////////
    // Prestige transition
    bool inPrestigeTransition{false};

    ////////////////////////////////////////////////////////////
    // Timers for transitions
    TargetedCountdown bubbleSpawnTimer{.startingValue = 3.f};
    TargetedCountdown catRemoveTimer{.startingValue = 100.f};

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
    sf::CPUDrawableBatch cpuCloudHudDrawableBatch;
    sf::CPUDrawableBatch cpuCloudUiDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatchBeforeCats;
    sf::CPUDrawableBatch cpuDrawableBatch;
    sf::CPUDrawableBatch cpuDrawableBatchAfterCats;
    sf::CPUDrawableBatch cpuDrawableBatchAdditive;
    sf::CPUDrawableBatch minimapDrawableBatch;
    sf::CPUDrawableBatch catTextDrawableBatch;
    sf::CPUDrawableBatch hudDrawableBatch;
    sf::CPUDrawableBatch hudTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch hudBottomDrawableBatch;  // drawn below ImGui
    sf::CPUDrawableBatch cpuTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch catTextTopDrawableBatch; // drawn on top of ImGui
    sf::CPUDrawableBatch tempDrawableBatch;       // for misc one-off draws (hexed cat effect)

    struct HexedCatDrawCommand // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        sf::base::SizeT renderTextureIndex;
        sf::Vec2f       position;
        float           phaseSeed;
        float           effectStrength;
        bool            top;
    };

    sf::base::Vector<HexedCatDrawCommand> hexedCatDrawCommands;

    ////////////////////////////////////////////////////////////
    void drawBatch(const sf::CPUDrawableBatch& batch, const sf::RenderStates& states);

    ////////////////////////////////////////////////////////////
    // Screen shake effect state
    float screenShakeAmount{0.f};
    float screenShakeTimer{0.f};

    ////////////////////////////////////////////////////////////
    // Debug-only multiplier applied to per-frame `deltaTimeMs` in the world update.
    // Always 1.0 outside debug mode.
    float debugTimeScale{1.f};

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
                            {
                                .string           = "",
                                .characterSize    = 48u,
                                .fillColor        = sf::Color::White,
                                .outlineColor     = colorBlueOutline,
                                .outlineThickness = 3.f,
                            }};

    sf::Text textStatusBuffer{fontSuperBakery,
                              {
                                  .string           = "",
                                  .characterSize    = 32u,
                                  .fillColor        = sf::Color::White,
                                  .outlineColor     = colorBlueOutline,
                                  .outlineThickness = 2.f,
                              }};

    sf::Text textMoneyBuffer{fontSuperBakery,
                             {
                                 .string           = "",
                                 .characterSize    = 24u,
                                 .fillColor        = sf::Color::White,
                                 .outlineColor     = colorBlueOutline,
                                 .outlineThickness = 1.5f,
                             }};

    ////////////////////////////////////////////////////////////
    // Spent money count-down effect
    MoneyType spentMoney{0u};
    Timer     spentMoneyTimer{.value = 0.f};

    ////////////////////////////////////////////////////////////
    // Thread pool
    sf::base::ThreadPool threadPool;

    ////////////////////////////////////////////////////////////
    // Cached views
    sf::View gameView{{1.f, 1.f}, {1.f, 1.f}};         // TODO P1: compute on the fly, don't cache...
    sf::View nonScaledHUDView{{1.f, 1.f}, {1.f, 1.f}}; // TODO P1: compute on the fly, don't cache...
    sf::View scaledHUDView{{1.f, 1.f}, {1.f, 1.f}};    // TODO P1: compute on the fly, don't cache...

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
    NotificationState notificationState;

    ////////////////////////////////////////////////////////////
    // FPS counter
    float fps{0.f};

    ////////////////////////////////////////////////////////////
    // UI state
    UIState uiState;

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
    // Victory state
    OptionalTargetedCountdown victoryTC;
    Countdown                 cdLetterAppear;
    Countdown                 cdLetterText;

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
    void saveMainPlaythroughToFile();

    ////////////////////////////////////////////////////////////
    void log(const char* format, ...); // NOLINT(modernize-avoid-variadic-functions)

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool inSpeedrunPlaythrough() const
    {
        return pt == &ptSpeedrun;
    }

    ////////////////////////////////////////////////////////////
    void addMoney(MoneyType reward);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::base::Vector<sf::base::Vector<sf::base::StringView>> makeShuffledCatNames(RNGFast& rng);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getTPWorkerCount();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SizeT getNextCatNameIdx(CatType catType);

    ////////////////////////////////////////////////////////////
    Particle& implEmplaceParticle(sf::Vec2f    position,
                                  ParticleType particleType,
                                  float        scaleMult,
                                  float        speedMult,
                                  float        opacity = 1.f);

    ////////////////////////////////////////////////////////////
    bool spawnSpentCoinParticle(const ParticleData& particleData);

    ////////////////////////////////////////////////////////////
    void spawnHUDTopParticle(const ParticleData& particleData, float hue, ParticleType particleType);

    ////////////////////////////////////////////////////////////
    void spawnHUDBottomParticle(const ParticleData& particleData, float hue, ParticleType particleType);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool spawnEarnedCoinParticle(sf::Vec2f startPosition);

    ////////////////////////////////////////////////////////////
    void spawnParticle(const ParticleData& particleData, float hue, ParticleType particleType);

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
    void statBubblePopped(BubbleType bubbleType, bool byHand, MoneyType reward);

    ////////////////////////////////////////////////////////////
    void statExplosionRevenue(MoneyType reward);

    ////////////////////////////////////////////////////////////
    void statFlightRevenue(MoneyType reward);

    ////////////////////////////////////////////////////////////
    void statHellPortalRevenue(MoneyType reward);

    ////////////////////////////////////////////////////////////
    void statSecondsPlayed();

    ////////////////////////////////////////////////////////////
    void statHighestStarBubblePopCombo(sf::base::U64 comboValue);

    ////////////////////////////////////////////////////////////
    void statHighestNovaBubblePopCombo(sf::base::U64 comboValue);

    ////////////////////////////////////////////////////////////
    void statAbsorbedStarBubble();

    ////////////////////////////////////////////////////////////
    void statSpellCast(SizeT spellIndex);

    ////////////////////////////////////////////////////////////
    void statMaintenance(SizeT nCatsHit);

    ////////////////////////////////////////////////////////////
    void statDisguise();

    ////////////////////////////////////////////////////////////
    void statDollCollected();

    ////////////////////////////////////////////////////////////
    void statRitual(CatType catType);

    ////////////////////////////////////////////////////////////
    void statHighestSimultaneousMaintenances(sf::base::U64 value);

    ////////////////////////////////////////////////////////////
    void statHighestDPS(sf::base::U64 value);


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool keyDown(sf::Keyboard::Key key) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool mBtnDown(sf::Mouse::Button button, bool penetrateUI) const;

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
    void playSound(const LoadedSound& ls, sf::base::SizeT maxOverlap = 255u);

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
    [[nodiscard]] Bubble* pickRandomBubbleInRadius(sf::Vec2f center, float radius);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getResolution() const;

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
    [[nodiscard]] sf::Vec2f getViewCenter() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getViewCenterWithoutScroll() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CullingBoundaries getViewCullingBoundaries(float offset) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::Vec2f                                    getCatRangeCenter(const Cat& cat);
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
    Cat& spawnCat(sf::Vec2f pos, CatType catType, float hue);

    ////////////////////////////////////////////////////////////
    Cat& spawnCatCentered(CatType catType, float hue, bool placeInHand = true);

    ////////////////////////////////////////////////////////////
    Cat& spawnSpecialCat(sf::Vec2f pos, CatType catType);

    ////////////////////////////////////////////////////////////
    void resetTipState();

    ////////////////////////////////////////////////////////////
    void doTip(const sf::base::String& str, SizeT maxPrestigeLevel = 0u);

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
    [[nodiscard]] float getAspectRatioScalingFactor(sf::Vec2f originalSize, sf::Vec2f windowSize) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getCappedGameViewAspectRatio(sf::Vec2f originalSize, sf::Vec2f windowSize) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getExpandedGameViewSize(sf::Vec2f originalSize, sf::Vec2f windowSize) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledGameView(sf::Vec2f originalSize, sf::Vec2f windowSize) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledTopGameView(sf::Vec2f originalSize, sf::Vec2f windowSize) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getCurrentGameViewSize() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float clampGameViewCenterX(float desiredCenterX, float viewWidth) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View makeScaledHUDView(sf::Vec2f resolution, float scale) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getHUDMousePos() const;

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

    struct TabButtonPalette
    {
        ImVec4 idle;
        ImVec4 hovered;
        ImVec4 active;
    };

    [[nodiscard]] bool drawTabButton(float                   scaleMult,
                                     const char*             label,
                                     bool                    selected,
                                     const TabButtonPalette& palette,
                                     ImVec2                  size   = {},
                                     bool                    square = false);

    [[nodiscard]] AnimatedButtonOutcome uiAnimatedButton(
        const sf::Texture& tx,
        const char*        label,
        const ImVec2&      btnSize,
        float              fontScale,
        float              fontScaleMult,
        float              btnSizeMult  = 1.f,
        bool               forceHovered = false);

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
        const char*              currencyFmt)
    {
        const bool maxedOut = psv.nPurchases == psv.data->nMaxPurchases;

        if (profile.hideMaxedOutPurchasables && maxedOut)
            return false;

        bool result = false;

        if (maxedOut)
            std::sprintf(uiState.uiBuffer, "MAX##%u", uiState.uiWidgetId++);
        else if (cost == 0u || times == 0u)
            std::sprintf(uiState.uiBuffer, "N/A##%u", uiState.uiWidgetId++);
        else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
            std::sprintf(uiState.uiBuffer, currencyFmt, toStringWithSeparators(cost), uiState.uiWidgetId++);
#pragma GCC diagnostic pop

        ImGui::BeginDisabled(uiCheckPurchasability(label, maxedOut || availability < cost || cost == 0u));

        uiMakeButtonLabels(label, uiState.uiLabelBuffer);
        if (uiMakeButtonImpl(label, uiState.uiBuffer))
        {
            result = true;
            availability -= cost;

            if (&availability == &pt->money)
                spentMoney += cost;

            psv.nPurchases += times;

            if (&availability == &pt->prestigePoints && times == 1u)
            {
                undoPPPurchase.emplaceBack([&psv, &availability, times, cost]
                {
                    psv.nPurchases -= times;
                    availability += cost;
                });

                undoPPPurchaseTimer.value = 10000.f;
            }
        }

        ImGui::EndDisabled();
        return result;
    }

    ////////////////////////////////////////////////////////////
    template <typename TCost>
    [[nodiscard]] bool makePurchasableButtonOneTimeByCurrency(
        const char* label,
        bool&       done,
        TCost       cost,
        TCost&      availability,
        const char* currencyFmt)
    {
        bool result = false;

        if (done)
            std::sprintf(uiState.uiBuffer, "DONE##%u", uiState.uiWidgetId++);
        else if (cost == 0u)
            std::sprintf(uiState.uiBuffer, "FREE##%u", uiState.uiWidgetId++);
        else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
            std::sprintf(uiState.uiBuffer, currencyFmt, toStringWithSeparators(cost), uiState.uiWidgetId++);
#pragma GCC diagnostic pop

        ImGui::BeginDisabled(uiCheckPurchasability(label, done || availability < cost));

        uiMakeButtonLabels(label, uiState.uiLabelBuffer);
        if (uiMakeButtonImpl(label, uiState.uiBuffer))
        {
            result = true;
            availability -= cost;

            if (&availability == &pt->money)
                spentMoney += cost;

            done = true;

            if (&availability == &pt->prestigePoints && cost > 0u)
            {
                undoPPPurchase.emplaceBack([&availability, &done, cost]
                {
                    done = false;
                    availability += cost;
                });

                undoPPPurchaseTimer.value = 10000.f;
            }
        }

        ImGui::EndDisabled();
        return result;
    }

    ////////////////////////////////////////////////////////////
    void switchToBGM(sf::base::SizeT index, bool force);

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
    sf::base::Optional<BGMBuffer>& getCurrentBGMBuffer();

    ////////////////////////////////////////////////////////////
    sf::base::Optional<BGMBuffer>& getNextBGMBuffer();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getHueByCatType(CatType catType);

    ////////////////////////////////////////////////////////////
    void beginPrestigeTransition(PrestigePointsType ppReward);

    void uiBeginColumns() const;
    void uiCenteredText(const char* str, float offsetX = 0.f, float offsetY = 0.f);
    void uiCenteredTextColored(sf::Color color, const char* str, float offsetX = 0.f, float offsetY = 0.f);
    [[nodiscard]] sf::Vec2f uiGetWindowPos() const;
    void                    uiDrawExitPopup(float newScalingFactor);
    void                    uiClearLabel();
    void                    uiSetLabel(const char* fmt, ...) BUBBLE_IDLE_PRINTF_FORMAT(2, 3);
    void                    uiSetTooltip(const char* fmt, ...) BUBBLE_IDLE_PRINTF_FORMAT(2, 3);
    void                    uiSetTooltipOnly(const char* fmt, ...) BUBBLE_IDLE_PRINTF_FORMAT(2, 3);

    [[nodiscard]] bool uiMakePrestigeOneTimeButton(
        const char*        buttonLabel,
        PrestigePointsType cost,
        bool&              done,
        const char*        tooltipFmt,
        ...) BUBBLE_IDLE_PRINTF_FORMAT(5, 6);

    template <typename TLabel, typename TCurrent, typename TNext>
    bool uiMakePrestigePsvButtonValue(
        const char*              buttonLabel,
        PurchasableScalingValue& psv,
        const char*              labelFmt,
        TLabel                   labelValue,
        const char*              tooltipFmt,
        TCurrent                 currentValue,
        TNext                    nextValue,
        const char*              maxTooltipFmt)
    {
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat-nonliteral"
    #pragma clang diagnostic ignored "-Wformat-security"
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    #pragma GCC diagnostic ignored "-Wformat-security"
#endif
        if (!psv.isMaxedOut())
            uiSetTooltip(tooltipFmt, currentValue, nextValue);
        else
            uiSetTooltip(maxTooltipFmt);

        uiSetLabel(labelFmt, labelValue);
#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
        return makePrestigePurchasablePPButtonPSV(buttonLabel, psv);
    }

    void                           uiDrawCloudWindowBackground();
    [[nodiscard]] bool             uiDrawQuickbarIconButton(const char* label, bool selected, float scaleMult = 1.f);
    void                           uiDrawQuickbarCopyCat(sf::Vec2f quickBarPos, Cat& copyCat);
    void                           uiDrawQuickbarBackgroundSelector(sf::Vec2f quickBarPos);
    void                           uiDrawQuickbarBGMSelector(sf::Vec2f quickBarPos);
    void                           uiDrawQuickbarQuickSettings(sf::Vec2f quickBarPos);
    void                           uiDrawQuickbarVolumeControls(sf::Vec2f quickBarPos);
    void                           uiDrawQuickbar();
    void                           uiDrawMinimapZoomButtons();
    void                           uiDrawDebugWindow();
    void                           uiDraw(sf::Vec2f mousePos);
    void                           uiDpsMeter();
    void                           uiSpeedrunning();
    void                           uiTabBar();
    void                           uiSetUnlockLabelY(sf::base::SizeT unlockId);
    [[nodiscard]] bool             checkUiUnlock(sf::base::SizeT unlockId, bool unlockCondition);
    void                           uiImageFromAtlas(const sf::Rect2f& txr, const sf::DrawTextureSettings& drawParams);
    void                           uiImgsep(const sf::Rect2f& txr, const char* sepLabel, bool first = false);
    void                           uiImgsep2(const sf::Rect2f& txr, const char* sepLabel);
    void                           uiTabBarShop();
    void                           uiShopDrawCoreUpgrades();
    void                           uiShopDrawSpecialCats();
    void                           uiShopDrawUniqueCatBonuses();
    [[nodiscard]] sf::base::String uiShopBuildNextGoalsText();
    void uiShopCooldownButton(const char* label, CatType catType, const char* additionalInfo = "");
    void uiShopRangeButton(const char* label, CatType catType, const char* additionalInfo = "");
    bool uiCheckbox(const char* label, bool* b);
    bool uiRadio(const char* label, int* i, int value);
    void uiTabBarPrestige();
    void uiPrestigeDrawOverview();
    void uiPrestigeDrawCoreUpgrades();
    void uiPrestigeUnsealButton(PrestigePointsType ppCost, const char* catName, CatType type);
    void uiPrestigeDrawShrineCatUpgrades();
    void uiTabBarMagic();
    void uiTabBarStats();
    void uiTabBarSettings();
    void uiSettingsDrawAudioTab();
    void uiSettingsDrawUiTab();
    void uiSettingsDrawGraphicsTab();
    void uiSettingsDrawDisplayTab();
    void uiSettingsDrawDataTab();
    void uiSettingsDrawDebugTab();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<sf::Rect2f> getAoEDragRect(sf::Vec2f mousePos) const;

    ////////////////////////////////////////////////////////////
    void resetAllDraggedCats();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::SizeT pickDragPivotCatIndex() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isCatBeingDragged(const Cat& cat) const;

    ////////////////////////////////////////////////////////////
    void stopDraggingCat(const Cat& cat);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isWizardBusy() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* findFirstCatByType(CatType catType) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getWitchCat() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getWizardCat() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getMouseCat() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getEngiCat() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getRepulsoCat() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getAttractoCat() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getCopyCat() const;

    ////////////////////////////////////////////////////////////
    void addCombo(int& xCombo, Countdown& xComboCountdown) const;

    ////////////////////////////////////////////////////////////
    static bool checkComboEnd(float deltaTimeMs, int& xCombo, Countdown& xComboCountdown);

    ////////////////////////////////////////////////////////////
    void turnBubbleInto(Bubble& bubble, BubbleType newType);

    ////////////////////////////////////////////////////////////
    void doWizardSpellStarpawConversion(Cat& wizardCat);

    ////////////////////////////////////////////////////////////
    void doWizardSpellMewltiplierAura(Cat& wizardCat);

    ////////////////////////////////////////////////////////////
    void doWizardSpellDarkUnion(Cat& wizardCat);

    ////////////////////////////////////////////////////////////
    void doWizardSpellStasisField(Cat& wizardCat);

    ////////////////////////////////////////////////////////////
    static inline constexpr ManaType spellManaCostByIndex[4] = {5u, 20u, 30u, 40u};

    ////////////////////////////////////////////////////////////
    void castSpellByIndex(sf::base::SizeT index, Cat* wizardCat, Cat* copyCat);

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
    [[nodiscard]] bool mustApplyMewltiplierAura(sf::Vec2f bubblePosition) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] MoneyType computeFinalReward(const Bubble& bubble, float multiplier, float comboMult, const Cat* popperCat) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::Vec2u getReasonableWindowSize(float scalingFactorMult = 1.f);

    ////////////////////////////////////////////////////////////
    struct SelectorEntry
    {
        int         index;
        const char* name;
    };

    ////////////////////////////////////////////////////////////
    [[nodiscard]] int pickSelectedIndex(const sf::base::Vector<SelectorEntry>& entries, int selectedIndex);

    ////////////////////////////////////////////////////////////
    void selectBackground(const sf::base::Vector<SelectorEntry>& entries, int selectedIndex);

    ////////////////////////////////////////////////////////////
    void selectBGM(const sf::base::Vector<SelectorEntry>& entries, int selectedIndex);

    ////////////////////////////////////////////////////////////
    void updateSelectedBackgroundSelectorIndex() const;

    ////////////////////////////////////////////////////////////
    void updateSelectedBGMSelectorIndex() const;

    ////////////////////////////////////////////////////////////
    struct SelectorData
    {
        sf::base::Vector<SelectorEntry> entries;
        int                             selectedIndex = -1;
    };

    ////////////////////////////////////////////////////////////
    SelectorData& getBGMSelectorData() const;

    ////////////////////////////////////////////////////////////
    SelectorData& getBackgroundSelectorData() const;

    ////////////////////////////////////////////////////////////
    void reseedRNGs(RNGSeedType newSeed);

    ////////////////////////////////////////////////////////////
    void forceResetGame(bool goToShopTab = true);

    ////////////////////////////////////////////////////////////
    void forceResetProfile();

    ////////////////////////////////////////////////////////////
    TextParticle& makeRewardTextParticle(sf::Vec2f position);

    ////////////////////////////////////////////////////////////
    void shrineCollectReward(Shrine& shrine, MoneyType reward, const Bubble& bubble);

    ////////////////////////////////////////////////////////////
    void doExplosion(Bubble& bubble);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f fromWorldToHud(sf::Vec2f point) const;

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
    [[nodiscard]] bool isBubbleInStasisField(const Bubble& bubble) const;

    ////////////////////////////////////////////////////////////
    void popWithRewardAndReplaceBubble(const BubblePopData& data);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDebugModeEnabled() const;

    ////////////////////////////////////////////////////////////
    void gameLoopCheats() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Mouse::Button getLMB() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Mouse::Button getRMB() const;
    [[nodiscard]] bool              gameLoopHandleEvents(FrameInput& frameInput, bool shouldDrawUI);
    void                            gameLoopPrepareInput(FrameInput& frameInput, float deltaTimeMs);
    void gameLoopUpdateFrameWorld(float deltaTimeMs, FrameInput& frameInput, FrameUpdateState& frameUpdate);
    void gameLoopUpdateFrameUi(sf::Time deltaTime, float deltaTimeMs, const FrameInput& frameInput);
    [[nodiscard]] FrameViewState gameLoopComputeViews();
    void                         gameLoopRenderFrame(float                   deltaTimeMs,
                                                     bool                    shouldDrawUI,
                                                     sf::base::U8            shouldDrawUIAlpha,
                                                     const FrameInput&       frameInput,
                                                     const FrameUpdateState& frameUpdate,
                                                     const FrameViewState&   frameViews);
    void                         gameLoopPresentFrame(const FrameViewState& frameViews);
    void               gameLoopUpdateScrolling(float deltaTimeMs, const sf::base::Vector<sf::Vec2f>& downFingers);
    void               gameLoopUpdateTransitions(float deltaTimeMs);
    void               gameLoopUpdateBubbles(float deltaTimeMs);
    void               gameLoopUpdateAttractoBuff(float deltaTimeMs) const;
    [[nodiscard]] bool gameLoopUpdateBubbleClick(sf::base::Optional<sf::Vec2f>& clickPosition);
    void               gameLoopUpdateCatActionNormal(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionUni(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionDevil(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionAstro(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionWarden(float /* deltaTimeMs */, Cat& cat);
    [[nodiscard]] Cat* getSessionTargetCat(const HexSession& session) const;
    [[nodiscard]] bool anyCatHexedOrCopyHexed() const;
    [[nodiscard]] bool canHexMore() const;
    [[nodiscard]] bool canCopyHexMore() const;
    void               hexCat(Cat& cat, SizeT catIdx, bool copy);
    void               gameLoopUpdateCatActionWitchImpl(float /* deltaTimeMs */,
                                                        Cat&                          cat,
                                                        sf::base::Vector<HexSession>& sessionsToUse,
                                                        SizeT                         nCatsToHex);
    void               gameLoopUpdateCatActionWitch(float deltaTimeMs, Cat& cat);
    void               gameLoopUpdateCatActionWizard(float deltaTimeMs, Cat& cat);
    void               gameLoopUpdateCatActionMouse(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionEngi(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionRepulso(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionAttracto(float /* deltaTimeMs */, Cat& cat);
    void               gameLoopUpdateCatActionCopy(float deltaTimeMs, Cat& cat);
    void               gameLoopUpdateCatActionDuck(float deltaTimeMs, Cat& cat);
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
            if (bubble.type == BubbleType::Combo)
                return ControlFlow::Continue;

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

    void collectDollImpl(Doll& d, HexSession& session, bool copy);
    void collectDoll(Doll& d, HexSession& session);
    void collectCopyDoll(Doll& d, HexSession& session);

    void addEventBubblefall(float regionCenterX);
    void addEventInvincibleBubble();

    void gameLoopUpdateDollsImpl(float deltaTimeMs, sf::Vec2f mousePos, sf::base::Vector<HexSession>& sessionsToUse, bool copy);
    void gameLoopUpdateDolls(float deltaTimeMs, sf::Vec2f mousePos);
    void gameLoopUpdateCopyDolls(float deltaTimeMs, sf::Vec2f mousePos);
    void gameLoopUpdateHellPortals(float deltaTimeMs);
    void gameLoopUpdateWitchBuffs(float deltaTimeMs);
    void gameLoopUpdateEvents(float deltaTimeMs);
    void gameLoopReapEphemeralBubbles();
    void gameLoopDrawEvents();

    [[nodiscard]] bool canCatNap(const Cat& cat) const;
    void               beginCatNap(Cat& cat, float sleepDurationMs);
    void               gameLoopUpdateNapScheduler(float deltaTimeMs);

    // Apply the Power Nap cooldown-reduction boost to a cat being forcibly
    // woken (shake or wardencat bonk). No-op when the upgrade isn't owned.
    void applyPowerNapBoost(Cat& cat);

    void popComboBubble(Bubble& bubble);
    void gameLoopUpdateComboBubblePayouts(float deltaTimeMs);
    void gameLoopDrawComboBubbleBurstingCoins();
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

        notificationState.queue.emplaceBack(title, sf::base::String{fmtBuffer});
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
                         const sf::Rect2f* const (&catTailTxrsByType)[nCatTypes]);

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
    void drawMinimap(bool back, sf::RenderTarget& rt, const sf::View& hudView, sf::Vec2f resolution, sf::base::U8 shouldDrawUIAlpha);
    void drawSplashScreen(sf::RenderTarget& rt, const sf::View& view, sf::Vec2f resolution, float hudScale) const;
    [[nodiscard]] sf::Rect2f getViewportPixelBounds(const sf::View& view, sf::Vec2f targetSize) const;
    void                     gameLoopDrawDollParticleBorder(float hueMod);
    void                     gameLoopTips(float deltaTimeMs);
    void                     recreateImGuiRenderTexture(sf::Vec2u newResolution);
    void                     recreateBackgroundRenderTexture(sf::Vec2u newResolution);
    void                     recreateGameRenderTexture(sf::Vec2u newResolution);
    void setPostProcessUniforms(float vibrance, float saturation, float lightness, float sharpness, float blur) const;
    void updateProcessedBackground();
    void drawActivatedShrineBackgroundEffects(sf::RenderTarget& rt,
                                              const sf::View&   backgroundView,
                                              sf::Vec2f         activeGameViewCenter) const;
    [[nodiscard]] sf::RenderTexture& getHexedCatRenderTexture(sf::base::SizeT index);
    void enqueueHexedCatDrawCommand(const sf::CPUDrawableBatch& batch, sf::Vec2f position, bool top, float phaseSeed, float effectStrength);
    void                drawHexedCatDrawCommands(const sf::View& view, bool top);
    void                gameLoopDisplayCloudBatch(const sf::CPUDrawableBatch& batch, const sf::View& view);
    void                recreateWindow();
    void                resizeWindow();
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

    void drawCloudFrame(const CloudFrameDrawSettings& settings);

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

#undef BUBBLE_IDLE_PRINTF_FORMAT
