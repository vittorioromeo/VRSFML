#pragma once


// #define BUBBLEBYTE_NO_AUDIO 1
#include "Achievements.hpp"
#include "Aliases.hpp"
#include "Bubble.hpp"
#include "BubbleType.hpp"
#include "CatType.hpp"
#include "Constants.hpp"
#include "ExactArray.hpp"
#include "HexSession.hpp"
#include "Particle.hpp"
#include "ParticleData.hpp"
#include "ParticleType.hpp"
#include "PlayerInput.hpp"
#include "PurchasableScalingValue.hpp"
#include "RNGSeedType.hpp"
#include "ShrineType.hpp"
#include "Sounds.hpp"
#include "TextEffectWiggle.hpp"
#include "TextParticle.hpp"
#include "TextShakeEffect.hpp"

#include "ExampleUtils/Progress.hpp"
#include "ExampleUtils/RNGFast.hpp"
#include "ExampleUtils/Sampler.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ThreadPool.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#if defined(__GNUC__) || defined(__clang__)
    #define BUBBLE_IDLE_PRINTF_FORMAT(fmtIndex, firstArgIndex) __attribute__((format(printf, fmtIndex, firstArgIndex)))
#else
    #define BUBBLE_IDLE_PRINTF_FORMAT(fmtIndex, firstArgIndex)
#endif


////////////////////////////////////////////////////////////
struct ImFont;
struct BGMBuffer;
struct ComboState;
struct LoadedSound;
struct SoundManager;
struct Doll;
struct Shrine;

namespace sf
{
class AudioContext;
class CPUDrawableBatch;
class Font;
class GraphicsContext;
class ImGuiContext;
class OutFile;
class Path;
class PlaybackDevice;
class RenderTarget;
class RenderTexture;
class RenderWindow;
class Text;
class Texture;
class TextureAtlas;
struct DrawTextureSettings;
struct Listener;
struct RenderStates;
struct Sprite;
struct TextData;
struct View;
} // namespace sf

struct BubbleIgnoreFlags;
struct CloudFrameDrawSettings;
struct FrameViewState;
struct GameConstants;
class InputHelper;
struct MainAtlasRects;
struct MainBGMStorage;
struct MainBombStorage;
struct MainDrawableBatches;
struct MainGameStorage;
struct MainShaders;
struct MainTextureStorage;
struct MainTextStorage;
struct NotificationState;
struct Playthrough;
struct Profile;
struct UIState;

using MainRenderTextureVector = sf::base::Vector<sf::RenderTexture>;

template <typename T>
struct MainOwnedDeleter
{
    void operator()(T* ptr) noexcept;
};

template <typename T>
using MainOwnedPtr = sf::base::UniquePtr<T, MainOwnedDeleter<T>>;


////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    #include "Steam.hpp"
#endif


////////////////////////////////////////////////////////////<
extern bool debugMode;


////////////////////////////////////////////////////////////
class SweepAndPrune;


////////////////////////////////////////////////////////////
bool handleBubbleCollision(float deltaTimeMs, Bubble& iBubble, Bubble& jBubble);


////////////////////////////////////////////////////////////
bool handleCatCollision(float deltaTimeMs, Cat& iCat, Cat& jCat);


////////////////////////////////////////////////////////////
bool handleCatShrineCollision(float deltaTimeMs, Cat& cat, Shrine& shrine);


////////////////////////////////////////////////////////////
// Definition in `BubbleIdleMainInline.hpp`
[[nodiscard]] Bubble makeRandomBubble(Playthrough& pt, RNGFast& rng, float mapLimit, float maxY);

/// Main struct
///
////////////////////////////////////////////////////////////
struct Main
{
////////////////////////////////////////////////////////////
// Audio context and playback device
#ifndef BUBBLEBYTE_NO_AUDIO
    MainOwnedPtr<sf::AudioContext>   audioContextStorage;
    sf::AudioContext&                audioContext;
    MainOwnedPtr<sf::PlaybackDevice> playbackDeviceStorage;
    sf::PlaybackDevice&              playbackDevice;
#endif

    ////////////////////////////////////////////////////////////
    // Graphics context
    MainOwnedPtr<sf::GraphicsContext> graphicsContextStorage;
    sf::GraphicsContext&              graphicsContext;

