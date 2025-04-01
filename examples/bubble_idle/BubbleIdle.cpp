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
#include "ImGuiNotify.hpp"
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

#include "SFML/ImGui/ImGui.hpp"

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
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
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
#include "SFML/System/RectUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Strcmp.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Lround.hpp"
#include "SFML/Base/Math/Pow.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/ThreadPool.hpp"

#include <cctype>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstring>


////////////////////////////////////////////////////////////
#ifndef SFML_SYSTEM_EMSCRIPTEN
    #define BUBBLEBYTE_USE_STEAMWORKS 1
#endif

#ifdef BUBBLEBYTE_USE_STEAMWORKS
    #include "Steam.hpp"
#endif


////////////////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


namespace
{
////////////////////////////////////////////////////////////
bool debugMode = false;

////////////////////////////////////////////////////////////
bool handleBubbleCollision(const float deltaTimeMs, Bubble& iBubble, Bubble& jBubble)
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
bool handleCatCollision(const float deltaTimeMs, Cat& iCat, Cat& jCat)
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
bool handleCatShrineCollision(const float deltaTimeMs, Cat& cat, Shrine& shrine)
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
// TODO P2: (lib) to lib?
[[nodiscard, gnu::const]] constexpr sf::FloatRect clampScissorRect(sf::FloatRect rect)
{
    // Clamp the position to the range [0, 1]
    rect.position.x = sf::base::clamp(rect.position.x, 0.f, 1.f);
    rect.position.y = sf::base::clamp(rect.position.y, 0.f, 1.f);

    // Ensure the size is non-negative
    rect.size.x = sf::base::max(rect.size.x, 0.f);
    rect.size.y = sf::base::max(rect.size.y, 0.f);

    // Adjust the size so that position + size doesn't exceed 1
    if (rect.position.x + rect.size.x > 1.f)
        rect.size.x = 1.f - rect.position.x;

    if (rect.position.y + rect.size.y > 1.f)
        rect.size.y = 1.f - rect.position.y;

    return rect;
}

////////////////////////////////////////////////////////////
void drawMinimap(
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
    const sf::Vector2f      resolution,
    const float             hudScale,
    const float             hueMod,
    const sf::base::U8      shouldDrawUIAlpha,
    sf::FloatRect&          minimapRect)
{
    //
    // Screen position of minimap's top-left corner
    constexpr sf::Vector2f minimapPos = {15.f, 15.f};

    //
    // Size of full map in minimap space
    const sf::Vector2f minimapSize = boundaries / minimapScale;

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
                         .outlineColor = sf::Color::Blue.withHueMod(hueMod).withAlpha(shouldDrawUIAlpha),
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

    const auto clampedScissorRect = clampScissorRect(preClampScissorRect);

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
    const sf::Vector2f backgroundRectSize{static_cast<float>(txBackgroundChunk.getSize().x) * nGameScreens,
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
void drawSplashScreen(sf::RenderTarget&        rt,
                      const sf::Texture&       txLogo,
                      const TargetedCountdown& splashCountdown,
                      const sf::Vector2f       resolution,
                      const float              hudScale)
{
    const auto progress = easeInOutCubic(splashCountdown.getProgressBounced());

    rt.draw(sf::Sprite{.position    = resolution / 2.f / hudScale,
                       .scale       = sf::Vector2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutCubic(progress)) / hudScale,
                       .origin      = txLogo.getSize().toVector2f() / 2.f,
                       .textureRect = txLogo.getRect(),
                       .color       = sf::Color::whiteMask(static_cast<U8>(easeInOutSine(progress) * 255.f))},
            {.texture = &txLogo});
}

} // namespace


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
    sf::PlaybackDevice playbackDevice{sf::PlaybackDevice::createDefault(audioContext).value()};
#endif

    ////////////////////////////////////////////////////////////
    // Graphics context
    sf::GraphicsContext graphicsContext{sf::GraphicsContext::create().value()};

    ////////////////////////////////////////////////////////////
    // Shader with hue support and bubble effects
    sf::Shader shader{[]
    {
        // TODO P2: (lib) nicer interface with designated inits
        // TODO P2: (lib) add support for `#include` in shaders
        auto result = sf::Shader::loadFromFile("resources/shader.vert", "resources/shader.frag").value();
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
        // TODO P2: (lib) nicer interface with designated inits
        // TODO P2: (lib) add support for `#include` in shaders
        auto result = sf::Shader::loadFromFile("resources/postprocess.frag", sf::Shader::Type::Fragment).value();
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

    ////////////////////////////////////////////////////////////
    // Render window
    sf::base::Optional<sf::RenderWindow> optWindow;
    bool                                 mustRecreateWindow = true;
    float                                dpiScalingFactor   = 1.f;

    ////////////////////////////////////////////////////////////
    // ImGui context
    sf::ImGui::ImGuiContext imGuiContext;

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
    sf::Font fontMouldyCheese{sf::Font::openFromFile("resources/mouldycheese.ttf").value()};

    ////////////////////////////////////////////////////////////
    // ImGui fonts
    ImFont* fontImGuiSuperBakery{nullptr};
    ImFont* fontImGuiMouldyCheese{nullptr};

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

    sf::base::Array<sf::base::Optional<sf::Music>, 2u> bgmBuffers{sf::base::nullOpt, sf::base::nullOpt}; // for smooth fade

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

    std::vector<DelayedAction> delayedActions;

    ////////////////////////////////////////////////////////////
    // Background and ImGui render textures
    sf::base::Optional<sf::RenderTexture> rtBackground;
    sf::base::Optional<sf::RenderTexture> rtImGui;

    ////////////////////////////////////////////////////////////
    // Game render texture (before post-processing)
    sf::base::Optional<sf::RenderTexture> rtGame;

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
    sf::FloatRect txrWhiteDot{textureAtlas.add(graphicsContext.getBuiltInWhiteDotTexture()).value()};
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

    ///////////////////////////////////////////////////////////
    // Profile (stores settings)
    Profile profile;
    MEMBER_SCOPE_GUARD(Main, {
        sf::cOut() << "Saving profile to file on exit\n";
        saveProfileToFile(self.profile);
    });

    ////////////////////////////////////////////////////////////
    // Playthrough (game state)
    Playthrough pt;
    MEMBER_SCOPE_GUARD(Main, {
        sf::cOut() << "Saving playthrough to file on exit\n";
        savePlaythroughToFile(self.pt);
    });

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
    // Spatial partitioning
    SweepAndPrune sweepAndPrune;

    ////////////////////////////////////////////////////////////
    // Particles
    std::vector<Particle>     particles;          // World space
    std::vector<TextParticle> textParticles;      // World space
    std::vector<Particle>     spentCoinParticles; // HUD space
    std::vector<Particle>     hudBottomParticles; // HUD space, drawn below ImGui
    std::vector<Particle>     hudTopParticles;    // HUD space, drawn on top of ImGui

    struct EarnedCoinParticle
    {
        sf::Vector2f startPosition;
        Timer        progress;
    };

    std::vector<EarnedCoinParticle> earnedCoinParticles; // HUD space

    ////////////////////////////////////////////////////////////
    // Random number generation
    RNG     rng{/* seed */ std::random_device{}()};
    RNGFast rngFast; // very fast, low-quality, but good enough for VFXs

    ////////////////////////////////////////////////////////////
    // Cat names
    std::vector<std::vector<std::string>> shuffledCatNamesPerType = makeShuffledCatNames(rng);

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
    sf::base::Optional<sf::Vector2f> dragPosition;
    float                            scroll{0.f};
    float                            actualScroll{0.f};

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
        [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr bool isInside(
            const sf::Vector2f point) const noexcept
        {
            return (point.x >= left) && (point.x <= right) && (point.y >= top) && (point.y <= bottom);
        }
    };

    CullingBoundaries hudCullingBoundaries{};
    CullingBoundaries particleCullingBoundaries{};
    CullingBoundaries bubbleCullingBoundaries{};

    ////////////////////////////////////////////////////////////
    // Last frame mouse position (world space)
    sf::Vector2f lastMousePos;

    ////////////////////////////////////////////////////////////
    // Cat dragging state
    float                            catDragPressDuration{0.f};
    sf::base::Optional<sf::Vector2f> catDragOrigin;
    std::vector<Cat*>                draggedCats;
    Cat*                             catToPlace{nullptr};

    ////////////////////////////////////////////////////////////
    // Touch state
    std::vector<sf::base::Optional<sf::Vector2f>> fingerPositions;

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
    std::unordered_map<sf::base::SizeT, sf::base::SizeT> bombIdxToCatIdx;

    ////////////////////////////////////////////////////////////
    // Notification queue
    struct NotificationData
    {
        const char* title;
        std::string content;
    };

    std::vector<NotificationData> notificationQueue;
    TargetedCountdown             notificationCountdown{.startingValue = 750.f};

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

    std::vector<PurchaseUnlockedEffect>   purchaseUnlockedEffects;
    std::unordered_map<std::string, bool> btnWasDisabled;

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
    std::ofstream logFile{"bubblebyte.log", std::ios::out | std::ios::app};

    ////////////////////////////////////////////////////////////
    void log(const char* format, ...)
    {
        if (!logFile)
            return;

        const auto timestamp = std::chrono::system_clock::now();
        const auto timeT     = std::chrono::system_clock::to_time_t(timestamp);
        const auto tm        = *std::localtime(&timeT);

        char buffer[1024];

        va_list args{};
        va_start(args, format);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        vsnprintf(buffer, sizeof(buffer), format, args);
#pragma GCC diagnostic pop
        va_end(args);

        logFile << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " - " << buffer << '\n';
        logFile.flush();
    }

    ////////////////////////////////////////////////////////////
    void addMoney(const MoneyType reward)
    {
        pt.money += reward;
        moneyGainedLastSecond += reward;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::vector<std::vector<std::string>> makeShuffledCatNames(RNG& rng)
    {
        std::vector<std::vector<std::string>> result(nCatTypes);

        for (SizeT i = 0u; i < nCatTypes; ++i)
            result[i] = getShuffledCatNames(static_cast<CatType>(i), rng.getEngine());

        return result;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getTPWorkerCount()
    {
        const unsigned int numThreads = std::thread::hardware_concurrency();
        return (numThreads == 0u) ? 3u : numThreads - 1u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SizeT getNextCatNameIdx(const CatType catType)
    {
        return pt.nextCatNamePerType[asIdx(catType)]++ % shuffledCatNamesPerType[asIdx(catType)].size();
    }

    ////////////////////////////////////////////////////////////
    Particle& implEmplaceParticle(const sf::Vector2f position,
                                  const ParticleType particleType,
                                  const float        scaleMult,
                                  const float        speedMult,
                                  const float        opacity = 1.f)
    {
        return particles.emplace_back(
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
            0.f,
            particleType);
    }

    ////////////////////////////////////////////////////////////
    bool spawnSpentCoinParticle(const ParticleData& particleData)
    {
        if (!profile.showParticles || !hudCullingBoundaries.isInside(particleData.position))
            return false;

        spentCoinParticles.emplace_back(particleData, 0u, ParticleType::Coin);
        return true;
    }

    ////////////////////////////////////////////////////////////
    void spawnHUDTopParticle(const ParticleData& particleData, const float hue, const ParticleType particleType)
    {
        if (!profile.showParticles || !hudCullingBoundaries.isInside(particleData.position))
            return;

        hudTopParticles.emplace_back(particleData, hueToByte(hue), particleType);
    }

    ////////////////////////////////////////////////////////////
    void spawnHUDBottomParticle(const ParticleData& particleData, const float hue, const ParticleType particleType)
    {
        if (!profile.showParticles || !hudCullingBoundaries.isInside(particleData.position))
            return;

        hudBottomParticles.emplace_back(particleData, hueToByte(hue), particleType);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool spawnEarnedCoinParticle(const sf::Vector2f startPosition)
    {
        if (!profile.showParticles || !profile.showCoinParticles || !hudCullingBoundaries.isInside(startPosition))
            return false;

        earnedCoinParticles.emplace_back(startPosition);
        return true;
    }

    ////////////////////////////////////////////////////////////
    void spawnParticle(const ParticleData& particleData, const float hue, const ParticleType particleType)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(particleData.position))
            return;

        // TODO P2: consider optimizing this pattern by just returning the emplaced particle and having the caller set the data
        particles.emplace_back(particleData, hueToByte(hue), particleType);
    }

    ////////////////////////////////////////////////////////////
    void spawnParticles(const SizeT n, const sf::Vector2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
            implEmplaceParticle(position, args...);
    }

    ////////////////////////////////////////////////////////////
    void spawnParticlesWithHue(const float hue, const SizeT n, const sf::Vector2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
            implEmplaceParticle(position, args...).hueByte = hueToByte(hue);
    }

    ////////////////////////////////////////////////////////////
    void spawnParticlesNoGravity(const SizeT n, const sf::Vector2f position, const auto... args)
    {
        if (!profile.showParticles || !particleCullingBoundaries.isInside(position))
            return;

        for (SizeT i = 0; i < n; ++i)
            implEmplaceParticle(position, args...).accelerationY = 0.f;
    }

    ////////////////////////////////////////////////////////////
    void spawnParticlesWithHueNoGravity(const float hue, const SizeT n, const sf::Vector2f position, const auto... args)
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
        func(pt.statsTotal);
        func(pt.statsSession);
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
    sf::RenderWindow& getWindow()
    {
        SFML_BASE_ASSERT(optWindow.hasValue());
        return *optWindow;
    }

    ////////////////////////////////////////////////////////////
    const sf::RenderWindow& getWindow() const
    {
        SFML_BASE_ASSERT(optWindow.hasValue());
        return *optWindow;
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
    void addDelayedAction(const float delayMs, sf::base::FixedFunction<void(), 128>&& f)
    {
        delayedActions.emplace_back(Countdown{.value = delayMs}, SFML_BASE_MOVE(f));
    }

    ////////////////////////////////////////////////////////////
    void forEachBubbleInRadiusSquared(const sf::Vector2f center, const float radiusSq, auto&& func)
    {
        for (Bubble& bubble : pt.bubbles)
            if ((bubble.position - center).lengthSquared() <= radiusSq)
                if (func(bubble) == ControlFlow::Break)
                    break;
    }

    ////////////////////////////////////////////////////////////
    void forEachBubbleInRadius(const sf::Vector2f center, const float radius, auto&& func)
    {
        forEachBubbleInRadiusSquared(center, radius * radius, func);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Bubble* pickRandomBubbleInRadiusMatching(const sf::Vector2f center, const float radius, auto&& predicate)
    {
        const float radiusSq = radius * radius;

        SizeT   count    = 0u;
        Bubble* selected = nullptr;

        for (Bubble& bubble : pt.bubbles)
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
    [[nodiscard]] Bubble* pickRandomBubbleInRadius(const sf::Vector2f center, const float radius)
    {
        return pickRandomBubbleInRadiusMatching(center,
                                                radius,
                                                [](const Bubble&) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN { return true; });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vector2f getResolution() const
    {
        return getWindow().getSize().toVector2f();
    }

    ////////////////////////////////////////////////////////////
    Cat& spawnCat(const sf::Vector2f pos, const CatType catType, const float hue)
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

        sounds.purrmeow.setPosition({pos.x, pos.y});
        sounds.purrmeow.setPitch(meowPitch);
        playSound(sounds.purrmeow);

        spawnParticles(32, pos, ParticleType::Star, 0.5f, 0.75f);

        catToPlace = nullptr;

        return pt.cats.emplace_back(Cat{
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
        const auto pos = getWindow().mapPixelToCoords((getResolution() / 2.f).toVector2i(), gameView);

        Cat& newCat = spawnCat(pos, catType, hue);

        if (placeInHand)
        {
            catToPlace = &newCat;

            draggedCats.clear();
            draggedCats.push_back(&newCat);

            newCat.position    = lastMousePos;
            newCat.pawPosition = lastMousePos;
        }

        return newCat;
    }

    ////////////////////////////////////////////////////////////
    Cat& spawnSpecialCat(const sf::Vector2f pos, const CatType catType)
    {
        ++pt.psvPerCatType[static_cast<SizeT>(catType)].nPurchases;
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
        if (!profile.tipsEnabled || pt.psvBubbleValue.nPurchases > maxPrestigeLevel)
            return;

        playSound(sounds.byteMeow, /* maxOverlap */ 1u);

        resetTipState();

        tipTCByte.emplace(TargetedCountdown{.startingValue = 500.f});
        tipTCByte->restart();

        tipString = str + "\t\t\t\t\t";
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr float uiNormalFontScale    = 0.95f;
    static inline constexpr float uiSubBulletFontScale = 0.75f;
    static inline constexpr float uiToolTipFontScale   = 0.65f;
    static inline constexpr float uiWindowWidth        = 425.f;
    static inline constexpr float uiButtonWidth        = 150.f;
    const float                   uiTooltipWidth       = uiWindowWidth;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float uiGetMaxWindowHeight() const
    {
        return sf::base::max(getResolution().y - 30.f, (getResolution().y - 30.f) / profile.uiScale);
    }

    ////////////////////////////////////////////////////////////
    char         uiBuffer[256]{};
    char         uiLabelBuffer[512]{};
    char         uiTooltipBuffer[1024]{};
    float        uiButtonHueMod = 0.f;
    unsigned int uiWidgetId     = 0u;
    float        lastFontScale  = 1.f;

    ////////////////////////////////////////////////////////////
    void uiSetFontScale(const float scale)
    {
        lastFontScale = scale;
        ImGui::SetWindowFontScale(scale);
    }

    ////////////////////////////////////////////////////////////
    void uiMakeButtonLabels(const char* label, const char* xLabelBuffer)
    {
        const char* readableLabelEnd = ImGui::FindRenderedTextEnd(label);

        const bool subBullet = label[0] == ' ' && label[1] == ' ';

        const char* labelStart = subBullet ? label + 2 : label;
        const auto  labelSize  = readableLabelEnd - labelStart;


        // button label
        uiSetFontScale((subBullet ? uiSubBulletFontScale : uiNormalFontScale) * 1.15f);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%.*s", static_cast<int>(labelSize), labelStart);
        ImGui::SameLine();

        // button top label
        uiSetFontScale(0.5f);
        ImGui::Text("%s", xLabelBuffer);
        uiSetFontScale(subBullet ? uiSubBulletFontScale : uiNormalFontScale);
        ImGui::SameLine();

        ImGui::NextColumn();
    }

    ////////////////////////////////////////////////////////////
    void uiPushButtonColors()
    {
        const auto convertColorWithHueMod = [&](const auto colorId)
        {
            return sf::Color::fromVec4(ImGui::GetStyleColorVec4(colorId)).withHueMod(uiButtonHueMod).template toVec4<ImVec4>();
        };

        ImGui::PushStyleColor(ImGuiCol_Button, convertColorWithHueMod(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, convertColorWithHueMod(ImGuiCol_ButtonHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, convertColorWithHueMod(ImGuiCol_ButtonActive));
        ImGui::PushStyleColor(ImGuiCol_Border, colorBlueOutline.withHueMod(uiButtonHueMod).toVec4<ImVec4>());
    }

    ////////////////////////////////////////////////////////////
    void uiPopButtonColors()
    {
        ImGui::PopStyleColor(4);
    }

    ////////////////////////////////////////////////////////////
    void uiBeginTooltip(const float width)
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0), ImVec2(width, FLT_MAX));

        ImGui::BeginTooltip();
        ImGui::PushFont(fontImGuiMouldyCheese);
        uiSetFontScale(uiToolTipFontScale);
    }

    ////////////////////////////////////////////////////////////
    void uiEndTooltip()
    {
        uiSetFontScale(uiNormalFontScale);
        ImGui::PopFont();
        ImGui::EndTooltip();
    }

    ////////////////////////////////////////////////////////////
    void uiMakeTooltip(bool small = false)
    {
        if (!ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) || std::strlen(uiTooltipBuffer) == 0u)
            return;

        const float width = small ? 176.f : uiTooltipWidth;

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x - width, ImGui::GetMousePos().y + (small ? -40.f : 20.f)));

        uiBeginTooltip(width);
        ImGui::TextWrapped("%s", uiTooltipBuffer);
        uiEndTooltip();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isUnicatTranscendenceActive() const
    {
        return pt.perm.unicatTranscendencePurchased && pt.perm.unicatTranscendenceEnabled;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isDevilcatHellsingedActive() const
    {
        return pt.perm.devilcatHellsingedPurchased && pt.perm.devilcatHellsingedEnabled;
    }

    ////////////////////////////////////////////////////////////
    void uiMakeShrineOrCatTooltip(const sf::Vector2f mousePos)
    {
        const auto* hoveredShrine = [&]() -> const Shrine*
        {
            for (Shrine& shrine : pt.shrines)
                if ((mousePos - shrine.position).lengthSquared() <= shrine.getRadiusSquared())
                    return &shrine;

            return nullptr;
        }();

        const auto* hoveredCat = [&]() -> const Cat*
        {
            if (hoveredShrine == nullptr)
                for (const Cat& cat : pt.cats)
                    if ((mousePos - cat.position).lengthSquared() <= cat.getRadiusSquared())
                        return &cat;

            return nullptr;
        }();

        if ((hoveredShrine == nullptr && hoveredCat == nullptr) || std::strlen(uiTooltipBuffer) == 0u)
            return;

        ImGui::SetNextWindowPos(ImVec2(getResolution().x - 15.f, getResolution().y - 15.f), 0, ImVec2(1, 1));
        uiBeginTooltip(uiTooltipWidth);

        if (hoveredShrine != nullptr)
        {
            std::sprintf(uiTooltipBuffer, "%s", shrineTooltipsByType[static_cast<SizeT>(hoveredShrine->type)] + 1);
        }
        else
        {
            const char* catNormalTooltip = R"(
~~ Cat ~~

Pops bubbles or bombs, whatever comes first. Not the brightest, despite not being orange.

Prestige points can be spent for their college tuition, making them more cleverer.)";

            if (pt.perm.geniusCatsPurchased)
            {
                catNormalTooltip =
                    R"(
~~ Genius Cat ~~

A truly intelligent being: prioritizes popping bombs first, then star bubbles, then normal bubbles. Can be instructed to ignore specific bubble types.

Through the sheer power of their intellect, they also get a x2 multiplier on all bubble values.

We do not speak of the origin of the large brain attached to their body.)";
            }
            else if (pt.perm.smartCatsPurchased)
            {
                catNormalTooltip =
                    R"(
~~ Smart Cat ~~

Pops bubbles or bombs. Smart enough to prioritizes bombs and star bubbles over normal bubbles, but can't really tell those two apart.

We do not speak of the tuition fees.)";
            }

            const char* catUniTooltip = R"(
~~ Unicat ~~

Imbued with the power of stars and rainbows, transforms bubbles into star bubbles, worth x15 more.

Must have eaten something they weren't supposed to, because they keep changing color.
)";

            if (isUnicatTranscendenceActive())
                catUniTooltip = R"(
~~ Transcended Unicat ~~

Imbued with the power of the void, transforms bubbles into nova bubbles, worth x50 more.

Radiates a somewhat creepy energy.
)";

            const char* catDevilTooltip = R"(
~~ Devilcat ~~

Hired diplomat of the NB (NOBUBBLES) political party. Convinces bubbles to turn into bombs and explode for the rightful cause.

Bubbles caught in explosions are worth x10 more.)";

            if (isDevilcatHellsingedActive())
                catDevilTooltip = R"(
~~ Hellsinged Devilcat ~~

Sold their soul to the devil. Opens up portals to hells that absorb bubbles with a x50 multiplier.

From politician to demon... the NB (NOBUBBLES) party is truly a mystery.)";

            const char* catAstroTooltip = R"(
~~ Astrocat ~~

Pride of the NCSA, a highly trained feline astronaut that continuously flies across the map, popping bubbles with a x20 multiplier.

Desperately trying to get funding from the government for a mission on the cheese moon. Perhaps some prestige points could help?)";

            if (pt.perm.astroCatInspirePurchased)
            {
                catAstroTooltip =
                    R"(
~~ Propagandist Astrocat ~~

Pride of the NCSA, a highly trained feline astronaut that continuously flies across the map, popping bubbles with a x20 multiplier.

Finally financed by the NB (NOBUBBLES) political party to inspire other cats to work faster when flying by.)";
            }

            const char* catTooltipsByType[]{
                catNormalTooltip,
                catUniTooltip,
                catDevilTooltip,
                catAstroTooltip,
                R"(
~~ Witchcat ~~
(unique cat)

Loves to perform rituals on other cats, hexing one of them at random and capturing their soul in voodoo dolls that appear around the map.

Collecting all the dolls will release the hex and trigger a powerful timed effect depending on the type of the cursed cat.

Their dark magic is puzzling... but not as puzzling as the sheer number of dolls they carry around.)",
                R"(
~~ Wizardcat ~~
(unique cat)

Ancient arcane feline capable of unleashing powerful spells, if only they could remember them.
Can absorb the magic of star bubbles to recall their past lives and remember spells.

The scriptures say that they "unlock a Magic menu", but nobody knows what that means.

Witchcat interaction: after being hexed, will grant a x3.5 faster mana regen buff.)",
                R"(
~~ Mousecat ~~
(unique cat)

They stole a Logicat gaming mouse and they're now on the run. Surprisingly, the mouse still works even though it's not plugged in to anything.

Able to keep up a combo like for manual popping, and empowers nearby cats to pop bubbles with Mousecat's current combo multiplier.

Is affected by both cat reward value multipliers and click reward value multipliers, including their own buff.

Provides a global click reward value multiplier (upgradable via PPs) by merely existing... Logicat does know how to make a good mouse.

Witchcat interaction: after being hexed, will grant a x10 click reward buff.)",
                R"(
~~ Engicat ~~
(unique cat)

Periodically performs maintenance on all nearby cats, temporarily increasing their engine efficiency and making them faster. (Note: this buff stacks with inspirational NB propaganda.)

Provides a global cat reward value multiplier (upgradable via PPs) by merely existing... guess they're a "10x engineer"?

Witchcat interaction: after being hexed, will grant a x2 global faster cat cooldown buff.)",
                R"(
~~ Repulsocat ~~
(unique cat)

Continuously pushes bubbles away with their powerful USB fan, powered by only Dog knows what kind of batteries. (Note: this effect is applied even while Repulsocat is being dragged.)

Bubbles being pushed away by Repulsocat are worth x2 more.

Using prestige points, the fan can be upgraded to filter specific bubble types and/or convert a percentage of bubbles to star bubbles.

Witchcat interaction: after being hexed, will grant a x2 bubble count buff and increase wind speed.)",
                R"(
~~ Attractocat ~~
(unique cat)

Continuously attracts bubbles with their huge magnet, because soap is definitely magnetic. (Note: this effect is applied even while Attractocat is being dragged.)

Bubbles being attracted by Attractocat are worth x2 more.

Using prestige points, the magnet can be upgraded to filter specific bubble types.

Witchcat interaction: after being hexed, all bombs or hell portals will attract bubbles.)",
                R"(
~~ Copycat ~~
(unique cat)

Mimics an existing unique cat, gaining their abilities and effects. (Note: the mimicked cat can be changed via the toolbar near the bottom of the screen.)

Mimicking Witchcat:
- Two separate rituals will be performed.

Mimicking Wizardcat:
- Spells will also be casted by the Copycat.
- Mewliplier Aura's bonus does not stack twice.

Mimicking Mousecat:
- The combo multiplier is shared.
- The global clicking buff multiplier is doubled.

Mimicking Engicat:
- The global clicking buff multiplier is doubled.

Witchcat interaction: after being hexed, will grant the same buff as the mimicked cat.)",
                R"(
~~ Duck ~~