    ////////////////////////////////////////////////////////////
    // Shaders + their uniform locations -- live in `MainShaders`
    MainOwnedPtr<MainShaders> shadersStorage;
    MainShaders&              shaders;

    float shaderTime = 0.f;

    ////////////////////////////////////////////////////////////
    // Context settings
    const unsigned int aaLevel;

    ////////////////////////////////////////////////////////////
    // Game-state storage: profile, game constants, playthroughs
    MainOwnedPtr<MainGameStorage> gameStorage;

    ///////////////////////////////////////////////////////////
    // Profile (stores settings)
    Profile& profile;

    ///////////////////////////////////////////////////////////
    // Game constants (loaded once on startup)
    GameConstants& gameConstants;

    ////////////////////////////////////////////////////////////
    // SFML fonts
    MainOwnedPtr<sf::Font> fontMouldyCheeseStorage;
    sf::Font&              fontMouldyCheese;

    ////////////////////////////////////////////////////////////
    // Render window
    [[nodiscard]] sf::RenderWindow makeWindow();
    MainOwnedPtr<sf::RenderWindow> windowStorage;
    sf::RenderWindow&              window;
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
    MainOwnedPtr<sf::ImGuiContext> imGuiContextStorage;
    sf::ImGuiContext&              imGuiContext;

    ////////////////////////////////////////////////////////////
    // Exiting status
    PlayerInputState playerInputState;
    bool             mustExit{false};

    ////////////////////////////////////////////////////////////
    // Texture atlas
    MainOwnedPtr<sf::TextureAtlas> textureAtlasStorage;
    sf::TextureAtlas&              textureAtlas; // TODO P0: make smaller

    ////////////////////////////////////////////////////////////
    // SFML fonts
    MainOwnedPtr<sf::Font> fontSuperBakeryStorage;
    sf::Font&              fontSuperBakery;

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

    MainOwnedPtr<MainBGMStorage> bgmStorage;
    MainBGMStorage&              bgm;

    ////////////////////////////////////////////////////////////
    // Sound management
    Sounds sounds{/* volumeMult */ 1.f};

    MainOwnedPtr<SoundManager> soundManagerStorage;
    SoundManager&              soundManager;
    MainOwnedPtr<sf::Listener> listenerStorage;
    sf::Listener&              listener;

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
    MainOwnedPtr<sf::RenderTexture> rtBackgroundStorage;
    sf::RenderTexture&              rtBackground;

    MainOwnedPtr<sf::RenderTexture> rtBackgroundProcessedStorage;
    sf::RenderTexture&              rtBackgroundProcessed;

    MainOwnedPtr<sf::RenderTexture> rtImGuiStorage;
    sf::RenderTexture&              rtImGui;

    MainOwnedPtr<sf::RenderTexture> rtCloudMaskStorage;
    sf::RenderTexture&              rtCloudMask;

    MainOwnedPtr<sf::RenderTexture> rtCloudProcessedStorage;
    sf::RenderTexture&              rtCloudProcessed;

    ////////////////////////////////////////////////////////////
    // Game render texture (before post-processing)
    MainOwnedPtr<sf::RenderTexture> rtGameStorage;
    sf::RenderTexture&              rtGame;

    ////////////////////////////////////////////////////////////
    // Hexed cat offscreen render textures (one per concurrent hex, for witch and copy-witch combined)
    static inline constexpr sf::Vec2u       hexedCatRenderTextureSize{640u, 640u};
    static inline constexpr sf::base::SizeT maxHexedCatRenderTextures = maxConcurrentHexes * 2u;

    MainOwnedPtr<MainRenderTextureVector> hexedCatRenderTexturesStorage;
    MainRenderTextureVector&              hexedCatRenderTextures;

    ////////////////////////////////////////////////////////////
    // Textures (not in atlas)
    MainOwnedPtr<MainTextureStorage> textureStorage;

    sf::Texture& txLogo;
    sf::Texture& txFixedBg;
    sf::Texture& txBackgroundChunk;
    sf::Texture& txBackgroundChunkDesaturated;
    sf::Texture& txClouds;
    sf::Texture& txTintedClouds;
    sf::Texture& txBgSwamp;
    sf::Texture& txBgObservatory;
    sf::Texture& txBgAimTraining;
    sf::Texture& txBgFactory;
    sf::Texture& txBgWindTunnel;
    sf::Texture& txBgMagnetosphere;
    sf::Texture& txBgAuditorium;
    sf::Texture& txDrawings;
    sf::Texture& txTipBg;
    sf::Texture& txTipByte;
    sf::Texture& txCursor;
    sf::Texture& txCursorMultipop;
    sf::Texture& txCursorLaser;
    sf::Texture& txCursorGrab;
    sf::Texture& txArrow;
    sf::Texture& txUnlock;
    sf::Texture& txPurchasable;
    sf::Texture& txLetter;
    sf::Texture& txLetterText;
    sf::Texture& txFrame;
    sf::Texture& txFrameTiny;
    sf::Texture& txCloudBtn;
    sf::Texture& txCloudBtnSmall;
    sf::Texture& txCloudBtnSquare;
    sf::Texture& txCloudBtnSquare2;

    ////////////////////////////////////////////////////////////
    // UI texture atlas
    MainOwnedPtr<sf::TextureAtlas> uiTextureAtlasStorage;
    sf::TextureAtlas&              uiTextureAtlas;

    ////////////////////////////////////////////////////////////
    // Atlas rects + cat animation rect arrays -- live in `MainAtlasRects`
    MainOwnedPtr<MainAtlasRects> atlasRectsStorage;
    MainAtlasRects&              atlasRects;

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
    // Playthrough (game state) -- storage lives in `gameStorage`
    Playthrough& ptMain;
    Playthrough& ptSpeedrun;

    ////////////////////////////////////////////////////////////
    // Currently active playthrough (game state)
    Playthrough* pt;

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
    MainOwnedPtr<MainTextStorage> textStorage;
    sf::Text&                     moneyText;
    TextShakeEffect               moneyTextShakeEffect;

    ////////////////////////////////////////////////////////////
    // Combo state
    static inline constexpr sf::Vec2f moneyTextInitialPosition{10.f, 70.f};
    MainOwnedPtr<ComboState>          comboStateStorage;
    ComboState&                       comboState;

    ////////////////////////////////////////////////////////////
    // HUD demo text
    sf::Text& demoText;

    ////////////////////////////////////////////////////////////
    // Spatial partitioning
    sf::base::UniquePtr<SweepAndPrune> sweepAndPrune;

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
        Progress  progress{};
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
    TimedCountdown bubbleSpawnTimer{.duration = 3.f};
    TimedCountdown catRemoveTimer{.duration = 100.f};

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
    MainOwnedPtr<MainDrawableBatches> drawableBatchesStorage;

    sf::CPUDrawableBatch& bubbleDrawableBatch;
    sf::CPUDrawableBatch& starBubbleDrawableBatch;
    sf::CPUDrawableBatch& bombBubbleDrawableBatch;
    sf::CPUDrawableBatch& cpuCloudDrawableBatch;
    sf::CPUDrawableBatch& cpuTopCloudDrawableBatch;
    sf::CPUDrawableBatch& cpuCloudHudDrawableBatch;
    sf::CPUDrawableBatch& cpuCloudUiDrawableBatch;
    sf::CPUDrawableBatch& cpuDrawableBatchBeforeCats;
    sf::CPUDrawableBatch& cpuDrawableBatch;
    sf::CPUDrawableBatch& cpuDrawableBatchAfterCats;
    sf::CPUDrawableBatch& cpuDrawableBatchAdditive;
    sf::CPUDrawableBatch& minimapDrawableBatch;
    sf::CPUDrawableBatch& catTextDrawableBatch;
    sf::CPUDrawableBatch& hudDrawableBatch;
    sf::CPUDrawableBatch& hudTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch& hudBottomDrawableBatch;  // drawn below ImGui
    sf::CPUDrawableBatch& cpuTopDrawableBatch;     // drawn on top of ImGui
    sf::CPUDrawableBatch& catTextTopDrawableBatch; // drawn on top of ImGui
    sf::CPUDrawableBatch& tempDrawableBatch;       // for misc one-off draws (hexed cat effect)

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
    TimedCountdown splashCountdown{.duration = 1.f}; // TODO P0: revert to 2500

    ////////////////////////////////////////////////////////////
    // Tip state
    sf::base::Optional<TimedCountdown> tipTCByte;
    sf::base::Optional<TimedCountdown> tipTCBackground;
    sf::base::Optional<TimedCountdown> tipTCBytePreEnd;
    sf::base::Optional<TimedCountdown> tipTCByteEnd;
    sf::base::Optional<TimedCountdown> tipTCBackgroundEnd;
    Countdown                          tipCountdownChar;
    sf::base::String                   tipString;
    TextEffectWiggle                   tipStringWiggle{0.00175f, 4.f};
    sf::base::SizeT                    tipCharIdx{0u};