It's a duck.)",
            };

            static_assert(sf::base::getArraySize(catTooltipsByType) == nCatTypes);

            SFML_BASE_ASSERT(hoveredCat != nullptr);
            std::sprintf(uiTooltipBuffer, "%s", catTooltipsByType[static_cast<SizeT>(hoveredCat->type)] + 1);
        }

        ImGui::TextWrapped("%s", uiTooltipBuffer);
        uiTooltipBuffer[0] = '\0';

        uiEndTooltip();
    }

    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] AnimatedButtonOutcome : sf::base::U8
    {
        None,
        Clicked,
        ClickedWhileDisabled,
    };

    ////////////////////////////////////////////////////////////
    [[nodiscard]] AnimatedButtonOutcome uiAnimatedButton(const char* label, const ImVec2& btnSize)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();

        if (window->SkipItems)
            return AnimatedButtonOutcome::None;

        const char* labelEnd = ImGui::FindRenderedTextEnd(label);

        const auto id = static_cast<ImGuiID>(std::strtoul(labelEnd + 2, nullptr, 10));

        const ImVec2 labelSize = ImGui::CalcTextSize(label, labelEnd, true);

        const ImVec2 size = ImGui::CalcItemSize(btnSize,
                                                labelSize.x + ImGui::GetStyle().FramePadding.x * 2.f,
                                                labelSize.y + ImGui::GetStyle().FramePadding.y * 2.f);

        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(bb);

        if (!ImGui::ItemAdd(bb, id))
            return AnimatedButtonOutcome::None;

        // Store animation state in window data
        struct AnimState
        {
            float hoverAnim;     // 0.f to 1.f for tilt
            float clickAnim;     // 0.f to 1.f for scale
            float lastClickTime; // in seconds
        };

        // Get or create animation state
        ImGuiStorage* storage     = window->DC.StateStorage;
        const ImGuiID animStateId = id + 1; // Use a different ID for the state

        auto* animState = static_cast<AnimState*>(storage->GetVoidPtr(animStateId));
        if (animState == nullptr)
        {
            // TODO P2: rewrite to use static unordered_map
            animState = new AnimState{0.f, 0.f, -1.f}; // intentionally leaked, shouldn't be an issue
            storage->SetVoidPtr(animStateId, animState);
        }

        const bool isCurrentlyDisabled  = (ImGui::GetItemFlags() & ImGuiItemFlags_Disabled) != 0;
        const bool hovered              = ImGui::ItemHoverable(bb, id, ImGuiItemFlags_None);
        const bool pressed              = !isCurrentlyDisabled && hovered && ImGui::IsMouseDown(0);
        const bool clicked              = !isCurrentlyDisabled && hovered && ImGui::IsMouseReleased(0);
        const bool clickedWhileDisabled = isCurrentlyDisabled && hovered && ImGui::IsMouseReleased(0);

        // Update animations
        const float deltaTime = ImGui::GetIO().DeltaTime;

        // Hover animation (tilt)
        animState->hoverAnim = hovered ? ImMin(animState->hoverAnim + deltaTime * 5.f, 1.f) : 0.f;

        // Click animation (scale)
        if (pressed)
            animState->lastClickTime = static_cast<float>(ImGui::GetTime());

        const float timeSinceClick = static_cast<float>(ImGui::GetTime()) - animState->lastClickTime;

        const float clickAnimDir = timeSinceClick > 0.f && timeSinceClick < 0.2f ? 1.f : -1.f;
        animState->clickAnim     = sf::base::clamp(animState->clickAnim + deltaTime * 4.5f * clickAnimDir, 0.f, 1.f);

        // Save current cursor pos
        const ImVec2 originalPos = window->DC.CursorPos;
        ImDrawList*  drawList    = window->DrawList;

        // Calculate center point for transformations
        const ImVec2 center = bb.Min + size * 0.5f;

        // Apply transformations (with a small extra clip so nothing gets cut off)
        const ImVec2 clipMin{bb.Min.x - 20.f, bb.Min.y - 20.f};
        const ImVec2 clipMax{bb.Max.x + 20.f, bb.Max.y + 20.f};
        drawList->PushClipRect(clipMin, clipMax, true);

        // Scale transform: shrink by up to 10% when clicked.
        const float scale = 1.f - easeInOutElastic(animState->clickAnim) * 0.75f;

        // Tilt transform: rotate by up to 0.05 radians (≈2.9°). (Use ~0.0873f for 5°.)
        const float tiltAngle = sf::base::sin(easeInOutSine(animState->hoverAnim) * sf::base::tau) * 0.1f;
        const float tiltCos   = sf::base::cos(tiltAngle);
        const float tiltSin   = sf::base::sin(tiltAngle);

        // Helper lambda: apply scale & rotation about the button center.
        const auto transformPoint = [&](const ImVec2& p) -> ImVec2
        {
            // Translate so that the center is at (0,0)
            const ImVec2 centered = p - center;

            // Apply scale
            const ImVec2 scaled = centered * scale;

            // Apply rotation
            const ImVec2 rotated{scaled.x * tiltCos - scaled.y * tiltSin, scaled.x * tiltSin + scaled.y * tiltCos};

            // Translate back
            return center + rotated;
        };

        // ── Draw Button Background as a Rounded Rectangle ──
        const ImU32 btnBgColor = ImGui::GetColorU32(
            pressed   ? ImGuiCol_ButtonActive
            : hovered ? ImGuiCol_ButtonHovered
                      : ImGuiCol_Button);

        const float rounding = ImGui::GetStyle().FrameRounding;

        drawList->PathClear();
        drawList->PathRect(bb.Min, bb.Max, rounding);

        for (int i = 0; i < drawList->_Path.Size; i++)
            drawList->_Path[i] = transformPoint(drawList->_Path[i]);

        drawList->PathFillConvex(btnBgColor);

        if (ImGui::GetStyle().FrameBorderSize > 0.f)
        {
            // Recreate the path for the border.
            drawList->PathClear();
            drawList->PathRect(bb.Min, bb.Max, rounding);

            for (int i = 0; i < drawList->_Path.Size; i++)
                drawList->_Path[i] = transformPoint(drawList->_Path[i]);

            drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Border), ImDrawFlags_Closed, ImGui::GetStyle().FrameBorderSize);
        }

        // ── Draw Text with the Same Transformation ──

        // First, compute the untransformed text position.
        const ImVec2 textPos = bb.Min + (size - labelSize) * 0.5f;

        // Record the current vertex buffer size...
        const int vtxBufferSizeBeforeTransformation = drawList->VtxBuffer.Size;

        // ...and add the text at its normal (unrotated/unscaled) position.
        drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label, labelEnd);

        // Then, apply our transform to only the new text vertices.
        for (int i = vtxBufferSizeBeforeTransformation; i < drawList->VtxBuffer.Size; i++)
            drawList->VtxBuffer[i].pos = transformPoint(drawList->VtxBuffer[i].pos);

        // Restore the previous clip rect and cursor position.
        drawList->PopClipRect();
        window->DC.CursorPos = originalPos;

        return clicked                ? AnimatedButtonOutcome::Clicked
               : clickedWhileDisabled ? AnimatedButtonOutcome::ClickedWhileDisabled
                                      : AnimatedButtonOutcome::None;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getAspectRatioScalingFactor(const sf::Vector2f& originalSize, const sf::Vector2f& windowSize) const
    {
        // Calculate the scale factors for both dimensions
        const float scaleX = windowSize.x / originalSize.x;
        const float scaleY = windowSize.y / originalSize.y;

        // Use the smaller scale factor to maintain aspect ratio
        return std::min(scaleX, scaleY);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledGameView(const sf::Vector2f& originalSize, const sf::Vector2f& windowSize) const
    {
        const float        scale      = getAspectRatioScalingFactor(originalSize, windowSize);
        const sf::Vector2f scaledSize = originalSize * scale;

        return {.center   = originalSize / 2.f,
                .size     = originalSize,
                .viewport = {(windowSize - scaledSize).componentWiseDiv(windowSize * 2.f),
                             scaledSize.componentWiseDiv(windowSize)}};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View createScaledTopGameView(const sf::Vector2f& originalSize, const sf::Vector2f& windowSize) const
    {
        const float        scale      = getAspectRatioScalingFactor(originalSize, windowSize);
        const sf::Vector2f scaledSize = originalSize * scale;

        // Compute the full window width in world coordinates.
        float newWidth = windowSize.x / scale;

        sf::View view;
        // Use the new width while keeping the original height.
        view.size = {newWidth, originalSize.y};

        // Align the left edge with that of the normal game view.
        // The left edge is given by (baseCenter.x - originalSize.x / 2).
        sf::Vector2f baseCenter = getViewCenter();
        float        left       = baseCenter.x - originalSize.x / 2.f;
        view.center             = {left + newWidth / 2.f, baseCenter.y};

        // Use the same vertical letterboxing as the regular game view.
        view.viewport = {{0.f, (windowSize.y - scaledSize.y) / (windowSize.y * 2.f)}, {1.f, scaledSize.y / windowSize.y}};

        return view;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::View makeScaledHUDView(const sf::Vector2f& resolution, float scale) const
    {
        return {.center = {resolution.x / (2.f * scale), resolution.y / (2.f * scale)}, .size = resolution / scale};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vector2f getHUDMousePos() const
    {
        return getWindow().mapPixelToCoords(sf::Mouse::getPosition(getWindow()), nonScaledHUDView);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool uiMakeButtonImpl(const char* label, const char* xBuffer)
    {
        const float scaledButtonWidth = uiButtonWidth * profile.uiScale;

        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - scaledButtonWidth - 2.5f, 0.f)); // Push to right
        ImGui::SameLine();

        uiPushButtonColors();

        bool clicked = false;
        if (const auto outcome = uiAnimatedButton(xBuffer, ImVec2(scaledButtonWidth, 0.f));
            outcome == AnimatedButtonOutcome::Clicked)
        {
            playSound(sounds.buy);
            clicked = true;

            for (SizeT i = 0u; i < 24u; ++i)
                spawnHUDTopParticle({.position      = getHUDMousePos(),
                                     .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}),
                                     .scale         = rngFast.getF(0.08f, 0.27f) * 0.7f,
                                     .scaleDecay    = 0.f,
                                     .accelerationY = 0.002f,
                                     .opacity       = 1.f,
                                     .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                                     .rotation      = rngFast.getF(0.f, sf::base::tau),
                                     .torque        = rngFast.getF(-0.002f, 0.002f)},
                                    /* hue */ wrapHue(165.f + uiButtonHueMod + currentBackgroundHue.asDegrees()),
                                    ParticleType::Star);
        }
        else if (outcome == AnimatedButtonOutcome::ClickedWhileDisabled)
        {
            playSound(sounds.failpopui);

            for (SizeT i = 0u; i < 6u; ++i)
                spawnHUDTopParticle({.position      = getHUDMousePos(),
                                     .velocity      = rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}) * 0.5f,
                                     .scale         = rngFast.getF(0.08f, 0.27f),
                                     .scaleDecay    = 0.f,
                                     .accelerationY = 0.002f * 0.75f,
                                     .opacity       = 1.f,
                                     .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                                     .rotation      = rngFast.getF(0.f, sf::base::tau),
                                     .torque        = rngFast.getF(-0.002f, 0.002f)},
                                    /* hue */ 0.f,
                                    ParticleType::Bubble);
        }

        uiPopButtonColors();

        uiMakeTooltip();

        if (label[0] == ' ')
            uiSetFontScale(uiNormalFontScale);

        ImGui::NextColumn();
        return clicked;
    }

    ////////////////////////////////////////////////////////////
    template <typename TCost>
    [[nodiscard]] bool makePSVButtonExByCurrency(
        const char*              label,
        PurchasableScalingValue& psv,
        const SizeT              times,
        const TCost              cost,
        TCost&                   availability,
        const char*              currencyFmt)
    {
        const bool maxedOut = psv.nPurchases == psv.data->nMaxPurchases;
        bool       result   = false;

        if (maxedOut)
            std::sprintf(uiBuffer, "MAX##%u", uiWidgetId++);
        else if (cost == 0u || times == 0u)
            std::sprintf(uiBuffer, "N/A##%u", uiWidgetId++);
        else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
            std::sprintf(uiBuffer, currencyFmt, toStringWithSeparators(cost), uiWidgetId++);
#pragma GCC diagnostic pop

        ImGui::BeginDisabled(checkPurchasability(label, maxedOut || availability < cost || cost == 0u));

        uiMakeButtonLabels(label, uiLabelBuffer);
        if (uiMakeButtonImpl(label, uiBuffer))
        {
            result = true;
            availability -= cost;

            if (&availability == &pt.money)
                spentMoney += cost;

            psv.nPurchases += times;
        }

        ImGui::EndDisabled();
        return result;
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
    [[nodiscard]] bool makePSVButtonEx(const char* label, PurchasableScalingValue& psv, const SizeT times, const MoneyType cost)
    {
        return makePSVButtonExByCurrency(label, psv, times, cost, pt.money, "$%s##%u");
    }

    ////////////////////////////////////////////////////////////
    bool makePSVButton(const char* label, PurchasableScalingValue& psv)
    {
        return makePSVButtonEx(label, psv, 1u, static_cast<MoneyType>(psv.nextCost()));
    }

    ////////////////////////////////////////////////////////////
    std::unordered_map<std::string, float> uiLabelToY;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool checkPurchasability(const char* label, const bool disabled)
    {
        uiLabelToY[label] = ImGui::GetCursorScreenPos().y;

        if (disabled)
        {
            btnWasDisabled[label] = true;
        }
        else if (btnWasDisabled[label] && !disabled)
        {
            btnWasDisabled[label] = false;

            const bool anyPurchaseUnlockedEffectWithSameLabel = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                                purchaseUnlockedEffects.end(),
                                                                                [&](const PurchaseUnlockedEffect& effect)
            { return effect.widgetLabel == label; });

            const bool anyPurchaseUnlockedEffectWithSameY = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                            purchaseUnlockedEffects.end(),
                                                                            [&](const PurchaseUnlockedEffect& effect)
            {
                const auto it = uiLabelToY.find(effect.widgetLabel);
                return it != uiLabelToY.end() && it->second == uiLabelToY[label];
            });

            if (!anyPurchaseUnlockedEffectWithSameLabel && !anyPurchaseUnlockedEffectWithSameY)
            {
                purchaseUnlockedEffects.push_back({
                    .widgetLabel    = label,
                    .countdown      = Countdown{.value = 1000.f},
                    .arrowCountdown = Countdown{.value = 2000.f},
                    .hue            = uiButtonHueMod,
                    .type           = 1, // now purchasable
                });

                playSound(sounds.purchasable, /* maxOverlap */ 1u);
                playSound(sounds.shimmer, /* maxOverlap */ 1u);
            }
        }

        return disabled;
    }

    ////////////////////////////////////////////////////////////
    template <typename TCost>
    [[nodiscard]] bool makePurchasableButtonOneTimeByCurrency(
        const char* label,
        bool&       done,
        const TCost cost,
        TCost&      availability,
        const char* currencyFmt)
    {
        bool result = false;

        if (done)
            std::sprintf(uiBuffer, "DONE##%u", uiWidgetId++);
        else if (cost == 0u)
            std::sprintf(uiBuffer, "FREE##%u", uiWidgetId++);
        else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
            std::sprintf(uiBuffer, currencyFmt, toStringWithSeparators(cost), uiWidgetId++);
#pragma GCC diagnostic pop

        ImGui::BeginDisabled(checkPurchasability(label, done || availability < cost));

        uiMakeButtonLabels(label, uiLabelBuffer);
        if (uiMakeButtonImpl(label, uiBuffer))
        {
            result = true;
            availability -= cost;

            if (&availability == &pt.money)
                spentMoney += cost;

            done = true;
        }

        ImGui::EndDisabled();
        return result;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool makePurchasableButtonOneTime(const char* label, const MoneyType cost, bool& done)
    {
        return makePurchasableButtonOneTimeByCurrency(label, done, cost, pt.money, "$%s##%u");
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool makePurchasablePPButtonOneTime(const char* label, const PrestigePointsType prestigePointsCost, bool& done)
    {
        return makePurchasableButtonOneTimeByCurrency(label, done, prestigePointsCost, pt.prestigePoints, "%s PPs##%u");
    }

    ////////////////////////////////////////////////////////////
    bool makePrestigePurchasablePPButtonPSV(const char* label, PurchasableScalingValue& psv)
    {
        return makePSVButtonExByCurrency(label,
                                         psv,
                                         /* times */ 1u,
                                         /* cost */ static_cast<PrestigePointsType>(psv.nextCost()),
                                         /* availability */ pt.prestigePoints,
                                         "%s PPs##%u");
    }

    ////////////////////////////////////////////////////////////
    void uiBeginColumns() const
    {
        ImGui::Columns(2, "twoColumns", false);
        ImGui::SetColumnWidth(0, (uiWindowWidth - uiButtonWidth - 40.f) * profile.uiScale);
        ImGui::SetColumnWidth(1, (uiButtonWidth + 10.f) * profile.uiScale);
    }

    ////////////////////////////////////////////////////////////
    void uiCenteredText(const char* str, const float offsetX = 0.f, const float offsetY = 0.f)
    {
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(str).x) * 0.5f + offsetX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

        ImGui::Text("%s", str);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vector2f uiGetWindowPos() const
    {
        const float ratio = getAspectRatioScalingFactor(gameScreenSize, getResolution());

        const float scaledWindowWidth = uiWindowWidth * profile.uiScale;

        const float rightAnchorX = sf::base::min(getResolution().x - scaledWindowWidth - 15.f * profile.uiScale,
                                                 gameScreenSize.x * ratio + 30.f * profile.uiScale);

        return {rightAnchorX, 15.f};
    }

    ////////////////////////////////////////////////////////////
    void uiDrawExitPopup(const float newScalingFactor)
    {
        if (!escWasPressed)
            return;

        constexpr float scaleMult = 1.25f;

        ImGui::SetNextWindowPos({getResolution().x / 2.f, getResolution().y / 2.f}, 0, {0.5f, 0.5f});
        ImGui::SetNextWindowSizeConstraints(ImVec2(400.f * scaleMult * newScalingFactor, 0.f),
                                            ImVec2(400.f * scaleMult * newScalingFactor,
                                                   300.f * scaleMult * newScalingFactor));

        ImGui::Begin("##exit",
                     nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoTitleBar);

        uiSetFontScale(scaleMult);

        uiCenteredText("Are you sure you want to exit?");

        ImGui::Dummy(ImVec2(0.f, 10.f * scaleMult));
        const float exitButtonWidth = 60.f * scaleMult * profile.uiScale;

        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - exitButtonWidth * 2.f - 5.f) * 0.5f);

        ImGui::BeginGroup();

        if (ImGui::Button("Yes", ImVec2(exitButtonWidth, 0.f)))
        {
            playSound(sounds.uitab);
            mustExit = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("No", ImVec2(exitButtonWidth, 0.f)))
        {
            playSound(sounds.uitab);
            escWasPressed = false;
        }

        ImGui::EndGroup();

        uiSetFontScale(uiNormalFontScale);

        ImGui::End();
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
    sf::base::Optional<sf::Music>& getCurrentBGMBuffer()
    {
        return bgmBuffers[currentBGMBufferIdx % 2u];
    }

    ////////////////////////////////////////////////////////////
    sf::base::Optional<sf::Music>& getNextBGMBuffer()
    {
        return bgmBuffers[(currentBGMBufferIdx + 1u) % 2u];
    }

    ////////////////////////////////////////////////////////////
    void switchToBGM(const sf::base::SizeT index, const bool force)
    {
#ifndef BUBBLEBYTE_NO_AUDIO
        if (!force && lastPlayedMusic == bgmPaths[index])
            return;

        lastPlayedMusic     = bgmPaths[index];
        bgmTransition.value = 1000.f;

        auto& optNextMusic = getNextBGMBuffer();

        optNextMusic = sf::Music::openFromFile(bgmPaths[index]);
        SFML_BASE_ASSERT(optNextMusic.hasValue());

        optNextMusic->setVolume(0.f);
        optNextMusic->setLooping(true);
        optNextMusic->setAttenuation(0.f);
        optNextMusic->setSpatializationEnabled(false);
        optNextMusic->play(playbackDevice);
#else
        (void)index;
        (void)force;
#endif
    }

    ////////////////////////////////////////////////////////////
    void uiDrawQuickbarCopyCat(const sf::Vector2f quickBarPos, Cat& copyCat)
    {
        const bool asWitchAndBusy = pt.copycatCopiedCatType == CatType::Witch &&
                                    (anyCatCopyHexed() || !pt.copyDolls.empty());

        const bool asWizardAndBusy = pt.copycatCopiedCatType == CatType::Wizard && isWizardBusy();

        const bool mustDisable = asWitchAndBusy || asWizardAndBusy;

        ImGui::BeginDisabled(mustDisable);

        constexpr const char* popupLabel = "CopyCatSelectorPopup";
        static sf::base::U8   opacity    = 168u;

        uiImageFromAtlas(txrIconCopyCat,
                         {.scale = {0.65f, 0.65f},
                          .color = (mustDisable ? sf::Color::Gray : sf::Color::White).withAlpha(opacity)});

        opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;


        std::sprintf(uiTooltipBuffer, "Select Copycat mask");
        uiMakeTooltip(/* small */ true);

        ImGui::SameLine();

        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup(popupLabel);
            playSound(sounds.uitab);
        }

        ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
        if (ImGui::BeginPopup(popupLabel))
        {
            ImGui::SetNextItemWidth(210.f * profile.uiScale);

            if (ImGui::BeginCombo("##copycatsel", CatConstants::typeNamesLong[asIdx(pt.copycatCopiedCatType)]))
            {
                for (SizeT i = asIdx(CatType::Normal); i < nCatTypes; ++i)
                {
                    if (static_cast<CatType>(i) == CatType::Duck)
                        continue;

                    if (!isUniqueCatType(static_cast<CatType>(i)))
                        continue;

                    if (i == asIdx(CatType::Copy))
                        continue;

                    if (findFirstCatByType(static_cast<CatType>(i)) == nullptr)
                        continue;

                    const bool isSelected = pt.copycatCopiedCatType == static_cast<CatType>(i);
                    if (ImGui::Selectable(CatConstants::typeNamesLong[i], isSelected))
                    {
                        statDisguise();
                        pt.copycatCopiedCatType = static_cast<CatType>(i);

                        copyCat.cooldown.value = pt.getComputedCooldownByCatType(pt.copycatCopiedCatType);
                        copyCat.hits           = 0u;

                        sounds.smokebomb.setPosition({copyCat.position.x, copyCat.position.y});
                        playSound(sounds.smokebomb);

                        for (sf::base::SizeT iP = 0u; iP < 8u; ++iP)
                            spawnParticle(ParticleData{.position = copyCat.position,
                                                       .velocity = {rngFast.getF(-0.15f, 0.15f), rngFast.getF(0.f, 0.1f)},
                                                       .scale         = rngFast.getF(0.75f, 1.f),
                                                       .scaleDecay    = -0.0005f,
                                                       .accelerationY = -0.00017f,
                                                       .opacity       = 1.f,
                                                       .opacityDecay  = rngFast.getF(0.00065f, 0.00075f),
                                                       .rotation      = rngFast.getF(0.f, sf::base::tau),
                                                       .torque        = rngFast.getF(-0.002f, 0.002f)},
                                          0.f,
                                          ParticleType::Smoke);

                        playSound(sounds.uitab);
                        ImGui::CloseCurrentPopup();
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::EndPopup();
        }

        ImGui::EndDisabled();
    }

    ////////////////////////////////////////////////////////////
    void uiDrawQuickbarBackgroundSelector(const sf::Vector2f quickBarPos)
    {
        constexpr const char* popupLabel = "BackgroundSelectorPopup";
        static sf::base::U8   opacity    = 168u;

        uiImageFromAtlas(txrIconBg, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

        opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

        std::sprintf(uiTooltipBuffer, "Select background");
        uiMakeTooltip(/* small */ true);

        ImGui::SameLine();

        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup(popupLabel);
            playSound(sounds.uitab);
        }

        ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
        if (ImGui::BeginPopup(popupLabel))
        {
            auto& [entries, selectedIndex] = getBackgroundSelectorData();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);

            if (ImGui::BeginCombo("##backgroundsel", entries[static_cast<sf::base::SizeT>(selectedIndex)].name))
            {
                for (SizeT i = 0u; i < entries.size(); ++i)
                {
                    const bool isSelected = selectedIndex == static_cast<int>(i);
                    if (ImGui::Selectable(entries[i].name, isSelected))
                    {
                        selectedIndex = static_cast<int>(i);

                        selectBackground(entries, static_cast<int>(i));

                        playSound(sounds.uitab);
                        ImGui::CloseCurrentPopup();
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::EndPopup();
        }
    }

    ////////////////////////////////////////////////////////////
    void uiDrawQuickbarBGMSelector(const sf::Vector2f quickBarPos)
    {
        constexpr const char* popupLabel = "MusicSelectorPopup";
        static sf::base::U8   opacity    = 168u;

        uiImageFromAtlas(txrIconBGM, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

        opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

        std::sprintf(uiTooltipBuffer, "Select music");
        uiMakeTooltip(/* small */ true);

        ImGui::SameLine();

        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup(popupLabel);
            playSound(sounds.uitab);
        }

        ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
        if (ImGui::BeginPopup(popupLabel))
        {
            auto& [entries, selectedIndex] = getBGMSelectorData();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);

            if (ImGui::BeginCombo("##musicsel", entries[static_cast<sf::base::SizeT>(selectedIndex)].name))
            {
                for (SizeT i = 0u; i < entries.size(); ++i)
                {
                    const bool isSelected = selectedIndex == static_cast<int>(i);
                    if (ImGui::Selectable(entries[i].name, isSelected))
                    {
                        selectedIndex = static_cast<int>(i);

                        selectBGM(entries, static_cast<int>(i));
                        switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ false);

                        playSound(sounds.uitab);
                        ImGui::CloseCurrentPopup();
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::EndPopup();
        }
    }

    ////////////////////////////////////////////////////////////
    void uiDrawQuickbarQuickSettings(const sf::Vector2f quickBarPos)
    {
        constexpr const char* popupLabel = "QuickSettingsPopup";
        static sf::base::U8   opacity    = 168u;

        uiImageFromAtlas(txrIconCfg, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

        opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

        std::sprintf(uiTooltipBuffer, "Quick settings");
        uiMakeTooltip(/* small */ true);

        ImGui::SameLine();

        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup(popupLabel);
            playSound(sounds.uitab);
        }

        ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
        if (ImGui::BeginPopup(popupLabel))
        {
            uiCheckbox("Enable tips", &profile.tipsEnabled);
            uiCheckbox("Enable notifications", &profile.enableNotifications);

            ImGui::Separator();

            uiCheckbox("Enable $/s meter", &profile.showDpsMeter);

            ImGui::Separator();

            uiCheckbox("Show cat range", &profile.showCatRange);
            uiCheckbox("Show cat text", &profile.showCatText);

            ImGui::Separator();

            uiCheckbox("Show particles", &profile.showParticles);

            ImGui::BeginDisabled(!profile.showParticles);
            uiCheckbox("Show coin particles", &profile.showCoinParticles);
            ImGui::EndDisabled();

            uiCheckbox("Show text particles", &profile.showTextParticles);

            ImGui::Separator();

            uiCheckbox("Enable screen shake", &profile.enableScreenShake);

            ImGui::Separator();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

            uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

            ImGui::Separator();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Minimap Scale", &profile.minimapScale, 5.f, 40.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("HUD Scale", &profile.hudScale, 0.5f, 2.f, "%.2f");

            ImGui::AlignTextToFramePadding();
            ImGui::Text("UI Scale");

            const auto makeUIScaleButton = [&](const char* label, const float scaleFactor)
            {
                ImGui::SameLine();
                if (ImGui::Button(label, ImVec2{46.f * profile.uiScale, 0.f}))
                {
                    playSound(sounds.buy);
                    profile.uiScale = scaleFactor;
                }
            };

            makeUIScaleButton("XXL", 1.75f);
            makeUIScaleButton("XL", 1.5f);
            makeUIScaleButton("L", 1.25f);
            makeUIScaleButton("M", 1.f);
            makeUIScaleButton("S", 0.75f);
            makeUIScaleButton("XS", 0.5f);

            ImGui::Separator();

            uiCheckbox("Bubble shader", &profile.useBubbleShader);

            ImGui::BeginDisabled(!profile.useBubbleShader);
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Bubble Lightness", &profile.bsBubbleLightness, -1.f, 1.f, "%.2f");
            ImGui::EndDisabled();

            ImGui::Separator();

            if (uiCheckbox("VSync", &profile.vsync))
                optWindow->setVerticalSyncEnabled(profile.vsync);

            ImGui::EndPopup();
        }
    }

    ////////////////////////////////////////////////////////////
    void uiDrawQuickbarVolumeControls(const sf::Vector2f quickBarPos)
    {
        constexpr const char* popupLabel = "VolumeSelectorPopup";
        static sf::base::U8   opacity    = 168u;

        uiImageFromAtlas(txrIconVolume, {.scale = {0.65f, 0.65f}, .color = sf::Color::whiteMask(opacity)});

        opacity = ImGui::IsItemHovered() || ImGui::IsPopupOpen(popupLabel) ? 255u : 168u;

        std::sprintf(uiTooltipBuffer, "Volume settings");
        uiMakeTooltip(/* small */ true);

        ImGui::SameLine();

        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup(popupLabel);
            playSound(sounds.uitab);
        }

        ImGui::SetNextWindowPos(ImVec2(quickBarPos) - ImVec2{0.f, 64.f}, 0, {1.f, 1.f});
        if (ImGui::BeginPopup(popupLabel))
        {
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Master##popupmastervolume", &profile.masterVolume, 0.f, 100.f, "%.f%%");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            if (ImGui::SliderFloat("SFX##popupsfxvolume", &profile.sfxVolume, 0.f, 100.f, "%.f%%"))
                sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Music##popupmusicvolume", &profile.musicVolume, 0.f, 100.f, "%.f%%");

            ImGui::EndPopup();
        }
    }

    ////////////////////////////////////////////////////////////
    void uiDrawQuickbar()
    {
        const float xStartOverlay = getAspectRatioScalingFactor(gameScreenSize, getResolution()) * gameScreenSize.x;

        const float xStart = lastUiSelectedTabIdx == 0 ? xStartOverlay : sf::base::min(xStartOverlay, uiGetWindowPos().x);

        const sf::Vector2f quickBarPos{xStart - 15.f, getResolution().y - 15.f};

        ImGui::SetNextWindowPos({quickBarPos.x, quickBarPos.y}, 0, {1.f, 1.f});
        ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, FLT_MAX));

        ImGui::Begin("##quickmenu",
                     nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

        if (cachedCopyCat != nullptr)
            uiDrawQuickbarCopyCat(quickBarPos, *cachedCopyCat);

        if (getBackgroundSelectorData().entries.size() > 1u)
            uiDrawQuickbarBackgroundSelector(quickBarPos);

        if (getBGMSelectorData().entries.size() > 1u)
            uiDrawQuickbarBGMSelector(quickBarPos);

        uiDrawQuickbarVolumeControls(quickBarPos);

        uiDrawQuickbarQuickSettings(quickBarPos);

        ImGui::End();
    }

    ////////////////////////////////////////////////////////////
    struct ImGuiStyleScales
    {
        const ImVec2 windowPadding;
        const float  windowRounding;
        const ImVec2 windowMinSize;
        const float  childRounding;
        const float  popupRounding;
        const ImVec2 framePadding;
        const float  frameRounding;
        const ImVec2 itemSpacing;
        const ImVec2 itemInnerSpacing;
        const ImVec2 cellPadding;
        const ImVec2 touchExtraPadding;
        const float  indentSpacing;
        const float  columnsMinSpacing;
        const float  scrollbarSize;
        const float  scrollbarRounding;
        const float  grabMinSize;
        const float  grabRounding;
        const float  logSliderDeadzone;
        const float  tabRounding;
        const float  tabCloseButtonMinWidthUnselected;
        const float  tabBarOverlineSize;
        const ImVec2 separatorTextPadding;
        const ImVec2 displayWindowPadding;
        const ImVec2 displaySafeAreaPadding;
        const float  mouseCursorScale;

        void applyWithScale(ImGuiStyle& style, const float scale, const bool steamDeck) const
        {
            style.WindowPadding                    = windowPadding * scale;
            style.WindowRounding                   = windowRounding * scale;
            style.WindowMinSize                    = windowMinSize * scale;
            style.ChildRounding                    = childRounding * scale;
            style.PopupRounding                    = popupRounding * scale;
            style.FramePadding                     = framePadding * scale;
            style.FrameRounding                    = frameRounding * scale;
            style.ItemSpacing                      = itemSpacing * scale;
            style.ItemInnerSpacing                 = itemInnerSpacing * scale;
            style.CellPadding                      = cellPadding * scale;
            style.TouchExtraPadding                = touchExtraPadding * scale;
            style.IndentSpacing                    = indentSpacing * scale;
            style.ColumnsMinSpacing                = columnsMinSpacing * scale;
            style.ScrollbarSize                    = scrollbarSize * scale * (steamDeck ? 2.f : 0.5f);
            style.ScrollbarRounding                = scrollbarRounding * scale;
            style.GrabMinSize                      = grabMinSize * scale;
            style.GrabRounding                     = grabRounding * scale;
            style.LogSliderDeadzone                = logSliderDeadzone * scale;
            style.TabRounding                      = tabRounding * scale;
            style.TabCloseButtonMinWidthUnselected = tabCloseButtonMinWidthUnselected * scale;
            style.TabBarOverlineSize               = tabBarOverlineSize * scale;
            style.SeparatorTextPadding             = separatorTextPadding * scale;
            style.DisplayWindowPadding             = displayWindowPadding * scale;
            style.DisplaySafeAreaPadding           = displaySafeAreaPadding * scale;
            style.MouseCursorScale                 = mouseCursorScale * scale;
        }
    };

    ////////////////////////////////////////////////////////////
    void uiDraw(const sf::Vector2f mousePos)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        style.FrameBorderSize = 2.f;
        style.FrameRounding   = 10.f;
        style.WindowRounding  = 5.f;

        static const auto initialStyleScales = [&]() -> ImGuiStyleScales
        {
            return {
                .windowPadding                    = style.WindowPadding,
                .windowRounding                   = style.WindowRounding,
                .windowMinSize                    = style.WindowMinSize,
                .childRounding                    = style.ChildRounding,
                .popupRounding                    = style.PopupRounding,
                .framePadding                     = style.FramePadding,
                .frameRounding                    = style.FrameRounding,
                .itemSpacing                      = style.ItemSpacing,
                .itemInnerSpacing                 = style.ItemInnerSpacing,
                .cellPadding                      = style.CellPadding,
                .touchExtraPadding                = style.TouchExtraPadding,
                .indentSpacing                    = style.IndentSpacing,
                .columnsMinSpacing                = style.ColumnsMinSpacing,
                .scrollbarSize                    = style.ScrollbarSize,
                .scrollbarRounding                = style.ScrollbarRounding,
                .grabMinSize                      = style.GrabMinSize,
                .grabRounding                     = style.GrabRounding,
                .logSliderDeadzone                = style.LogSliderDeadzone,
                .tabRounding                      = style.TabRounding,
                .tabCloseButtonMinWidthUnselected = style.TabCloseButtonMinWidthUnselected,
                .tabBarOverlineSize               = style.TabBarOverlineSize,
                .separatorTextPadding             = style.SeparatorTextPadding,
                .displayWindowPadding             = style.DisplayWindowPadding,
                .displaySafeAreaPadding           = style.DisplaySafeAreaPadding,
                .mouseCursorScale                 = style.MouseCursorScale,
            };
        }();

        uiWidgetId = 0u;

        ImGui::PushFont(fontImGuiSuperBakery);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f); // Set corner radius

        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 0.65f); // 65% transparent black
        style.Colors[ImGuiCol_Border]   = colorBlueOutline.toVec4<ImVec4>();

        const float newScalingFactor = profile.uiScale;
        initialStyleScales.applyWithScale(style, newScalingFactor, onSteamDeck);

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

        ImGui::GetIO().FontGlobalScale = newScalingFactor;

        if (profile.showDpsMeter && !debugHideUI)
            uiDpsMeter();

        if (!debugHideUI)
            uiDrawQuickbar();

        ImGui::SetNextWindowPos({uiGetWindowPos().x, uiGetWindowPos().y}, 0, {0.f, 0.f});
        ImGui::SetNextWindowSizeConstraints(ImVec2(uiWindowWidth * newScalingFactor, 0.f),
                                            ImVec2(uiWindowWidth * newScalingFactor,
                                                   uiGetMaxWindowHeight() * newScalingFactor));

        ImGui::Begin("##menu",
                     nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoTitleBar);


        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_DrawSelectedOverline))
        {
            uiTabBar();
            ImGui::EndTabBar();
        }

        if (!ImGui::GetIO().WantCaptureMouse && particleCullingBoundaries.isInside(mousePos))
            uiMakeShrineOrCatTooltip(mousePos);

        ImGui::End();

        uiDrawExitPopup(newScalingFactor);

        ImGui::PopStyleVar();
        ImGui::PopFont();
    }

    ////////////////////////////////////////////////////////////
    void uiDpsMeter()
    {
        const auto resolution = getResolution();

        const float  dpsMeterScale = profile.uiScale;
        const ImVec2 dpsMeterSize(240.f * dpsMeterScale, 65.f * dpsMeterScale);

        ImGui::SetNextWindowPos({15.f, resolution.y - 15.f}, 0, {0.f, 1.f});
        ImGui::SetNextWindowSizeConstraints(dpsMeterSize, dpsMeterSize);

        ImGui::Begin("##dpsmeter",
                     nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoTitleBar);

        uiSetFontScale(0.75f);

        static thread_local std::vector<float> sampleBuffer(60);
        samplerMoneyPerSecond.writeSamplesInOrder(sampleBuffer.data());

        const auto average = static_cast<MoneyType>(samplerMoneyPerSecond.getAverage());

        static thread_local char avgBuffer[64];
        std::sprintf(avgBuffer, "%s $/s", toStringWithSeparators(average));

        ImGui::PlotLines("##dpsmeter",
                         sampleBuffer.data(),
                         static_cast<int>(sampleBuffer.size()),
                         0,
                         avgBuffer,
                         0.f,
                         FLT_MAX,
                         ImVec2(dpsMeterSize.x - 15.f * dpsMeterScale, dpsMeterSize.y - 17.f * dpsMeterScale));

        ImGui::End();
    }

    ////////////////////////////////////////////////////////////
    ImGuiTabItemFlags_ shopSelectOnce       = ImGuiTabItemFlags_SetSelected;
    int                lastUiSelectedTabIdx = 1;

    ////////////////////////////////////////////////////////////
    void uiTabBar()
    {
        const float childHeight = uiGetMaxWindowHeight() - (60.f * profile.uiScale);

        const auto keyboardSelectedTab = [&](const sf::Keyboard::Key key)
        {
            return !ImGui::GetIO().WantCaptureKeyboard && inputHelper.wasKeyJustPressed(key)
                       ? ImGuiTabItemFlags_SetSelected
                       : ImGuiTabItemFlags_{};
        };

        const auto selectedTab = [&](int idx)
        {
            if (shopSelectOnce == ImGuiTabItemFlags_{} && lastUiSelectedTabIdx != idx)
                playSound(sounds.uitab);

            lastUiSelectedTabIdx = idx;
        };

        if (ImGui::BeginTabItem("X",
                                nullptr,
                                keyboardSelectedTab(sf::Keyboard::Key::Slash) | keyboardSelectedTab(sf::Keyboard::Key::Grave) |
                                    keyboardSelectedTab(sf::Keyboard::Key::Apostrophe) |
                                    keyboardSelectedTab(sf::Keyboard::Key::Backslash)))
        {
            selectedTab(0);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Shop", nullptr, shopSelectOnce | keyboardSelectedTab(sf::Keyboard::Key::Num1)))
        {
            selectedTab(1);

            shopSelectOnce = {};

            ImGui::BeginChild("ShopScroll",
                              ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                              0,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar);
            uiTabBarShop();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        sf::base::SizeT nextTabKeyIndex = 0u;

        constexpr sf::Keyboard::Key tabKeys[] = {
            sf::Keyboard::Key::Num2,
            sf::Keyboard::Key::Num3,
            sf::Keyboard::Key::Num4,
            sf::Keyboard::Key::Num5,
            sf::Keyboard::Key::Num6,
        };

        if (cachedWizardCat != nullptr &&
            ImGui::BeginTabItem("Magic", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
        {
            selectedTab(2);

            ImGui::BeginChild("MagicScroll",
                              ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                              0,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar);

            uiTabBarMagic();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (pt.isBubbleValueUnlocked())
        {
            if (!pt.prestigeTipShown)
            {
                pt.prestigeTipShown = true;
                doTip("Prestige to increase bubble value\nand unlock permanent upgrades!");
            }

            const bool canPrestige = pt.canBuyNextPrestige();

            if (canPrestige)
            {
                ImGui::PushStyleColor(ImGuiCol_Tab, IM_COL32(135, 50, 84, 255));
                ImGui::PushStyleColor(ImGuiCol_TabHovered, IM_COL32(136, 65, 105, 255));
                ImGui::PushStyleColor(ImGuiCol_TabSelected, IM_COL32(136, 65, 105, 255));
            }

            if (ImGui::BeginTabItem("Prestige", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
            {
                selectedTab(3);

                ImGui::BeginChild("PrestigeScroll",
                                  ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                                  0,
                                  ImGuiWindowFlags_AlwaysVerticalScrollbar);

                uiTabBarPrestige();

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            if (canPrestige)
                ImGui::PopStyleColor(3);
        }

        if (ImGui::BeginTabItem("Info", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
        {
            selectedTab(4);

            ImGui::BeginChild("StatsScroll",
                              ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                              0,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar);

            uiTabBarStats();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Options", nullptr, keyboardSelectedTab(tabKeys[nextTabKeyIndex++])))
        {
            selectedTab(5);

            ImGui::BeginChild("OptionsScroll",
                              ImVec2(ImGui::GetContentRegionAvail().x, childHeight),
                              0,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar);

            uiTabBarSettings();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }
    }

    ////////////////////////////////////////////////////////////
    void uiSetUnlockLabelY(const sf::base::SizeT unlockId)
    {
        const std::string label = std::to_string(unlockId);
        uiLabelToY[label]       = ImGui::GetCursorScreenPos().y;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool checkUiUnlock(const sf::base::SizeT unlockId, const bool unlockCondition)
    {
        const std::string label = std::to_string(unlockId);

        if (!unlockCondition)
        {
            profile.uiUnlocks[unlockId] = false;
            return false;
        }

        if (!profile.uiUnlocks[unlockId])
        {
            profile.uiUnlocks[unlockId] = true;

            const bool anyPurchaseUnlockedEffectWithSameLabel = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                                purchaseUnlockedEffects.end(),
                                                                                [&](const PurchaseUnlockedEffect& effect)
            { return effect.widgetLabel == label; });

            const bool anyPurchaseUnlockedEffectWithSameY = sf::base::anyOf(purchaseUnlockedEffects.begin(),
                                                                            purchaseUnlockedEffects.end(),
                                                                            [&](const PurchaseUnlockedEffect& effect)
            {
                const auto it = uiLabelToY.find(effect.widgetLabel);
                return it != uiLabelToY.end() && it->second == uiLabelToY[label];
            });

            if (!anyPurchaseUnlockedEffectWithSameLabel && !anyPurchaseUnlockedEffectWithSameY)
            {
                purchaseUnlockedEffects.push_back({
                    .widgetLabel    = label,
                    .countdown      = Countdown{.value = 1000.f},
                    .arrowCountdown = Countdown{.value = 2000.f},
                    .hue            = uiButtonHueMod,
                    .type           = 0, // now unlocked
                });

                playSound(sounds.unlock, /* maxOverlap */ 1u);
                playSound(sounds.shimmer, /* maxOverlap */ 1u);
            }
        }

        return true;
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
    void uiImageFromAtlas(const sf::FloatRect& txr, const sf::RenderTarget::TextureDrawParams& drawParams)
    {
        imGuiContext.image(
            sf::Sprite{
                .position    = drawParams.position,
                .scale       = drawParams.scale * profile.uiScale,
                .origin      = txr.size,
                .textureRect = txr,
            },
            uiTextureAtlas.getTexture(),
            drawParams.color);
    }

    ////////////////////////////////////////////////////////////
    void imgsep(const sf::FloatRect& txr, const char* sepLabel, const bool first = false)
    {
        if (!first)
        {
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
        }

        ImGui::Columns(1);
        uiImageFromAtlas(txr, {.scale = {0.46f, 0.5f}});

        const auto oldFontScale = lastFontScale;
        uiSetFontScale(0.75f);
        uiCenteredText(sepLabel, -5.f * profile.uiScale, -8.f * profile.uiScale);
        uiSetFontScale(oldFontScale);

        ImGui::Separator();
        ImGui::Spacing();

        uiBeginColumns();
    }

    ////////////////////////////////////////////////////////////
    void imgsep2(const sf::FloatRect& txr, const char* sepLabel)
    {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Columns(1);

        uiImageFromAtlas(txr, {.scale = {0.46f, 0.5f}});

        const auto oldFontScale = lastFontScale;
        uiSetFontScale(0.75f);
        uiCenteredText(sepLabel, -5.f * profile.uiScale, -6.f * profile.uiScale);
        uiSetFontScale(oldFontScale);

        uiBeginColumns();
    }

    ////////////////////////////////////////////////////////////
    void uiTabBarShop()
    {
        const auto nCatNormal = pt.getCatCountByType(CatType::Normal);
        const auto nCatUni    = pt.getCatCountByType(CatType::Uni);
        const auto nCatDevil  = pt.getCatCountByType(CatType::Devil);
        const auto nCatAstro  = pt.getCatCountByType(CatType::Astro);

        Cat* catWitch    = cachedWitchCat;
        Cat* catWizard   = cachedWizardCat;
        Cat* catMouse    = cachedMouseCat;
        Cat* catEngi     = cachedEngiCat;
        Cat* catRepulso  = cachedRepulsoCat;
        Cat* catAttracto = cachedAttractoCat;

        const bool anyUniqueCat = catWitch != nullptr || catWizard != nullptr || catMouse != nullptr ||
                                  catEngi != nullptr || catRepulso != nullptr || catAttracto != nullptr;

        ImGui::Spacing();
        ImGui::Spacing();

        imgsep(txrMenuSeparator0, "click upgrades", /* first */ true);

        std::sprintf(uiTooltipBuffer,
                     "Build your combo by popping bubbles quickly, increasing the value of each subsequent "
                     "one.\n\nCombos expire on misclicks and over time, but can be upgraded to last "
                     "longer.\n\nStar bubbles are affected -- pop them while your multiplier is high!");
        uiLabelBuffer[0] = '\0';
        if (makePurchasableButtonOneTime("Combo", 20u, pt.comboPurchased))
        {
            combo = 0;
            doTip("Pop bubbles quickly to keep\nyour combo up and make more money!");
        }

        if (checkUiUnlock(0u, pt.comboPurchased))
        {
            const char* mouseNote = catMouse == nullptr ? "" : "\n\n(Note: this also applies to the Mousecat!)";

            const float currentComboStartTime = pt.psvComboStartTime.currentValue();
            const float nextComboStartTime    = pt.psvComboStartTime.nextValue();

            uiSetUnlockLabelY(0u);
            std::sprintf(uiTooltipBuffer,
                         "Increase combo duration from %.2fs to %.2fs. We are in it for the long haul!%s",
                         static_cast<double>(currentComboStartTime),
                         static_cast<double>(nextComboStartTime),
                         mouseNote);

            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentComboStartTime));
            makePSVButton("  Longer combo", pt.psvComboStartTime);
        }

        if (checkUiUnlock(1u, nCatNormal > 0 && pt.psvComboStartTime.nPurchases > 0))
        {
            imgsep(txrMenuSeparator1, "exploration");

            uiSetUnlockLabelY(1u);
            std::sprintf(uiTooltipBuffer,
                         "Extend the map and enable scrolling.\n\nExtending the map will increase the total number "
                         "of "
                         "bubbles you can work with, and will also reveal magical shrines that grant unique cats "
                         "upon "
                         "completion.\n\nYou can scroll the map with the scroll wheel, holding right click, by "
                         "dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou can jump around the "
                         "map "
                         "by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");
            uiLabelBuffer[0] = '\0';
            if (makePurchasableButtonOneTime("Map scrolling", 1000u, pt.mapPurchased))
            {
                scroll = 0.f;
                doTip(
                    "Explore the map by using the mouse wheel,\ndragging via right click, or with your "
                    "keyboard.\nYou can also click on the minimap!");

                if (pt.psvBubbleValue.nPurchases == 0u)
                    scrollArrowCountdown.value = 2000.f;
            }

            if (checkUiUnlock(2u, pt.mapPurchased))
            {
                uiSetUnlockLabelY(2u);
                std::sprintf(uiTooltipBuffer, "Extend the map further by one screen.");
                std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(pt.getMapLimit() / boundaries.x * 100.f));
                makePSVButton("  Extend map", pt.psvMapExtension);

                ImGui::BeginDisabled(pt.psvShrineActivation.nPurchases > pt.psvMapExtension.nPurchases);
                std::sprintf(uiTooltipBuffer,
                             "Activates the next shrine, enabling it to absorb nearby popped bubbles. Once enough "
                             "bubbles are absorbed by a shrine, it will grant a unique cat.");
                std::sprintf(uiLabelBuffer, "%zu/9", pt.psvShrineActivation.nPurchases);
                if (makePSVButton("  Activate next shrine", pt.psvShrineActivation))
                {
                    if (!pt.shrineActivateTipShown)
                    {
                        pt.shrineActivateTipShown = true;

                        if (pt.psvBubbleValue.nPurchases == 0u)
                        {
                            doTip(
                                "Move your cats near the shrine to help\n it absorb bubbles and unlock a unique "
                                "cat!\nWill there be consequences?");
                        }
                    }
                }
                ImGui::EndDisabled();
            }
        }

        if (checkUiUnlock(3u, nCatNormal > 0 && pt.psvComboStartTime.nPurchases > 0))
        {
            imgsep(txrMenuSeparator2, "bubble upgrades");

            uiSetUnlockLabelY(3u);
            std::sprintf(uiTooltipBuffer,
                         "Increase the total number of bubbles. Scales with map size.\n\nMore bubbles, "
                         "more money, fewer FPS!");
            std::sprintf(uiLabelBuffer, "%zu bubbles", static_cast<SizeT>(pt.psvBubbleCount.currentValue()));
            makePSVButton("More bubbles", pt.psvBubbleCount);
        }

        if (checkUiUnlock(4u, pt.comboPurchased && pt.psvComboStartTime.nPurchases > 0))
        {
            imgsep(txrMenuSeparator3, "cats");

            uiSetUnlockLabelY(4u);
            std::sprintf(uiTooltipBuffer,
                         "Cats pop nearby bubbles or bombs. Their cooldown and range can be upgraded. Their "
                         "behavior can be permanently upgraded with prestige points.\n\nCats can be dragged around "
                         "to "
                         "position them strategically.\n\nNo, you can't get rid of a cat once purchased, you "
                         "monster.");
            std::sprintf(uiLabelBuffer, "%zu cats", nCatNormal);
            if (makePSVButton("Cat", pt.psvPerCatType[asIdx(CatType::Normal)]))
            {
                spawnCatCentered(CatType::Normal, getHueByCatType(CatType::Normal));

                if (nCatNormal == 0)
                    doTip("Cats periodically pop bubbles for you!\nYou can drag them around to position them.");

                if (nCatNormal == 2)
                    doTip(
                        "Multiple cats can be dragged at once by\nholding shift while clicking the mouse.\nRelease "
                        "either button to drop them!");
            }
        }

        const auto makeCooldownButton = [this](const char* label, const CatType catType, const char* additionalInfo = "")
        {
            auto& psv = pt.psvCooldownMultsPerCatType[asIdx(catType)];

            const float currentCooldown = CatConstants::baseCooldowns[asIdx(catType)] * psv.currentValue();
            const float nextCooldown    = CatConstants::baseCooldowns[asIdx(catType)] * psv.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Decrease cooldown from %.2fs to %.2fs.%s\n\n(Note: can be reverted by right-clicking, "
                         "but no "
                         "refunds!)",
                         static_cast<double>(currentCooldown / 1000.f),
                         static_cast<double>(nextCooldown / 1000.f),
                         additionalInfo);

            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(pt.getComputedCooldownByCatType(catType) / 1000.f));

            makePSVButton(label, psv);

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right) && psv.nPurchases > 0u)
            {
                --psv.nPurchases;
                playSound(sounds.buy);
            }
        };

        const auto makeRangeButton = [this](const char* label, const CatType catType, const char* additionalInfo = "")
        {
            auto& psv = pt.psvRangeDivsPerCatType[asIdx(catType)];

            const float currentRange = CatConstants::baseRanges[asIdx(catType)] / psv.currentValue();
            const float nextRange    = CatConstants::baseRanges[asIdx(catType)] / psv.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase range from %.2fpx to %.2fpx.%s\n\n(Note: can be reverted by right-clicking, but "
                         "no "
                         "refunds!)",
                         static_cast<double>(currentRange),
                         static_cast<double>(nextRange),
                         additionalInfo);
            std::sprintf(uiLabelBuffer, "%.2fpx", static_cast<double>(pt.getComputedRangeByCatType(catType)));
            makePSVButton(label, psv);

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right) && psv.nPurchases > 0u)
            {
                --psv.nPurchases;
                playSound(sounds.buy);
            }
        };

        const bool catUpgradesUnlocked = pt.psvBubbleCount.nPurchases > 0 && nCatNormal >= 2 && nCatUni >= 1;
        if (checkUiUnlock(5u, catUpgradesUnlocked))
        {
            uiSetUnlockLabelY(5u);
            makeCooldownButton("  cooldown##Normal", CatType::Normal);
            makeRangeButton("  range##Normal", CatType::Normal);
        }

        // UNICAT
        const bool catUnicornUnlocked         = pt.psvBubbleCount.nPurchases > 0 && nCatNormal >= 3;
        const bool catUnicornUpgradesUnlocked = catUnicornUnlocked && nCatUni >= 2 && nCatDevil >= 1;
        if (checkUiUnlock(6u, catUnicornUnlocked))
        {
            imgsep(txrMenuSeparator4, "unicats");

            uiSetUnlockLabelY(6u);
            std::sprintf(uiTooltipBuffer,
                         "Unicats transform bubbles into star bubbles, which are worth x15 more!\n\nHave "
                         "your cats pop them for you, or pop them towards the end of a combo for huge rewards!");
            std::sprintf(uiLabelBuffer, "%zu unicats", nCatUni);
            if (makePSVButton("Unicat", pt.psvPerCatType[asIdx(CatType::Uni)]))
            {
                spawnCatCentered(CatType::Uni, getHueByCatType(CatType::Uni));

                if (nCatUni == 0)
                    doTip("Unicats transform bubbles in star bubbles,\nworth x15! Pop them at the end of a combo!");
            }

            if (checkUiUnlock(7u, catUnicornUpgradesUnlocked))
            {
                uiSetUnlockLabelY(7u);
                makeCooldownButton("  cooldown##Uni", CatType::Uni);

                if (pt.perm.unicatTranscendencePurchased && pt.perm.unicatTranscendenceAOEPurchased)
                    makeRangeButton("  range##Uni", CatType::Uni);
            }
        }

        // DEVILCAT
        const bool catDevilUnlocked = pt.psvBubbleValue.nPurchases > 0 && nCatNormal >= 6 && nCatUni >= 4 &&
                                      pt.nShrinesCompleted >= 1;
        const bool catDevilUpgradesUnlocked = catDevilUnlocked && nCatDevil >= 2 && nCatAstro >= 1;
        if (checkUiUnlock(8u, catDevilUnlocked))
        {
            imgsep(txrMenuSeparator5, "devilcats");

            uiSetUnlockLabelY(8u);
            std::sprintf(uiTooltipBuffer,
                         "Devilcats transform bubbles into bombs that explode when popped. Bubbles affected by the "
                         "explosion are worth x10 more! Bomb explosion range can be upgraded.");
            std::sprintf(uiLabelBuffer, "%zu devilcats", nCatDevil);
            if (makePSVButton("Devilcat", pt.psvPerCatType[asIdx(CatType::Devil)]))
            {
                spawnCatCentered(CatType::Devil, getHueByCatType(CatType::Devil));

                if (nCatDevil == 0)
                    doTip(
                        "Devilcats transform bubbles in bombs!\nExplode them to pop nearby "
                        "bubbles\nwith a x10 money multiplier!",
                        /* maxPrestigeLevel */ 1);
            }

            if (checkUiUnlock(9u, nCatDevil >= 1) && !isDevilcatHellsingedActive())
            {
                const float currentExplosionRadius = pt.psvExplosionRadiusMult.currentValue();
                const float nextExplosionRadius    = pt.psvExplosionRadiusMult.nextValue();

                uiSetUnlockLabelY(9u);
                std::sprintf(uiTooltipBuffer,
                             "Increase bomb explosion radius from x%.2f to x%.2f.",
                             static_cast<double>(currentExplosionRadius),
                             static_cast<double>(nextExplosionRadius));
                std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentExplosionRadius));
                makePSVButton("  Explosion radius", pt.psvExplosionRadiusMult);
            }

            if (checkUiUnlock(10u, catDevilUpgradesUnlocked))
            {
                uiSetUnlockLabelY(10u);
                makeCooldownButton("  cooldown##Devil", CatType::Devil);

                if (pt.perm.devilcatHellsingedPurchased)
                    makeRangeButton("  range##Devil", CatType::Devil);
            }
        }

        // ASTROCAT
        const bool astroCatUnlocked = nCatNormal >= 10 && nCatUni >= 5 && nCatDevil >= 2 && pt.nShrinesCompleted >= 2;
        const bool astroCatUpgradesUnlocked = astroCatUnlocked && nCatDevil >= 9 && nCatAstro >= 5;
        if (checkUiUnlock(11u, astroCatUnlocked))
        {
            imgsep(txrMenuSeparator6, "astrocats");

            uiSetUnlockLabelY(11u);
            std::sprintf(uiTooltipBuffer,
                         "Astrocats periodically fly across the map, popping bubbles they hit with a huge x20 "
                         "money "
                         "multiplier!\n\nThey can be permanently upgraded with prestige points to inspire cats "
                         "watching them fly past to pop bubbles faster.");
            std::sprintf(uiLabelBuffer, "%zu astrocats", nCatAstro);
            if (makePSVButton("Astrocat", pt.psvPerCatType[asIdx(CatType::Astro)]))
            {
                spawnCatCentered(CatType::Astro, getHueByCatType(CatType::Astro));

                if (nCatAstro == 0)
                    doTip(
                        "Astrocats periodically fly across\nthe entire map, with a huge\nx20 "
                        "money multiplier!",
                        /* maxPrestigeLevel */ 1);
            }

            if (checkUiUnlock(12u, astroCatUpgradesUnlocked))
            {
                uiSetUnlockLabelY(12u);
                makeCooldownButton("  cooldown##Astro", CatType::Astro);
                makeRangeButton("  range##Astro", CatType::Astro);
            }
        }

        // UNIQUE CAT BONUSES
        if (anyUniqueCat)
        {
            imgsep(txrMenuSeparator7, "unique cats");

            if (checkUiUnlock(13u, catWitch != nullptr))
            {
                uiSetUnlockLabelY(13u);
                makeCooldownButton("  witchcat cooldown",
                                   CatType::Witch,
                                   "\n\nEffectively increases the frequency of rituals.");

                if (checkUiUnlock(14u, pt.perm.witchCatBuffPowerScalesWithNCats))
                {
                    uiSetUnlockLabelY(14u);
                    makeRangeButton("  witchcat range",
                                    CatType::Witch,
                                    "\n\nAllows more cats to participate in group rituals, increasing the duration "
                                    "of "
                                    "buffs.");
                }
            }

            if (checkUiUnlock(15u, catWizard != nullptr))
            {
                ImGui::Separator();

                uiSetUnlockLabelY(15u);
                makeCooldownButton("  wizardcat cooldown",
                                   CatType::Wizard,
                                   "\n\nDoes *not* increase mana generation rate, but increases star bubble "
                                   "absorption "
                                   "rate and decreases cooldown between spell casts.");

                makeRangeButton("  wizardcat range",
                                CatType::Wizard,
                                "\n\nEffectively increases the area of effect of most spells, and star bubble "
                                "absorption range.");
            }

            if (checkUiUnlock(16u, catMouse != nullptr))
            {
                ImGui::Separator();

                uiSetUnlockLabelY(16u);
                makeCooldownButton("  mousecat cooldown##Mouse", CatType::Mouse);
                makeRangeButton("  mousecat range##Mouse", CatType::Mouse);
            }

            if (checkUiUnlock(17u, catEngi != nullptr))
            {
                ImGui::Separator();

                uiSetUnlockLabelY(17u);
                makeCooldownButton("  engicat cooldown",
                                   CatType::Engi,
                                   "\n\nEffectively increases the frequency of maintenances.");

                makeRangeButton("  engicat range",
                                CatType::Engi,
                                "\n\nAllows more cats to be boosted by maintenance at once.");
            }

            if (checkUiUnlock(18u, catRepulso != nullptr))
            {
                ImGui::Separator();

                uiSetUnlockLabelY(18u);
                // makeCooldownButton("  repulsocat cooldown##Repulso", CatType::Repulso);
                makeRangeButton("  repulsocat range##Repulso", CatType::Repulso);
            }

            if (checkUiUnlock(19u, catAttracto != nullptr))
            {
                ImGui::Separator();

                uiSetUnlockLabelY(19u);
                // makeCooldownButton("  attractocat cooldown##Attracto", CatType::Attracto);
                makeRangeButton("  attractocat range##Attracto", CatType::Attracto);
            }
        }

        const auto nextGoalsText = [&]() -> std::string
        {
            std::string result; // Use a single local variable for NRVO

            if (!pt.comboPurchased)
            {
                result = "- buy combo to earn money faster";
                return result;
            }

            if (pt.psvComboStartTime.nPurchases == 0)
            {
                result = "- buy longer combo to unlock cats";
                return result;
            }

            if (nCatNormal == 0)
            {
                result = "- buy a cat";
                return result;
            }

            const auto startList = [&](const char* s)
            {
                result += result.empty() ? "" : "\n\n";
                result += s;
            };

            const auto needNCats = [&](const SizeT& count, const SizeT needed)
            {
                const char* name = "";

                // clang-format off
                if      (&count == &nCatNormal) name = "cat";
                else if (&count == &nCatUni)    name = "unicat";
                else if (&count == &nCatDevil)  name = "devilcat";
                else if (&count == &nCatAstro)  name = "astrocat";
                // clang-format on

                if (count < needed)
                    result += "\n    - buy " + std::to_string(needed - count) + " more " + name + "(s)";
            };

            if (!pt.mapPurchased)
            {
                startList("- to extend playing area:");
                result += "\n    - buy map scrolling";
            }

            if (!catUnicornUnlocked)
            {
                startList("- to unlock unicats:");

                if (pt.psvBubbleCount.nPurchases == 0)
                    result += "\n    - buy more bubbles";

                needNCats(nCatNormal, 3);
            }

            if (!catUpgradesUnlocked && catUnicornUnlocked)
            {
                startList("- to unlock cat upgrades:");

                if (pt.psvBubbleCount.nPurchases == 0)
                    result += "\n    - buy more bubbles";

                needNCats(nCatNormal, 2);
                needNCats(nCatUni, 1);
            }

            // TODO P2: change dynamically
            if (catUnicornUnlocked && !pt.isBubbleValueUnlocked())
            {
                startList("- to unlock prestige:");

                if (pt.psvBubbleCount.nPurchases == 0)
                    result += "\n    - buy more bubbles";

                if (pt.nShrinesCompleted < 1)
                    result += "\n    - complete at least one shrine";

                needNCats(nCatUni, 3);
            }

            if (catUnicornUnlocked && pt.isBubbleValueUnlocked() && !catDevilUnlocked)
            {
                startList("- to unlock devilcats:");

                if (pt.psvBubbleValue.nPurchases == 0)
                    result += "\n    - prestige at least once";

                if (pt.nShrinesCompleted < 1)
                    result += "\n    - complete at least one shrine";

                if (pt.psvBubbleValue.nPurchases > 0u)
                {
                    needNCats(nCatNormal, 6);
                    needNCats(nCatUni, 4);
                }
            }

            if (catUnicornUnlocked && catDevilUnlocked && !catUnicornUpgradesUnlocked)
            {
                startList("- to unlock unicat upgrades:");
                needNCats(nCatUni, 2);
                needNCats(nCatDevil, 1);
            }

            if (catUnicornUnlocked && catDevilUnlocked && !astroCatUnlocked)
            {
                startList("- to unlock astrocats:");

                if (pt.nShrinesCompleted < 2)
                    result += "\n    - complete at least two shrines";

                needNCats(nCatNormal, 10);
                needNCats(nCatUni, 5);
                needNCats(nCatDevil, 2);
            }

            if (catUnicornUnlocked && catDevilUnlocked && astroCatUnlocked && !catDevilUpgradesUnlocked)
            {
                startList("- to unlock devilcat upgrades:");
                needNCats(nCatDevil, 2);
                needNCats(nCatAstro, 1);
            }

            if (catUnicornUnlocked && catDevilUnlocked && astroCatUnlocked && !astroCatUpgradesUnlocked)
            {
                startList("- to unlock astrocat upgrades:");
                needNCats(nCatDevil, 9);
                needNCats(nCatAstro, 5);
            }

            return result;
        }();

        ImGui::Columns(1);

        if (nextGoalsText != "")
        {
            imgsep(txrMenuSeparator8, "next goals");
            ImGui::Columns(1);

            uiSetFontScale(uiSubBulletFontScale);
            ImGui::Text("%s", nextGoalsText.c_str());
            uiSetFontScale(uiNormalFontScale);
        }

        ImGui::Columns(1);
    }

    ////////////////////////////////////////////////////////////
    bool uiCheckbox(const char* label, bool* b)
    {
        if (!ImGui::Checkbox(label, b))
            return false;

        playSound(sounds.btnswitch);
        return true;
    }

    ////////////////////////////////////////////////////////////
    bool uiRadio(const char* label, int* i, const int value)
    {
        if (!ImGui::RadioButton(label, i, value))
            return false;

        playSound(sounds.btnswitch);
        return true;
    }

    ////////////////////////////////////////////////////////////
    void beginPrestigeTransition(const PrestigePointsType ppReward)
    {
        playSound(sounds.prestige);

        inPrestigeTransition = true;
        scroll               = 0.f;

        resetAllDraggedCats();
        pt.onPrestige(ppReward);

        profile.selectedBackground = 0;
        profile.selectedBGM        = 0;

        updateSelectedBackgroundSelectorIndex();
        updateSelectedBGMSelectorIndex();

        switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ true);
    }

    ////////////////////////////////////////////////////////////
    void uiTabBarPrestige()
    {
        ImGui::Spacing();
        ImGui::Spacing();

        imgsep(txrPrestigeSeparator0, "prestige", /* first */ true);
        ImGui::Columns(1);

        uiSetFontScale(uiNormalFontScale);

        std::sprintf(uiTooltipBuffer,
                     "WARNING: this will reset your progress!\n\nPrestige to increase bubble value permanently and "
                     "obtain prestige points. Prestige points can be used to unlock powerful permanent "
                     "upgrades.\n\nYou will sacrifice all your cats, bubbles, and money, but you will keep your "
                     "prestige points and permanent upgrades, and the value of bubbles will be permanently "
                     "increased.\n\nDo not be afraid to prestige -- it is what enables you to progress further!");
        std::sprintf(uiLabelBuffer, "current bubble value x%llu", pt.getComputedRewardByBubbleType(BubbleType::Normal));

        // Figure out how many times we can prestige in a row

        const auto currentPrestigeLevel    = pt.psvBubbleValue.nPurchases;
        const auto currentCompletedShrines = pt.nShrinesCompleted;
        const auto maxPrestigeLevel        = sf::base::SizeT{20u};

        sf::base::SizeT maxPurchaseablePrestigeLevel = currentPrestigeLevel;
        MoneyType       maxCost                      = 0u;

        for (sf::base::SizeT iPrestige = currentPrestigeLevel + 1u; iPrestige < maxPrestigeLevel; ++iPrestige)
        {
            const auto requiredMoney = static_cast<MoneyType>(
                pt.psvBubbleValue.cumulativeCostBetween(currentPrestigeLevel, iPrestige));

            const auto requiredCompletedShrines = Playthrough::getShrinesCompletedNeededForPrestigeLevel(iPrestige);

            if (pt.money >= requiredMoney && currentCompletedShrines >= requiredCompletedShrines)
            {
                maxPurchaseablePrestigeLevel = iPrestige;
                maxCost                      = requiredMoney;

                continue;
            }

            break;
        }

        const auto prestigeTimes = maxPurchaseablePrestigeLevel - currentPrestigeLevel;
        const auto ppReward      = pt.calculatePrestigePointReward(prestigeTimes);

        const auto printNextPrestigeRequirements = [&](const sf::base::SizeT level)
        {
            const auto nextCost = static_cast<MoneyType>(
                pt.psvBubbleValue.cumulativeCostBetween(currentPrestigeLevel, level));

            const auto nextRequiredShrines = Playthrough::getShrinesCompletedNeededForPrestigeLevel(level);

            ImGui::Text("    (level %zu -> %zu): $%s, %zu completed shrines",
                        currentPrestigeLevel + 1u,
                        level + 1u,
                        toStringWithSeparators(nextCost),
                        nextRequiredShrines);
        };

        uiSetFontScale(0.75f);

        const bool canPrestigePlus1 = maxPurchaseablePrestigeLevel + 1u < maxPrestigeLevel;
        const bool canPrestigePlus2 = maxPurchaseablePrestigeLevel + 2u < maxPrestigeLevel;

        if (canPrestigePlus1 || canPrestigePlus2)
        {
            ImGui::Text("  next prestige requirements:");

            if (canPrestigePlus1)
                printNextPrestigeRequirements(maxPurchaseablePrestigeLevel + 1u);

            if (canPrestigePlus2)
                printNextPrestigeRequirements(maxPurchaseablePrestigeLevel + 2u);

            ImGui::Spacing();
            ImGui::Spacing();
        }

        uiSetFontScale(uiNormalFontScale);

        uiBeginColumns();

        uiButtonHueMod = 120.f;

        ImGui::BeginDisabled(prestigeTimes == 0u);
        if (makePSVButtonEx("Prestige", pt.psvBubbleValue, prestigeTimes, maxCost))
        {
            beginPrestigeTransition(ppReward);
        }
        ImGui::EndDisabled();

        ImGui::Columns(1);

        uiButtonHueMod = 0.f;
        uiSetFontScale(0.75f);

        const auto currentMult = static_cast<SizeT>(pt.psvBubbleValue.currentValue()) + 1u;

        if (prestigeTimes > 0u)
        {
            ImGui::Text(
                "  prestige %zu time(s) at once\n  - increase bubble value from x%zu to x%zu\n  - obtain %llu "
                "prestige "
                "point(s)",
                prestigeTimes,
                currentMult,
                currentMult + prestigeTimes,
                ppReward);
        }
        else if (pt.psvBubbleValue.nPurchases == 19u)
        {
            ImGui::Text("  max prestige level reached!");

            uiSetFontScale(uiSubBulletFontScale);
            uiBeginColumns();

            std::sprintf(uiTooltipBuffer, "Spend money to immediately get prestige points.");
            uiLabelBuffer[0] = '\0';

            bool done = false;
            if (makePurchasableButtonOneTime("  Buy 100 PPs", 1'000'000'000'000u, done))
                pt.prestigePoints += 100u;

            ImGui::Columns(1);
        }
        else
        {
            if (maxCost == 0u)
                ImGui::Text("  not enough money to prestige");

            const SizeT shrinesNeeded = pt.getShrinesCompletedNeededForNextPrestige();

            if (pt.nShrinesCompleted < shrinesNeeded)
                ImGui::Text("  must complete %zu more shrine(s)", shrinesNeeded - pt.nShrinesCompleted);
        }

        uiSetFontScale(uiNormalFontScale);

        imgsep(txrPrestigeSeparator1, "permanent upgrades");
        ImGui::Columns(1);

        uiSetFontScale(uiNormalFontScale * 1.1f);
        char ppBuf[256];
        std::snprintf(ppBuf, sizeof(ppBuf), "%llu PPs available", pt.prestigePoints);
        uiCenteredText(ppBuf);
        uiSetFontScale(uiNormalFontScale);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        uiBeginColumns();

        uiButtonHueMod = 190.f;

        if (checkUiUnlock(47u, pt.psvBubbleValue.nPurchases >= 3u))
        {
            imgsep2(txrPrestigeSeparator4, "faster beginning");

            uiSetUnlockLabelY(47u);
            std::sprintf(uiTooltipBuffer, "Begin your next prestige with $1000.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Starter pack", 1u, pt.perm.starterPackPurchased);
        }

        if (checkUiUnlock(48u, pt.psvBubbleValue.nPurchases >= 1u))
        {
            imgsep2(txrPrestigeSeparator2, "clicking tools");

            uiSetUnlockLabelY(48u);
            std::sprintf(uiTooltipBuffer,
                         "Manually popping a bubble now also pops nearby bubbles automatically!\n\n(Note: combo "
                         "multiplier still only increases once per successful click.)\n\n(Note: this effect can be "
                         "toggled "
                         "at will.)");
            uiLabelBuffer[0] = '\0';
            if (makePurchasablePPButtonOneTime("Multipop click", 0u, pt.perm.multiPopPurchased))
                doTip("Popping a bubble now also pops\nnearby bubbles automatically!",
                      /* maxPrestigeLevel */ UINT_MAX);

            if (checkUiUnlock(49u, pt.perm.multiPopPurchased))
            {
                uiSetUnlockLabelY(49u);

                if (pt.psvBubbleValue.nPurchases >= 2u)
                {
                    const float currentRange = pt.psvPPMultiPopRange.currentValue();
                    const float nextRange    = pt.psvPPMultiPopRange.nextValue();

                    std::sprintf(uiTooltipBuffer,
                                 "Increase the range of the multipop effect from %.2fpx to %.2fpx.",
                                 static_cast<double>(currentRange),
                                 static_cast<double>(nextRange));
                    std::sprintf(uiLabelBuffer, "%.2fpx", static_cast<double>(currentRange));
                    makePrestigePurchasablePPButtonPSV("  range", pt.psvPPMultiPopRange);
                }

                uiSetFontScale(uiSubBulletFontScale);
                uiCheckbox("enable ##multipop", &pt.multiPopEnabled);
                if (cachedMouseCat != nullptr)
                {
                    ImGui::SameLine();
                    uiCheckbox("mousecat##multipopmousecat", &pt.multiPopMouseCatEnabled);
                }
                uiSetFontScale(uiNormalFontScale);
                ImGui::NextColumn();
                ImGui::NextColumn();
            }
        }

        imgsep2(txrPrestigeSeparator3, "wind effects");

        std::sprintf(uiTooltipBuffer,
                     "A giant fan (off-screen) will produce an intense wind, making bubbles move and "
                     "flow much faster.\n\n(Note: this effect can be toggled at will.)");
        uiLabelBuffer[0] = '\0';
        if (makePurchasablePPButtonOneTime("Giant fan", 6u, pt.perm.windPurchased))
            doTip("Hold onto something!", /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(50u, pt.perm.windPurchased))
        {
            uiSetFontScale(uiSubBulletFontScale);
            ImGui::Columns(1);

            uiSetUnlockLabelY(50u);
            uiRadio("off##windOff", &pt.windStrength, 0);
            ImGui::SameLine();
            uiRadio("slow##windSlow", &pt.windStrength, 1);
            ImGui::SameLine();
            uiRadio("medium##windMed", &pt.windStrength, 2);
            ImGui::SameLine();
            uiRadio("fast##windFast", &pt.windStrength, 3);

            uiBeginColumns();
            uiSetFontScale(uiNormalFontScale);
            ImGui::NextColumn();
            ImGui::NextColumn();
        }

        imgsep2(txrPrestigeSeparator5, "cats");

        uiBeginColumns();

        std::sprintf(uiTooltipBuffer,
                     "Cats have graduated!\n\nThey still cannot resist their popping insticts, but they will go "
                     "for star bubbles and bombs first, ensuring they are not wasted!");
        uiLabelBuffer[0] = '\0';
        if (makePurchasablePPButtonOneTime("Smart cats", 1u, pt.perm.smartCatsPurchased))
            doTip("Cats will now prioritize popping\nspecial bubbles over basic ones!",
                  /* maxPrestigeLevel */ UINT_MAX);

        if (checkUiUnlock(51u, pt.perm.smartCatsPurchased))
        {
            uiSetUnlockLabelY(51u);
            std::sprintf(uiTooltipBuffer,
                         "Embrace the glorious evolution!\n\nCats have ascended beyond their primal "
                         "insticts and will now prioritize bombs, then star bubbles, then normal "
                         "bubbles!\n\nThey will also ignore any bubble type of your choosing.\n\nThrough the sheer "
                         "power of their intellect, they also get a x2 multiplier on all bubble values.\n\n(Note: "
                         "this effect can be toggled at will.)");
            uiLabelBuffer[0] = '\0';
            if (makePurchasablePPButtonOneTime("genius cats", 8u, pt.perm.geniusCatsPurchased))
                doTip("Genius cats prioritize bombs and\ncan be instructed to ignore certain bubbles!",
                      /* maxPrestigeLevel */ UINT_MAX);
        }

        if (checkUiUnlock(52u, pt.perm.geniusCatsPurchased))
        {
            ImGui::Columns(1);
            uiSetFontScale(uiSubBulletFontScale);

            uiSetUnlockLabelY(52u);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("  ignore: ");
            ImGui::SameLine();

            auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt.geniusCatIgnoreBubbles;

            uiCheckbox("normal##genius", &ignoreNormal);
            ImGui::SameLine();

            uiCheckbox("star##genius", &ignoreStar);
            ImGui::SameLine();

            uiCheckbox("bombs##genius", &ignoreBomb);

            uiSetFontScale(uiNormalFontScale);
            uiBeginColumns();
        }

        if (checkUiUnlock(53u, pt.psvBubbleValue.nPurchases >= 3))
        {
            imgsep2(txrPrestigeSeparator6, "unicats");

            uiBeginColumns();

            uiSetUnlockLabelY(53u);
            std::sprintf(uiTooltipBuffer,
                         "Unicats transcend their physical form, becoming a higher entity that transforms bubbles "
                         "into "
                         "nova bubbles, worth x50.");
            uiLabelBuffer[0] = '\0';

            if (makePurchasablePPButtonOneTime("transcendence", 96u, pt.perm.unicatTranscendencePurchased))
                doTip("Are you ready for that sweet x50?", /* maxPrestigeLevel */ UINT_MAX);

            if (checkUiUnlock(54u, pt.perm.unicatTranscendencePurchased))
            {
                uiSetUnlockLabelY(54u);
                std::sprintf(uiTooltipBuffer,
                             "Unicats can now transform all bubbles in range at once. Also unlocks Unicat range "
                             "upgrades.");
                uiLabelBuffer[0] = '\0';

                if (makePurchasablePPButtonOneTime("nova expanse", 128u, pt.perm.unicatTranscendenceAOEPurchased))
                    doTip("It's about to get crazy...", /* maxPrestigeLevel */ UINT_MAX);

                if (pt.perm.unicatTranscendencePurchased)
                {
                    ImGui::Columns(1);
                    uiCheckbox("enable transcendent unicats", &pt.perm.unicatTranscendenceEnabled);
                    uiBeginColumns();
                }
            }
        }

        if (checkUiUnlock(55u, pt.psvBubbleValue.nPurchases >= 3))
        {
            imgsep2(txrPrestigeSeparator7, "devilcats");

            uiBeginColumns();

            uiSetUnlockLabelY(55u);
            std::sprintf(uiTooltipBuffer,
                         "Devilcats become touched by the flames of hell, opening stationary portals that teleport "
                         "bubbles into the abyss, with a x50 multiplier. Also unlocks Devilcat range upgrades.");
            uiLabelBuffer[0] = '\0';

            if (makePurchasablePPButtonOneTime("hellsinged", 192u, pt.perm.devilcatHellsingedPurchased))
                doTip("I'm starting to get a bit scared...", /* maxPrestigeLevel */ UINT_MAX);

            if (pt.perm.devilcatHellsingedPurchased)
            {
                ImGui::Columns(1);
                uiCheckbox("enable hellsinged devilcats", &pt.perm.devilcatHellsingedEnabled);
                uiBeginColumns();
            }
        }

        if (checkUiUnlock(56u, pt.getCatCountByType(CatType::Astro) >= 1u || pt.psvBubbleValue.nPurchases >= 3))
        {
            imgsep2(txrPrestigeSeparator8, "astrocats");

            uiBeginColumns();

            uiSetUnlockLabelY(56u);
            std::sprintf(uiTooltipBuffer,
                         "Astrocats are now equipped with fancy patriotic flags, inspiring cats watching "
                         "them fly by to work faster!");
            uiLabelBuffer[0] = '\0';

            if (makePurchasablePPButtonOneTime("Space propaganda", 16u, pt.perm.astroCatInspirePurchased))
                doTip("Astrocats will inspire other cats\nto work faster when flying by!",
                      /* maxPrestigeLevel */ UINT_MAX);

            if (checkUiUnlock(57u, pt.perm.astroCatInspirePurchased))
            {
                const float currentDuration = pt.getComputedInspirationDuration();
                const float nextDuration    = pt.getComputedNextInspirationDuration();

                uiSetUnlockLabelY(57u);
                std::sprintf(uiTooltipBuffer,
                             "Increase the duration of the inspiration effect from %.2fs to %.2fs.",
                             static_cast<double>(currentDuration / 1000.f),
                             static_cast<double>(nextDuration / 1000.f));
                std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentDuration / 1000.f));

                makePrestigePurchasablePPButtonPSV("inspire duration", pt.psvPPInspireDurationMult);
            }
        }

        const auto makeUnsealButton = [&](const PrestigePointsType ppCost, const char* catName, const CatType type)
        {
            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(type)])
                return;

            std::sprintf(uiTooltipBuffer,
                         "Permanently release the %s from their shrine. They will be waiting for you right "
                         "outside when the shrine is activated.\n\n(Note: completing the shrine will now grant "
                         "1.5x the money it absorbed.)",
                         catName);
            uiLabelBuffer[0] = '\0';

            char buf[256];
            std::snprintf(buf, sizeof(buf), "%s##%s", "Break the seal", catName);

            (void)makePurchasablePPButtonOneTime(buf, ppCost, pt.perm.unsealedByType[asIdx(type)]);
        };

        if (checkUiUnlock(58u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]))
        {
            imgsep2(txrPrestigeSeparator9, "witchcat");

            uiBeginColumns();

            uiSetUnlockLabelY(58u);
            makeUnsealButton(4u, "Witchcat", CatType::Witch);
            ImGui::Separator();

            const float currentDuration = pt.psvPPWitchCatBuffDuration.currentValue();
            const float nextDuration    = pt.psvPPWitchCatBuffDuration.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the base duration of Witchcat buffs from %.2fs to %.2fs.",
                         static_cast<double>(currentDuration),
                         static_cast<double>(nextDuration));
            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentDuration));
            makePrestigePurchasablePPButtonPSV("Buff duration", pt.psvPPWitchCatBuffDuration);

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer,
                         "The duration of Witchcat buffs scales with the number of cats in range of the ritual.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Group ritual", 4u, pt.perm.witchCatBuffPowerScalesWithNCats);

            std::sprintf(uiTooltipBuffer, "The duration of Witchcat buffs scales with the size of the explored map.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Worldwide cult", 4u, pt.perm.witchCatBuffPowerScalesWithMapSize);

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer, "Half as many voodoo dolls will appear per ritual.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Material shortage", 8u, pt.perm.witchCatBuffFewerDolls);

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer, "Dolls are automatically collected by Devilcat bomb explosions.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Flammable dolls", 8u, pt.perm.witchCatBuffFlammableDolls);

            std::sprintf(uiTooltipBuffer, "Dolls are automatically collected by Astrocats during their flyby.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Orbital dolls", 16u, pt.perm.witchCatBuffOrbitalDolls);

            ImGui::Separator();

            const float currentUniPercentage = pt.psvPPUniRitualBuffPercentage.currentValue();
            const float nextUniPercentage    = pt.psvPPUniRitualBuffPercentage.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the star bubble spawn chance during the Unicat vododoo ritual buff from %.2f%% "
                         "to "
                         "%.2f%%.",
                         static_cast<double>(currentUniPercentage),
                         static_cast<double>(nextUniPercentage));
            std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentUniPercentage));
            makePrestigePurchasablePPButtonPSV("Star Spawn %", pt.psvPPUniRitualBuffPercentage);

            const float currentDevilPercentage = pt.psvPPDevilRitualBuffPercentage.currentValue();
            const float nextDevilPercentage    = pt.psvPPDevilRitualBuffPercentage.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the bomb spawn chance during the Devil vododoo ritual buff from %.2f%% to "
                         "%.2f%%.",
                         static_cast<double>(currentDevilPercentage),
                         static_cast<double>(nextDevilPercentage));
            std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentDevilPercentage));
            makePrestigePurchasablePPButtonPSV("Bomb Spawn %", pt.psvPPDevilRitualBuffPercentage);
        }

        if (checkUiUnlock(59u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]))
        {
            imgsep2(txrPrestigeSeparator10, "wizardcat");

            uiBeginColumns();

            uiSetUnlockLabelY(59u);
            makeUnsealButton(8u, "Wizardcat", CatType::Wizard);
            ImGui::Separator();

            const float currentManaCooldown = pt.getComputedManaCooldown();
            const float nextManaCooldown    = pt.getComputedManaCooldownNext();

            std::sprintf(uiTooltipBuffer,
                         "Decrease mana generation cooldown from %.2fs to %.2fs.",
                         static_cast<double>(currentManaCooldown / 1000.f),
                         static_cast<double>(nextManaCooldown / 1000.f));
            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(currentManaCooldown / 1000.f));
            makePrestigePurchasablePPButtonPSV("Mana cooldown", pt.psvPPManaCooldownMult);

            const ManaType currentMaxMana = pt.getComputedMaxMana();
            const ManaType nextMaxMana    = pt.getComputedMaxManaNext();

            std::sprintf(uiTooltipBuffer, "Increase the maximum mana from %llu to %llu.", currentMaxMana, nextMaxMana);
            std::sprintf(uiLabelBuffer, "%llu mana", currentMaxMana);
            makePrestigePurchasablePPButtonPSV("Mana limit", pt.psvPPManaMaxMult);

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer,
                         "Allow the Wizardcat to automatically cast spells when enough mana is available. Can be "
                         "enabled and configured from the \"Magic\" tab.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Autocast", 4u, pt.perm.autocastPurchased);

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer,
                         "Starpaw conversion ignores bombs, transforming only normal bubbles around the wizard "
                         "into star bubbles.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Selective starpaw", 4u, pt.perm.starpawConversionIgnoreBombs);

            if (pt.perm.unicatTranscendencePurchased && pt.perm.starpawConversionIgnoreBombs)
            {
                std::sprintf(uiTooltipBuffer, "Starpaw conversion now turns all normal bubbles into nova bubbles.");
                uiLabelBuffer[0] = '\0';
                (void)makePurchasablePPButtonOneTime("Nova starpaw", 64u, pt.perm.starpawNova);
            }

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer, "The duration of Mewltiplier Aura is extended from 6s to 12s.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Meeeeeewltiplier", 64u, pt.perm.wizardCatDoubleMewltiplierDuration);

            ImGui::Separator();

            std::sprintf(uiTooltipBuffer, "The duration of Stasis Field is extended from 6s to 12s.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Pop Stuck In Time", 256u, pt.perm.wizardCatDoubleStasisFieldDuration);
        }

        if (checkUiUnlock(60u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]))
        {
            imgsep2(txrPrestigeSeparator11, "mousecat");

            uiBeginColumns();

            uiSetUnlockLabelY(60u);
            makeUnsealButton(8u, "Mousecat", CatType::Mouse);
            ImGui::Separator();

            const float currentReward = pt.psvPPMouseCatGlobalBonusMult.currentValue();
            const float nextReward    = pt.psvPPMouseCatGlobalBonusMult.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the global click reward value multiplier from x%.2f to x%.2f.",
                         static_cast<double>(currentReward),
                         static_cast<double>(nextReward));
            std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentReward));
            makePrestigePurchasablePPButtonPSV("Global click mult", pt.psvPPMouseCatGlobalBonusMult);
        }

        if (checkUiUnlock(61u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]))
        {
            imgsep2(txrPrestigeSeparator12, "engicat");

            uiBeginColumns();

            uiSetUnlockLabelY(61u);
            makeUnsealButton(16u, "Engicat", CatType::Engi);
            ImGui::Separator();

            const float currentReward = pt.psvPPEngiCatGlobalBonusMult.currentValue();
            const float nextReward    = pt.psvPPEngiCatGlobalBonusMult.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the global cat reward value multiplierfrom x%.2f to x%.2f.",
                         static_cast<double>(currentReward),
                         static_cast<double>(nextReward));
            std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentReward));
            makePrestigePurchasablePPButtonPSV("Global cat mult", pt.psvPPEngiCatGlobalBonusMult);
        }

        if (checkUiUnlock(62u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]))
        {
            imgsep2(txrPrestigeSeparator13, "repulsocat");

            uiBeginColumns();

            uiSetUnlockLabelY(62u);
            makeUnsealButton(128u, "Repulsocat", CatType::Repulso);
            ImGui::Separator();

            std::sprintf(uiTooltipBuffer, "The Repulsocat cordially asks their fan to filter repelled bubbles by type.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Repulsion filter", 16u, pt.perm.repulsoCatFilterPurchased);

            if (checkUiUnlock(63u, pt.perm.repulsoCatFilterPurchased))
            {
                ImGui::Columns(1);
                uiSetFontScale(uiSubBulletFontScale);

                uiSetUnlockLabelY(63u);
                ImGui::Text("  ignore: ");
                ImGui::SameLine();

                auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt.repulsoCatIgnoreBubbles;

                uiCheckbox("normal##repulso", &ignoreNormal);
                ImGui::SameLine();

                uiCheckbox("star##repulso", &ignoreStar);
                ImGui::SameLine();

                uiCheckbox("bombs##repulso", &ignoreBomb);

                uiSetFontScale(uiNormalFontScale);
                uiBeginColumns();
            }

            std::sprintf(uiTooltipBuffer,
                         "The Repulsocat coats the fan blades with star powder, giving it a chance to convert "
                         "repelled bubbles to star bubbles.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Conversion field", 32u, pt.perm.repulsoCatConverterPurchased);

            if (checkUiUnlock(64u, pt.perm.repulsoCatConverterPurchased))
            {
                uiSetFontScale(uiSubBulletFontScale);
                uiSetUnlockLabelY(64u);
                uiCheckbox("enable ##repulsoconv", &pt.repulsoCatConverterEnabled);
                uiSetFontScale(uiNormalFontScale);
                ImGui::NextColumn();
                ImGui::NextColumn();

                const float currentChance = pt.psvPPRepulsoCatConverterChance.currentValue();
                const float nextChance    = pt.psvPPRepulsoCatConverterChance.nextValue();

                std::sprintf(uiTooltipBuffer,
                             "Increase the repelled bubble conversion chance from %.2f%% to %.2f%%.",
                             static_cast<double>(currentChance),
                             static_cast<double>(nextChance));
                std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentChance));
                makePrestigePurchasablePPButtonPSV("Conversion chance", pt.psvPPRepulsoCatConverterChance);

                std::sprintf(uiTooltipBuffer, "Bubbles are converted into nova bubbles instead of star bubbles.");
                uiLabelBuffer[0] = '\0';
                (void)makePurchasablePPButtonOneTime("Nova conversion", 96u, pt.perm.repulsoCatNovaConverterPurchased);
            }
        }

        if (checkUiUnlock(65u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]))
        {
            imgsep2(txrPrestigeSeparator14, "attractocat");

            uiBeginColumns();

            uiSetUnlockLabelY(65u);
            makeUnsealButton(256u, "Attractocat", CatType::Attracto);
            ImGui::Separator();

            std::sprintf(uiTooltipBuffer,
                         "The Attractocat does some quantum science stuff to its magnet to allow filtering of "
                         "attracted bubbles by type.");
            uiLabelBuffer[0] = '\0';
            (void)makePurchasablePPButtonOneTime("Attraction filter", 96u, pt.perm.attractoCatFilterPurchased);

            if (checkUiUnlock(66u, pt.perm.attractoCatFilterPurchased))
            {
                ImGui::Columns(1);
                uiSetFontScale(uiSubBulletFontScale);

                uiSetUnlockLabelY(66u);
                ImGui::Text("  ignore: ");
                ImGui::SameLine();

                auto& [ignoreNormal, ignoreStar, ignoreBomb] = pt.attractoCatIgnoreBubbles;

                uiCheckbox("normal##attracto", &ignoreNormal);
                ImGui::SameLine();

                uiCheckbox("star##attracto", &ignoreStar);
                ImGui::SameLine();

                uiCheckbox("bombs##attracto", &ignoreBomb);

                uiSetFontScale(uiNormalFontScale);
                uiBeginColumns();
            }
        }

        if (checkUiUnlock(67u, pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]))
        {
            imgsep2(txrPrestigeSeparator15, "copycat");

            uiBeginColumns();

            uiSetUnlockLabelY(67u);
            makeUnsealButton(512u, "Copycat", CatType::Copy);
            ImGui::Separator();

            // TODO P1: something?
        }

        uiButtonHueMod = 0.f;

        ImGui::Columns(1);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<sf::FloatRect> getAoEDragRect(const sf::Vector2f mousePos) const
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
        sf::Vector2f centroid;

        for (const Cat* cat : draggedCats)
            centroid += cat->position;

        centroid /= static_cast<float>(draggedCats.size());

        // Find the position closest to the centroid
        sf::base::SizeT closestIndex       = 0u;
        float           minDistanceSquared = FLT_MAX;

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

        return pt.absorbingWisdom || wizardCat->cooldown.value != 0.f || wizardCat->isHexedOrCopyHexed() ||
               isCatBeingDragged(*wizardCat);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* findFirstCatByType(const CatType catType)
    {
        for (Cat& cat : pt.cats)
            if (cat.type == catType)
                return &cat;

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Cat* findFirstCatByType(const CatType catType) const
    {
        for (const Cat& cat : pt.cats)
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
            xComboCountdown.value = pt.psvComboStartTime.currentValue() * 1000.f;
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

        sounds.cast0.setPosition({wizardCat.position.x, wizardCat.position.y});
        playSound(sounds.cast0);

        spawnParticlesNoGravity(256, wizardCat.position, ParticleType::Star, rngFast.getF(0.25f, 1.25f), rngFast.getF(0.5f, 3.f));

        forEachBubbleInRadius(wizardCat.position,
                              range,
                              [&](Bubble& bubble)
        {
            if (pt.perm.starpawConversionIgnoreBombs && bubble.type != BubbleType::Normal)
                return ControlFlow::Continue;

            if (rng.getF(0.f, 99.f) > pt.psvStarpawPercentage.currentValue())
                return ControlFlow::Continue;

            bubble.type   = pt.perm.starpawNova ? BubbleType::Nova : BubbleType::Star;
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

        sounds.cast0.setPosition({wizardCat.position.x, wizardCat.position.y});
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

        const bool castSuccessful = pt.dolls.empty() && witchCat != nullptr &&
                                    (witchCat->position - wizardCat.position).lengthSquared() <= range * range;

        if (castSuccessful)
        {
            sounds.cast0.setPosition({wizardCat.position.x, wizardCat.position.y});
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

            witchCat->cooldown.value -= witchCat->cooldown.value * (pt.psvDarkUnionPercentage.currentValue() / 100.f);
        }
        else
        {
            sounds.failcast.setPosition({wizardCat.position.x, wizardCat.position.y});
            playSound(sounds.failcast);
        }

        ++wizardCat.hits;
        wizardCat.cooldown.value = maxCooldown * 4.f;
    }

    ////////////////////////////////////////////////////////////
    void doWizardSpellStasisField(Cat& wizardCat)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(wizardCat.type);

        sounds.cast0.setPosition({wizardCat.position.x, wizardCat.position.y});
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

        const bool copyCatMustCast = copyCat != nullptr && pt.copycatCopiedCatType == CatType::Wizard;

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
            pt.mewltiplierAuraTimer += pt.perm.wizardCatDoubleMewltiplierDuration ? 12'000.f : 6000.f;

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
            pt.stasisFieldTimer += pt.perm.wizardCatDoubleStasisFieldDuration ? 12'000.f : 6000.f;

            doWizardSpellStasisField(*wizardCat);

            if (copyCatMustCast)
                doWizardSpellStasisField(*copyCat);

            return;
        }
    }

    ////////////////////////////////////////////////////////////
    void uiTabBarMagic()
    {
        uiSetFontScale(uiNormalFontScale);

        Cat* wizardCat = cachedWizardCat;

        if (wizardCat == nullptr)
        {
            ImGui::Text("The wizardcat is missing!");
            return;
        }

        Cat* copyCat = cachedCopyCat;

        ImGui::Spacing();
        ImGui::Spacing();

        imgsep(txrMagicSeparator0, "wisdom", /* first */ true);
        ImGui::Columns(1);

        ImGui::Text("Wisdom points: %s WP", toStringWithSeparators(pt.wisdom));

        uiCheckbox("Absorb wisdom from star bubbles", &pt.absorbingWisdom);
        std::sprintf(uiTooltipBuffer,
                     "The Wizardcat concentrates, absorbing wisdom points from nearby star bubbles. While the "
                     "Wizardcat is concentrating, it cannot cast spells nor be moved around.");
        uiMakeTooltip();

        uiBeginColumns();
        uiButtonHueMod = 45.f;

        std::sprintf(uiTooltipBuffer,
                     "The Wizardcat taps into memories of past lives, remembering a powerful spell.\n\nMana "
                     "costs:\n- "
                     "1st spell: 5 mana\n- 2nd spell: 20 mana\n- 3rd spell: 30 mana\n- 4th spell: 40 mana\n\nNote: "
                     "You "
                     "won't be able to cast a spell if the cost exceeds your maximum mana!");
        std::sprintf(uiLabelBuffer, "%zu/%zu", pt.psvSpellCount.nPurchases, pt.psvSpellCount.data->nMaxPurchases);
        (void)makePSVButtonExByCurrency("Remember spell",
                                        pt.psvSpellCount,
                                        1u,
                                        static_cast<MoneyType>(pt.psvSpellCount.nextCost()),
                                        pt.wisdom,
                                        "%s WP##%u");

        uiButtonHueMod = 0.f;

        imgsep(txrMagicSeparator1, "mana");
        ImGui::Columns(1);

        ImGui::Text("Mana: %llu / %llu", pt.mana, pt.getComputedMaxMana());

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Next mana:");

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(157, 0, 255, 128));
        ImGui::ProgressBar(pt.manaTimer / pt.getComputedManaCooldown());
        ImGui::PopStyleColor();

        ImGui::Text("Wizard cooldown: %.2fs", static_cast<double>(wizardCat->cooldown.value / 1000.f));

        imgsep(txrMagicSeparator2, "spells");

        if (pt.psvSpellCount.nPurchases == 0)
            ImGui::Text("No spells revealed yet...");

        ImGui::Columns(1);
        uiSetFontScale(0.8f);

        if (pt.psvSpellCount.nPurchases > 0)
        {
            if (pt.absorbingWisdom)
                uiCenteredText("Cannot cast spells while absorbing wisdom...");
            else if (wizardCat->isHexedOrCopyHexed())
                uiCenteredText("Cannot cast spells while hexed...");
            else if (wizardCat->cooldown.value > 0.f)
                uiCenteredText("Cannot cast spells while on cooldown...");
            else if (isCatBeingDragged(*wizardCat))
                uiCenteredText("Cannot cast spells while being dragged...");
            else
            {
                const bool anySpellCastable = pt.mana >= spellManaCostByIndex[0] && pt.psvSpellCount.nPurchases >= 1;

                if (anySpellCastable)
                    uiCenteredText("Ready to cast a spell!");
                else
                    uiCenteredText("Not enough mana to cast any spell...");
            }
        }

        ImGui::Separator();
        uiSetFontScale(uiNormalFontScale);

        uiBeginColumns();
        uiButtonHueMod = 45.f;

        //
        // SPELL 0
        if (checkUiUnlock(32u, pt.psvSpellCount.nPurchases >= 1))
        {
            uiSetUnlockLabelY(32u);
            std::sprintf(uiTooltipBuffer,
                         "Transforms a percentage of bubbles around the Wizardcat into star bubbles "
                         "immediately.\n\nCan be upgraded to ignore bombs with prestige points.");
            uiLabelBuffer[0] = '\0';
            bool done        = false;

            ImGui::BeginDisabled(isWizardBusy());
            if (makePurchasableButtonOneTimeByCurrency("Starpaw Conversion",
                                                       done,
                                                       spellManaCostByIndex[0],
                                                       pt.mana,
                                                       "%s mana##%u"))
            {
                castSpellByIndex(0u, wizardCat, copyCat);
                done = false;
            }
            ImGui::EndDisabled();

            const float currentPercentage = pt.psvStarpawPercentage.currentValue();
            const float nextPercentage    = pt.psvStarpawPercentage.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the percentage of bubbles converted into star bubbles from %.2f%% to %.2f%%.",
                         static_cast<double>(currentPercentage),
                         static_cast<double>(nextPercentage));
            std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentPercentage));
            (void)makePSVButtonExByCurrency("  higher percentage##starpawperc",
                                            pt.psvStarpawPercentage,
                                            1u,
                                            static_cast<MoneyType>(pt.psvStarpawPercentage.nextCost()),
                                            pt.wisdom,
                                            "%s WP##%u");
        }

        //
        // SPELL 1
        if (checkUiUnlock(33u, pt.psvSpellCount.nPurchases >= 2))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(33u);
            std::sprintf(uiTooltipBuffer,
                         "Creates a value multiplier aura around the Wizardcat that affects all cats and bubbles. "
                         "Lasts %d seconds.\n\nCasting this spell multiple times will accumulate the aura "
                         "duration.",
                         pt.perm.wizardCatDoubleMewltiplierDuration ? 12 : 6);
            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(pt.mewltiplierAuraTimer / 1000.f));
            bool done = false;

            ImGui::BeginDisabled(isWizardBusy());
            if (makePurchasableButtonOneTimeByCurrency("Mewltiplier Aura",
                                                       done,
                                                       spellManaCostByIndex[1],
                                                       pt.mana,
                                                       "%s mana##%u"))
            {
                castSpellByIndex(1u, wizardCat, copyCat);
                done = false;
            }
            ImGui::EndDisabled();

            const float currentMultiplier = pt.psvMewltiplierMult.currentValue();
            const float nextMultiplier    = pt.psvMewltiplierMult.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the multiplier applied while the aura is active from x%.2f to x%.2f.",
                         static_cast<double>(currentMultiplier),
                         static_cast<double>(nextMultiplier));
            std::sprintf(uiLabelBuffer, "x%.2f", static_cast<double>(currentMultiplier));
            (void)makePSVButtonExByCurrency("  higher multiplier",
                                            pt.psvMewltiplierMult,
                                            1u,
                                            static_cast<MoneyType>(pt.psvMewltiplierMult.nextCost()),
                                            pt.wisdom,
                                            "%s WP##%u");
        }

        //
        // SPELL 2
        if (checkUiUnlock(34u, pt.psvSpellCount.nPurchases >= 3))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(34u);
            std::sprintf(uiTooltipBuffer,
                         "The Wizardcat uses their magic to empower a nearby Witchcat, reducing their remaining "
                         "ritual cooldown.\n\nNote: This spell has no effect if there is no Witchcat "
                         "nearby, or if there are voodoo dolls left to collect.");
            uiLabelBuffer[0] = '\0';

            bool done = false;

            ImGui::BeginDisabled(isWizardBusy());
            if (makePurchasableButtonOneTimeByCurrency("Dark Union",
                                                       done,
                                                       spellManaCostByIndex[2],
                                                       pt.mana,
                                                       "%s mana##%u"))
            {
                castSpellByIndex(2u, wizardCat, copyCat);
                done = false;
            }
            ImGui::EndDisabled();

            const float currentPercentage = pt.psvDarkUnionPercentage.currentValue();
            const float nextPercentage    = pt.psvDarkUnionPercentage.nextValue();

            std::sprintf(uiTooltipBuffer,
                         "Increase the cooldown reduction percentage from %.2f%% to %.2f%%.",
                         static_cast<double>(currentPercentage),
                         static_cast<double>(nextPercentage));
            std::sprintf(uiLabelBuffer, "%.2f%%", static_cast<double>(currentPercentage));
            (void)makePSVButtonExByCurrency("  higher reduction##darkunionperc",
                                            pt.psvDarkUnionPercentage,
                                            1u,
                                            static_cast<MoneyType>(pt.psvDarkUnionPercentage.nextCost()),
                                            pt.wisdom,
                                            "%s WP##%u");
        }

        //
        // SPELL 3
        if (checkUiUnlock(35u, pt.psvSpellCount.nPurchases >= 4))
        {
            ImGui::Separator();

            uiSetUnlockLabelY(35u);
            std::sprintf(uiTooltipBuffer,
                         "The Wizardcat controls time itself, creating a stasis field for %d seconds. All bubbles "
                         "caught in the field become frozen in time, unable to move or be destroyed. However, they "
                         "can "
                         "still be popped, as many times as you want!\n\nCasting this spell multiple times will "
                         "accumulate the field duration.\n\nNote: This spell has no effect if there are no bubbles "
                         "nearby. Bombs are also affected by the stasis field.",
                         pt.perm.wizardCatDoubleStasisFieldDuration ? 12 : 6);
            std::sprintf(uiLabelBuffer, "%.2fs", static_cast<double>(pt.stasisFieldTimer / 1000.f));

            bool done = false;

            ImGui::BeginDisabled(isWizardBusy());
            if (makePurchasableButtonOneTimeByCurrency("Stasis Field",
                                                       done,
                                                       spellManaCostByIndex[3],
                                                       pt.mana,
                                                       "%s mana##%u"))
            {
                castSpellByIndex(3u, wizardCat, copyCat);
                done = false;
            }
            ImGui::EndDisabled();
        }

        uiButtonHueMod = 0.f;
        ImGui::Columns(1);

        if (pt.psvSpellCount.nPurchases > 0 && pt.perm.autocastPurchased)
        {
            imgsep(txrMagicSeparator3, "autocast");

            ImGui::Columns(1);
            uiButtonHueMod = 45.f;

            constexpr const char* entries[]{
                "None",
                "Starpaw Conversion",
                "Mewltiplier Aura",
                "Dark Union",
                "Stasis Field",
            };

            if (ImGui::BeginCombo("Spell##autocastspell", entries[pt.perm.autocastIndex]))
            {
                for (SizeT i = 0u; i < pt.psvSpellCount.nPurchases + 1; ++i)
                {
                    const bool isSelected = pt.perm.autocastIndex == i;
                    if (ImGui::Selectable(entries[i], isSelected))
                    {
                        pt.perm.autocastIndex = i;
                        playSound(sounds.uitab);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            uiButtonHueMod = 0.f;
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
    void uiTabBarStats()
    {
        const auto displayStats = [&](const Stats& stats)
        {
            ImGui::Spacing();
            ImGui::Spacing();

            const auto [h, m, s] = formatTime(stats.secondsPlayed);
            ImGui::Text("Time played: %lluh %llum %llus", h, m, s);

            ImGui::Spacing();
            ImGui::Spacing();

            const auto bubblesPopped            = stats.getTotalNBubblesPopped();
            const auto bubblesHandPopped        = stats.getTotalNBubblesHandPopped();
            const auto bubblesPoppedRevenue     = stats.getTotalRevenue();
            const auto bubblesHandPoppedRevenue = stats.getTotalRevenueHand();

            ImGui::Text("Bubbles popped: %s", toStringWithSeparators(bubblesPopped));
            ImGui::Indent();
            ImGui::Text("Clicked: %s", toStringWithSeparators(bubblesHandPopped));
            ImGui::Text("By cats: %s", toStringWithSeparators(bubblesPopped - bubblesHandPopped));
            ImGui::Unindent();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Highest $/s: %s", toStringWithSeparators(stats.highestDPS));

            ImGui::Text("Revenue: $%s", toStringWithSeparators(bubblesPoppedRevenue));
            ImGui::Indent();
            ImGui::Text("Clicked: $%s", toStringWithSeparators(bubblesHandPoppedRevenue));
            ImGui::Text("By cats: $%s", toStringWithSeparators(bubblesPoppedRevenue - bubblesHandPoppedRevenue));
            ImGui::Indent();
            ImGui::Text("Bombs:  $%s", toStringWithSeparators(stats.explosionRevenue));
            ImGui::Text("Flights: $%s", toStringWithSeparators(stats.flightRevenue));
            ImGui::Text("Portals: $%s", toStringWithSeparators(stats.hellPortalRevenue));
            ImGui::Unindent();
            ImGui::Unindent();
        };

        if (ImGui::BeginTabBar("TabBarStats", ImGuiTabBarFlags_DrawSelectedOverline))
        {
            static int lastSelectedTabIdx = 0;

            const auto selectedTab = [&](int idx)
            {
                if (lastSelectedTabIdx != idx)
                    playSound(sounds.uitab);

                lastSelectedTabIdx = idx;
            };

            uiSetFontScale(0.75f);
            if (ImGui::BeginTabItem(" Tips "))
            {
                selectedTab(0);

                ImGui::BeginChild("TipsScroll", ImVec2(ImGui::GetContentRegionAvail().x, uiGetMaxWindowHeight() - 125.f));

                const auto addTip = [&](const char* title, const char* description)
                {
                    uiSetFontScale(uiNormalFontScale * 2.0f);
                    uiCenteredText(title);

                    ImGui::PushFont(fontImGuiMouldyCheese);
                    uiSetFontScale(uiNormalFontScale);
                    ImGui::TextWrapped("%s", description);
                    ImGui::PopFont();

                    ImGui::Separator();
                };

                addTip("Getting Started",
                       "Click on bubbles to pop them and earn money.\n\nPurchase upgrades and cats to increase "
                       "your "
                       "income and automate your bubble popping journey.");

                if (pt.comboPurchased)
                    addTip("Combos",
                           "Popping bubbles in quick succession will increase your combo multiplier, boosting your "
                           "revenue. Keep the combo going for maximum profit!\n\nPopping high-value bubbles such "
                           "as "
                           "star bubbles while your combo multiplier is high will yield even more revenue.");

                if (pt.getCatCountByType(CatType::Normal) > 0)
                {
                    addTip("Regular Cats",
                           "Regular cats will automatically pop bubbles for you, even while you are away or the "
                           "game "
                           "is in the background.\n\nThey are the bread and butter of any cat formation!");

                    addTip("Cat Dragging",
                           "You can drag cats around the screen to reposition them.\n\nMoving individual cats can "
                           "be "
                           "done by clicking and dragging them.\n\nMultiple cats can be moved at once by holding "
                           "down "
                           "left shift and dragging a selection box around them. After that, either release left "
                           "shift "
                           "or the mouse button and drag them to their intended position. This is a great way to "
                           "move "
                           "an entire formation of cats at once.");
                }

                if (pt.getCatCountByType(CatType::Uni) > 0)
                    addTip("Unicats",
                           "Unicats will convert normal bubbles into star bubbles, which are worth x15 the value "
                           "of "
                           "normal bubbles.\n\nPopping star bubbles manually while your combo multiplier is high "
                           "(towards the end of a combo) is a great way of making money early.\n\nAlternatively, "
                           "you "
                           "can place regular cats under unicats to have them pop the star bubbles for you.");

                if (pt.mapPurchased)
                {
                    addTip("Map Exploration",
                           "Expand the map to discover shrines containing powerful unique cats and to have more "
                           "real estate for your cat army.\n\nYou can scroll the map with the scroll wheel, "
                           "holding "
                           "right click, by dragging with two fingers, by using the A/D/Left/Right keys.\n\nYou "
                           "can "
                           "jump around the map by clicking on the minimap or using the PgUp/PgDn/Home/End keys.");

                    addTip("Shrines",
                           "Shrines contain powerful cats with unique powers and synergies.\n\nIn order to unseal "
                           "the "
                           "cats, the shrine must first be activated by purchasing \"Activate next shrine\" in the "
                           "shop.\n\nAfterwards, the shrine must be completed by popping bubbles in its range "
                           "until "
                           "the required amount of revenue is collected.");
                }

                if (pt.psvBubbleValue.nPurchases > 0 ||
                    (pt.getCatCountByType(CatType::Uni) >= 3 && pt.nShrinesCompleted > 0))
                    addTip("Prestige",
                           "Prestige to reset your current progress, permanently increasing the value of bubbles "
                           "and "
                           "unlocking powerful permanent upgrades that persist between prestiges.\n\nDo not be "
                           "afraid "
                           "of prestiging, as its benefits will allow you to return to your current state very "
                           "quickly "
                           "and progress much further than it was possible before!");

                if (pt.getCatCountByType(CatType::Devil) > 0)
                    addTip("Devilcats",
                           "Devilcats will convert normal bubbles into bombs, which explode when popped. Bubbles "
                           "caught in the explosion are worth x10 their original value. This means that every star "
                           "bubble caught in the explosion will be worth x150 the value of a normal "
                           "bubble.\n\nPosition regular cats beneath Devilcats to automatically pop bombs, and "
                           "Unicats "
                           "nearby to maximize the chance of having star bubbles caught in the explosion.");

                if (pt.getCatCountByType(CatType::Astro) > 0)
                    addTip("Astrocats",
                           "Astrocats will periodically fly across the map, looping around when they reach the "
                           "edge of "
                           "the screen.\n\nAny bubble they touch while flying will be popped with a x20 "
                           "multiplier.\n\nUpgrading the total bubble count and expanding the map will indirectly "
                           "increase the effectiveness of Astrocats.");

                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
                    addTip("Witchcat",
                           "The Witchcat periodically perform voodoo rituals.\n\nDuring a ritual, a random cat in "
                           "range of the Witchcat will be hexed and will become inactive until the ritual "
                           "ends.\n\nAt "
                           "the same time, voodoo dolls will appear throughout the map -- collect all of them to "
                           "end "
                           "the ritual and gain a powerful timed buff depending on the type of cat that was "
                           "hexed.");


                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
                    addTip("Wizardcat",
                           "The Wizardcat casts powerful spells using mana that regenerates over time.\n\nIn order "
                           "to "
                           "learn new spells, the Wizardcat must concentrate and absorb wisdom from star bubbles, "
                           "earning \"wisdom points\".\n\nCasting spells or changing the Wizardcat's state can be "
                           "done "
                           "in the \"Magic\" menu. ");

                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
                    addTip("Mousecat",
                           "The Mousecat pops nearby bubbles keeping up its own personal combo.\n\nCombo/click "
                           "upgrades you purchased also apply to the Mousecat.\n\nRegular cats in range of the "
                           "Mousecat will gain the same combo multiplier as the Mousecat.\n\nFurthermore, the "
                           "Mousecat "
                           "provides a global click revenue value buff.");


                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
                    addTip("Engicat",
                           "The Engicat periodically increases the speed of nearby cats, effectively decreasing "
                           "their "
                           "cooldown.\n\nFurthermore, the Mousecat provides a global cat revenue value buff.");

                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
                    addTip("Repulsocat",
                           "The Repulsocat blows nearby bubbles away.\n\nRecently blown bubbles are worth x2 their "
                           "value.");

                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
                    addTip("Attractocat",
                           "The Attractocat attracts nearby bubbles.\n\nBubbles in range of the Attractocat are "
                           "worth "
                           "x2 their value.");

                if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
                    addTip("Copycat",
                           "The Copycat can mimic the abilities, effects, and properties of any other unique "
                           "cat.\n\nThe mimicked cat can be chosen through the disguise menu near the bottom of "
                           "the "
                           "screen.");

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            uiSetFontScale(0.75f);
            if (ImGui::BeginTabItem(" Statistics "))
            {
                selectedTab(1);

                uiSetFontScale(1.f);
                uiCenteredText(" ~~ Lifetime ~~ ");

                uiSetFontScale(0.75f);
                displayStats(profile.statsLifetime);

                ImGui::Separator();

                uiSetFontScale(1.f);
                uiCenteredText(" ~~ This playthrough ~~ ");

                uiSetFontScale(0.75f);
                displayStats(pt.statsTotal);

                ImGui::Separator();

                uiSetFontScale(1.f);
                uiCenteredText(" ~~ This prestige ~~ ");

                uiSetFontScale(0.75f);
                displayStats(pt.statsSession);

                ImGui::Spacing();
                ImGui::Spacing();

                // per cat type
                MoneyType revenueByCatType[nCatTypes] = {};

                for (const Cat& cat : pt.cats)
                    revenueByCatType[asIdx(cat.type)] += cat.moneyEarned;

                ImGui::Text("Cat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Normal)]));
                ImGui::Text("Devilcat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Devil)]));

                ImGui::Text("Astrocat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Astro)]));
                ImGui::Text("Mousecat revenue: $%s", toStringWithSeparators(revenueByCatType[asIdx(CatType::Mouse)]));

                uiSetFontScale(uiNormalFontScale);

                ImGui::EndTabItem();
            }

            uiSetFontScale(0.75f);
            if (ImGui::BeginTabItem(" Milestones "))
            {
                selectedTab(2);

                uiSetFontScale(0.75f);

                const auto doMilestone = [&](const char* name, const MilestoneTimestamp value)
                {
                    if (value == maxMilestone)
                    {
                        ImGui::Text("%s: N/A", name);
                        return;
                    }

                    const auto [h, m, s] = formatTime(value);
                    ImGui::Text("%s: %lluh %llum %llus", name, h, m, s);
                };

                doMilestone("1st Cat", pt.milestones.firstCat);
                doMilestone("5th Cat", pt.milestones.fiveCats);
                doMilestone("10th Cat", pt.milestones.tenCats);

                ImGui::Separator();

                doMilestone("1st Unicat", pt.milestones.firstUnicat);
                doMilestone("5th Unicat", pt.milestones.fiveUnicats);
                doMilestone("10th Unicat", pt.milestones.tenUnicats);

                ImGui::Separator();

                doMilestone("1st Devilcat", pt.milestones.firstDevilcat);
                doMilestone("5th Devilcat", pt.milestones.fiveDevilcats);
                doMilestone("10th Devilcat", pt.milestones.tenDevilcats);

                ImGui::Separator();

                doMilestone("1st Astrocat", pt.milestones.firstAstrocat);
                doMilestone("5th Astrocat", pt.milestones.fiveAstrocats);
                doMilestone("10th Astrocat", pt.milestones.tenAstrocats);

                ImGui::Separator();

                doMilestone("Prestige Level 2", pt.milestones.prestigeLevel2);
                doMilestone("Prestige Level 3", pt.milestones.prestigeLevel3);
                doMilestone("Prestige Level 4", pt.milestones.prestigeLevel4);
                doMilestone("Prestige Level 5", pt.milestones.prestigeLevel5);
                doMilestone("Prestige Level 6", pt.milestones.prestigeLevel6);
                doMilestone("Prestige Level 10", pt.milestones.prestigeLevel10);
                doMilestone("Prestige Level 15", pt.milestones.prestigeLevel15);
                doMilestone("Prestige Level 20", pt.milestones.prestigeLevel20);

                ImGui::Separator();

                doMilestone("$10.000 Revenue", pt.milestones.revenue10000);
                doMilestone("$100.000 Revenue", pt.milestones.revenue100000);
                doMilestone("$1.000.000 Revenue", pt.milestones.revenue1000000);
                doMilestone("$10.000.000 Revenue", pt.milestones.revenue10000000);
                doMilestone("$100.000.000 Revenue", pt.milestones.revenue100000000);
                doMilestone("$1.000.000.000 Revenue", pt.milestones.revenue1000000000);

                ImGui::Separator();

                for (SizeT i = 0u; i < nShrineTypes; ++i)
                    doMilestone(shrineNames[i], pt.milestones.shrineCompletions[i]);

                ImGui::EndTabItem();
            }

            uiSetFontScale(0.75f);
            if (ImGui::BeginTabItem(" Achievements "))
            {
                selectedTab(3);

                const sf::base::SizeT nAchievementsUnlocked = sf::base::count(profile.unlockedAchievements,
                                                                              profile.unlockedAchievements + nAchievements);

                uiSetFontScale(uiNormalFontScale);
                ImGui::Text("%zu / %zu achievements unlocked", nAchievementsUnlocked, sf::base::getArraySize(achievementData));

                static bool showCompleted = true;
                uiSetFontScale(0.75f);
                uiCheckbox("Show completed", &showCompleted);

                ImGui::Separator();
                uiSetFontScale(0.75f);

                ImGui::BeginChild("AchScroll", ImVec2(ImGui::GetContentRegionAvail().x, uiGetMaxWindowHeight() - 125.f));

                sf::base::U64 id = 0u;
                for (const auto& [name, description, secret] : achievementData)
                {
                    const bool unlocked = profile.unlockedAchievements[id];

                    if (!showCompleted && unlocked)
                    {
                        ++id;
                        continue;
                    }

                    const float opacity = unlocked ? 1.f : 0.5f;

                    const ImVec4 textColor{1.f, 1.f, 1.f, opacity};

                    uiSetFontScale(uiNormalFontScale);
                    ImGui::TextColored(textColor, "%llu - %s", id++, (!secret || unlocked) ? name : "???");

                    ImGui::PushFont(fontImGuiMouldyCheese);
                    uiSetFontScale(0.75f);
                    ImGui::TextColored(textColor, "%s", (!secret || unlocked) ? description : "(...secret achievement...)");
                    ImGui::PopFont();

                    ImGui::Separator();
                }

                if (debugMode)
                {
                    uiButtonHueMod = 120.f;
                    uiPushButtonColors();

                    uiSetFontScale(uiNormalFontScale);
                    if (ImGui::Button("Reset stats and achievements"))
                    {
                        withAllStats([](Stats& stats) { stats = {}; });

                        for (bool& b : profile.unlockedAchievements)
                            b = false;
                    }

                    uiPopButtonColors();
                    uiButtonHueMod = 0.f;
                }

                ImGui::EndChild();

                ImGui::EndTabItem();
            }

            uiSetFontScale(uiNormalFontScale);
            ImGui::EndTabBar();
        }

        uiSetFontScale(uiNormalFontScale);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool mustApplyMewltiplierAura(const sf::Vector2f bubblePosition) const
    {
        if (pt.mewltiplierAuraTimer <= 0.f)
            return false;

        const Cat* wizardCat = cachedWizardCat;
        if (wizardCat == nullptr)
            return false;

        const float wizardCatRangeSquared = pt.getComputedSquaredRangeByCatType(CatType::Wizard);

        if ((wizardCat->position - bubblePosition).lengthSquared() <= wizardCatRangeSquared)
            return true;

        const Cat* copyCat = cachedCopyCat;
        if (copyCat == nullptr || pt.copycatCopiedCatType != CatType::Wizard)
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
                               (popperCat->type == CatType::Copy && pt.copycatCopiedCatType == CatType::Mouse));

        const bool popperCatIsNormal = !byPlayerClick && popperCat->type == CatType::Normal;

        const bool mustApplyHandMult = byPlayerClick || popperCatIsMousecat; // mousecat benefits from click and cat mults
        const bool mustApplyCatMult = !byPlayerClick;

        const bool nearShrineOfClicking = byPlayerClick && ([&]
        {
            for (const Shrine& shrine : pt.shrines)
                if (shrine.type == ShrineType::Clicking && shrine.isInRange(bubble.position))
                    return true;

            return false;
        })();

        // Base reward: bubble value by type multiplied by static multiplier (e.g. x10 for bombs, x20 for astro)
        float result = static_cast<float>(pt.getComputedRewardByBubbleType(bubble.type)) * multiplier;

        // Combo mult: applied for player clicks or mousecat clicks
        result *= comboMult;

        // Wizard spells: mewltiplier aura
        if (mustApplyMewltiplierAura(bubble.position))
            result *= pt.psvMewltiplierMult.currentValue();

        // Global bonus -- mousecat (applies to clicks)
        const bool isMouseBeingCopied = cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Mouse;
        if (mustApplyHandMult && cachedMouseCat != nullptr)
            result *= pt.psvPPMouseCatGlobalBonusMult.currentValue() * (isMouseBeingCopied ? 2.f : 1.f);

        // Global bonus -- engicat (applies to cats)
        const bool isEngiBeingCopied = cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Engi;
        if (mustApplyCatMult && cachedEngiCat != nullptr)
            result *= pt.psvPPEngiCatGlobalBonusMult.currentValue() * (isEngiBeingCopied ? 2.f : 1.f);

        // Shrine of clicking: x5 reward for clicks
        if (mustApplyHandMult && nearShrineOfClicking)
            result *= 5.f;

        // Ritual buff -- normalcat: x5 reward for cats
        if (mustApplyCatMult && pt.buffCountdownsPerType[asIdx(CatType::Normal)].value > 0.f)
            result *= 5.f;

        // Ritual buff -- mousecat: x10 reward for clicks
        if (mustApplyHandMult && pt.buffCountdownsPerType[asIdx(CatType::Mouse)].value > 0.f)
            result *= 10.f;

        // Genius cats: x2 reward for normal cats only
        if (!byPlayerClick && popperCatIsNormal && pt.perm.geniusCatsPurchased)
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
    [[nodiscard]] static sf::Vector2u getReasonableWindowSize(const float scalingFactorMult = 1.f)
    {
        constexpr float gameRatio = gameScreenSize.x / gameScreenSize.y;

        const auto  fullscreenSize = sf::VideoModeUtils::getDesktopMode().size.toVector2f();
        const float aspectRatio    = fullscreenSize.x / fullscreenSize.y;

        const bool isUltrawide = aspectRatio >= 2.f;
        const bool isWide      = aspectRatio >= 1.6f && aspectRatio < 2.f;

        const float scalingFactor = isUltrawide ? 0.9f : isWide ? 0.8f : 0.7f;

        const auto windowSize = fullscreenSize * scalingFactor * scalingFactorMult;

        const auto windowedWidth = windowSize.y * gameRatio + (uiWindowWidth + 35.f);

        return sf::Vector2f{windowedWidth, windowSize.y}.toVector2u();
    }

    ////////////////////////////////////////////////////////////
    struct SelectorEntry
    {
        int         index;
        const char* name;
    };

    ////////////////////////////////////////////////////////////
    [[nodiscard]] int pickSelectedIndex(const std::vector<SelectorEntry>& entries, const int selectedIndex)
    {
        const auto selectedIndexU = static_cast<sf::base::SizeT>(selectedIndex);
        return selectedIndexU < entries.size() ? entries[selectedIndexU].index : 0;
    }

    ////////////////////////////////////////////////////////////
    void selectBackground(const std::vector<SelectorEntry>& entries, const int selectedIndex)
    {
        profile.selectedBackground = pickSelectedIndex(entries, selectedIndex);
    }

    ////////////////////////////////////////////////////////////
    void selectBGM(const std::vector<SelectorEntry>& entries, const int selectedIndex)
    {
        profile.selectedBGM = pickSelectedIndex(entries, selectedIndex);
    }

    ////////////////////////////////////////////////////////////
    void updateSelectedBackgroundSelectorIndex()
    {
        auto& [entries, selectedIndex] = getBackgroundSelectorData();

        for (sf::base::SizeT i = 0u; i < entries.size(); ++i)
            if (profile.selectedBackground == entries[i].index)
                selectedIndex = static_cast<int>(i);
    }

    ////////////////////////////////////////////////////////////
    void updateSelectedBGMSelectorIndex()
    {
        auto& [entries, selectedIndex] = getBGMSelectorData();

        for (sf::base::SizeT i = 0u; i < entries.size(); ++i)
            if (profile.selectedBGM == entries[i].index)
                selectedIndex = static_cast<int>(i);
    }

    ////////////////////////////////////////////////////////////
    struct SelectorData
    {
        std::vector<SelectorEntry> entries;
        int                        selectedIndex = -1;
    };

    ////////////////////////////////////////////////////////////
    SelectorData& getBGMSelectorData()
    {
        static thread_local SelectorData data;
        data.entries.clear();

        data.entries.emplace_back(0, "Default");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
            data.entries.emplace_back(1, "Ritual Circle");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
            data.entries.emplace_back(2, "The Wise One");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
            data.entries.emplace_back(3, "Click N Chill");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
            data.entries.emplace_back(4, "Use More Cat");

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
    SelectorData& getBackgroundSelectorData()
    {
        static thread_local SelectorData data;
        data.entries.clear();

        data.entries.emplace_back(0, "Default");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
            data.entries.emplace_back(1, "Swamp");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
            data.entries.emplace_back(2, "Observatory");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
            data.entries.emplace_back(3, "Aim Labs");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
            data.entries.emplace_back(4, "Factory");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
            data.entries.emplace_back(5, "Wind Tunnel");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
            data.entries.emplace_back(6, "Magnetosphere");

        if (pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
            data.entries.emplace_back(7, "Auditorium");

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
    void forceResetGame()
    {
        sounds.stopPlayingAll(sounds.ritual);
        sounds.stopPlayingAll(sounds.copyritual);

        rng.reseed(std::random_device{}());
        shuffledCatNamesPerType = makeShuffledCatNames(rng);

        pt      = Playthrough{};
        pt.seed = rng.getSeed();

        wasPrestigeAvailableLastFrame = false;
        buyReminder                   = 0u;

        resetAllDraggedCats();

        particles.clear();
        spentCoinParticles.clear();
        hudTopParticles.clear();
        hudBottomParticles.clear();
        textParticles.clear();
        earnedCoinParticles.clear();

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
    void uiTabBarSettings()
    {
        bool sgActive = false;
        SFML_BASE_SCOPE_GUARD({
            if (sgActive)
                ImGui::EndTabBar();
        });
        sgActive = ImGui::BeginTabBar("TabBarSettings", ImGuiTabBarFlags_DrawSelectedOverline);

        static int lastSelectedTabIdx = 0;

        const auto selectedTab = [&](int idx)
        {
            if (lastSelectedTabIdx != idx)
                playSound(sounds.uitab);

            lastSelectedTabIdx = idx;
        };

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Audio "))
        {
            selectedTab(0);

            uiSetFontScale(uiNormalFontScale);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Master volume", &profile.masterVolume, 0.f, 100.f, "%.f%%");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            if (ImGui::SliderFloat("SFX volume", &profile.sfxVolume, 0.f, 100.f, "%.f%%"))
                sounds.setupSounds(/* volumeOnly */ true, profile.sfxVolume / 100.f);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Music volume", &profile.musicVolume, 0.f, 100.f, "%.f%%");

            uiCheckbox("Play audio in background", &profile.playAudioInBackground);
            uiCheckbox("Enable combo scratch sound", &profile.playComboEndSound);
            uiCheckbox("Enable ritual sounds", &profile.playWitchRitualSounds);

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Interface "))
        {
            selectedTab(1);

            uiSetFontScale(uiNormalFontScale);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Minimap Scale", &profile.minimapScale, 5.f, 40.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("HUD Scale", &profile.hudScale, 0.5f, 2.f, "%.2f");

            ImGui::Separator();

            ImGui::AlignTextToFramePadding();
            ImGui::Text("UI Scale");

            const auto makeUIScaleButton = [&](const char* label, const float scaleFactor)
            {
                ImGui::SameLine();
                if (ImGui::Button(label, ImVec2{46.f * profile.uiScale, 0.f}))
                {
                    playSound(sounds.buy);
                    profile.uiScale = scaleFactor;
                }
            };

            makeUIScaleButton("XXL", 1.75f);
            makeUIScaleButton("XL", 1.5f);
            makeUIScaleButton("L", 1.25f);
            makeUIScaleButton("M", 1.f);
            makeUIScaleButton("S", 0.75f);
            makeUIScaleButton("XS", 0.5f);

            ImGui::Separator();

            uiCheckbox("Enable tips", &profile.tipsEnabled);

            ImGui::Separator();

            uiCheckbox("Enable notifications", &profile.enableNotifications);

            ImGui::BeginDisabled(!profile.enableNotifications);
            uiCheckbox("Enable full mana notification", &profile.showFullManaNotification);
            ImGui::EndDisabled();

            ImGui::Separator();

            uiCheckbox("Enable $/s meter", &profile.showDpsMeter);

            ImGui::Separator();

            constexpr const char* trailModeNames[3]{"Combo only", "Always", "Never"};

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::Combo("Cursor trail mode", &profile.cursorTrailMode, trailModeNames, 3);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Cursor trail scale", &profile.cursorTrailScale, 0.25f, 5.f, "%.2f");

            ImGui::Separator();

            uiCheckbox("High-visibility cursor", &profile.highVisibilityCursor);

            ImGui::BeginDisabled(!profile.highVisibilityCursor);
            {
                uiSetFontScale(0.75f);

                uiCheckbox("Multicolor", &profile.multicolorCursor);

                ImGui::BeginDisabled(profile.multicolorCursor);
                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Hue", &profile.cursorHue, 0.f, 360.f, "%.2f");
                ImGui::EndDisabled();

                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Scale", &profile.cursorScale, 0.3f, 1.5f, "%.2f");

                uiSetFontScale(uiNormalFontScale);
            }
            ImGui::EndDisabled();

            ImGui::Separator();

            uiCheckbox("Accumulating combo effect", &profile.accumulatingCombo);
            uiCheckbox("Show cursor combo text", &profile.showCursorComboText);
            uiCheckbox("Show cursor combo bar", &profile.showCursorComboBar);

            ImGui::Separator();

            uiCheckbox("Invert mouse buttons", &profile.invertMouseButtons);

            ImGui::Separator();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Cat drag timer", &profile.catDragPressDuration, 50.f, 500.f, "%.2fms");

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Graphics "))
        {
            selectedTab(2);

            uiSetFontScale(uiNormalFontScale);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Background Opacity", &profile.backgroundOpacity, 0.f, 100.f, "%.f%%");

            uiCheckbox("Always show drawings", &profile.alwaysShowDrawings);

            ImGui::Separator();

            uiCheckbox("Show cat range", &profile.showCatRange);
            uiCheckbox("Show cat text", &profile.showCatText);
            uiCheckbox("Enable cat bobbing", &profile.enableCatBobbing);

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Cat range thickness", &profile.catRangeOutlineThickness, 1.f, 4.f, "%.2fpx");

            ImGui::Separator();

            uiCheckbox("Show particles", &profile.showParticles);

            ImGui::BeginDisabled(!profile.showParticles);
            uiCheckbox("Show coin particles", &profile.showCoinParticles);
            ImGui::EndDisabled();

            uiCheckbox("Show text particles", &profile.showTextParticles);

            ImGui::Separator();

            uiCheckbox("Enable screen shake", &profile.enableScreenShake);

            ImGui::Separator();

            uiCheckbox("Show bubbles", &profile.showBubbles);

            ImGui::Separator();

            uiCheckbox("Show doll particle border", &profile.showDollParticleBorder);

            ImGui::Separator();

            uiCheckbox("Bubble shader", &profile.useBubbleShader);

            ImGui::BeginDisabled(!profile.useBubbleShader);
            {
                uiSetFontScale(0.75f);

                if (isDebugModeEnabled())
                {
                    ImGui::SetNextItemWidth(210.f * profile.uiScale);
                    ImGui::SliderFloat("Iridescence", &profile.bsIridescenceStrength, 0.f, 1.f, "%.2f");

                    ImGui::SetNextItemWidth(210.f * profile.uiScale);
                    ImGui::SliderFloat("Edge Factor Min", &profile.bsEdgeFactorMin, 0.f, 1.f, "%.2f");

                    ImGui::SetNextItemWidth(210.f * profile.uiScale);
                    ImGui::SliderFloat("Edge Factor Max", &profile.bsEdgeFactorMax, 0.f, 1.f, "%.2f");

                    ImGui::SetNextItemWidth(210.f * profile.uiScale);
                    ImGui::SliderFloat("Edge Factor Strength", &profile.bsEdgeFactorStrength, 0.f, 10.f, "%.2f");

                    ImGui::SetNextItemWidth(210.f * profile.uiScale);
                    ImGui::SliderFloat("Distortion Strength", &profile.bsDistortionStrength, 0.f, 1.f, "%.2f");

                    ImGui::SetNextItemWidth(210.f * profile.uiScale);
                    ImGui::SliderFloat("Lens Distortion", &profile.bsLensDistortion, 0.f, 10.f, "%.2f");
                }

                ImGui::BeginDisabled(!profile.useBubbleShader);
                ImGui::SetNextItemWidth(210.f * profile.uiScale);
                ImGui::SliderFloat("Bubble Lightness", &profile.bsBubbleLightness, -1.f, 1.f, "%.2f");
                ImGui::EndDisabled();

                uiSetFontScale(uiNormalFontScale);
            }
            ImGui::EndDisabled();

            ImGui::Separator();

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Vibrance", &profile.ppSVibrance, 0.f, 2.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Saturation", &profile.ppSSaturation, 0.f, 2.f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Lightness", &profile.ppSLightness, 0.5f, 1.5f, "%.2f");

            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            ImGui::SliderFloat("Sharpness", &profile.ppSSharpness, 0.f, 1.f, "%.2f");

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Display "))
        {
            selectedTab(3);

            uiSetFontScale(uiNormalFontScale);

            ImGui::Text("Auto resolution");

            uiSetFontScale(0.85f);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Windowed");

            ImGui::SameLine();

            if (ImGui::Button("Large"))
            {
                playSound(sounds.buy);

                profile.resWidth = getReasonableWindowSize(1.f);
                profile.windowed = true;

                mustRecreateWindow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Medium"))
            {
                playSound(sounds.buy);

                profile.resWidth = getReasonableWindowSize(0.9f);
                profile.windowed = true;

                mustRecreateWindow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Small"))
            {
                playSound(sounds.buy);

                profile.resWidth = getReasonableWindowSize(0.8f);
                profile.windowed = true;

                mustRecreateWindow = true;
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Fullscreen");

            ImGui::SameLine();

            if (ImGui::Button("Borderless"))
            {
                playSound(sounds.buy);

                profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
                profile.windowed = true;

                mustRecreateWindow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Exclusive"))
            {
                playSound(sounds.buy);

                profile.resWidth = sf::VideoModeUtils::getDesktopMode().size;
                profile.windowed = false;

                mustRecreateWindow = true;
            }

            ImGui::Separator();

            if (uiCheckbox("VSync", &profile.vsync))
                optWindow->setVerticalSyncEnabled(profile.vsync);

            static auto fpsLimit = static_cast<float>(profile.frametimeLimit);
            ImGui::SetNextItemWidth(210.f * profile.uiScale);
            if (ImGui::DragFloat("FPS Limit", &fpsLimit, 1.f, 60.f, 144.f, "%.f", ImGuiSliderFlags_AlwaysClamp))
            {
                profile.frametimeLimit = static_cast<unsigned int>(fpsLimit);
                optWindow->setFramerateLimit(profile.frametimeLimit);
            }

            uiSetFontScale(uiNormalFontScale);

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (ImGui::BeginTabItem(" Data "))
        {
            selectedTab(4);

            uiSetFontScale(uiNormalFontScale);

            ImGui::Text("!!! Danger Zone !!!");

            uiSetFontScale(0.75f);
            ImGui::Text("Be careful with these dangerous settings!\nYour progress might be lost forever!");
            uiSetFontScale(uiNormalFontScale);

            static bool dangerZone = false;
            uiCheckbox("Enable Danger Zone (!)", &dangerZone);

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::BeginDisabled(!dangerZone);

            uiButtonHueMod = 120.f;
            uiPushButtonColors();

            if (ImGui::Button("Reset *current* prestige##dangerzoneforceprestige"))
            {
                dangerZone = false;
                beginPrestigeTransition(0u);
            }

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::Button("Reset game##dangerzoneresetgame"))
            {
                dangerZone = false;
                forceResetGame();
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset profile##dangerzoneresetprofile"))
            {
                dangerZone = false;
                forceResetProfile();
            }

            uiPopButtonColors();
            uiButtonHueMod = 0.f;

            ImGui::EndDisabled();

            ImGui::EndTabItem();
        }

        uiSetFontScale(0.75f);
        if (isDebugModeEnabled() && ImGui::BeginTabItem(" Debug "))
        {
            selectedTab(5);

            if (ImGui::Button("Slide"))
            {
                fixedBgSlideTarget += 1.f;

                if (fixedBgSlideTarget >= 3.f)
                    fixedBgSlideTarget = 0.f;
            }

            ImGui::Separator();

            if (ImGui::Button("Save game"))
                savePlaythroughToFile(pt);

            ImGui::SameLine();

            if (ImGui::Button("Load game"))
                loadPlaythroughFromFileAndReseed();

            ImGui::SameLine();

            uiButtonHueMod = 120.f;
            uiPushButtonColors();

            if (ImGui::Button("Reset game"))
                forceResetGame();

            ImGui::SameLine();

            if (ImGui::Button("Reset profile"))
                forceResetProfile();

            uiPopButtonColors();
            uiButtonHueMod = 0.f;

            ImGui::Separator();

            static int catTypeN = 0;
            ImGui::SetNextItemWidth(320.f * profile.uiScale);
            ImGui::Combo("typeN", &catTypeN, CatConstants::typeNames, nCatTypes);

            if (ImGui::Button("Spawn"))
            {
                const auto catType = static_cast<CatType>(catTypeN);

                if (isUniqueCatType(catType))
                {
                    const auto pos = getWindow().mapPixelToCoords((getResolution() / 2.f).toVector2i(), gameView);
                    spawnSpecialCat(pos, catType);
                }
                else
                    spawnCatCentered(catType, getHueByCatType(catType));
            }

            ImGui::SameLine();

            if (ImGui::Button("Do Ritual"))
                if (auto* wc = cachedWitchCat)
                    wc->cooldown.value = 10.f;

            ImGui::SameLine();

            if (ImGui::Button("Do Copy Ritual"))
                if (auto* wc = cachedCopyCat)
                    wc->cooldown.value = 10.f;

            ImGui::SameLine();

            if (ImGui::Button("Do Letter"))
            {
                victoryTC.emplace(TargetedCountdown{.startingValue = 6500.f});
                victoryTC->restart();
                delayedActions.emplace_back(Countdown{.value = 7000.f}, [this] { playSound(sounds.letterchime); });
            }

            if (ImGui::Button("Do Tip"))
                doTip("Hello, I am a tip!\nHello world... How are you doing today?\nTest test test");

            ImGui::SameLine();

            if (ImGui::Button("Do Prestige"))
            {
                ++pt.psvBubbleValue.nPurchases;
                const auto ppReward = pt.calculatePrestigePointReward(1u);
                beginPrestigeTransition(ppReward);
            }

            ImGui::Separator();

            ImGui::Checkbox("hide ui", &debugHideUI);

            ImGui::Separator();

            ImGui::PushFont(fontImGuiMouldyCheese);
            uiSetFontScale(uiToolTipFontScale);

            SizeT step    = 1u;
            SizeT counter = 0u;

            static char filenameBuf[128] = "userdata/custom.json";

            ImGui::SetNextItemWidth(320.f * profile.uiScale);
            ImGui::InputText("##Filename", filenameBuf, sizeof(filenameBuf));

            if (ImGui::Button("Custom save"))
                savePlaythroughToFile(pt, filenameBuf);

            ImGui::SameLine();

            if (ImGui::Button("Custom load"))
                (void)loadPlaythroughFromFile(pt, filenameBuf);

            ImGui::Separator();

            if (ImGui::Button("Feed next shrine"))
            {
                for (Shrine& shrine : pt.shrines)
                {
                    if (!shrine.isActive() || shrine.tcDeath.hasValue())
                        continue;

                    const auto requiredReward = pt.getComputedRequiredRewardByShrineType(shrine.type);
                    shrine.collectedReward += requiredReward / 3u;
                    break;
                }
            }

            ImGui::Separator();

            ImGui::SetNextItemWidth(240.f * profile.uiScale);
            ImGui::InputScalar("Money", ImGuiDataType_U64, &pt.money, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

            ImGui::SetNextItemWidth(240.f * profile.uiScale);
            ImGui::InputScalar("PPs", ImGuiDataType_U64, &pt.prestigePoints, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

            ImGui::SetNextItemWidth(240.f * profile.uiScale);
            ImGui::InputScalar("WPs", ImGuiDataType_U64, &pt.wisdom, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

            ImGui::SetNextItemWidth(240.f * profile.uiScale);
            ImGui::InputScalar("Mana", ImGuiDataType_U64, &pt.mana, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal);

            ImGui::Separator();

            const auto scalarInput = [&](const char* label, float& value)
            {
                std::string lbuf = label;
                lbuf += "##";
                lbuf += std::to_string(counter++);

                ImGui::SetNextItemWidth(140.f * profile.uiScale);
                if (ImGui::InputScalar(lbuf.c_str(), ImGuiDataType_Float, &value, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
                    value = sf::base::clamp(value, 0.f, 10'000.f);
            };

            const auto psvScalarInput = [&](const char* label, PurchasableScalingValue& psv)
            {
                if (psv.data->nMaxPurchases == 0u)
                    return;

                std::string lbuf = label;
                lbuf += "##";
                lbuf += std::to_string(counter++);

                ImGui::SetNextItemWidth(140.f * profile.uiScale);
                if (ImGui::InputScalar(lbuf.c_str(), ImGuiDataType_U64, &psv.nPurchases, &step, nullptr, nullptr, ImGuiInputTextFlags_CharsDecimal))
                    psv.nPurchases = sf::base::clamp(psv.nPurchases, SizeT{0u}, psv.data->nMaxPurchases);
            };

            psvScalarInput("ComboStartTime", pt.psvComboStartTime);
            psvScalarInput("MapExtension", pt.psvMapExtension);
            psvScalarInput("ShrineActivation", pt.psvShrineActivation);
            psvScalarInput("BubbleCount", pt.psvBubbleCount);
            psvScalarInput("SpellCount", pt.psvSpellCount);
            psvScalarInput("BubbleValue", pt.psvBubbleValue);
            psvScalarInput("ExplosionRadiusMult", pt.psvExplosionRadiusMult);

            ImGui::Separator();

            for (SizeT i = 0u; i < nCatTypes; ++i)
            {
                scalarInput((std::to_string(i) + "Buff").c_str(), pt.buffCountdownsPerType[i].value);
            }

            ImGui::Separator();

            for (SizeT i = 0u; i < nCatTypes; ++i)
            {
                ImGui::Text("%s", CatConstants::typeNames[i]);
                psvScalarInput("PerCatType", pt.psvPerCatType[i]);
                psvScalarInput("CooldownMultsPerCatType", pt.psvCooldownMultsPerCatType[i]);
                psvScalarInput("RangeDivsPerCatType", pt.psvRangeDivsPerCatType[i]);

                ImGui::Separator();
            }

            psvScalarInput("PPMultiPopRange", pt.psvPPMultiPopRange);
            psvScalarInput("PPInspireDurationMult", pt.psvPPInspireDurationMult);
            psvScalarInput("PPManaCooldownMult", pt.psvPPManaCooldownMult);
            psvScalarInput("PPManaMaxMult", pt.psvPPManaMaxMult);
            psvScalarInput("PPMouseCatGlobalBonusMult", pt.psvPPMouseCatGlobalBonusMult);
            psvScalarInput("PPEngiCatGlobalBonusMult", pt.psvPPEngiCatGlobalBonusMult);
            psvScalarInput("PPRepulsoCatConverterChance", pt.psvPPRepulsoCatConverterChance);
            psvScalarInput("PPWitchCatBuffDuration", pt.psvPPWitchCatBuffDuration);
            psvScalarInput("PPUniRitualBuffPercentage", pt.psvPPUniRitualBuffPercentage);
            psvScalarInput("PPDevilRitualBuffPercentage", pt.psvPPDevilRitualBuffPercentage);

            ImGui::Separator();

            ImGui::Checkbox("starterPackPurchased", &pt.perm.starterPackPurchased);
            ImGui::Checkbox("multiPopPurchased", &pt.perm.multiPopPurchased);
            ImGui::Checkbox("smartCatsPurchased", &pt.perm.smartCatsPurchased);
            ImGui::Checkbox("geniusCatsPurchased", &pt.perm.geniusCatsPurchased);
            ImGui::Checkbox("windPurchased", &pt.perm.windPurchased);
            ImGui::Checkbox("astroCatInspirePurchased", &pt.perm.astroCatInspirePurchased);
            ImGui::Checkbox("starpawConversionIgnoreBombs", &pt.perm.starpawConversionIgnoreBombs);
            ImGui::Checkbox("starpawNova", &pt.perm.starpawNova);
            ImGui::Checkbox("repulsoCatFilterPurchased", &pt.perm.repulsoCatFilterPurchased);
            ImGui::Checkbox("repulsoCatConverterPurchased", &pt.perm.repulsoCatConverterPurchased);
            ImGui::Checkbox("repulsoCatNovaConverterPurchased", &pt.perm.repulsoCatNovaConverterPurchased);
            ImGui::Checkbox("attractoCatFilterPurchased", &pt.perm.attractoCatFilterPurchased);
            ImGui::Checkbox("witchCatBuffPowerScalesWithNCats", &pt.perm.witchCatBuffPowerScalesWithNCats);
            ImGui::Checkbox("witchCatBuffPowerScalesWithMapSize", &pt.perm.witchCatBuffPowerScalesWithMapSize);
            ImGui::Checkbox("witchCatBuffFewerDolls", &pt.perm.witchCatBuffFewerDolls);
            ImGui::Checkbox("witchCatBuffFlammableDolls", &pt.perm.witchCatBuffFlammableDolls);
            ImGui::Checkbox("witchCatBuffOrbitalDolls", &pt.perm.witchCatBuffOrbitalDolls);
            ImGui::Checkbox("wizardCatDoubleMewltiplierDuration", &pt.perm.wizardCatDoubleMewltiplierDuration);
            ImGui::Checkbox("wizardCatDoubleStasisFieldDuration", &pt.perm.wizardCatDoubleStasisFieldDuration);
            ImGui::Checkbox("unicatTranscendencePurchased", &pt.perm.unicatTranscendencePurchased);
            ImGui::Checkbox("unicatTranscendenceAOEPurchased", &pt.perm.unicatTranscendenceAOEPurchased);
            ImGui::Checkbox("devilcatHellsingedPurchased", &pt.perm.devilcatHellsingedPurchased);
            ImGui::Checkbox("unicatTranscendenceEnabled", &pt.perm.unicatTranscendenceEnabled);
            ImGui::Checkbox("devilcatHellsingedEnabled", &pt.perm.devilcatHellsingedEnabled);
            ImGui::Checkbox("autocastPurchased", &pt.perm.autocastPurchased);

            ImGui::Separator();

            ImGui::Checkbox("shrineCompleted Witch", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)]);
            ImGui::Checkbox("shrineCompleted Wizard", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)]);
            ImGui::Checkbox("shrineCompleted Mouse", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)]);
            ImGui::Checkbox("shrineCompleted Engi", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)]);
            ImGui::Checkbox("shrineCompleted Attracto", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)]);
            ImGui::Checkbox("shrineCompleted Repulso", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)]);
            ImGui::Checkbox("shrineCompleted Copy", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)]);
            ImGui::Checkbox("shrineCompleted Duck", &pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Duck)]);

            uiSetFontScale(uiNormalFontScale);
            ImGui::PopFont();

            ImGui::EndTabItem();
        }

        ImGui::Separator();
        uiSetFontScale(uiNormalFontScale);

        ImGui::Text("FPS: %.2f", static_cast<double>(fps));
    }

    ////////////////////////////////////////////////////////////
    TextParticle& makeRewardTextParticle(const sf::Vector2f position)
    {
        return textParticles.emplace_back(TextParticle{
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
        sounds.explosion.setPosition({bubble.position.x, bubble.position.y});
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
        const auto bubbleIdx  = static_cast<sf::base::SizeT>(&bubble - pt.bubbles.data());
        const auto bombIdxItr = bombIdxToCatIdx.find(bubbleIdx);

        Cat* catWhoMadeBomb = bombIdxItr != bombIdxToCatIdx.end() ? pt.cats.data() + bombIdxItr->second : nullptr;

        const float explosionRadius = pt.getComputedBombExplosionRadius();

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

        if (pt.perm.witchCatBuffFlammableDolls)
        {
            for (Doll& doll : pt.dolls)
                if ((doll.position - bubble.position).length() <= explosionRadius && !doll.tcDeath.hasValue())
                    collectDoll(doll);

            for (Doll& copyDoll : pt.copyDolls)
                if ((copyDoll.position - bubble.position).length() <= explosionRadius && !copyDoll.tcDeath.hasValue())
                    collectCopyDoll(copyDoll);
        }

        if (catWhoMadeBomb != nullptr)
            bombIdxToCatIdx.erase(bombIdxItr);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vector2f fromWorldToHud(const sf::Vector2f point) const
    {
        // From game coordinates to screen coordinates
        const sf::Vector2i screenPos = getWindow().mapCoordsToPixel(point, gameView);

        // From screen coordinates to HUD view coordinates
        return getWindow().mapPixelToCoords(screenPos, scaledHUDView);
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
        if (pt.stasisFieldTimer <= 0.f)
            return false;

        const auto rangeSquared = pt.getComputedSquaredRangeByCatType(CatType::Wizard);

        if (cachedWizardCat != nullptr)
            if ((bubble.position - cachedWizardCat->position).lengthSquared() <= rangeSquared)
                return true;

        if (cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Wizard)
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
        for (Shrine& shrine : pt.shrines)
        {
            if ((bubble.position - shrine.position).lengthSquared() > shrine.getRangeSquared())
                continue;

            collectorShrine = &shrine;
            shrineCollectReward(shrine, reward, bubble);
        }

        const bool         collectedByShrine = collectorShrine != nullptr;
        const sf::Vector2f tpPosition        = collectedByShrine ? collectorShrine->getDrawPosition() : bubble.position;

        if (profile.showTextParticles)
        {
            auto& tp = makeRewardTextParticle(tpPosition);
            std::snprintf(tp.buffer, sizeof(tp.buffer), "+$%llu", reward);
        }

        if (profile.accumulatingCombo && !multiPop && byPlayerClick && pt.comboPurchased && !collectedByShrine)
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
                    {.position      = moneyText.getCenterRight() + sf::Vector2f{32.f, rngFast.getF(-12.f, 12.f)},
                     .velocity      = {-0.25f, 0.f},
                     .scale         = 0.25f,
                     .scaleDecay    = 0.f,
                     .accelerationY = 0.f,
                     .opacity       = 0.f,
                     .opacityDecay  = -0.003f,
                     .rotation      = rngFast.getF(0.f, sf::base::tau),
                     .torque        = 0.f});


            const sf::Vector2f hudPos = fromWorldToHud(bubble.position);

            if ((!profile.accumulatingCombo || !pt.comboPurchased || !byPlayerClick) && !collectedByShrine &&
                spawnEarnedCoinParticle(hudPos))
            {
                sounds.coindelay.setPosition({getViewCenter().x - gameScreenSize.x / 2.f + 25.f,
                                              getViewCenter().y - gameScreenSize.y / 2.f + 25.f});

                sounds.coindelay.setPitch(1.f);
                sounds.coindelay.setVolume(profile.sfxVolume / 100.f * 0.5f);

                playSound(sounds.coindelay, /* maxOverlap */ 64);
            }
        }

        sounds.pop.setPosition({bubble.position.x, bubble.position.y});
        sounds.pop.setPitch(remap(static_cast<float>(xCombo), 1, 10, 1.f, 2.f));

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
            bubble = makeRandomBubble(pt, rng, pt.getMapLimit(), 0.f);
            bubble.position.y -= bubble.radius;
        }
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDebugModeEnabled() const
    {
        return debugMode;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopCheats()
    {
        if (!isDebugModeEnabled())
            return;

        if (keyDown(sf::Keyboard::Key::F4))
        {
            pt.comboPurchased = true;
            pt.mapPurchased   = true;
        }
        else if (keyDown(sf::Keyboard::Key::F5))
        {
            pt.money = 1'000'000'000u;
        }
        else if (keyDown(sf::Keyboard::Key::F6))
        {
            pt.money += 15u;
        }
        else if (keyDown(sf::Keyboard::Key::F7))
        {
            pt.prestigePoints += 15u;
        }
    }

    ////////////////////////////////////////////////////////////
    void turnBubbleNormal(Bubble& bubble)
    {
        if (bubble.type == BubbleType::Bomb)
            bombIdxToCatIdx.erase(static_cast<sf::base::SizeT>(&bubble - pt.bubbles.data()));

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
    void gameLoopUpdateScrolling(const float deltaTimeMs, const std::vector<sf::Vector2f>& downFingers)
    {
        // Reset map scrolling
        if (keyDown(sf::Keyboard::Key::LShift) || (downFingers.size() != 2u && !mBtnDown(getRMB(), /* penetrateUI */ true)))
            dragPosition.reset();

        //
        // Scrolling
        scroll = sf::base::clamp(scroll,
                                 0.f,
                                 sf::base::min(pt.getMapLimit() / 2.f - gameScreenSize.x / 2.f,
                                               (boundaries.x - gameScreenSize.x) / 2.f));

        actualScroll = exponentialApproach(actualScroll, scroll, deltaTimeMs, 75.f);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateTransitions(const float deltaTimeMs)
    {
        // Compute screen count
        constexpr auto nMaxScreens       = boundaries.x / gameScreenSize.x;
        const auto     nPurchasedScreens = static_cast<SizeT>(pt.getMapLimit() / gameScreenSize.x) + 1u;

        // Compute total target bubble count
        const auto targetBubbleCountPerScreen = static_cast<SizeT>(pt.psvBubbleCount.currentValue() / nMaxScreens);

        auto targetBubbleCount = targetBubbleCountPerScreen * nPurchasedScreens;

        const bool repulsoBuffActive = pt.buffCountdownsPerType[asIdx(CatType::Repulso)].value > 0.f;

        if (repulsoBuffActive)
            targetBubbleCount *= 2u;

        // Helper functions
        const auto playReversePopAt = [this](const sf::Vector2f position)
        {
            // TODO P2: refactor into function for any sound and reuse
            sounds.reversePop.setPosition({position.x, position.y});
            playSound(sounds.reversePop, /* maxOverlap */ 1u);
        };

        // If we are still displaying the splash screen, exit early
        if (splashCountdown.updateAndStop(deltaTimeMs) != CountdownStatusStop::AlreadyFinished)
            return;

        // Spawn bubbles and shrines during normal gmaeplay
        if (!inPrestigeTransition)
        {
            // Spawn shrines if required
            pt.spawnAllShrinesIfNeeded();

            // Spawn bubbles (or remove extra bubbles via debug menu)
            if (pt.bubbles.size() < targetBubbleCount)
            {
                const SizeT times = (targetBubbleCount - pt.bubbles.size()) > 500u ? 25u : 1u;

                for (SizeT i = 0; i < times; ++i)
                {
                    auto& bubble = pt.bubbles.emplace_back(makeRandomBubble(pt, rng, pt.getMapLimit(), boundaries.y));
                    const auto bPos = bubble.position;

                    if (repulsoBuffActive)
                        bubble.velocity += {0.18f, 0.18f};

                    spawnParticles(8, bPos, ParticleType::Bubble, 0.5f, 0.5f);
                    playReversePopAt(bPos);
                }
            }
            else if (pt.bubbles.size() > targetBubbleCount)
            {
                const SizeT times = (pt.bubbles.size() - targetBubbleCount) > 500u ? 25u : 1u;

                for (SizeT i = 0; i < times; ++i)
                {
                    const auto bPos = pt.bubbles.back().position;
                    pt.bubbles.pop_back();

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
            if (!pt.cats.empty())
            {
                for (auto& cat : pt.cats)
                {
                    cat.astroState.reset();
                    cat.cooldown.value = 100.f;
                }

                // Find rightmost cat
                const auto rightmostIt = std::max_element(pt.cats.begin(),
                                                          pt.cats.end(),
                                                          [](const Cat& a, const Cat& b)
                { return a.position.x < b.position.x; });

                const float targetScroll = (rightmostIt->position.x - gameScreenSize.x / 2.f) / 2.f;
                scroll                   = exponentialApproach(scroll, targetScroll, deltaTimeMs, 15.f);

                if (rightmostIt != pt.cats.end())
                    std::swap(*rightmostIt, pt.cats.back());

                const auto cPos = pt.cats.back().position;
                pt.cats.pop_back();

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

            if (!pt.shrines.empty())
            {
                const auto cPos = pt.shrines.back().position;
                pt.shrines.pop_back();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }

            if (!pt.dolls.empty())
            {
                const auto cPos = pt.dolls.back().position;
                pt.dolls.pop_back();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }

            if (!pt.copyDolls.empty())
            {
                const auto cPos = pt.copyDolls.back().position;
                pt.copyDolls.pop_back();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }

            if (!pt.hellPortals.empty())
            {
                const auto cPos = pt.hellPortals.back().position;
                pt.hellPortals.pop_back();

                spawnParticles(24, cPos, ParticleType::Star, 1.f, 0.5f);
                playReversePopAt(cPos);
            }
        }

        const bool gameElementsRemoved = pt.cats.empty() && pt.shrines.empty() && pt.dolls.empty() &&
                                         pt.copyDolls.empty() && pt.hellPortals.empty();

        // Reset map extension and scroll, and remove bubbles outside of view
        if (gameElementsRemoved)
        {
            pt.mapPurchased               = false;
            pt.psvMapExtension.nPurchases = 0u;

            scroll = 0.f;

            sf::base::vectorEraseIf(pt.bubbles, [&](const Bubble& b) { return b.position.x > pt.getMapLimit() + 128.f; });
        }

        // Despawn bubbles after other things
        if (gameElementsRemoved && !pt.bubbles.empty() &&
            bubbleSpawnTimer.updateAndLoop(deltaTimeMs) == CountdownStatusLoop::Looping)
        {
            const SizeT times = pt.bubbles.size() > 500u ? 25u : 1u;

            for (SizeT i = 0; i < times; ++i)
            {
                const auto bPos = pt.bubbles.back().position;
                pt.bubbles.pop_back();

                spawnParticles(8, bPos, ParticleType::Bubble, 0.5f, 0.5f);
                playReversePopAt(bPos);
            }
        }

        // End prestige transition
        if (gameElementsRemoved && pt.bubbles.empty())
        {
            pt.statsSession = Stats{};

            inPrestigeTransition = false;
            pt.money             = pt.perm.starterPackPurchased ? 1000u : 0u;

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

        for (Bubble& bubble : pt.bubbles)
        {
            if (bubble.velocity.lengthSquared() > maxVelocityMagnitude * maxVelocityMagnitude)
                bubble.velocity = bubble.velocity.normalized() * maxVelocityMagnitude;

            if (bubble.type == BubbleType::Bomb)
                bubble.rotation += deltaTimeMs * 0.01f;

            if (bubble.type == BubbleType::Star || bubble.type == BubbleType::Nova)
                bubble.hueMod += deltaTimeMs * 0.125f;

            float windVelocity = windMult[pt.windStrength] * (bubble.type == BubbleType::Bomb ? 0.01f : 0.9f);

            if (pt.buffCountdownsPerType[asIdx(CatType::Repulso)].value > 0.f)
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
            if (bubble.position.x - bubble.radius > pt.getMapLimit())
                bubble.position.x = -bubble.radius;
            else if (bubble.position.x + bubble.radius < 0.f)
                bubble.position.x = pt.getMapLimit() + bubble.radius;

            // Y-axis below and above screen
            if (bubble.position.y - bubble.radius > boundaries.y)
            {
                bubble.position.x = rng.getF(0.f, pt.getMapLimit());
                bubble.position.y = -bubble.radius * rng.getF(1.f, 2.f);

                bubble.velocity.y = windStartVelocityY[pt.windStrength];

                if (sf::base::fabs(bubble.velocity.x) > 0.04f)
                    bubble.velocity.x = 0.04f;

                const bool uniBuffEnabled       = pt.buffCountdownsPerType[asIdx(CatType::Uni)].value > 0.f;
                const bool devilBombBuffEnabled = !isDevilcatHellsingedActive() &&
                                                  pt.buffCountdownsPerType[asIdx(CatType::Devil)].value > 0.f;

                const bool willBeStar = uniBuffEnabled &&
                                        rng.getF(0.f, 100.f) <= pt.psvPPUniRitualBuffPercentage.currentValue();
                const bool willBeBomb = devilBombBuffEnabled &&
                                        rng.getF(0.f, 100.f) <= pt.psvPPDevilRitualBuffPercentage.currentValue();

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
    void gameLoopUpdateAttractoBuff(const float deltaTimeMs)
    {
        if (pt.buffCountdownsPerType[asIdx(CatType::Attracto)].value <= 0.f)
            return;

        const auto sqAttractoRange = pt.getComputedSquaredRangeByCatType(CatType::Attracto);
        const auto attractoRange   = SFML_BASE_MATH_SQRTF(sqAttractoRange);

        static thread_local std::vector<Bubble*> bombs;
        bombs.clear();

        for (Bubble& bubble : pt.bubbles)
            if (bubble.type == BubbleType::Bomb)
                bombs.push_back(&bubble);

        const auto attract = [&](const sf::Vector2f pos, Bubble& bubble)
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

        for (Bubble& bubble : pt.bubbles)
        {
            if (bubble.type == BubbleType::Bomb)
                continue;

            for (const HellPortal& hp : pt.hellPortals)
                attract(hp.position, bubble);

            for (const Bubble* bomb : bombs)
                attract(bomb->position, bubble);
        }
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool gameLoopUpdateBubbleClick(sf::base::Optional<sf::Vector2f>& clickPosition)
    {
        if (!clickPosition.hasValue())
            return false;

        const auto clickPos = optWindow->mapPixelToCoords(clickPosition->toVector2i(), gameView);

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
            for (Shrine& shrine : pt.shrines)
                if (shrine.type == ShrineType::Automation && shrine.isInRange(clickPos))
                {
                    sounds.failpop.setPosition({clickPos.x, clickPos.y});
                    playSound(sounds.failpop);

                    return ControlFlow::Break;
                }

            anyBubblePoppedByClicking = true;

            if (pt.comboPurchased)
            {
                if (!pt.laserPopEnabled)
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
                                                             pt.psvComboStartTime.currentValue() * 100.f);

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
                                            /* comboMult  */ getComboValueMult(combo, pt.laserPopEnabled ? playerComboDecayLaser : playerComboDecay),
                                            /* popperCat  */ nullptr);

            popWithRewardAndReplaceBubble({
                .reward          = reward,
                .bubble          = bubble,
                .xCombo          = combo,
                .popSoundOverlap = true,
                .popperCat       = nullptr,
                .multiPop        = false,
            });

            if (pt.multiPopEnabled && !pt.laserPopEnabled)
                forEachBubbleInRadius(clickPos,
                                      pt.psvPPMultiPopRange.currentValue(),
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
    [[nodiscard]] static sf::Vector2f getCatRangeCenter(const Cat& cat)
    {
        return cat.position + CatConstants::rangeOffsets[asIdx(cat.type)];
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionNormal(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto maxCooldown = pt.getComputedCooldownByCatType(cat.type);
        const auto range       = pt.getComputedRangeByCatType(cat.type);
        const auto [cx, cy]    = getCatRangeCenter(cat);

        const auto normalCatPopBubble = [&](Bubble& bubble)
        {
            cat.pawPosition = bubble.position;
            cat.pawOpacity  = 255.f;
            cat.pawRotation = (bubble.position - cat.position).angle() + sf::degrees(45);

            const float squaredMouseCatRange = pt.getComputedSquaredRangeByCatType(CatType::Mouse);

            const bool inMouseCatRange = cachedMouseCat != nullptr &&
                                         (cachedMouseCat->position - cat.position).lengthSquared() <= squaredMouseCatRange;

            const bool inCopyMouseCatRange = cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Mouse &&
                                             (cachedCopyCat->position - cat.position).lengthSquared() <= squaredMouseCatRange;

            const int comboMult = (inMouseCatRange || inCopyMouseCatRange) ? pt.mouseCatCombo : 1;

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

        if (!pt.perm.smartCatsPurchased)
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

        if (!pt.perm.geniusCatsPurchased)
        {
            if (Bubble* specialBubble = pickAny(BubbleType::Nova, BubbleType::Star, BubbleType::Bomb))
                normalCatPopBubble(*specialBubble);
            else if (Bubble* b = pickRandomBubbleInRadius({cx, cy}, range))
                normalCatPopBubble(*b);

            return;
        }

        if (Bubble* bBomb = pickAny(BubbleType::Bomb); bBomb != nullptr && !pt.geniusCatIgnoreBubbles.bomb)
            normalCatPopBubble(*bBomb);
        else if (Bubble* bNova = pickAny(BubbleType::Nova); bNova != nullptr && !pt.geniusCatIgnoreBubbles.star)
            normalCatPopBubble(*bNova);
        else if (Bubble* bStar = pickAny(BubbleType::Star); bStar != nullptr && !pt.geniusCatIgnoreBubbles.star)
            normalCatPopBubble(*bStar);
        else if (Bubble* bNormal = pickAny(BubbleType::Normal); bNormal != nullptr && !pt.geniusCatIgnoreBubbles.normal)
            normalCatPopBubble(*bNormal);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionUni(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto starBubbleType = isUnicatTranscendenceActive() ? BubbleType::Nova : BubbleType::Star;
        const auto nStarParticles = pt.perm.unicatTranscendenceAOEPurchased ? 1u : 4u;

        const auto transformBubble = [&](Bubble& bToTransform)
        {
            bToTransform.type       = starBubbleType;
            bToTransform.hueMod     = rng.getF(0.f, 360.f);
            bToTransform.velocity.y = rng.getF(-0.1f, -0.05f);

            spawnParticles(nStarParticles, bToTransform.position, ParticleType::Star, 0.5f, 0.35f);
            ++cat.hits;
        };

        const auto maxCooldown = pt.getComputedCooldownByCatType(cat.type);
        const auto range       = pt.getComputedRangeByCatType(cat.type);

        if (pt.perm.unicatTranscendenceAOEPurchased)
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

            sounds.shine2.setPosition({firstBubble->position.x, firstBubble->position.y});
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

            sounds.shine.setPosition({b->position.x, b->position.y});
            playSound(sounds.shine);
        }

        cat.textStatusShakeEffect.bump(rngFast, 1.5f);
        cat.cooldown.value = maxCooldown;
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionDevil(const float /* deltaTimeMs */, Cat& cat)
    {
        const auto maxCooldown = pt.getComputedCooldownByCatType(cat.type);
        const auto range       = pt.getComputedRangeByCatType(cat.type);

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

            const auto bubbleIdx = static_cast<sf::base::SizeT>(&bubble - pt.bubbles.data());
            const auto catIdx    = static_cast<sf::base::SizeT>(&cat - pt.cats.data());

            bombIdxToCatIdx[bubbleIdx] = catIdx;

            bubble.velocity.y += rng.getF(0.1f, 0.2f);
            sounds.makeBomb.setPosition({bubble.position.x, bubble.position.y});
            playSound(sounds.makeBomb);

            spawnParticles(8, bubble.position, ParticleType::Fire, 1.25f, 0.35f);
        }
        else
        {
            const auto portalPos = getCatRangeCenter(cat);

            pt.hellPortals.push_back({
                .position = portalPos,
                .life     = Countdown{.value = 1750.f},
                .catIdx   = static_cast<sf::base::SizeT>(&cat - pt.cats.data()),
            });

            sounds.makeBomb.setPosition({portalPos.x, portalPos.y});
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

        if (cat.astroState.hasValue())
            return;

        sounds.launch.setPosition({cx, cy});
        playSound(sounds.launch);

        ++cat.hits;
        cat.astroState.emplace(/* startX */ cat.position.x, /* velocityX */ 0.f, /* wrapped */ false);
        --cat.position.x;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getHexedCat()
    {
        for (Cat& cat : pt.cats)
            if (cat.hexedTimer.hasValue())
                return &cat;

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cat* getCopyHexedCat()
    {
        for (Cat& cat : pt.cats)
            if (cat.hexedCopyTimer.hasValue())
                return &cat;

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyCatHexedOrCopyHexed() const
    {
        return sf::base::anyOf(pt.cats.begin(), pt.cats.end(), [](const Cat& cat) { return cat.isHexedOrCopyHexed(); });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyCatHexed() const
    {
        return sf::base::anyOf(pt.cats.begin(), pt.cats.end(), [](const Cat& cat) { return cat.hexedTimer.hasValue(); });
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool anyCatCopyHexed() const
    {
        return sf::base::anyOf(pt.cats.begin(),
                               pt.cats.end(),
                               [](const Cat& cat) { return cat.hexedCopyTimer.hasValue(); });
    }

    ////////////////////////////////////////////////////////////
    void hexCat(Cat& cat, const bool copy)
    {
        if (isCatBeingDragged(cat))
            stopDraggingCat(cat);

        sounds.soulsteal.setPosition({cat.position.x, cat.position.y});
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
        return pt.getComputedCooldownByCatType(catType == CatType::Copy ? pt.copycatCopiedCatType : catType);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline float getComputedRangeByCatTypeOrCopyCat(const CatType catType) const
    {
        return pt.getComputedRangeByCatType(catType == CatType::Copy ? pt.copycatCopiedCatType : catType);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionWitchImpl(const float /* deltaTimeMs */, Cat& cat, std::vector<Doll>& dollsToUse)
    {
        const auto maxCooldown = getComputedCooldownByCatTypeOrCopyCat(cat.type);
        const auto range       = getComputedRangeByCatTypeOrCopyCat(cat.type);

        SizeT otherCatCount = 0u;
        Cat*  selected      = nullptr;

        for (Cat& otherCat : pt.cats)
        {
            if (otherCat.type == CatType::Duck)
                continue;

            if (otherCat.type == CatType::Witch)
                continue;

            if (otherCat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Witch)
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

            hexCat(*selected, /* copy */ &dollsToUse == &pt.copyDolls);

            float buffPower = pt.psvPPWitchCatBuffDuration.currentValue();

            if (pt.perm.witchCatBuffPowerScalesWithNCats)
                buffPower += sf::base::ceil(sf::base::pow(static_cast<float>(otherCatCount), 0.9f)) * 0.5f;

            if (pt.perm.witchCatBuffPowerScalesWithMapSize)
            {
                const float nMapExtensions = (pt.mapPurchased ? 1.f : 0.f) +
                                             static_cast<float>(pt.psvMapExtension.nPurchases);

                buffPower += static_cast<float>(nMapExtensions) * 0.75f;
            }

            const auto nDollsToSpawn = sf::base::max(SizeT{2u},
                                                     static_cast<SizeT>(
                                                         buffPower * (pt.perm.witchCatBuffFewerDolls ? 1.f : 2.f) / 4.f));

            SFML_BASE_ASSERT(dollsToUse.empty());

            statRitual(selected->type);

            const auto isPositionFarFromOtherDolls = [&](const sf::Vector2f& position) -> bool
            {
                for (const Doll& d : dollsToUse)
                    if ((d.position - position).lengthSquared() < (256.f * 256.f))
                        return false;

                return true;
            };

            const auto isOnTopOfAnyCat = [&](const sf::Vector2f& position) -> bool
            {
                for (const Cat& c : pt.cats)
                    if ((c.position - position).lengthSquared() < c.getRadiusSquared())
                        return true;

                return false;
            };

            const auto isOnTopOfAnyShrine = [&](const sf::Vector2f& position) -> bool
            {
                for (const Shrine& s : pt.shrines)
                    if ((s.position - position).lengthSquared() < s.getRadiusSquared())
                        return true;

                return false;
            };

            const auto rndDollPosition = [&]
            {
                constexpr float offset = 64.f;
                return rng.getVec2f({offset, offset}, {pt.getMapLimit() - offset - uiWindowWidth, boundaries.y - offset});
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
                auto& d = dollsToUse.emplace_back(
                    Doll{.position      = pickDollPosition(),
                         .wobbleRadians = rng.getF(0.f, sf::base::tau),
                         .buffPower     = buffPower,
                         .catType       = selected->type == CatType::Copy ? pt.copycatCopiedCatType : selected->type,
                         .tcActivation  = {.startingValue = rng.getF(300.f, 600.f) * static_cast<float>(i + 1)},
                         .tcDeath       = {}});

                d.tcActivation.restart();
            }

            const bool copy = &dollsToUse == &pt.copyDolls;
            spawnParticlesWithHue(copy ? 180.f : 0.f, 128, selected->position, ParticleType::Hex, 0.5f, 0.35f);

            cat.textStatusShakeEffect.bump(rngFast, 1.5f);
            cat.hits += 1u;

            if (!pt.dollTipShown)
            {
                pt.dollTipShown = true;
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
        gameLoopUpdateCatActionWitchImpl(deltaTimeMs, cat, pt.dolls);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatActionWizard(const float deltaTimeMs, Cat& cat)
    {
        if (!pt.absorbingWisdom)
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
            const auto wisdomReward = pt.getComputedRewardByBubbleType(bubble.type);

            if (profile.showTextParticles)
            {
                auto& tp = makeRewardTextParticle(drawPosition);
                std::snprintf(tp.buffer, sizeof(tp.buffer), "+%llu WP", wisdomReward);
            }

            sounds.absorb.setPosition({bubble.position.x, bubble.position.y});
            playSound(sounds.absorb, /* maxOverlap */ 1u);

            spawnParticlesWithHue(230.f, 16, bubble.position, ParticleType::Star, 0.5f, 0.35f);

            pt.wisdom += wisdomReward;
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

        addCombo(pt.mouseCatCombo, pt.mouseCatComboCountdown);
        pt.mouseCatCombo = sf::base::min(pt.mouseCatCombo, 999); // cap at 999x

        const auto savedBubblePos = bubble.position;

        const MoneyType reward = computeFinalReward(/* bubble     */ bubble,
                                                    /* multiplier */ pt.mouseCatCombo == 999 ? 5.f : 1.f,
                                                    /* comboMult  */ getComboValueMult(pt.mouseCatCombo, mouseCatComboDecay),
                                                    /* popperCat  */ &cat);

        popWithRewardAndReplaceBubble({
            .reward          = reward,
            .bubble          = bubble,
            .xCombo          = pt.mouseCatCombo,
            .popSoundOverlap = true,
            .popperCat       = &cat,
            .multiPop        = false,
        });

        if (pt.multiPopMouseCatEnabled)
            forEachBubbleInRadius(savedBubblePos,
                                  pt.psvPPMultiPopRange.currentValue(),
                                  [&](Bubble& otherBubble)
            {
                if (&otherBubble == &bubble)
                    return ControlFlow::Continue;

                popWithRewardAndReplaceBubble({
                    .reward          = reward,
                    .bubble          = otherBubble,
                    .xCombo          = pt.mouseCatCombo,
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

        for (Cat& otherCat : pt.cats)
        {
            if (otherCat.type == CatType::Engi)
                continue;

            if ((otherCat.position - cat.position).lengthSquared() > rangeSquared)
                continue;

            ++nCatsHit;

            spawnParticles(8, otherCat.getDrawPosition(profile.enableCatBobbing), ParticleType::Cog, 0.25f, 0.5f);

            sounds.maintenance.setPosition({otherCat.position.x, otherCat.position.y});
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

        if (pt.repulsoCatConverterEnabled && !pt.repulsoCatIgnoreBubbles.normal)
        {
            Bubble* b = pickRandomBubbleInRadiusMatching(cat.position,
                                                         range,
                                                         [&](Bubble& bubble)
            { return bubble.type != BubbleType::Star && bubble.type != BubbleType::Nova; });

            if (b != nullptr && rng.getF(0.f, 100.f) < pt.psvPPRepulsoCatConverterChance.currentValue())
            {
                b->type   = pt.perm.repulsoCatNovaConverterPurchased ? BubbleType::Nova : BubbleType::Star;
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
        if (pt.copycatCopiedCatType == CatType::Witch)
        {
            SFML_BASE_ASSERT(!anyCatCopyHexed());
            gameLoopUpdateCatActionWitchImpl(deltaTimeMs, cat, pt.copyDolls);
        }
        else if (pt.copycatCopiedCatType == CatType::Wizard)
            gameLoopUpdateCatActionWizard(deltaTimeMs, cat);
        else if (pt.copycatCopiedCatType == CatType::Mouse)
            gameLoopUpdateCatActionMouse(deltaTimeMs, cat);
        else if (pt.copycatCopiedCatType == CatType::Engi)
            gameLoopUpdateCatActionEngi(deltaTimeMs, cat);
        else if (pt.copycatCopiedCatType == CatType::Repulso)
            gameLoopUpdateCatActionRepulso(deltaTimeMs, cat);
        else if (pt.copycatCopiedCatType == CatType::Attracto)
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
        return mults[pt.windStrength];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getWindAttractionMult() const
    {
        constexpr float mults[4] = {1.f, 1.5f, 3.f, 4.5f};
        return mults[pt.windStrength];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] auto makeMagnetAction(
        const sf::Vector2f position,
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

        for (Cat& cat : pt.cats)
        {
            // Keep cat in boundaries
            const float catRadius = cat.getRadius();

            // Keep cats away from shrine of clicking
            // Buff cats in shrine of automation
            for (Shrine& shrine : pt.shrines)
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
                                                               {pt.getMapLimit() - catRadius, boundaries.y - catRadius});

            const auto maxCooldown  = pt.getComputedCooldownByCatType(cat.type);
            const auto range        = pt.getComputedRangeByCatType(cat.type);
            const auto rangeSquared = range * range;

            const auto drawPosition = cat.getDrawPosition(profile.enableCatBobbing);

            auto diff = cat.pawPosition - drawPosition - sf::Vector2f{-25.f, 25.f};
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
                        soundRitualEnd.setPosition({cat.position.x, cat.position.y});

                        if (profile.playWitchRitualSounds)
                            playSound(soundRitualEnd);
                    }
                }

                if (cat.cooldown.value < 10'000.f)
                {
                    if (cat.cooldown.value > 100.f && sounds.countPlayingPooled(soundRitual) == 0u)
                    {
                        soundRitual.setPosition({cat.position.x, cat.position.y});

                        if (profile.playWitchRitualSounds)
                            playSound(soundRitual);
                    }

                    const float intensity = remap(cat.cooldown.value, 0.f, 10'000.f, 1.f, 0.f);

                    for (Cat& otherCat : pt.cats)
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
                                                       sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
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

            if (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Witch && !anyCatCopyHexed())
                doWitchBehavior(/* hueMod */ 180.f, sounds.copyritual, sounds.copyritualend);

            if (cat.hexedTimer.hasValue() || (cat.type == CatType::Witch && (anyCatHexed() || !pt.dolls.empty())))
            {
                spawnParticle({.position = drawPosition + sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
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

            if (cat.hexedCopyTimer.hasValue() || (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Witch &&
                                                  (anyCatCopyHexed() || !pt.copyDolls.empty())))
            {
                spawnParticle({.position = drawPosition + sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius - 9.f},
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

            if (pt.buffCountdownsPerType[asIdx(CatType::Normal)].value > 0.f && cat.pawOpacity >= 75.f)
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
                spawnParticle({.position = drawPosition + sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius},
                               .velocity = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                               .scale    = rngFast.getF(0.08f, 0.27f) * 0.2f,
                               .scaleDecay    = 0.f,
                               .accelerationY = -0.002f,
                               .opacity       = 1.f,
                               .opacityDecay  = rngFast.getF(0.00025f, 0.0015f),
                               .rotation      = rngFast.getF(0.f, sf::base::tau),
                               .torque        = rngFast.getF(-0.002f, 0.002f)},
                              /* hue */ 0.f,
                              ParticleType::Star);
            }

            const float globalBoost = pt.buffCountdownsPerType[asIdx(CatType::Engi)].value;
            if ((globalBoost > 0.f || cat.boostCountdown.value > 0.f) && rngFast.getF(0.f, 1.f) > 0.75f)
            {
                spawnParticle({.position = drawPosition + sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius - 25.f},
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
                    spawnParticle({.position = drawPosition + sf::Vector2f{rngFast.getF(-catRadius + 15.f, +catRadius - 5.f),
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

                    spawnParticle({.position      = drawPosition + sf::Vector2f{-52.f * 0.2f, -85.f * 0.2f},
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

                    spawnParticle({.position      = drawPosition + sf::Vector2f{-140.f * 0.2f, -90.f * 0.2f},
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
                    sounds.rocket.setPosition({cx, cy});
                    playSound(sounds.rocket, /* maxOverlap */ 1u);

                    spawnParticles(1, drawPosition + sf::Vector2f{56.f, 45.f}, ParticleType::Fire, 1.5f, 0.25f, 0.65f);

                    if (rngFast.getI(0, 10) > 5)
                        spawnParticle(ParticleData{.position   = drawPosition + sf::Vector2f{56.f, 45.f},
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
                            pt.achAstrocatPopBomb = true;

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
                    cat.position.x = pt.getMapLimit() + catRadius;

                    if (pt.buffCountdownsPerType[asIdx(CatType::Astro)].value == 0.f) // loop if astro buff active
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
                if (pt.absorbingWisdom)
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
                    spawnParticle({.position = drawPosition + sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius},
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

            if (cat.type == CatType::Mouse || (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Mouse))
            {
                for (const Cat& otherCat : pt.cats)
                {
                    if (otherCat.type != CatType::Normal)
                        continue;

                    if ((otherCat.position - cat.position).lengthSquared() > rangeSquared)
                        continue;

                    if (rngFast.getF(0.f, 1.f) > 0.95f)
                        spawnParticle({.position = otherCat.getDrawPosition(profile.enableCatBobbing) +
                                                   sf::Vector2f{rngFast.getF(-catRadius, +catRadius), catRadius - 25.f},
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

            if (cat.type == CatType::Repulso || (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Repulso))
                forEachBubbleInRadius(cat.position,
                                      pt.getComputedRangeByCatType(CatType::Repulso),
                                      makeMagnetAction(cat.position,
                                                       cat.type,
                                                       deltaTimeMs,
                                                       &Bubble::repelledCountdown,
                                                       1500.f,
                                                       getWindRepulsionMult(),
                                                       -1.f,
                                                       pt.repulsoCatIgnoreBubbles));

            if (cat.type == CatType::Attracto || (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Attracto))
                forEachBubbleInRadius(cat.position,
                                      pt.getComputedRangeByCatType(CatType::Attracto),
                                      makeMagnetAction(cat.position,
                                                       cat.type,
                                                       deltaTimeMs,
                                                       &Bubble::attractedCountdown,
                                                       750.f,
                                                       getWindAttractionMult(),
                                                       1.f,
                                                       pt.attractoCatIgnoreBubbles));

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

        if (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Witch && anyCatCopyHexed())
            return false;

        if (cat.type == CatType::Witch && cat.cooldown.value <= 10'000.f)
            return false;

        if (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Witch && cat.cooldown.value <= 10'000.f)
            return false;

        return true;
    }

    [[nodiscard]] bool isAOESelecting() const
    {
        return (keyDown(sf::Keyboard::Key::LShift) || keyDown(sf::Keyboard::Key::LControl)) &&
               mBtnDown(getLMB(), /* penetrateUI */ true);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCatDragging(const float deltaTimeMs, const SizeT countFingersDown, const sf::Vector2f mousePos)
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

            for (Cat& cat : pt.cats)
            {
                if (!isCatDraggable(cat))
                    continue;

                if (!dragRect.contains(cat.position))
                    continue;

                draggedCats.push_back(&cat);
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

                static thread_local std::vector<sf::Vector2f> relativeCatPositions;
                relativeCatPositions.clear();
                relativeCatPositions.reserve(draggedCats.size());

                for (const Cat* cat : draggedCats)
                    relativeCatPositions.push_back(cat->position - pivotCat.position);

                pivotCat.position = exponentialApproach(pivotCat.position, mousePos + sf::Vector2f{-10.f, 13.f}, deltaTimeMs, 25.f);

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
                for (Cat& cat : pt.cats)
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
                    draggedCats.push_back(hoveredCat);

                    if (hoveredCat->type == CatType::Duck)
                    {
                        sounds.quack.setPosition({hoveredCat->position.x, hoveredCat->position.y});
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
        for (SizeT i = 0u; i < pt.psvShrineActivation.nPurchases; ++i)
        {
            for (Shrine& shrine : pt.shrines)
            {
                if (shrine.tcActivation.hasValue() || shrine.type != static_cast<ShrineType>(i))
                    continue;

                shrine.tcActivation.emplace(TargetedCountdown{.startingValue = 2000.f});
                shrine.tcActivation->restart();

                sounds.earthquakeFast.setPosition({shrine.position.x, shrine.position.y});
                playSound(sounds.earthquakeFast);

                screenShakeAmount = 4.5f;
                screenShakeTimer  = 1000.f;
            }
        }

        // Should only be triggered in testing via cheats
        for (SizeT i = pt.psvShrineActivation.nPurchases; i < nShrineTypes; ++i)
            for (Shrine& shrine : pt.shrines)
                if (shrine.type == static_cast<ShrineType>(i))
                    shrine.tcActivation.reset();

        for (Shrine& shrine : pt.shrines)
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

                    if (pt.perm.unsealedByType[asCatType])
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
                            sounds.absorb.setPosition({bubble.position.x, bubble.position.y});
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

            if (shrine.collectedReward >= pt.getComputedRequiredRewardByShrineType(shrine.type))
            {
                if (!shrine.tcDeath.hasValue())
                {
                    shrine.tcDeath.emplace(TargetedCountdown{.startingValue = 5000.f});
                    shrine.tcDeath->restart();

                    sounds.earthquake.setPosition({shrine.position.x, shrine.position.y});
                    playSound(sounds.earthquake);

                    screenShakeAmount = 4.5f;
                }
                else
                {
                    const auto cdStatus = shrine.tcDeath->updateAndStop(deltaTimeMs);

                    if (cdStatus == CountdownStatusStop::JustFinished)
                    {
                        playSound(sounds.woosh);
                        ++pt.nShrinesCompleted;

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

                                sounds.kaching.setPosition({shrine.position.x, shrine.position.y});
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

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Witch)])
                                doTip(
                                    "The Witchcat has been unsealed!\nThey perform voodoo rituals on nearby "
                                    "cats,\ngiving you powerful timed buffs.");
                        }
                        else if (shrine.type == ShrineType::Magic)
                        {
                            doShrineReward(CatType::Wizard);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Wizard)])
                                doTip(
                                    "The Wizardcat has been unsealed!\nThey absorb star bubbles to learn "
                                    "spells,\nwhich can be casted on demand.");
                        }
                        else if (shrine.type == ShrineType::Clicking)
                        {
                            doShrineReward(CatType::Mouse);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Mouse)])
                                doTip(
                                    "The Mousecat has been unsealed!\nThey combo-click bubbles, buff nearby "
                                    "cats,\nand "
                                    "provide a global click buff.");
                        }
                        else if (shrine.type == ShrineType::Automation)
                        {
                            doShrineReward(CatType::Engi);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Engi)])
                                doTip(
                                    "The Engicat has been unsealed!\nThey speed-up nearby cats and provide\na "
                                    "global "
                                    "cat buff.");
                        }
                        else if (shrine.type == ShrineType::Repulsion)
                        {
                            doShrineReward(CatType::Repulso);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Repulso)])
                                doTip(
                                    "The Repulsocat has been unsealed!\nNearby bubbles getting pushed away "
                                    "from\nthem "
                                    "gain a x2 multiplier.");
                        }
                        else if (shrine.type == ShrineType::Attraction)
                        {
                            doShrineReward(CatType::Attracto);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Attracto)])
                                doTip(
                                    "The Attractocat has been unsealed!\nNearby bubbles getting attracted to\nthem "
                                    "gain a x2 multiplier.");
                        }
                        else if (shrine.type == ShrineType::Camouflage)
                        {
                            doShrineReward(CatType::Copy);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Copy)])
                                doTip(
                                    "The Copycat has been unsealed!\nThey can mimic other unique cats,\ngaining "
                                    "their "
                                    "powers!");
                        }
                        else if (shrine.type == ShrineType::Victory)
                        {
                            doShrineReward(CatType::Duck);

                            if (!pt.perm.shrineCompletedOnceByCatType[asIdx(CatType::Duck)])
                                doTip(
                                    "It's... a duck. I am as confused\nas you are! But hey,\nyou won! "
                                    "Congratulations!");

                            playSound(sounds.quack);

                            victoryTC.emplace(TargetedCountdown{.startingValue = 6500.f});
                            victoryTC->restart();
                            delayedActions.emplace_back(Countdown{.value = 7000.f},
                                                        [this] { playSound(sounds.letterchime); });
                        }

                        const auto catType = asIdx(shrineTypeToCatType(shrine.type));
                        if (!pt.perm.shrineCompletedOnceByCatType[catType])
                        {
                            pushNotification("New unlocks!", "A new background and BGM have been unlocked!");

                            pt.perm.shrineCompletedOnceByCatType[catType] = true;

                            profile.selectedBackground = static_cast<int>(shrine.type) + 1;
                            profile.selectedBGM        = static_cast<int>(shrine.type) + 1;

                            updateSelectedBackgroundSelectorIndex();
                            updateSelectedBGMSelectorIndex();

                            switchToBGM(static_cast<sf::base::SizeT>(profile.selectedBGM), /* force */ false);
                        }
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

        sf::base::vectorEraseIf(pt.shrines, [](const Shrine& shrine) { return shrine.getDeathProgress() >= 1.f; });
    }

    ////////////////////////////////////////////////////////////
    void collectDollImpl(Doll& d, const std::vector<Doll>& dollsToUse)
    {
        SFML_BASE_ASSERT(!d.tcDeath.hasValue());

        const bool copy = &dollsToUse == &pt.copyDolls;

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

            const float currentBuff = pt.buffCountdownsPerType[asIdx(d.catType)].value;

            const float factor = (currentBuff < buffDurationSoftCap)
                                     ? std::pow((buffDurationSoftCap - currentBuff) / buffDurationSoftCap, 0.15f)
                                     : 0.1f;

            pt.buffCountdownsPerType[asIdx(d.catType)].value += buffDuration * factor;

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

            sounds.soulreturn.setPosition({d.position.x, d.position.y});
            playSound(sounds.soulreturn);
        }
        else
        {
            sounds.hex.setPosition({d.position.x, d.position.y});
            playSound(sounds.hex);
        }
    }

    ////////////////////////////////////////////////////////////
    void collectDoll(Doll& d)
    {
        collectDollImpl(d, pt.dolls);
    }

    ////////////////////////////////////////////////////////////
    void collectCopyDoll(Doll& d)
    {
        collectDollImpl(d, pt.copyDolls);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateDollsImpl(const float deltaTimeMs, const sf::Vector2f mousePos, std::vector<Doll>& dollsToUse, Cat* hexedCat)
    {
        const bool copy = &dollsToUse == &pt.copyDolls;

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
                    spawnParticle({.position   = d.getDrawPosition() + sf::Vector2f{rngFast.getF(-32.f, +32.f), 32.f},
                                   .velocity   = rngFast.getVec2f({-0.05f, -0.05f}, {0.05f, 0.05f}),
                                   .scale      = rngFast.getF(0.08f, 0.27f) * 0.5f,
                                   .scaleDecay = 0.f,
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
    void gameLoopUpdateDolls(const float deltaTimeMs, const sf::Vector2f mousePos)
    {
        if (cachedWitchCat == nullptr)
            return;

        gameLoopUpdateDollsImpl(deltaTimeMs, mousePos, pt.dolls, getHexedCat());
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCopyDolls(const float deltaTimeMs, const sf::Vector2f mousePos)
    {
        if (cachedCopyCat == nullptr || pt.copycatCopiedCatType != CatType::Witch)
            return;

        gameLoopUpdateDollsImpl(deltaTimeMs, mousePos, pt.copyDolls, getCopyHexedCat());
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateHellPortals(const float deltaTimeMs)
    {
        const float hellPortalRadius = pt.getComputedRangeByCatType(CatType::Devil);

        for (HellPortal& hp : pt.hellPortals)
        {
            if (hp.life.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            {
                sounds.makeBomb.setPosition({hp.position.x, hp.position.y});
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

        sf::base::vectorEraseIf(pt.hellPortals, [](const HellPortal& hp) { return hp.life.isDone(); });
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateWitchBuffs(const float deltaTimeMs)
    {
        for (Countdown& buffCountdown : pt.buffCountdownsPerType)
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
        const float manaMult = pt.buffCountdownsPerType[asIdx(CatType::Wizard)].value > 0.f ? 3.5f : 1.f;

        //
        // Mana
        if (pt.mana < pt.getComputedMaxMana())
            pt.manaTimer += deltaTimeMs * manaMult;
        else
            pt.manaTimer = 0.f;

        if (pt.manaTimer >= pt.getComputedManaCooldown())
        {
            pt.manaTimer = 0.f;

            if (pt.mana < pt.getComputedMaxMana())
            {
                pt.mana += 1u;

                if (profile.showFullManaNotification && pt.mana == pt.getComputedMaxMana())
                    pushNotification("Mana fully charged!", "The Wizardcat is eager to cast a spell...");
            }
        }

        //
        // Mewltiplier Aura spell
        if (pt.mewltiplierAuraTimer > 0.f)
        {
            pt.mewltiplierAuraTimer -= deltaTimeMs;
            pt.mewltiplierAuraTimer = sf::base::max(pt.mewltiplierAuraTimer, 0.f);

            const float wizardRange = pt.getComputedRangeByCatType(CatType::Wizard);

            if (cachedWizardCat != nullptr)
                for (SizeT i = 0u; i < 8u; ++i)
                    spawnParticlesWithHueNoGravity(230.f,
                                                   1,
                                                   rngFast.getPointInCircle(cachedWizardCat->position, wizardRange),
                                                   ParticleType::Star,
                                                   0.15f,
                                                   0.05f);

            if (cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Wizard)
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
        if (pt.stasisFieldTimer > 0.f)
        {
            pt.stasisFieldTimer -= deltaTimeMs;
            pt.stasisFieldTimer = sf::base::max(pt.stasisFieldTimer, 0.f);

            const float wizardRange = pt.getComputedRangeByCatType(CatType::Wizard);

            if (cachedWizardCat != nullptr)
                for (SizeT i = 0u; i < 8u; ++i)
                    spawnParticlesWithHueNoGravity(50.f,
                                                   1,
                                                   rngFast.getPointInCircle(cachedWizardCat->position, wizardRange),
                                                   ParticleType::Star,
                                                   0.15f,
                                                   0.05f);

            if (cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Wizard)
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
        if (cachedWizardCat == nullptr || !pt.perm.autocastPurchased || pt.perm.autocastIndex == 0u || isWizardBusy())
            return;

        const auto spellIndex = pt.perm.autocastIndex - 1u;

        if (pt.mana >= spellManaCostByIndex[spellIndex])
        {
            pt.mana -= spellManaCostByIndex[spellIndex];
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

        notificationQueue.emplace_back(title, std::string{fmtBuffer});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateMilestones()
    {
        const auto updateMilestone = [&](const char* name, sf::base::U64& milestone)
        {
            const auto oldMilestone = milestone;

            milestone = sf::base::min(milestone, pt.statsTotal.secondsPlayed);

            if (milestone != oldMilestone)
            {
                const auto [h, m, s] = formatTime(milestone);
                pushNotification("Milestone reached!", "'%s' at %lluh %llum %llus", name, h, m, s);
            }
        };

        const auto nCatNormal = pt.getCatCountByType(CatType::Normal);
        const auto nCatUni    = pt.getCatCountByType(CatType::Uni);
        const auto nCatDevil  = pt.getCatCountByType(CatType::Devil);
        const auto nCatAstro  = pt.getCatCountByType(CatType::Astro);

        if (nCatNormal >= 1)
            updateMilestone("1st Cat", pt.milestones.firstCat);

        if (nCatUni >= 1)
            updateMilestone("1st Unicat", pt.milestones.firstUnicat);

        if (nCatDevil >= 1)
            updateMilestone("1st Devilcat", pt.milestones.firstDevilcat);

        if (nCatAstro >= 1)
            updateMilestone("1st Astrocat", pt.milestones.firstAstrocat);

        if (nCatNormal >= 5)
            updateMilestone("5th Cat", pt.milestones.fiveCats);

        if (nCatUni >= 5)
            updateMilestone("5th Unicat", pt.milestones.fiveUnicats);

        if (nCatDevil >= 5)
            updateMilestone("5th Devilcat", pt.milestones.fiveDevilcats);

        if (nCatAstro >= 5)
            updateMilestone("5th Astrocat", pt.milestones.fiveAstrocats);

        if (nCatNormal >= 10)
            updateMilestone("10th Cat", pt.milestones.tenCats);

        if (nCatUni >= 10)
            updateMilestone("10th Unicat", pt.milestones.tenUnicats);

        if (nCatDevil >= 10)
            updateMilestone("10th Devilcat", pt.milestones.tenDevilcats);

        if (nCatAstro >= 10)
            updateMilestone("10th Astrocat", pt.milestones.tenAstrocats);

        if (pt.psvBubbleValue.nPurchases >= 1)
            updateMilestone("Prestige Level 2", pt.milestones.prestigeLevel2);

        if (pt.psvBubbleValue.nPurchases >= 2)
            updateMilestone("Prestige Level 3", pt.milestones.prestigeLevel3);

        if (pt.psvBubbleValue.nPurchases >= 3)
            updateMilestone("Prestige Level 4", pt.milestones.prestigeLevel4);

        if (pt.psvBubbleValue.nPurchases >= 4)
            updateMilestone("Prestige Level 5", pt.milestones.prestigeLevel5);

        if (pt.psvBubbleValue.nPurchases >= 5)
            updateMilestone("Prestige Level 6", pt.milestones.prestigeLevel6);

        if (pt.psvBubbleValue.nPurchases >= 9)
            updateMilestone("Prestige Level 10", pt.milestones.prestigeLevel10);

        if (pt.psvBubbleValue.nPurchases >= 14)
            updateMilestone("Prestige Level 15", pt.milestones.prestigeLevel15);

        if (pt.psvBubbleValue.nPurchases >= 19)
            updateMilestone("Prestige Level 20 (MAX)", pt.milestones.prestigeLevel20);

        const auto totalRevenue = pt.statsTotal.getTotalRevenue();

        if (totalRevenue >= 10'000)
            updateMilestone("$10.000 Revenue", pt.milestones.revenue10000);

        if (totalRevenue >= 100'000)
            updateMilestone("$100.000 Revenue", pt.milestones.revenue100000);

        if (totalRevenue >= 1'000'000)
            updateMilestone("$1.000.000 Revenue", pt.milestones.revenue1000000);

        if (totalRevenue >= 10'000'000)
            updateMilestone("$10.000.000 Revenue", pt.milestones.revenue10000000);

        if (totalRevenue >= 100'000'000)
            updateMilestone("$100.000.000 Revenue", pt.milestones.revenue100000000);

        if (totalRevenue >= 1'000'000'000)
            updateMilestone("$1.000.000.000 Revenue", pt.milestones.revenue1000000000);

        for (SizeT i = 0u; i < pt.nShrinesCompleted; ++i)
        {
            const char* shrineName = i >= pt.getMapLimitIncreases() ? "Shrine Of ???" : shrineNames[i];
            updateMilestone(shrineName, pt.milestones.shrineCompletions[i]);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateAchievements()
    {
#ifdef BUBBLEBYTE_USE_STEAMWORKS
        static bool mustGetFromSteam = true; // sync achievements from Steam only once
#endif

        SizeT nextId = 0u;

        const auto unlockIf = [&](const bool condition)
        {
            const auto achievementId = nextId++;

#ifdef BUBBLEBYTE_USE_STEAMWORKS
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

        const auto bubblesHandPopped = profile.statsLifetime.getTotalNBubblesHandPopped();
        const auto bubblesCatPopped  = profile.statsLifetime.getTotalNBubblesCatPopped();

        unlockIf(bubblesHandPopped >= 1);
        unlockIf(bubblesHandPopped >= 10);
        unlockIf(bubblesHandPopped >= 100);
        unlockIf(bubblesHandPopped >= 1000);
        unlockIf(bubblesHandPopped >= 10'000);
        unlockIf(bubblesHandPopped >= 100'000);
        unlockIf(bubblesHandPopped >= 1'000'000);

        unlockIf(bubblesCatPopped >= 1);
        unlockIf(bubblesCatPopped >= 100);
        unlockIf(bubblesCatPopped >= 1000);
        unlockIf(bubblesCatPopped >= 10'000);
        unlockIf(bubblesCatPopped >= 100'000);
        unlockIf(bubblesCatPopped >= 1'000'000);
        unlockIf(bubblesCatPopped >= 10'000'000);
        unlockIf(bubblesCatPopped >= 100'000'000);

        unlockIf(pt.comboPurchased);

        unlockIf(pt.psvComboStartTime.nPurchases >= 5);
        unlockIf(pt.psvComboStartTime.nPurchases >= 10);
        unlockIf(pt.psvComboStartTime.nPurchases >= 15);
        unlockIf(pt.psvComboStartTime.nPurchases >= 20);

        unlockIf(pt.mapPurchased); //
        unlockIf(pt.psvMapExtension.nPurchases >= 1);
        unlockIf(pt.psvMapExtension.nPurchases >= 3);
        unlockIf(pt.psvMapExtension.nPurchases >= 5);
        unlockIf(pt.psvMapExtension.nPurchases >= 7);

        unlockIf(pt.psvBubbleCount.nPurchases >= 1);
        unlockIf(pt.psvBubbleCount.nPurchases >= 5);
        unlockIf(pt.psvBubbleCount.nPurchases >= 10);
        unlockIf(pt.psvBubbleCount.nPurchases >= 20);
        unlockIf(pt.psvBubbleCount.nPurchases >= 30);

        unlockIf(pt.psvPerCatType[asIdx(CatType::Normal)].nPurchases >= 1);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Normal)].nPurchases >= 5);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Normal)].nPurchases >= 10);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Normal)].nPurchases >= 20);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Normal)].nPurchases >= 30);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Normal)].nPurchases >= 40);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Normal)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Normal)].nPurchases >= 9);

        unlockIf(pt.psvPerCatType[asIdx(CatType::Uni)].nPurchases >= 1);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Uni)].nPurchases >= 5);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Uni)].nPurchases >= 10);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Uni)].nPurchases >= 20);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Uni)].nPurchases >= 30);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Uni)].nPurchases >= 40);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Uni)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Uni)].nPurchases >= 9);

        unlockIf(pt.perm.unicatTranscendencePurchased);
        unlockIf(pt.perm.unicatTranscendenceAOEPurchased);

        unlockIf(pt.psvPerCatType[asIdx(CatType::Devil)].nPurchases >= 1);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Devil)].nPurchases >= 5);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Devil)].nPurchases >= 10);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Devil)].nPurchases >= 20);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Devil)].nPurchases >= 30);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Devil)].nPurchases >= 40);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Devil)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Devil)].nPurchases >= 9);

        unlockIf(pt.psvExplosionRadiusMult.nPurchases >= 1);
        unlockIf(pt.psvExplosionRadiusMult.nPurchases >= 5);
        unlockIf(pt.psvExplosionRadiusMult.nPurchases >= 10);

        unlockIf(pt.perm.devilcatHellsingedPurchased);

        unlockIf(pt.psvPerCatType[asIdx(CatType::Astro)].nPurchases >= 1);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Astro)].nPurchases >= 5);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Astro)].nPurchases >= 10);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Astro)].nPurchases >= 20);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Astro)].nPurchases >= 25);
        unlockIf(pt.psvPerCatType[asIdx(CatType::Astro)].nPurchases >= 30);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Astro)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Astro)].nPurchases >= 9);

        unlockIf(pt.psvBubbleValue.nPurchases >= 1);
        unlockIf(pt.psvBubbleValue.nPurchases >= 2);
        unlockIf(pt.psvBubbleValue.nPurchases >= 3);
        unlockIf(pt.psvBubbleValue.nPurchases >= 5);
        unlockIf(pt.psvBubbleValue.nPurchases >= 10);
        unlockIf(pt.psvBubbleValue.nPurchases >= 15);
        unlockIf(pt.psvBubbleValue.nPurchases >= 19);

        unlockIf(pt.perm.starterPackPurchased);

        unlockIf(pt.perm.multiPopPurchased);
        unlockIf(pt.psvPPMultiPopRange.nPurchases >= 1);
        unlockIf(pt.psvPPMultiPopRange.nPurchases >= 2);
        unlockIf(pt.psvPPMultiPopRange.nPurchases >= 5);
        unlockIf(pt.psvPPMultiPopRange.nPurchases >= 10);

        unlockIf(pt.perm.windPurchased);

        unlockIf(pt.perm.smartCatsPurchased);
        unlockIf(pt.perm.geniusCatsPurchased);

        unlockIf(pt.perm.astroCatInspirePurchased);
        unlockIf(pt.psvPPInspireDurationMult.nPurchases >= 1);
        unlockIf(pt.psvPPInspireDurationMult.nPurchases >= 4);
        unlockIf(pt.psvPPInspireDurationMult.nPurchases >= 8);
        unlockIf(pt.psvPPInspireDurationMult.nPurchases >= 12);
        unlockIf(pt.psvPPInspireDurationMult.nPurchases >= 16);

        unlockIf(combo >= 5);
        unlockIf(combo >= 10);
        unlockIf(combo >= 15);
        unlockIf(combo >= 20);
        unlockIf(combo >= 25);

        unlockIf(profile.statsLifetime.highestStarBubblePopCombo >= 5);
        unlockIf(profile.statsLifetime.highestStarBubblePopCombo >= 10);
        unlockIf(profile.statsLifetime.highestStarBubblePopCombo >= 15);
        unlockIf(profile.statsLifetime.highestStarBubblePopCombo >= 20);
        unlockIf(profile.statsLifetime.highestStarBubblePopCombo >= 25);

        const auto nStarBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Star);
        const auto nStarBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Star);

        unlockIf(nStarBubblesPoppedByHand >= 1);
        unlockIf(nStarBubblesPoppedByHand >= 100);
        unlockIf(nStarBubblesPoppedByHand >= 1000);
        unlockIf(nStarBubblesPoppedByHand >= 10'000);
        unlockIf(nStarBubblesPoppedByHand >= 100'000);

        unlockIf(nStarBubblesPoppedByCat >= 1);
        unlockIf(nStarBubblesPoppedByCat >= 100);
        unlockIf(nStarBubblesPoppedByCat >= 1000);
        unlockIf(nStarBubblesPoppedByCat >= 10'000);
        unlockIf(nStarBubblesPoppedByCat >= 100'000);
        unlockIf(nStarBubblesPoppedByCat >= 1'000'000);
        unlockIf(nStarBubblesPoppedByCat >= 10'000'000);

        unlockIf(profile.statsLifetime.highestNovaBubblePopCombo >= 5);
        unlockIf(profile.statsLifetime.highestNovaBubblePopCombo >= 10);
        unlockIf(profile.statsLifetime.highestNovaBubblePopCombo >= 15);
        unlockIf(profile.statsLifetime.highestNovaBubblePopCombo >= 20);
        unlockIf(profile.statsLifetime.highestNovaBubblePopCombo >= 25);

        const auto nNovaBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Nova);
        const auto nNovaBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Nova);

        unlockIf(nNovaBubblesPoppedByHand >= 1);
        unlockIf(nNovaBubblesPoppedByHand >= 100);
        unlockIf(nNovaBubblesPoppedByHand >= 1000);
        unlockIf(nNovaBubblesPoppedByHand >= 10'000);
        unlockIf(nNovaBubblesPoppedByHand >= 100'000);

        unlockIf(nNovaBubblesPoppedByCat >= 1);
        unlockIf(nNovaBubblesPoppedByCat >= 100);
        unlockIf(nNovaBubblesPoppedByCat >= 1000);
        unlockIf(nNovaBubblesPoppedByCat >= 10'000);
        unlockIf(nNovaBubblesPoppedByCat >= 100'000);
        unlockIf(nNovaBubblesPoppedByCat >= 1'000'000);
        unlockIf(nNovaBubblesPoppedByCat >= 10'000'000);

        const auto nBombBubblesPoppedByHand = profile.statsLifetime.getNBubblesHandPopped(BubbleType::Bomb);
        const auto nBombBubblesPoppedByCat  = profile.statsLifetime.getNBubblesCatPopped(BubbleType::Bomb);

        unlockIf(nBombBubblesPoppedByHand >= 1);
        unlockIf(nBombBubblesPoppedByHand >= 100);
        unlockIf(nBombBubblesPoppedByHand >= 1000);
        unlockIf(nBombBubblesPoppedByHand >= 10'000);

        unlockIf(nBombBubblesPoppedByCat >= 1);
        unlockIf(nBombBubblesPoppedByCat >= 100);
        unlockIf(nBombBubblesPoppedByCat >= 1000);
        unlockIf(nBombBubblesPoppedByCat >= 10'000);
        unlockIf(nBombBubblesPoppedByCat >= 100'000);

        unlockIf(pt.achAstrocatPopBomb);

        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Normal)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Uni)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Devil)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Witch)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Wizard)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Mouse)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Engi)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Repulso)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Attracto)]);
        unlockIf(pt.achAstrocatInspireByType[asIdx(CatType::Copy)]);

        unlockIf(pt.psvShrineActivation.nPurchases >= 1);
        unlockIf(pt.psvShrineActivation.nPurchases >= 2);
        unlockIf(pt.psvShrineActivation.nPurchases >= 3);
        unlockIf(pt.psvShrineActivation.nPurchases >= 4);
        unlockIf(pt.psvShrineActivation.nPurchases >= 5);
        unlockIf(pt.psvShrineActivation.nPurchases >= 6);
        unlockIf(pt.psvShrineActivation.nPurchases >= 7);
        unlockIf(pt.psvShrineActivation.nPurchases >= 8);

        unlockIf(pt.nShrinesCompleted >= 1);
        unlockIf(pt.nShrinesCompleted >= 2);
        unlockIf(pt.nShrinesCompleted >= 3);
        unlockIf(pt.nShrinesCompleted >= 4);
        unlockIf(pt.nShrinesCompleted >= 5);
        unlockIf(pt.nShrinesCompleted >= 6);
        unlockIf(pt.nShrinesCompleted >= 7);
        unlockIf(pt.nShrinesCompleted >= 8);

        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Witch)]);
        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Wizard)]);
        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Mouse)]);
        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Engi)]);
        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Repulso)]);
        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Attracto)]);
        unlockIf(pt.perm.unsealedByType[asIdx(CatType::Copy)]);

        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Normal)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Uni)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Devil)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Astro)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Wizard)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Mouse)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Engi)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Repulso)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Attracto)] >= 1);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Copy)] >= 1);

        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Normal)] >= 500);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Uni)] >= 100);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Devil)] >= 100);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Astro)] >= 50);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Wizard)] >= 10);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Mouse)] >= 10);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Engi)] >= 10);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Repulso)] >= 10);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Attracto)] >= 10);
        unlockIf(profile.statsLifetime.nWitchcatRitualsPerCatType[asIdx(CatType::Copy)] >= 10);

        unlockIf(profile.statsLifetime.nWitchcatDollsCollected >= 1);
        unlockIf(profile.statsLifetime.nWitchcatDollsCollected >= 10);
        unlockIf(profile.statsLifetime.nWitchcatDollsCollected >= 100);
        unlockIf(profile.statsLifetime.nWitchcatDollsCollected >= 1000);
        unlockIf(profile.statsLifetime.nWitchcatDollsCollected >= 10'000);

        unlockIf(pt.psvPPWitchCatBuffDuration.nPurchases >= 1);
        unlockIf(pt.psvPPWitchCatBuffDuration.nPurchases >= 3);
        unlockIf(pt.psvPPWitchCatBuffDuration.nPurchases >= 6);
        unlockIf(pt.psvPPWitchCatBuffDuration.nPurchases >= 9);
        unlockIf(pt.psvPPWitchCatBuffDuration.nPurchases >= 12);

        unlockIf(pt.perm.witchCatBuffPowerScalesWithNCats);
        unlockIf(pt.perm.witchCatBuffPowerScalesWithMapSize);
        unlockIf(pt.perm.witchCatBuffFewerDolls);
        unlockIf(pt.perm.witchCatBuffFlammableDolls);
        unlockIf(pt.perm.witchCatBuffOrbitalDolls);

        unlockIf(pt.psvPPUniRitualBuffPercentage.nPurchases >= 1);
        unlockIf(pt.psvPPUniRitualBuffPercentage.nPurchases >= 6);
        unlockIf(pt.psvPPUniRitualBuffPercentage.nPurchases >= 12);
        unlockIf(pt.psvPPUniRitualBuffPercentage.nPurchases >= 18);
        unlockIf(pt.psvPPUniRitualBuffPercentage.nPurchases >= 24);

        unlockIf(pt.psvPPDevilRitualBuffPercentage.nPurchases >= 1);
        unlockIf(pt.psvPPDevilRitualBuffPercentage.nPurchases >= 6);
        unlockIf(pt.psvPPDevilRitualBuffPercentage.nPurchases >= 12);
        unlockIf(pt.psvPPDevilRitualBuffPercentage.nPurchases >= 18);
        unlockIf(pt.psvPPDevilRitualBuffPercentage.nPurchases >= 24);

        const auto nActiveBuffs = sf::base::countIf(pt.buffCountdownsPerType,
                                                    pt.buffCountdownsPerType + nCatTypes,
                                                    [](const Countdown& c) { return c.value > 0.f; });

        unlockIf(nActiveBuffs >= 2);
        unlockIf(nActiveBuffs >= 3);
        unlockIf(nActiveBuffs >= 4);
        unlockIf(nActiveBuffs >= 5);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Witch)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Witch)].nPurchases >= 9);

        unlockIf(profile.statsLifetime.nAbsorbedStarBubbles >= 1);
        unlockIf(profile.statsLifetime.nAbsorbedStarBubbles >= 100);
        unlockIf(profile.statsLifetime.nAbsorbedStarBubbles >= 1000);
        unlockIf(profile.statsLifetime.nAbsorbedStarBubbles >= 10'000);
        unlockIf(profile.statsLifetime.nAbsorbedStarBubbles >= 100'000);

        unlockIf(pt.psvSpellCount.nPurchases >= 1);
        unlockIf(pt.psvSpellCount.nPurchases >= 2);
        unlockIf(pt.psvSpellCount.nPurchases >= 3);
        unlockIf(pt.psvSpellCount.nPurchases >= 4);

        unlockIf(pt.psvStarpawPercentage.nPurchases >= 1);
        unlockIf(pt.psvStarpawPercentage.nPurchases >= 4);
        unlockIf(pt.psvStarpawPercentage.nPurchases >= 8);

        unlockIf(pt.psvMewltiplierMult.nPurchases >= 1);
        unlockIf(pt.psvMewltiplierMult.nPurchases >= 5);
        unlockIf(pt.psvMewltiplierMult.nPurchases >= 10);
        unlockIf(pt.psvMewltiplierMult.nPurchases >= 15);

        unlockIf(pt.psvDarkUnionPercentage.nPurchases >= 1);
        unlockIf(pt.psvDarkUnionPercentage.nPurchases >= 4);
        unlockIf(pt.psvDarkUnionPercentage.nPurchases >= 8);

        unlockIf(profile.statsLifetime.nSpellCasts[0] >= 1);
        unlockIf(profile.statsLifetime.nSpellCasts[0] >= 10);
        unlockIf(profile.statsLifetime.nSpellCasts[0] >= 100);
        unlockIf(profile.statsLifetime.nSpellCasts[0] >= 1000);

        unlockIf(profile.statsLifetime.nSpellCasts[1] >= 1);
        unlockIf(profile.statsLifetime.nSpellCasts[1] >= 10);
        unlockIf(profile.statsLifetime.nSpellCasts[1] >= 100);
        unlockIf(profile.statsLifetime.nSpellCasts[1] >= 1000);

        unlockIf(profile.statsLifetime.nSpellCasts[2] >= 1);
        unlockIf(profile.statsLifetime.nSpellCasts[2] >= 10);
        unlockIf(profile.statsLifetime.nSpellCasts[2] >= 100);
        unlockIf(profile.statsLifetime.nSpellCasts[2] >= 1000);

        unlockIf(profile.statsLifetime.nSpellCasts[3] >= 1);
        unlockIf(profile.statsLifetime.nSpellCasts[3] >= 10);
        unlockIf(profile.statsLifetime.nSpellCasts[3] >= 100);
        unlockIf(profile.statsLifetime.nSpellCasts[3] >= 1000);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Wizard)].nPurchases >= 9);

        unlockIf(pt.psvPPManaCooldownMult.nPurchases >= 1);
        unlockIf(pt.psvPPManaCooldownMult.nPurchases >= 4);
        unlockIf(pt.psvPPManaCooldownMult.nPurchases >= 8);
        unlockIf(pt.psvPPManaCooldownMult.nPurchases >= 12);
        unlockIf(pt.psvPPManaCooldownMult.nPurchases >= 16);

        unlockIf(pt.psvPPManaMaxMult.nPurchases >= 1);
        unlockIf(pt.psvPPManaMaxMult.nPurchases >= 4);
        unlockIf(pt.psvPPManaMaxMult.nPurchases >= 8);
        unlockIf(pt.psvPPManaMaxMult.nPurchases >= 12);
        unlockIf(pt.psvPPManaMaxMult.nPurchases >= 16);
        unlockIf(pt.psvPPManaMaxMult.nPurchases >= 20);

        unlockIf(pt.perm.starpawConversionIgnoreBombs);
        unlockIf(pt.perm.starpawNova);
        unlockIf(pt.perm.wizardCatDoubleMewltiplierDuration);
        unlockIf(pt.perm.wizardCatDoubleStasisFieldDuration);

        unlockIf(pt.mouseCatCombo >= 25);
        unlockIf(pt.mouseCatCombo >= 50);
        unlockIf(pt.mouseCatCombo >= 75);
        unlockIf(pt.mouseCatCombo >= 100);
        unlockIf(pt.mouseCatCombo >= 125);
        unlockIf(pt.mouseCatCombo >= 150);
        unlockIf(pt.mouseCatCombo >= 175);
        unlockIf(pt.mouseCatCombo >= 999);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Mouse)].nPurchases >= 9);

        unlockIf(pt.psvPPMouseCatGlobalBonusMult.nPurchases >= 1);
        unlockIf(pt.psvPPMouseCatGlobalBonusMult.nPurchases >= 2);
        unlockIf(pt.psvPPMouseCatGlobalBonusMult.nPurchases >= 6);
        unlockIf(pt.psvPPMouseCatGlobalBonusMult.nPurchases >= 10);
        unlockIf(pt.psvPPMouseCatGlobalBonusMult.nPurchases >= 14);

        unlockIf(profile.statsLifetime.nMaintenances >= 1);
        unlockIf(profile.statsLifetime.nMaintenances >= 10);
        unlockIf(profile.statsLifetime.nMaintenances >= 100);
        unlockIf(profile.statsLifetime.nMaintenances >= 1000);
        unlockIf(profile.statsLifetime.nMaintenances >= 10'000);
        unlockIf(profile.statsLifetime.nMaintenances >= 100'000);
        unlockIf(profile.statsLifetime.nMaintenances >= 1'000'000);

        unlockIf(profile.statsLifetime.highestSimultaneousMaintenances >= 3);
        unlockIf(profile.statsLifetime.highestSimultaneousMaintenances >= 6);
        unlockIf(profile.statsLifetime.highestSimultaneousMaintenances >= 9);
        unlockIf(profile.statsLifetime.highestSimultaneousMaintenances >= 12);
        unlockIf(profile.statsLifetime.highestSimultaneousMaintenances >= 15);

        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases >= 1);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases >= 3);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases >= 6);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases >= 9);
        unlockIf(pt.psvCooldownMultsPerCatType[asIdx(CatType::Engi)].nPurchases >= 12);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Engi)].nPurchases >= 9);

        unlockIf(pt.psvPPEngiCatGlobalBonusMult.nPurchases >= 1);
        unlockIf(pt.psvPPEngiCatGlobalBonusMult.nPurchases >= 2);
        unlockIf(pt.psvPPEngiCatGlobalBonusMult.nPurchases >= 6);
        unlockIf(pt.psvPPEngiCatGlobalBonusMult.nPurchases >= 10);
        unlockIf(pt.psvPPEngiCatGlobalBonusMult.nPurchases >= 14);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Repulso)].nPurchases >= 9);

        unlockIf(pt.perm.repulsoCatFilterPurchased);
        unlockIf(pt.perm.repulsoCatConverterPurchased);
        unlockIf(pt.perm.repulsoCatNovaConverterPurchased);

        unlockIf(pt.psvPPRepulsoCatConverterChance.nPurchases >= 1);
        unlockIf(pt.psvPPRepulsoCatConverterChance.nPurchases >= 4);
        unlockIf(pt.psvPPRepulsoCatConverterChance.nPurchases >= 8);
        unlockIf(pt.psvPPRepulsoCatConverterChance.nPurchases >= 12);
        unlockIf(pt.psvPPRepulsoCatConverterChance.nPurchases >= 16);

        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases >= 1);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases >= 3);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases >= 6);
        unlockIf(pt.psvRangeDivsPerCatType[asIdx(CatType::Attracto)].nPurchases >= 9);

        unlockIf(pt.perm.attractoCatFilterPurchased);

        unlockIf(profile.statsLifetime.nDisguises >= 1);
        unlockIf(profile.statsLifetime.nDisguises >= 5);
        unlockIf(profile.statsLifetime.nDisguises >= 25);
        unlockIf(profile.statsLifetime.nDisguises >= 100);

        unlockIf(buyReminder >= 5); // Secret
        unlockIf(pt.geniusCatIgnoreBubbles.normal && pt.geniusCatIgnoreBubbles.star && pt.geniusCatIgnoreBubbles.bomb); // Secret
        unlockIf(wastedEffort);

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

        for (SizeT i = 0u; i < pt.bubbles.size(); ++i)
        {
            Bubble& bubble = pt.bubbles[i];

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

        rtGame->draw(bubbleDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame->draw(starBubbleDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame->draw(bombBubbleDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursorTrail(const sf::Vector2f mousePos)
    {
        if (profile.cursorTrailMode == 2 /* disabled */)
            return;

        if (combo <= 1 && profile.cursorTrailMode == 0 /* combo mode */)
            return;

        const sf::Vector2f mousePosDiff    = lastMousePos - mousePos;
        const float        mousePosDiffLen = mousePosDiff.length();

        if (mousePosDiffLen == 0.f)
            return;

        const float chunks   = mousePosDiffLen / 0.5f;
        const float chunkLen = mousePosDiffLen / chunks;

        const float trailHue = wrapHue(profile.cursorHue + currentBackgroundHue.asDegrees());

        const sf::Vector2f trailStep = mousePosDiff.normalized() * chunkLen;

        const float trailScaleMult = pt.laserPopEnabled ? 1.5f : 1.f;

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

        for (const Shrine& shrine : pt.shrines)
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

        for (const Cat& cat : pt.cats)
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

        for (const Doll& doll : pt.dolls)
        {
            minimapDrawableBatch.add(
                sf::Sprite{.position    = doll.position,
                           .scale       = {0.5f, 0.5f},
                           .origin      = txrDollNormal.size / 2.f,
                           .rotation    = sf::radians(0.f),
                           .textureRect = txrDollNormal,
                           .color       = hueColor(doll.hue, 255u)});
        }

        for (const Doll& doll : pt.copyDolls)
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
        if (rtBackground.hasValue() && !shader.setUniform(suBackgroundTexture, rtBackground->getTexture()))
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

        rtGame->draw(bubbleDrawableBatch, bubbleStates);

        shader.setUniform(suBubbleLightness, profile.bsBubbleLightness * 1.25f);
        shader.setUniform(suIridescenceStrength, profile.bsIridescenceStrength * 0.01f);
        shader.setUniform(suSubTexOrigin, txrBubbleStar.position);
        shader.setUniform(suSubTexSize, txrBubbleStar.size);

        rtGame->draw(starBubbleDrawableBatch, bubbleStates);

        shader.setUniform(suBubbleEffect, false);

        rtGame->draw(bombBubbleDrawableBatch, bubbleStates);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCats(const sf::Vector2f mousePos, const float deltaTimeMs)
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
        const sf::Vector2f catTailOffsetsByType[] = {
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
        for (Cat& cat : pt.cats)
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
    void gameLoopDrawCat(Cat&               cat,
                         const float        deltaTimeMs,
                         const sf::Vector2f mousePos,
                         const sf::FloatRect* const (&catTxrsByType)[nCatTypes],
                         const sf::FloatRect* const (&catPawTxrsByType)[nCatTypes],
                         const sf::FloatRect* const (&catTailTxrsByType)[nCatTypes],
                         const sf::Vector2f (&catTailOffsetsByType)[nCatTypes],
                         const float (&catHueByType)[nCatTypes])
    {
        auto& batchToUse     = catToPlace == &cat ? cpuTopDrawableBatch : cpuDrawableBatch;
        auto& textBatchToUse = catToPlace == &cat ? catTextTopDrawableBatch : catTextDrawableBatch;

        const sf::FloatRect& catTxr = *catTxrsByType[asIdx(cat.type)];

        if (catToPlace != &cat && !bubbleCullingBoundaries.isInside(cat.position))
            return;

        const auto isCopyCatWithType = [&](const CatType copiedType)
        { return cat.type == CatType::Copy && pt.copycatCopiedCatType == copiedType; };

        const bool beingDragged = isCatBeingDragged(cat);

        const sf::base::Optional<sf::FloatRect> dragRect = getAoEDragRect(mousePos);

        const bool insideDragRect = dragRect.hasValue() && dragRect->contains(cat.position);

        const bool hovered = (mousePos - cat.position).lengthSquared() <= cat.getRadiusSquared();

        const bool shouldDisplayRangeCircle = !beingDragged && !cat.isAstroAndInFlight() && hovered &&
                                              !mBtnDown(getLMB(), /* penetrateUI */ true);

        const U8 rangeInnerAlpha = shouldDisplayRangeCircle ? 75u : 0u;

        const sf::FloatRect& catPawTxr = *catPawTxrsByType[asIdx(isCopyCatWithType(CatType::Mouse) ? CatType::Mouse : cat.type)];
        const sf::FloatRect& catTailTxr    = *catTailTxrsByType[asIdx(cat.type)];
        const sf::Vector2f   catTailOffset = catTailOffsetsByType[asIdx(cat.type)];

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

                const auto range        = pt.getComputedRangeByCatType(CatType::Witch);
                const auto rangeSquared = range * range;

                const bool catInWitchRange = (witch.position - cat.position).lengthSquared() <= rangeSquared;

                if (&cat == &witch || (pt.perm.witchCatBuffPowerScalesWithNCats && catInWitchRange))
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
        else if (cachedCopyCat != nullptr && pt.copycatCopiedCatType == CatType::Witch)
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

        const auto circleColor        = CatConstants::colors[asIdx(cat.type)].withHueMod(cat.hue).withLightness(0.75f);
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
        const auto  catScale     = sf::Vector2f{0.2f, 0.2f} * catScaleMult;

        const auto catAnchor = beingDragged ? cat.position : cat.getDrawPosition(profile.enableCatBobbing);

        const auto anchorOffset = [&](const sf::Vector2f offset)
        { return catAnchor + (offset / 2.f * 0.2f * catScaleMult).rotatedBy(sf::radians(catRotation)); };

        const float tailRotationMult = cat.type == CatType::Uni ? 0.4f : 1.f;

        const auto tailWiggleRotation = sf::radians(
            catRotation + ((beingDragged ? -0.2f : 0.f) +
                           std::sin(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * tailRotationMult));

        const auto tailWiggleRotationInvertedDragged = sf::radians(
            catRotation + ((beingDragged ? 0.2f : 0.f) +
                           std::sin(cat.wobbleRadians) * (beingDragged ? 0.125f : 0.075f) * tailRotationMult));

        const sf::Vector2f pushDown{0.f, beingDragged ? 75.f : 0.f};

        const auto attachmentHue = hueColor(catHueByType[asIdx(cat.type)] + cat.hue, alpha);

        // Devilcat: draw tail behind
        if (cat.type == CatType::Devil)
        {
            batchToUse.add(
                sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vector2f{905.f, 10.f} + pushDown * 2.f),
                           .scale       = catScale * 1.25f,
                           .origin      = {320.f, 32.f},
                           .rotation    = tailWiggleRotationInvertedDragged,
                           .textureRect = catTailTxr,
                           .color       = catColor});
        }

        //
        // Draw brain jar in the background
        if (cat.type == CatType::Normal && pt.perm.geniusCatsPurchased)
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
                                      .origin      = txrUniCatWings.size / 2.f - sf::Vector2f{35.f, 10.f},
                                      .rotation    = wingRotation,
                                      .textureRect = txrUniCatWings,
                                      .color       = hueColor(cat.hue + 180.f, 180u)});
        }

        //
        // Devilcat: draw book
        if (cat.type == CatType::Devil)
        {
            batchToUse.add(
                sf::Sprite{.position    = catAnchor + sf::Vector2f{10.f, 20.f},
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
                sf::Sprite{.position = cat.pawPosition + (beingDragged ? sf::Vector2f{-6.f, 6.f} : sf::Vector2f{4.f, 2.f}),
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
            batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{335.f, -65.f} + pushDown),
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
            if (cat.type == CatType::Normal && pt.perm.smartCatsPurchased)
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
                    sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vector2f{-131.f, -365.f}),
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

                batchToUse.add(sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vector2f{-221.f, 25.f}),
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
            if (cat.type == CatType::Normal && pt.perm.smartCatsPurchased) // Smart cat diploma
            {
                batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{295.f, 355.f} + pushDown),
                                          .scale       = catScale,
                                          .origin      = {23.f, 150.f},
                                          .rotation    = tailWiggleRotation,
                                          .textureRect = txrSmartCatDiploma,
                                          .color       = catColor});
            }
            else if (cat.type == CatType::Astro && pt.perm.astroCatInspirePurchased) // Astro cat flag
            {
                batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{395.f, 225.f} + pushDown),
                                          .scale       = catScale,
                                          .origin      = {98.f, 330.f},
                                          .rotation    = tailWiggleRotation,
                                          .textureRect = txrAstroCatFlag,
                                          .color       = catColor});
            }
            else if (cat.type == CatType::Engi ||
                     (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Engi)) // Engi cat wrench
            {
                batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{295.f, 385.f} + pushDown),
                                          .scale       = catScale,
                                          .origin      = {36.f, 167.f},
                                          .rotation    = tailWiggleRotation,
                                          .textureRect = txrEngiCatWrench,
                                          .color       = catColor});
            }
            else if (cat.type == CatType::Attracto ||
                     (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Attracto)) // Attracto cat magnet
            {
                batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{190.f, 315.f} + pushDown),
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
                const auto originOffset = cat.type == CatType::Uni ? sf::Vector2f{250.f, 0.f} : sf::Vector2f{0.f, 0.f};
                const auto offset = cat.type == CatType::Uni ? sf::Vector2f{-130.f, 405.f} : sf::Vector2f{0.f, 0.f};

                batchToUse.add(
                    sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vector2f{475.f, 240.f} + offset + originOffset),
                               .scale       = catScale,
                               .origin      = originOffset + sf::Vector2f{320.f, 32.f},
                               .rotation    = tailWiggleRotation,
                               .textureRect = catTailTxr,
                               .color       = catColor});
            }

            //
            // Mousecat: mouse
            if (cat.type == CatType::Mouse || (cat.type == CatType::Copy && pt.copycatCopiedCatType == CatType::Mouse))
            {
                batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{-275.f, -15.f}),
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
                batchToUse.add(sf::Sprite{.position    = anchorOffset(catTailOffset + sf::Vector2f{-185.f, -185.f}),
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
                    sf::Sprite{.position = anchorOffset(catTailOffset + sf::Vector2f{-185.f, -185.f}),
                               .scale    = catScale,
                               .origin   = txrCatEyeLid0.size / 2.f,
                               .rotation = sf::radians(catRotation),
                               .textureRect = *eyelidArray[static_cast<unsigned int>(cat.blinkAnimCountdown.value / 75.f) % nEyeLidRects],
                               .color = attachmentHue});
            }

            if (cat.type == CatType::Normal && pt.perm.geniusCatsPurchased)
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
                    sf::Sprite{.position = cat.pawPosition +
                                           (beingDragged ? sf::Vector2f{-12.f, 12.f} : sf::Vector2f{0.f, 0.f}),
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
                    if (pt.copycatCopiedCatType == CatType::Witch)
                        return &txrCCMaskWitch;

                    if (pt.copycatCopiedCatType == CatType::Wizard)
                        return &txrCCMaskWizard;

                    if (pt.copycatCopiedCatType == CatType::Mouse)
                        return &txrCCMaskMouse;

                    if (pt.copycatCopiedCatType == CatType::Engi)
                        return &txrCCMaskEngi;

                    if (pt.copycatCopiedCatType == CatType::Repulso)
                        return &txrCCMaskRepulso;

                    if (pt.copycatCopiedCatType == CatType::Attracto)
                        return &txrCCMaskAttracto;

                    return nullptr;
                }();

                if (txrMaskToUse != nullptr)
                    batchToUse.add(sf::Sprite{.position    = anchorOffset(sf::Vector2f{265.f, 115.f}),
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

            if (pt.perm.smartCatsPurchased && cat.type == CatType::Normal && cat.nameIdx % 2u == 0u)
                catNameBuffer += "Dr. ";

            catNameBuffer += shuffledCatNamesPerType[asIdx(cat.type)][cat.nameIdx];

            if (pt.perm.smartCatsPurchased && cat.type == CatType::Normal && cat.nameIdx % 2u != 0u)
                catNameBuffer += ", PhD";

            // Name text
            textNameBuffer.setString(catNameBuffer);
            textNameBuffer.position = cat.position.addY(48.f);
            textNameBuffer.origin   = textNameBuffer.getLocalBounds().size / 2.f;
            textNameBuffer.scale    = sf::Vector2f{0.5f, 0.5f} * catScaleMult;
            textNameBuffer.setOutlineColor(textOutlineColor);
            textBatchToUse.add(textNameBuffer);

            // Status text
            if (cat.type != CatType::Repulso && cat.type != CatType::Attracto && cat.type != CatType::Duck &&
                !isCopyCatWithType(CatType::Repulso) && !isCopyCatWithType(CatType::Attracto))
            {
                const char* actionName = CatConstants::actionNames[asIdx(
                    cat.type == CatType::Copy ? pt.copycatCopiedCatType : cat.type)];

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
                    actionString += std::to_string(pt.mouseCatCombo + 1);
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
                    .size               = sf::Vector2f{cat.cooldown.value / maxCooldown * 64.f, 3.f}.clampX(1.f, 64.f),
                    .cornerRadius       = 1.f,
                    .cornerPointCount   = 8u,
                });
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawShrines(const sf::Vector2f mousePos)
    {
        Shrine* hoveredShrine = nullptr;

        for (Shrine& shrine : pt.shrines)
        {
            U8 rangeInnerAlpha = 0u;

            if (hoveredShrine == nullptr && (mousePos - shrine.position).lengthSquared() <= shrine.getRadiusSquared() &&
                !mBtnDown(getLMB(), /* penetrateUI */ true))
            {
                hoveredShrine   = &shrine;
                rangeInnerAlpha = 75u;

                if (!pt.shrineHoverTipShown)
                {
                    pt.shrineHoverTipShown = true;

                    if (pt.psvBubbleValue.nPurchases == 0u)
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

            const auto circleColor        = sf::Color{231u, 198u, 39u}.withHueMod(shrine.getHue()).withLightness(0.75f);
            const auto circleOutlineColor = circleColor.withAlpha(rangeInnerAlpha);
            const auto textOutlineColor   = circleColor.withLightness(0.25f);

            cpuDrawableBatch.add(
                sf::Sprite{.position = shrine.getDrawPosition(),
                           .scale    = sf::Vector2f{0.3f, 0.3f} * invDeathProgress +
                                    sf::Vector2f{1.25f, 1.25f} * shrine.textStatusShakeEffect.grow * 0.015f,
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
            textNameBuffer.scale    = sf::Vector2f{0.5f, 0.5f} * invDeathProgress;
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
                shrineStatus += toStringWithSeparators(pt.getComputedRequiredRewardByShrineType(shrine.type));

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

            if (pt.psvBubbleValue.nPurchases == 0u)
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
    void gameLoopDrawDolls(const sf::Vector2f mousePos)
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
                               .scale       = sf::Vector2f{0.22f, 0.22f} * progress,
                               .origin      = dollTxr.size / 2.f,
                               .rotation    = sf::radians(-0.15f + 0.3f * sf::base::sin(doll.wobbleRadians / 2.f)),
                               .textureRect = dollTxr,
                               .color       = hueColor(doll.hue + hueMod, dollAlpha)});
            }
        };

        processDolls(pt.dolls, /* hueMod */ 0.f);
        processDolls(pt.copyDolls, /* hueMod */ 180.f);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawHellPortals()
    {
        const float hellPortalRadius = pt.getComputedRangeByCatType(CatType::Devil);

        for (HellPortal& hp : pt.hellPortals)
        {
            const float scaleMult = //
                (hp.life.value > 1500.f)  ? easeOutBack(remap(hp.life.value, 1500.f, 1750.f, 1.f, 0.f))
                : (hp.life.value < 250.f) ? easeOutBack(remap(hp.life.value, 0.f, 250.f, 0.f, 1.f))
                                          : 1.f;

            cpuDrawableBatch.add(
                sf::Sprite{.position    = hp.getDrawPosition(),
                           .scale       = sf::Vector2f{1.f, 1.f} * scaleMult * hellPortalRadius / 256.f * 1.15f,
                           .origin      = txrHellPortal.size / 2.f,
                           .rotation    = sf::radians(hp.life.value / 200.f),
                           .textureRect = txrHellPortal,
                           .color       = sf::Color::White});
        }
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vector2f getViewCenter() const
    {
        return {sf::base::clamp(gameScreenSize.x / 2.f + actualScroll * 2.f,
                                gameScreenSize.x / 2.f,
                                boundaries.x - gameScreenSize.x / 2.f),
                gameScreenSize.y / 2.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vector2f getViewCenterWithoutScroll() const
    {
        return {gameScreenSize.x / 2.f, gameScreenSize.y / 2.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] CullingBoundaries getViewCullingBoundaries(const float offset) const
    {
        const sf::Vector2f viewCenter{getViewCenter()};

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

        const auto bezier = [](const sf::Vector2f& start, const sf::Vector2f& end, const float t)
        {
            const sf::Vector2f control(start.x, end.y);
            const float        u = 1.f - t;

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
                .scale       = sf::Vector2f{0.25f, 0.25f} * opacityScale,
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

        rtGame->draw(txArrow,
                     {.position = {gameScreenSize.x - 15.f, 15.f + (gameScreenSize.y / 5.f) * 1.f},
                      .origin   = txArrow.getRect().getCenterRight(),
                      .color    = sf::Color::whiteMask(static_cast<U8>(blinkOpacity))});

        rtGame->draw(txArrow,
                     {.position = {gameScreenSize.x - 15.f, gameScreenSize.y - 15.f - (gameScreenSize.y / 5.f) * 1.f},
                      .origin   = txArrow.getRect().getCenterRight(),
                      .color    = sf::Color::whiteMask(static_cast<U8>(blinkOpacity))});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawImGui(const sf::base::U8 shouldDrawUIAlpha)
    {
        if (profile.enableNotifications)
            ImGui::RenderNotifications(/* paddingY */ (profile.showDpsMeter ? (15.f + 60.f + 15.f) : 15.f) * profile.uiScale,
                                       [&]
            {
                ImGui::PushFont(fontImGuiMouldyCheese);
                uiSetFontScale(uiToolTipFontScale);
            },
                                       [&]
            {
                uiSetFontScale(uiNormalFontScale);
                ImGui::PopFont();
            });

        if (rtImGui.hasValue())
        {
            imGuiContext.setCurrentWindow(*optWindow);

            rtImGui->setView(scaledHUDView);
            rtImGui->clear(sf::Color::Transparent);
            imGuiContext.render(*rtImGui);
            rtImGui->display();

            rtGame->draw(rtImGui->getTexture(),
                         {.scale = {1.f / profile.hudScale, 1.f / profile.hudScale},
                          .color = hueColor(currentBackgroundHue.asDegrees(), shouldDrawUIAlpha)},
                         {.shader = &shader});
        }
    }

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

                const auto pos = sf::Vector2f{uiGetWindowPos().x + x,
                                              y + (14.f + rngFast.getF(-14.f, 14.f)) * profile.uiScale};

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

                rtGame->draw(tx,
                             {.position = {uiGetWindowPos().x, y + 14.f * profile.uiScale},
                              .scale = sf::Vector2f{0.25f, 0.25f} * (profile.uiScale + -0.15f * easeInOutBack(blinkProgress)),
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
        auto& window = *optWindow;

        const sf::Vector2i windowSpaceMousePos = sf::Mouse::getPosition(window);

        const bool mouseNearWindowEdges = windowSpaceMousePos.x < 4 || windowSpaceMousePos.y < 4 ||
                                          windowSpaceMousePos.x > static_cast<int>(window.getSize().x) - 4 ||
                                          windowSpaceMousePos.y > static_cast<int>(window.getSize().y) - 4;

        window.setMouseCursorVisible(!profile.highVisibilityCursor || mouseNearWindowEdges);

        if (!profile.highVisibilityCursor)
            return;

        if (profile.multicolorCursor)
            profile.cursorHue += deltaTimeMs * 0.5f;

        profile.cursorHue = wrapHue(profile.cursorHue);

        rtGame->draw(shouldDrawGrabbingCursor() ? txCursorGrab
                     : pt.laserPopEnabled       ? txCursorLaser
                     : pt.multiPopEnabled       ? txCursorMultipop
                                                : txCursor,
                     {.position = sf::Mouse::getPosition(window).toVector2f(),
                      .scale    = sf::Vector2f{profile.cursorScale, profile.cursorScale} *
                               ((1.f + easeInOutBack(cursorGrow) * std::pow(static_cast<float>(combo), 0.09f)) *
                                dpiScalingFactor),
                      .origin = {5.f, 5.f},
                      .color  = hueColor(profile.cursorHue + currentBackgroundHue.asDegrees(), 255u)},
                     {.shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursorComboText(const float deltaTimeMs, const float cursorGrow)
    {
        if (!pt.comboPurchased || !profile.showCursorComboText || shouldDrawGrabbingCursor())
            return;

        static float alpha = 0.f;

        auto&       window    = *optWindow;
        const float scaleMult = profile.cursorScale * dpiScalingFactor;

        if (combo >= 1)
            alpha = 255.f;
        else if (alpha > 0.f)
            alpha -= deltaTimeMs * 0.5f;

        const auto alphaU8 = static_cast<U8>(sf::base::clamp(alpha, 0.f, 255.f));

        cursorComboText.position = sf::Mouse::getPosition(window).toVector2f() + sf::Vector2f{30.f, 48.f} * scaleMult;

        cursorComboText.setFillColor(sf::Color::blackMask(alphaU8));
        cursorComboText.setOutlineColor(
            sf::Color{111u, 170u, 244u, alphaU8}.withHueMod(profile.cursorHue + currentBackgroundHue.asDegrees()));

        if (combo > 0)
            cursorComboText.setString("x" + std::to_string(combo + 1));

        comboTextShakeEffect.applyToText(cursorComboText);

        cursorComboText.scale *= (static_cast<float>(combo) * 0.65f) * cursorGrow * 0.3f;
        cursorComboText.scale += {0.85f, 0.85f};
        cursorComboText.scale += sf::Vector2f{1.f, 1.f} * comboFailCountdown.value / 325.f;
        cursorComboText.scale *= scaleMult;

        const auto minScale = sf::Vector2f{0.25f, 0.25f} + sf::Vector2f{0.25f, 0.25f} * comboFailCountdown.value / 125.f;

        cursorComboText.scale = cursorComboText.scale.componentWiseClamp(minScale, {1.5f, 1.5f});

        if (comboFailCountdown.value > 0.f)
        {
            cursorComboText.position += rngFast.getVec2f({-5.f, -5.f}, {5.f, 5.f});
            cursorComboText.setFillColor(sf::Color::Red.withAlpha(alphaU8));
        }

        rtGame->draw(cursorComboText, {.shader = &shader});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopDrawCursorComboBar()
    {
        if (!pt.comboPurchased || !profile.showCursorComboBar || comboCountdown.value == 0.f || shouldDrawGrabbingCursor())
            return;

        auto&       window    = *optWindow;
        const float scaleMult = profile.cursorScale * dpiScalingFactor;

        const auto cursorComboBarPosition = sf::Mouse::getPosition(window).toVector2f() + sf::Vector2f{52.f, 14.f} * scaleMult;

        rtGame->draw(sf::RectangleShapeData{
            .position           = cursorComboBarPosition,
            .outlineTextureRect = txrWhiteDot,
            .fillColor          = sf::Color::blackMask(80u),
            .outlineColor       = cursorComboText.getOutlineColor(),
            .outlineThickness   = 1.f,
            .size = {64.f * scaleMult * pt.psvComboStartTime.currentValue() * 1000.f / 700.f, 24.f * scaleMult},
        });

        rtGame->draw(sf::RectangleShapeData{
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
    [[nodiscard]] sf::FloatRect getViewportPixelBounds(const sf::View& view, const sf::Vector2f targetSize) const
    {
        return {{view.viewport.position.x * targetSize.x, view.viewport.position.y * targetSize.y},
                {view.viewport.size.x * targetSize.x, view.viewport.size.y * targetSize.y}};
    }

    ////////////////////////////////////////////////////////////
    // Returns a random position along the edges of the provided bounds.
    [[nodiscard]] sf::Vector2f getEdgeSpawnPosition(const sf::FloatRect& bounds, const float thickness)
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
            const sf::Vector2f  spawnPos       = getEdgeSpawnPosition(gameViewBounds, 10.f);

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
                                       .scale = sf::Vector2f{0.4f, 0.4f} + sf::Vector2f{0.4f, 0.4f} * easeInOutBack(bgProgress),
                                       .origin      = txTipBg.getSize().toVector2f() / 2.f,
                                       .textureRect = txTipBg.getRect(),
                                       .color = sf::Color::whiteMask(static_cast<U8>(tipBackgroundAlpha * 0.85f))};


        SFML_BASE_ASSERT(profile.hudScale > 0.f);

        tipBackgroundSprite.setBottomCenter(
            {getResolution().x / 2.f / profile.hudScale, getResolution().y / profile.hudScale - 50.f});

        rtGame->draw(tipBackgroundSprite, {.texture = &txTipBg});

        sf::Sprite tipByteSprite{.position    = {},
                                 .scale       = sf::Vector2f{0.85f, 0.85f} * easeInOutBack(byteProgress),
                                 .origin      = txTipByte.getSize().toVector2f() / 2.f,
                                 .rotation    = sf::radians(sf::base::tau * easeInOutBack(byteProgress)),
                                 .textureRect = txTipByte.getRect(),
                                 .color       = sf::Color::whiteMask(static_cast<U8>(tipByteAlpha))};

        tipByteSprite.setCenter(tipBackgroundSprite.getCenterRight().addY(-40.f));
        rtGame->draw(tipByteSprite, {.texture = &txTipByte});

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
                sounds.byteSpeak.setPitch(1.6f);
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
                          .scale            = sf::Vector2f{0.5f, 0.5f} * easeInOutBack(byteProgress),
                          .string           = tipString.substr(0, tipCharIdx),
                          .characterSize    = 60u,
                          .fillColor        = sf::Color::whiteMask(static_cast<sf::base::U8>(tipByteAlpha)),
                          .outlineColor     = outlineHueColor.withAlpha(static_cast<sf::base::U8>(tipByteAlpha)),
                          .outlineThickness = 4.f}};

        tipText.setTopLeft(tipBackgroundSprite.getTopLeft() + sf::Vector2f{45.f, 65.f});

        tipStringWiggle.advance(deltaTimeMs);
        tipStringWiggle.apply(tipText);

        rtGame->draw(tipText);

        tipStringWiggle.unapply(tipText);
    }

    ////////////////////////////////////////////////////////////
    void recreateImGuiRenderTexture(const sf::Vector2u newResolution)
    {
        rtImGui.emplace(
            sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value());
    }

    ////////////////////////////////////////////////////////////
    void recreateGameRenderTexture(const sf::Vector2u newResolution)
    {
        rtGame.emplace(
            sf::RenderTexture::create(newResolution, {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value());
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool gameLoopRecreateWindowIfNeeded()
    {
        // TODO P2: (lib) all this stuff shouldn't be needed, also it creates a brand new opengl context every time
        // TODO P2: (lib) need test where glcontext 1 is active, rendertexture is created, then glcontext 1 is destroyed, and variations

        if (!mustRecreateWindow)
            return true;

        mustRecreateWindow = false;

        const sf::Vector2u newResolution = profile.resWidth == sf::Vector2u{} ? getReasonableWindowSize(0.9f) : profile.resWidth;

        const bool takesAllScreen = newResolution == sf::VideoModeUtils::getDesktopMode().size;

        rtBackground.reset(); // TODO P2: (lib) workaround to unregister framebuffers
        rtImGui.reset();      // TODO P2: (lib) workaround to unregister framebuffers
        rtGame.reset();       // TODO P2: (lib) workaround to unregister framebuffers

        optWindow.emplace(
            sf::WindowSettings{.size            = newResolution,
                               .title           = "BubbleByte " BUBBLEBYTE_VERSION_STR,
                               .fullscreen      = !profile.windowed,
                               .resizable       = !takesAllScreen,
                               .closable        = !takesAllScreen,
                               .hasTitlebar     = !takesAllScreen,
                               .vsync           = profile.vsync,
                               .frametimeLimit  = sf::base::clamp(profile.frametimeLimit, 60u, 144u),
                               .contextSettings = contextSettings});

        rtBackground.emplace(
            sf::RenderTexture::create(gameScreenSize.toVector2u(), {.antiAliasingLevel = aaLevel, .sRgbCapable = false}).value());

        recreateImGuiRenderTexture(newResolution);
        recreateGameRenderTexture(newResolution);

        dpiScalingFactor = optWindow->getWindowDisplayScale();

        static bool imguiInit = false;
        if (!imguiInit)
        {
            imguiInit = true;

            if (!imGuiContext.init(*optWindow))
            {
                sf::cOut() << "Error: ImGui context initialization failed\n";
                return false;
            }

            fontImGuiSuperBakery  = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/superbakery.ttf", 26.f);
            fontImGuiMouldyCheese = ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/mouldycheese.ttf", 26.f);
        }

        return true;
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
    void gameLoopUpdateCombo(const float                            deltaTimeMs,
                             const bool                             anyBubblePoppedByClicking,
                             const sf::Vector2f                     mousePos,
                             const sf::base::Optional<sf::Vector2f> clickPosition)
    {
        // Mousecat combo
        checkComboEnd(deltaTimeMs, pt.mouseCatCombo, pt.mouseCatComboCountdown);

        // Combo failure countdown for red text effect
        (void)comboFailCountdown.updateAndStop(deltaTimeMs);

        // Player combo data
        const auto playerLastCombo      = combo;
        bool       playerJustEndedCombo = false;

        // Player combo failure due to timer end
        if (checkComboEnd(deltaTimeMs, combo, comboCountdown))
            playerJustEndedCombo = true;


        // Player combo failure due to missed click
        if (!anyBubblePoppedByClicking && clickPosition.hasValue() && !pt.laserPopEnabled)
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

                    sounds.coindelay.setPosition({getViewCenter().x - gameScreenSize.x / 2.f + 25.f,
                                                  getViewCenter().y - gameScreenSize.y / 2.f + 25.f});
                    sounds.coindelay.setPitch(0.8f + static_cast<float>(iComboAccReward) * 0.04f);
                    sounds.coindelay.setVolume(profile.sfxVolume / 100.f);

                    playSound(sounds.coindelay, /* maxOverlap */ 64);
                }
            }

            if (iComboAccStarReward < comboAccStarReward &&
                accComboStarDelay.updateAndLoop(deltaTimeMs, 75.f) == CountdownStatusLoop::Looping)
            {
                ++iComboAccStarReward;

                sounds.shine3.setPosition({mousePos.x, mousePos.y});
                sounds.shine3.setPitch(0.75f + static_cast<float>(iComboAccStarReward) * 0.075f);
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
            handleBubbleCollision(deltaTimeMs, pt.bubbles[bubbleIdxI], pt.bubbles[bubbleIdxJ]);
        };

        const sf::base::SizeT nWorkers = threadPool.getWorkerCount();
        sweepAndPrune.forEachUniqueIndexPair(nWorkers, threadPool, func);
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCollisionsCatCat(const float deltaTimeMs)
    {
        for (SizeT i = 0u; i < pt.cats.size(); ++i)
            for (SizeT j = i + 1; j < pt.cats.size(); ++j)
            {
                Cat& iCat = pt.cats[i];
                Cat& jCat = pt.cats[j];

                if (isCatBeingDragged(iCat) || isCatBeingDragged(jCat))
                    continue;

                const auto applyAstroInspireAndIgnore = [this](Cat& catA, Cat& catB)
                {
                    if (!catA.isAstroAndInFlight())
                        return false;

                    if (pt.perm.astroCatInspirePurchased && catB.type != CatType::Astro &&
                        detectCollision(catA.position, catB.position, catA.getRadius(), catB.getRadius()))
                    {
                        catB.inspiredCountdown.value = pt.getComputedInspirationDuration();

                        pt.achAstrocatInspireByType[asIdx(catB.type)] = true;
                    }

                    return true;
                };

                if (applyAstroInspireAndIgnore(iCat, jCat))
                    continue;

                // NOLINTNEXTLINE(readability-suspicious-call-argument)
                if (applyAstroInspireAndIgnore(jCat, iCat))
                    continue;

                handleCatCollision(deltaTimeMs, pt.cats[i], pt.cats[j]);
            }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCollisionsCatShrine(const float deltaTimeMs)
    {
        for (Cat& cat : pt.cats)
        {
            if (cat.isAstroAndInFlight())
                continue;

            if (isCatBeingDragged(cat))
                continue;

            for (Shrine& shrine : pt.shrines)
                handleCatShrineCollision(deltaTimeMs, cat, shrine);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCollisionsCatDoll()
    {
        const auto checkCollisionWithDoll = [&](Doll& d, auto collectFn)
        {
            for (Cat& cat : pt.cats)
            {
                if (!cat.isAstroAndInFlight())
                    continue;

                if (pt.perm.witchCatBuffOrbitalDolls && d.isActive() && !d.tcDeath.hasValue() &&
                    detectCollision(cat.position, d.position, cat.getRadius(), d.getRadius()))
                {
                    collectFn(d);
                }
            }
        };

        for (Doll& doll : pt.dolls)
            checkCollisionWithDoll(doll, [&](Doll& d) { collectDoll(d); });

        for (Doll& copyDoll : pt.copyDolls)
            checkCollisionWithDoll(copyDoll, [&](Doll& d) { collectCopyDoll(d); });
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateCollisionsBubbleHellPortal()
    {
        if (!frameProcThisFrame)
            return;

        const float hellPortalRadius        = pt.getComputedRangeByCatType(CatType::Devil) * 1.25f;
        const float hellPortalRadiusSquared = hellPortalRadius * hellPortalRadius;

        for (HellPortal& hellPortal : pt.hellPortals)
        {
            Cat* linkedCat = hellPortal.catIdx < pt.cats.size() ? &pt.cats[hellPortal.catIdx] : nullptr;

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

        const bool anyShrineDying = sf::base::anyOf(pt.shrines.begin(),
                                                    pt.shrines.end(),
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
    void gameLoopUpdateSounds(const float deltaTimeMs, const sf::Vector2f mousePos)
    {
#ifndef BUBBLEBYTE_NO_AUDIO
        const float volumeMult = profile.playAudioInBackground || getWindow().hasFocus() ? 1.f : 0.f;

        listener.position = {sf::base::clamp(mousePos.x, 0.f, pt.getMapLimit()),
                             sf::base::clamp(mousePos.y, 0.f, boundaries.y),
                             0.f};

        listener.volume = profile.masterVolume / 100.f * volumeMult;

        (void)playbackDevice.updateListener(listener);

        auto& optCurrentMusic = getCurrentBGMBuffer();
        auto& optNextMusic    = getNextBGMBuffer();

        if (!bgmTransition.isDone())
        {
            SFML_BASE_ASSERT(optNextMusic.hasValue());

            const auto processMusic = [&](sf::base::Optional<sf::Music>& optMusic, const float transitionMult)
            {
                if (!optMusic.hasValue())
                    return;

                optMusic->setPosition(listener.position);
                optMusic->setVolume(profile.musicVolume / 100.f * volumeMult * transitionMult);

                if (sounds.countPlayingPooled(sounds.prestige) > 0u)
                    optMusic->setVolume(0.f);
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
            const auto processMusic = [&](sf::base::Optional<sf::Music>& optMusic)
            {
                if (!optMusic.hasValue())
                    return;

                optMusic->setPosition(listener.position);
                optMusic->setVolume(profile.musicVolume / 100.f * volumeMult);

                if (sounds.countPlayingPooled(sounds.prestige) > 0u)
                    optMusic->setVolume(0.f);
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
        autosaveUsAccumulator += elapsedUs;

        if (autosaveUsAccumulator >= 180'000'000) // 3 min
        {
            autosaveUsAccumulator = 0;

            sf::cOut() << "Autosaving...\n";
            savePlaythroughToFile(pt);
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
        const auto  sz       = txFixedBg.getSize().toVector2f();

        // Result of linear regression and trial-and-error >:3
        const float fixedBgOffsetX = 1648.f * ratio - 3216.62f;

        rtGame->draw(txFixedBg,
                     {
                         .position = {sz.x + resolution.x / 2.f - actualScroll / 20.f - fixedBgX + fixedBgOffsetX, sz.y},
                         .scale       = {ratio, ratio},
                         .origin      = {sz.x / 2.f, sz.y / 1.5f},
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

        if (!rtBackground.hasValue())
            return;

        rtBackground->clear(outlineHueColor);

        rtBackground->setView(gameBackgroundView);
        rtBackground->setWrapMode(sf::TextureWrapMode::Repeat); // TODO P2: (lib) add RenderTextureCreateSettings

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
        outlineHueColor = colorBlueOutline.withHueMod(currentBackgroundHue.asDegrees());

        rtBackground->draw(*chunkTx[idx],
                           {
                               .scale       = {0.5f, 0.5f},
                               .textureRect = {{actualScroll + backgroundScroll * 0.25f, 0.f},
                                               txBackgroundChunk.getSize().toVector2f() * 2.f},
                               .color       = hueColor(currentBackgroundHue.asDegrees(), getAlpha(255.f)),
                           },
                           {.shader = &shader});

        if (idx == 0u || profile.alwaysShowDrawings)
            rtBackground->draw(txDrawings,
                               {
                                   .textureRect = {{actualScroll * 2.f, 0.f}, txBackgroundChunk.getSize().toVector2f() * 2.f},
                                   .color = sf::Color::whiteMask(getAlpha(200.f)),
                               });

        rtBackground->draw(*detailTx[idx],
                           {
                               .scale       = {0.75f, 0.75f},
                               .textureRect = {{actualScroll * 2.f + backgroundScroll * 0.5f, 0.f},
                                               txBackgroundChunk.getSize().toVector2f() * 1.5f},
                               .color       = sf::Color::whiteMask(getAlpha(175.f)),
                           });

        rtBackground->draw(txClouds,
                           {
                               .scale       = {1.25f, 1.25f},
                               .textureRect = {{actualScroll * 4.f + backgroundScroll * 3.f, 0.f},
                                               txBackgroundChunk.getSize().toVector2f()},
                               .color       = sf::Color::whiteMask(getAlpha(128.f)),
                           });

        rtBackground->display();

        auto gameViewNoScroll   = gameView;
        gameViewNoScroll.center = getViewCenterWithoutScroll();

        rtGame->setView(gameViewNoScroll);
        rtGame->draw(rtBackground->getTexture(), {.textureRect{{0.f, 0.f}, gameScreenSize}});
    }

    ////////////////////////////////////////////////////////////
    void gameLoopUpdateMoneyText(const float deltaTimeMs, const float yBelowMinimap)
    {
        moneyText.setString("$" + std::string(toStringWithSeparators(pt.money + spentMoney)));

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
                 .velocity      = sf::Vector2f{3.f, 0.f},
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
        if (!pt.comboPurchased)
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

        const SizeT nDollsToClick = sf::base::countIf(pt.dolls.begin(),
                                                      pt.dolls.end(),
                                                      [](const Doll& doll) { return !doll.tcDeath.hasValue(); });

        const SizeT nCopyDollsToClick = sf::base::countIf(pt.copyDolls.begin(),
                                                          pt.copyDolls.end(),
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

        if (pt.mewltiplierAuraTimer > 0.f)
            writeIdx += static_cast<SizeT>(
                std::snprintf(buffStrBuffer + writeIdx,
                              sizeof(buffStrBuffer) - writeIdx,
                              "Mewltiplier Aura (x%.1f Any Reward): %.2fs\n",
                              static_cast<double>(pt.psvMewltiplierMult.currentValue()),
                              static_cast<double>(pt.mewltiplierAuraTimer / 1000.f)));

        if (pt.stasisFieldTimer > 0.f)
            writeIdx += static_cast<SizeT>(
                std::snprintf(buffStrBuffer + writeIdx,
                              sizeof(buffStrBuffer) - writeIdx,
                              "Stasis Field (Bubbles Stuck In Time): %.2fs\n",
                              static_cast<double>(pt.stasisFieldTimer / 1000.f)));

        for (SizeT i = 0u; i < nCatTypes; ++i)
        {
            const float buffTime = pt.buffCountdownsPerType[i].value;

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
        if (!wasPrestigeAvailableLastFrame && pt.canBuyNextPrestige())
        {
            pushNotification("Prestige available!", "Purchase through the \"Prestige\" menu!");

            if (pt.psvBubbleValue.nPurchases == 0u)
                doTip("You can now prestige for the first time!");
        }

        wasPrestigeAvailableLastFrame = pt.canBuyNextPrestige();
    }

    ////////////////////////////////////////////////////////////
    void gameLoopReminderBuyCombo()
    {
        if (pt.comboPurchased)
            return;

        const auto mult = pt.psvBubbleValue.nPurchases + 1u;
        const auto bias = pt.perm.starterPackPurchased ? 1000u : 0u;

        if (pt.money >= ((25u * mult) + bias) && buyReminder == 0)
        {
            buyReminder = 1;
            doTip("Remember to buy the combo upgrade!", /* maxPrestigeLevel */ UINT_MAX);
        }
        else if (pt.money >= ((50u * mult) + bias) && buyReminder == 1)
        {
            buyReminder = 2;
            doTip("You should really buy the upgrade now!", /* maxPrestigeLevel */ UINT_MAX);
        }
        else if (pt.money >= ((100u * mult) + bias) && buyReminder == 2)
        {
            buyReminder = 3;
            doTip("What are you trying to prove...?", /* maxPrestigeLevel */ UINT_MAX);
        }
        else if (pt.money >= ((200u * mult) + bias) && buyReminder == 3)
        {
            buyReminder = 4;
            doTip("There is no achievement for doing this!", /* maxPrestigeLevel */ UINT_MAX);
        }
        else if (pt.money >= ((300u * mult) + bias) && buyReminder == 4)
        {
            buyReminder = 5;
            doTip("Fine, have it your way!\nHere's your dumb achievement!\nAnd now buy the upgrade!",
                  /* maxPrestigeLevel */ UINT_MAX);
        }
    }

    ////////////////////////////////////////////////////////////
    void gameLoopReminderSpendPPs()
    {
        if (!pt.spendPPTipShown && pt.psvBubbleValue.nPurchases == 1u && pt.prestigePoints > 0u && pt.money > 500u)
        {
            pt.spendPPTipShown = true;
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
    void gameLoopUpdateNotificationQueue(const float deltaTimeMs)
    {
        if (tipTCByte.hasValue())
            return;

        if (notificationQueue.empty())
            return;

        if (notificationCountdown.updateAndIsActive(deltaTimeMs))
            return;

        notificationCountdown.restart();

        const auto& notification = notificationQueue.front();

        ImGuiToast toast{ImGuiToastType::None, 4500};
        toast.setTitle(notification.title);
        toast.setContent("%s", notification.content.c_str());

        ImGui::InsertNotification(toast);
        playSound(sounds.notification);

        // pop front
        notificationQueue.erase(notificationQueue.begin());
    }

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

        if (!gameLoopRecreateWindowIfNeeded())
            return false;

        auto& window = *optWindow;

        fps = 1.f / fpsClock.getElapsedTime().asSeconds();
        fpsClock.restart();

        sf::base::Optional<sf::Vector2f> clickPosition;

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
                fingerPositions[e0->finger].emplace(e0->position.toVector2f());

                if (!clickPosition.hasValue())
                    clickPosition.emplace(e0->position.toVector2f());
            }
            else if (const auto* e1 = event->getIf<sf::Event::TouchEnded>())
            {
                fingerPositions[e1->finger].reset();
            }
            else if (const auto* e2 = event->getIf<sf::Event::TouchMoved>())
            {
                fingerPositions[e2->finger].emplace(e2->position.toVector2f());

                if (pt.laserPopEnabled)
                    if (!clickPosition.hasValue())
                        clickPosition.emplace(e2->position.toVector2f());
            }
            else if (const auto* e3 = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (e3->button == getLMB())
                    clickPosition.emplace(e3->position.toVector2f());

                if (e3->button == getRMB() && !dragPosition.hasValue())
                {
                    clickPosition.reset();

                    dragPosition.emplace(e3->position.toVector2f());
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
                if (pt.mapPurchased && dragPosition.hasValue())
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
                    clickPosition.emplace(sf::Mouse::getPosition(window).toVector2f());
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

        //
        // TODO PO laser cursor
        if (pt.laserPopEnabled)
            if (keyDown(sf::Keyboard::Key::Z) || keyDown(sf::Keyboard::Key::X) || keyDown(sf::Keyboard::Key::Y) ||
                mBtnDown(getLMB(), /* penetrateUI */ false))
            {
                if (!clickPosition.hasValue())
                    clickPosition.emplace(sf::Mouse::getPosition(window).toVector2f());
            }

        //
        // Number of fingers
        std::vector<sf::Vector2f> downFingers;
        for (const auto maybeFinger : fingerPositions)
            if (maybeFinger.hasValue())
                downFingers.push_back(*maybeFinger);

        //
        // Map scrolling via keyboard and touch
        if (pt.mapPurchased)
        {
            // Jump to beginning/end of map
            if (inputHelper.wasKeyJustPressed(sf::Keyboard::Key::Home))
                scroll = 0.f;
            else if (inputHelper.wasKeyJustPressed(sf::Keyboard::Key::End))
                scroll = static_cast<float>(pt.getMapLimitIncreases()) * gameScreenSize.x * 0.5f;

            const auto currentScrollScreenIndex = static_cast<sf::base::SizeT>(
                sf::base::lround(scroll / (gameScreenSize.x * 0.5f)));

            // Jump to previous/next screen
            if (inputHelper.wasKeyJustPressed(sf::Keyboard::Key::PageDown) ||
                inputHelper.wasMouseButtonJustPressed(sf::Mouse::Button::Extra2))
            {
                const auto nextScrollScreenIndex = sf::base::min(currentScrollScreenIndex + 1u, pt.getMapLimitIncreases());
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
                    std::pair<sf::base::Optional<sf::Vector2f>, sf::base::Optional<sf::Vector2f>> result;

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
        const sf::Vector2f resolution = getResolution();

        hudCullingBoundaries      = {0.f, resolution.x, 0.f, resolution.y};
        particleCullingBoundaries = getViewCullingBoundaries(/* offset */ 0.f);
        bubbleCullingBoundaries   = getViewCullingBoundaries(/* offset */ -64.f);

        //
        // World-space mouse position
        const auto windowSpaceMouseOrFingerPos = downFingers.size() == 1u ? downFingers[0].toVector2i()
                                                                          : sf::Mouse::getPosition(window);

        const auto mousePos = window.mapPixelToCoords(windowSpaceMouseOrFingerPos, gameView);

        //
        // Game startup, prestige transitions, etc...
        gameLoopUpdateTransitions(deltaTimeMs);

        //
        // Update spatial partitioning (needs to be done before updating bubbles)
        sweepAndPrune.clear();
        sweepAndPrune.populate(pt.bubbles);

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
        // Update ImGui
        imGuiContext.update(window, deltaTime);

        //
        // Draw ImGui menu
        uiDraw(mousePos);

        //
        // Compute views
        const auto screenShake = profile.enableScreenShake ? rngFast.getVec2f({-screenShakeAmount, -screenShakeAmount},
                                                                              {screenShakeAmount, screenShakeAmount})
                                                           : sf::Vector2f{0.f, 0.f};

        nonScaledHUDView = {.center = resolution / 2.f, .size = resolution};
        scaledHUDView    = makeScaledHUDView(resolution, profile.hudScale);

        gameView                     = createScaledGameView(gameScreenSize, resolution);
        gameView.viewport.position.x = 0.f;
        gameView.center              = getViewCenter() + screenShake;

        scaledTopGameView                     = createScaledTopGameView(gameScreenSize, resolution);
        scaledTopGameView.viewport.position.x = 0.f;

        {
            const float        scale      = getAspectRatioScalingFactor(gameScreenSize, resolution);
            const sf::Vector2f scaledSize = gameScreenSize * scale;

            gameBackgroundView                     = createScaledGameView(gameScreenSize, scaledSize);
            gameBackgroundView.viewport.position.x = 0.f;
            gameBackgroundView.center              = getViewCenterWithoutScroll() + screenShake;
        }

        //
        // Clear window
        rtGame->clear(outlineHueColor);

        //
        // Underlying menu background
        rtGame->setView(nonScaledHUDView);
        gameLoopUpdateAndDrawFixedMenuBackground(deltaTimeMs, elapsedUs);

        //
        // Game background
        gameLoopUpdateAndDrawBackground(deltaTimeMs);

        //
        // Draw bubbles (separate batch to avoid showing in minimap and for shader support)
        rtGame->setView(gameView);
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
        if (pt.multiPopEnabled && draggedCats.empty())
        {
            const auto range = pt.psvPPMultiPopRange.currentValue() * 0.9f;

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
        rtGame->draw(cpuDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame->draw(catTextDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});

        //
        // Scroll arrow hint
        gameLoopDrawScrollArrowHint(deltaTimeMs);

        //
        // AoE Dragging Reticle
        if (const auto dragRect = getAoEDragRect(mousePos); dragRect.hasValue())
            rtGame->draw(sf::RectangleShapeData{.position         = dragRect->position,
                                                .origin           = {0.f, 0.f},
                                                .fillColor        = sf::Color::whiteMask(64u),
                                                .outlineColor     = sf::Color::whiteMask(176u),
                                                .outlineThickness = 4.f,
                                                .size             = dragRect->size});

        //
        // Draw border around gameview
        rtGame->setView(nonScaledHUDView);

        // Bottom-level hud particles
        if (shouldDrawUI)
        {
            hudBottomDrawableBatch.clear();
            gameLoopDrawHUDBottomParticles();
            rtGame->draw(hudBottomDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        }

        rtGame->draw(sf::RectangleShapeData{.position         = gameView.viewport.position.componentWiseMul(resolution),
                                            .fillColor        = sf::Color::Transparent,
                                            .outlineColor     = outlineHueColor,
                                            .outlineThickness = 4.f,
                                            .size             = gameView.viewport.size.componentWiseMul(resolution)});

        rtGame->setView(scaledHUDView);

        if (shouldDrawUI)
        {
            hudDrawableBatch.clear();

            if (!debugHideUI)
                gameLoopDrawHUDParticles();

            gameLoopDrawEarnedCoinParticles();
            rtGame->draw(hudDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        }

        //
        // Y coordinate below minimap to position money, combo, and buff texts
        const float yBelowMinimap = pt.mapPurchased ? (boundaries.y / profile.minimapScale) + 12.f : 0.f;

        //
        // Money text & spent money effect
        gameLoopUpdateMoneyText(deltaTimeMs, yBelowMinimap);
        gameLoopUpdateSpentMoneyEffect(deltaTimeMs); // handles both text smoothly doing down and particles
        if (!debugHideUI)
        {
            moneyText.setFillColorAlpha(shouldDrawUIAlpha);
            moneyText.setOutlineColorAlpha(shouldDrawUIAlpha);
            rtGame->draw(moneyText);
        }

        //
        // Combo text
        gameLoopUpdateComboText(deltaTimeMs, yBelowMinimap);
        if (!debugHideUI && pt.comboPurchased)
        {
            comboText.setFillColorAlpha(shouldDrawUIAlpha);
            comboText.setOutlineColorAlpha(shouldDrawUIAlpha);
            rtGame->draw(comboText);
        }

        //
        // Portal storm buff
        if (isDevilcatHellsingedActive() && pt.buffCountdownsPerType[asIdx(CatType::Devil)].value > 0.f)
        {
            if (portalStormTimer.updateAndLoop(deltaTimeMs, 10.f) == CountdownStatusLoop::Looping &&
                rng.getF(0.f, 100.f) <= pt.psvPPDevilRitualBuffPercentage.currentValue())
            {
                const float offset = 64.f;
                const auto portalPos = rng.getVec2f({offset, offset}, {pt.getMapLimit() - offset, boundaries.y - offset});

                pt.hellPortals.push_back({
                    .position = portalPos,
                    .life     = Countdown{.value = 1750.f},
                    .catIdx   = 100'000u, // invalid
                });

                sounds.makeBomb.setPosition({portalPos.x, portalPos.y});
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
            rtGame->draw(buffText);
        }

        //
        // Combo bar
        if (shouldDrawUI && !debugHideUI)
            rtGame->draw(sf::RectangleShapeData{.position  = {comboText.getCenterRight().x + 3.f, yBelowMinimap + 56.f},
                                                .fillColor = sf::Color{255, 255, 255, 75},
                                                .size      = {100.f * comboCountdown.value / 700.f, 20.f}});

        //
        // Minimap
        if (!debugHideUI && pt.mapPurchased)
        {
            drawMinimap(shader,
                        profile.minimapScale,
                        pt.getMapLimit(),
                        gameView,
                        scaledHUDView,
                        *rtGame,
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
            const auto p = getWindow().mapPixelToCoords(windowSpaceMouseOrFingerPos, scaledHUDView);
            if (minimapRect.contains(p) && mBtnDown(sf::Mouse::Button::Left, /* penetrateUI */ true))
            {
                const auto minimapPos = p - minimapRect.position;
                scroll = minimapPos.x * 0.5f * pt.getMapLimit() / minimapRect.size.x - gameScreenSize.x * 0.25f;
            }
        }

        //
        // UI and Toasts
        gameLoopDrawImGui(shouldDrawUIAlpha);

        //
        // Draw cats on top of UI
        rtGame->setView(scaledTopGameView);
        rtGame->draw(cpuTopDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        rtGame->draw(catTextTopDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});

        //
        // Purchase unlocked/available effects
        rtGame->setView(nonScaledHUDView);

        if (shouldDrawUI)
            gameLoopUpdatePurchaseUnlockedEffects(deltaTimeMs);

        // Top-level hud particles
        if (shouldDrawUI)
        {
            hudTopDrawableBatch.clear();
            gameLoopDrawHUDTopParticles();
            rtGame->draw(hudTopDrawableBatch, {.texture = &textureAtlas.getTexture(), .shader = &shader});
        }

        //
        // High visibility cursor
        rtGame->setView(nonScaledHUDView);
        gameLoopDrawCursor(deltaTimeMs, cursorGrow);
        gameLoopDrawCursorComboText(deltaTimeMs, cursorGrow);
        gameLoopDrawCursorComboBar();

        //
        // Splash screen
        rtGame->setView(scaledHUDView);
        if (splashCountdown.value > 0.f)
            drawSplashScreen(*rtGame, txLogo, splashCountdown, resolution, profile.hudScale);

        //
        // Letter
        if (victoryTC.hasValue())
        {
            if (victoryTC->updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
            {
                cdLetterAppear.value = 4000.f;
                delayedActions.emplace_back(Countdown{.value = 4000.f}, [this] { playSound(sounds.paper); });
            }

            if (victoryTC->isDone())
            {
                if (cdLetterAppear.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                    cdLetterText.value = 10'000.f;

                const float progress = cdLetterAppear.getProgressBounced(4000.f);

                rtGame->draw(sf::Sprite{.position = resolution / 2.f / profile.hudScale,
                                        .scale = sf::Vector2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(progress)) /
                                                 profile.hudScale * 2.f,
                                        .origin      = txLetter.getSize().toVector2f() / 2.f,
                                        .textureRect = txLetter.getRect(),
                                        .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(progress) * 255.f))},
                             {.texture = &txLetter});
            }

            (void)cdLetterText.updateAndStop(deltaTimeMs);

            const float textProgress = cdLetterText.value > 9000.f ? remap(cdLetterText.value, 9000.f, 10'000.f, 1.f, 0.f)
                                       : cdLetterText.value < 1000.f ? cdLetterText.value / 1000.f
                                                                     : 1.f;

            rtGame->draw(sf::Sprite{.position = resolution / 2.f / profile.hudScale,
                                    .scale = sf::Vector2f{0.9f, 0.9f} * (0.35f + 0.65f * easeInOutQuint(textProgress)) /
                                             profile.hudScale * 1.45f,
                                    .origin      = txLetterText.getSize().toVector2f() / 2.f,
                                    .textureRect = txLetterText.getRect(),
                                    .color = sf::Color::whiteMask(static_cast<U8>(easeInOutQuint(textProgress) * 255.f))},
                         {.texture = &txLetterText});
        }

        //
        // Doll on screen particle border
        if (!pt.dolls.empty())
            gameLoopDrawDollParticleBorder(0.f);

        if (!pt.copyDolls.empty())
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
        rtGame->display();

        shaderPostProcess.setUniform(suPPVibrance, profile.ppSVibrance);
        shaderPostProcess.setUniform(suPPSaturation, profile.ppSSaturation);
        shaderPostProcess.setUniform(suPPLightness, profile.ppSLightness);
        shaderPostProcess.setUniform(suPPSharpness, profile.ppSSharpness);

        window.setView({window.getSize().toVector2f() / 2.f, window.getSize().toVector2f()});

        window.clear();
        window.draw(rtGame->getTexture(), {.shader = &shaderPostProcess});
        window.display();

        //
        // Save last mouse pos
        lastMousePos = mousePos;

        return true;
    }

    ////////////////////////////////////////////////////////////
    void loadPlaythroughFromFileAndReseed()
    {
        const sf::base::StringView loadMessage = loadPlaythroughFromFile(pt);

        if (!loadMessage.empty())
            pushNotification("Playthrough version updated", "%s", loadMessage.data());

        rng.reseed(pt.seed);
        shuffledCatNamesPerType = makeShuffledCatNames(rng);
    }

////////////////////////////////////////////////////////////
#ifdef BUBBLEBYTE_USE_STEAMWORKS
    Main(hg::Steam::SteamManager& xSteamMgr) : steamMgr(xSteamMgr), onSteamDeck(steamMgr.isOnSteamDeck())
#else
    Main() : onSteamDeck(false)
#endif
    {
        //
        // Profile
        if (sf::Path{"userdata/profile.json"}.exists())
        {
            loadProfileFromFile(profile);
            sf::cOut() << "Loaded profile from file on startup\n";
        }

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
            pt.seed = rng.getSeed();
        }

        //
        // Reserve memory
        particles.reserve(512);
        spentCoinParticles.reserve(512);
        textParticles.reserve(256);
        pt.bubbles.reserve(32'768);
        pt.cats.reserve(512);

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


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main(int argc, const char** argv)
{
    if (argc >= 2 && SFML_BASE_STRCMP(argv[1], "dev") == 0)
        debugMode = true;

#ifdef BUBBLEBYTE_USE_STEAMWORKS
    hg::Steam::SteamManager steamMgr;
    steamMgr.requestStatsAndAchievements();
    steamMgr.runCallbacks();

    Main{steamMgr}.run();
#else
    Main{}.run();
#endif
}

// TODO P1: add UNDO button for PP point purchase
// TODO P1: review all tooltips
// TODO P1: instead of new BGMs, attracto/repulso could unlock speed/pitch shifting for BGMs
// TODO P1: tooltips for options, reorganize them
// TODO P1: credits somewhere
// TODO P1: more steam deck improvements
// TODO P1: drag click PP upgrade, stacks with multipop
// TODO P1: Ability to hide maxed out upgrades would help reduce visual clutter in the Shop and Prestige menu in the
// late game.
// TODO P1: Endgame upgrade and cat costs are so large that they don't fit inside the buttons. Maybe the option to swap
// to a different notation would be good.
// TODO P1: Let me hold the "Buy PPS" button. Having to click it hundreds of times to get all upgrade achievements does
// not feel good at all.
// TODO P1: "it still shows the tooltip for another upgrade despite it being maxed out"
// TODO P1: demo & sponsor on playmygame with ss
// TODO P1: "remember to buy the combo upgrade" still shows up after prestige...

// TODO P2: idea for PP: when astrocat touches hellcat portal its buffed
// TODO P2: rested buff 1PP: 1.25x mult, enables after Xs of inactivity, can be upgraded with PPs naybe?
// TODO P2: configurable particle spawn chance
// TODO P2: configurable pop sound play chance
// TODO P2: maybe 64PP prestige buff for multipop that allows "misses"
// TODO P2: reduce size of game textures and try to reduce atlas size
// TODO P2: some sort of beacon cat that is only effective when near n-m cats but no less nor more
// TODO P2: decorations for unique cats (e.g. wizard cape, witch?, engi tesla coil,  ?)
// TODO P2: upgrade for ~512PPs "brain takes over" that turns cats into brains with 50x mult with corrupted zalgo names
// TODO P2: pp upgrade around 128pp that makes manually clicked bombs worth 100x (or maybe all bubbles)
// TODO P2: built-in speedrun splits system
// TODO P2: challenge runs, separate saves, separate speedrun save