    ////////////////////////////////////////////////////////////
    // Text buffers
    sf::Text& textNameBuffer;
    sf::Text& textStatusBuffer;
    sf::Text& textMoneyBuffer;

    ////////////////////////////////////////////////////////////
    // Spent money count-down effect
    MoneyType spentMoney{0u};
    Progress  spentMoneyTimer{};

    ////////////////////////////////////////////////////////////
    // Thread pool
    sf::base::ThreadPool threadPool;

    ////////////////////////////////////////////////////////////
    // Cached views
    MainOwnedPtr<sf::View> gameViewStorage;
    sf::View&              gameView; // TODO P1: compute on the fly, don't cache...

    MainOwnedPtr<sf::View> nonScaledHUDViewStorage;
    sf::View&              nonScaledHUDView; // TODO P1: compute on the fly, don't cache...

    MainOwnedPtr<sf::View> scaledHUDViewStorage;
    sf::View&              scaledHUDView; // TODO P1: compute on the fly, don't cache...

    ////////////////////////////////////////////////////////////
    // $ps sampler
    MoneyType      moneyGainedLastSecond{0u};
    Sampler<float> samplerMoneyPerSecond{/* capacity */ 60u};
    sf::base::I64  moneyGainedUsAccumulator{0};

    ////////////////////////////////////////////////////////////
    // Bomb-cat tracker for money earned
    MainOwnedPtr<MainBombStorage> bombStorage;

    ////////////////////////////////////////////////////////////
    // Notification queue
    MainOwnedPtr<NotificationState> notificationStateStorage;
    NotificationState&              notificationState;

    ////////////////////////////////////////////////////////////
    // FPS counter
    float fps{0.f};

    ////////////////////////////////////////////////////////////
    // UI state
    MainOwnedPtr<UIState> uiStateStorage;
    UIState&              uiState;

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
    sf::base::Optional<TimedCountdown> victoryTC;
    Countdown                          cdLetterAppear;
    Countdown                          cdLetterText;

    ////////////////////////////////////////////////////////////
    // Input management
    MainOwnedPtr<InputHelper> inputHelperStorage;
    InputHelper&              inputHelper;

    ////////////////////////////////////////////////////////////
    // Logging -- `logFile` is a non-owning view that may be null if
    // the log file failed to open (e.g. read-only filesystem).
    MainOwnedPtr<sf::OutFile> logFileStorage;
    sf::OutFile*              logFile;

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
    // Definitions in `BubbleIdleMainInline.hpp`
    void spawnParticles(SizeT n, sf::Vec2f position, auto... args);
    void spawnParticlesWithHue(float hue, SizeT n, sf::Vec2f position, auto... args);
    void spawnParticlesNoGravity(SizeT n, sf::Vec2f position, auto... args);
    void spawnParticlesWithHueNoGravity(float hue, SizeT n, sf::Vec2f position, auto... args);

    ////////////////////////////////////////////////////////////
    // Definition in `BubbleIdleMainInline.hpp`
    void withAllStats(auto&& func);

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
    [[nodiscard]] sf::Rect2f addImgResourceToAtlas(const sf::Path& path);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f addImgResourceToUIAtlas(const sf::Path& path);

    ////////////////////////////////////////////////////////////
    void playSound(const LoadedSound& ls, sf::base::SizeT maxOverlap = 255u);

    ////////////////////////////////////////////////////////////
    // Definitions in `BubbleIdleMainInline.hpp`
    void                  forEachBubbleInRadiusSquared(sf::Vec2f center, float radiusSq, auto&& func);
    void                  forEachBubbleInRadius(sf::Vec2f center, float radius, auto&& func);
    [[nodiscard]] Bubble* pickRandomBubbleInRadiusMatching(sf::Vec2f center, float radius, auto&& predicate);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Bubble* pickRandomBubbleInRadius(sf::Vec2f center, float radius);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getResolution() const;

    ////////////////////////////////////////////////////////////
    // Definitions in `BubbleIdleMainInline.hpp`
    [[nodiscard]] float getComputedCooldownByCatTypeOrCopyCat(CatType catType) const;
    [[nodiscard]] float getComputedRangeByCatTypeOrCopyCat(CatType catType) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getViewCenter() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getViewCenterWithoutScroll() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CullingBoundaries getViewCullingBoundaries(float offset) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static sf::Vec2f         getCatRangeCenter(const Cat& cat);
    [[nodiscard]] float                    getWindRepulsionMult() const;
    [[nodiscard]] float                    getWindAttractionMult() const;
    [[nodiscard]] static constexpr CatType shrineTypeToCatType(ShrineType shrineType);
    [[nodiscard]] sf::Sprite               particleToSprite(const Particle& particle) const;
    [[nodiscard]] sf::Vec2f                getEdgeSpawnPosition(const sf::Rect2f& bounds, float thickness);
    [[nodiscard]] sf::Vec2u                getNewResolution() const;

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
    // Definitions in `BubbleIdleMainInline.hpp`
    [[nodiscard]] bool isUnicatTranscendenceActive() const;
    [[nodiscard]] bool isDevilcatHellsingedActive() const;

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
        sf::Color idle;
        sf::Color hovered;
        sf::Color active;
    };

    [[nodiscard]] bool drawTabButton(float                   scaleMult,
                                     const char*             label,
                                     bool                    selected,
                                     const TabButtonPalette& palette,
                                     sf::Vec2f               size   = {},
                                     bool                    square = false);

    [[nodiscard]] AnimatedButtonOutcome uiAnimatedButton(
        const sf::Texture& tx,
        const char*        label,
        sf::Vec2f          btnSize,
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
    [[nodiscard]] bool makePSVButtonExByCurrency(
        const char*              label,
        PurchasableScalingValue& psv,
        SizeT                    times,
        MoneyType                cost,
        MoneyType&               availability,
        const char*              currencyFmt);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool makePurchasableButtonOneTimeByCurrency(
        const char* label,
        bool&       done,
        MoneyType   cost,
        MoneyType&  availability,
        const char* currencyFmt);

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
    struct FormatTimeResult
    {
        sf::base::U64 h;
        sf::base::U64 m;
        sf::base::U64 s;
    };

    [[nodiscard]] static constexpr FormatTimeResult formatTime(const sf::base::U64 seconds)
    {
        return {seconds / 3600u, (seconds / 60u) % 60u, seconds % 60u};
    }

    ////////////////////////////////////////////////////////////
    struct FormatSpeedrunTimeResult
    {
        sf::base::U64 hours;
        sf::base::U64 mins;
        sf::base::U64 secs;
        sf::base::U64 millis;
    };

    [[nodiscard]] static constexpr FormatSpeedrunTimeResult formatSpeedrunTime(const sf::Time time)
    {
        const sf::base::I64 elapsedTime       = time.asMicroseconds();
        const sf::base::U64 totalMicroseconds = (elapsedTime >= 0) ? static_cast<sf::base::U64>(elapsedTime) : 0ULL;

        constexpr sf::base::U64 usPerMs   = 1000ULL;
        constexpr sf::base::U64 usPerSec  = 1000ULL * usPerMs; // 1,000,000
        constexpr sf::base::U64 usPerMin  = 60ULL * usPerSec;  // 60,000,000
        constexpr sf::base::U64 usPerHour = 60ULL * usPerMin;  // 3,600,000,000

        return {totalMicroseconds / usPerHour,
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

    // Resolve a queued wardencat windup: deals the bonk to
    // `cat.pawBonkPendingTargetIdx` (if still valid + napping), kicks the
    // travel/hold animation, and clears the pending state. Called from the
    // per-frame paw update when the windup countdown reaches zero.
    void               resolveWardenBonkStrike(Cat& cat);
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
        sf::Vec2f          position,
        CatType            catType,
        float              deltaTimeMs,
        auto               countdownPm,
        float              countdownTime,
        float              strengthMult,
        float              direction,
        BubbleIgnoreFlags& ignoreFlags);

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
    void pushNotification(const char* title, const char* format, ...) BUBBLE_IDLE_PRINTF_FORMAT(3, 4);

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
    sf::TextData        gameLoopUpdateComboText(float deltaTimeMs, float yBelowMinimap);
    sf::TextData        gameLoopUpdateBuffText(const sf::Rect2f& comboBounds);
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
    ~Main();

    void run();
};

#undef BUBBLE_IDLE_PRINTF_FORMAT
