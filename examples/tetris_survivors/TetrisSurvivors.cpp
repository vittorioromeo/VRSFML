#include "../bubble_idle/Easing.hpp"       // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"     // TODO P1: avoid the relative path...?
#include "../bubble_idle/LoadedSound.hpp"  // TODO P1: avoid the relative path...?
#include "../bubble_idle/MathUtils.hpp"    // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/SoundManager.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Timer.hpp"        // TODO P1: avoid the relative path...?


#define SFEX_PROFILER_ENABLED
#include "Profiler.hpp"

//
#include "AnimationCommands.hpp"
#include "AnimationTimeline.hpp"
#include "BitmapFont.hpp"
#include "BitmapTextAlignment.hpp"
#include "BitmapTextUtils.hpp"
#include "Block.hpp"
#include "BlockGrid.hpp"
#include "BlockMatrix.hpp"
#include "Constants.hpp"
#include "ControlFlow.hpp"
#include "DefaultPerks.hpp"
#include "DrillDirection.hpp"
#include "LaserBeam.hpp"
#include "LaserDirection.hpp"
#include "LaserableBlocksInfo.hpp"
#include "LightningBolt.hpp"
#include "Perk.hpp"
#include "RandomBag.hpp"
#include "ShapeDimension.hpp"
#include "StringUtils.hpp"
#include "Tetramino.hpp"
#include "TetraminoShapes.hpp"
#include "Utils.hpp"
#include "World.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/VideoMode.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Math/Fmod.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/OverloadSet.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsConst.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <format>
#include <string>


namespace tsurv
{
//////////////////////////////////////////////////////////////
[[nodiscard]] static BitmapFont loadMinogramData()
{
    BitmapFont result;

    constexpr sf::base::Array<sf::base::StringView, 7>
        glyphRows{"ABCDEFGHIJKLM",
                  "NOPQRSTUVWXYZ",
                  "abcdefghijklm",
                  "nopqrstuvwxyz",
                  "0123456789+-=",
                  "()[]{}<>/*:#%",
                  "!?.,'\"@&$"};

    for (sf::base::SizeT iY = 0; iY < glyphRows.size(); ++iY)
        for (sf::base::SizeT iX = 0; iX < glyphRows[iY].size(); ++iX)
        {
            const char c = glyphRows[iY][iX];
            result.addGlyph(c, {iX * 6u, iY * 10u}, {6u, 10u});
        }

    return result;
}


//////////////////////////////////////////////////////////////
[[nodiscard]] static BitmapFont loadTiny5Data()
{
    BitmapFont result;

    constexpr sf::base::Array<sf::base::StringView, 7>
        glyphRows{"ABCDEFGHIJKLM",
                  "NOPQRSTUVWXYZ",
                  "abcdefghijklm",
                  "nopqrstuvwxyz",
                  "0123456789+-=",
                  "()[]{}<>/*:#%",
                  "!?.,'\"@&$"};

    sf::base::SizeT stepX = 15;
    sf::base::SizeT stepY = 17;

    for (sf::base::SizeT iY = 0; iY < glyphRows.size(); ++iY)
        for (sf::base::SizeT iX = 0; iX < glyphRows[iY].size(); ++iX)
        {
            const char c = glyphRows[iY][iX];
            result.addGlyph(c, {4 + (stepX * iX), 7 + (stepY * iY)}, {6u, 8u});
        }

    result.adjustSize('I', {-3, 0});
    result.adjustSize('i', {-3, 0});
    result.adjustSize('l', {-3, 0});
    result.adjustSize(':', {-3, 0});
    result.adjustSize('!', {-3, 0});

    result.adjustSize('a', {-1, 0});
    result.adjustSize('b', {-1, 0});
    result.adjustSize('c', {-1, 0});
    result.adjustSize('d', {-1, 0});
    result.adjustSize('e', {-1, 0});
    result.adjustSize('f', {-2, 0});
    result.adjustSize('g', {-1, 0});
    result.adjustSize('h', {-1, 0});
    result.adjustSize('k', {-1, 0});
    result.adjustSize('n', {-1, 0});
    result.adjustSize('o', {-1, 0});
    result.adjustSize('p', {-1, 0});
    result.adjustSize('q', {-1, 0});
    result.adjustSize('r', {-2, 0});
    result.adjustSize('t', {-2, 0});
    result.adjustSize('u', {-1, 0});
    result.adjustSize('v', {-1, 0});
    result.adjustSize('x', {-1, 0});
    result.adjustSize('y', {-1, 0});
    result.adjustSize('z', {-1, 0});
    result.adjustSize('j', {-2, 0});

    result.adjustSize('J', {-1, 0});
    result.adjustSize('L', {-1, 0});
    result.adjustSize('T', {-1, 0});
    result.adjustSize('V', {-1, 0});
    result.adjustSize('X', {-1, 0});
    result.adjustSize('Y', {-1, 0});
    result.adjustSize('Z', {-1, 0});

    result.adjustSize('0', {-1, 0});
    result.adjustSize('1', {-2, 0});
    result.adjustSize('2', {-1, 0});
    result.adjustSize('3', {-1, 0});
    result.adjustSize('4', {-1, 0});
    result.adjustSize('5', {-1, 0});
    result.adjustSize('6', {-1, 0});
    result.adjustSize('7', {-1, 0});
    result.adjustSize('8', {-1, 0});
    result.adjustSize('9', {-1, 0});

    result.adjustSize('(', {-2, 0});
    result.adjustSize(')', {-2, 0});
    result.adjustSize('[', {-2, 0});
    result.adjustSize(']', {-2, 0});

    result.adjustSize('{', {-1, 0});
    result.adjustSize('}', {-1, 0});
    result.adjustSize('<', {-1, 0});
    result.adjustSize('>', {-1, 0});
    result.adjustSize('+', {-1, 0});
    result.adjustSize('-', {-1, 0});
    result.adjustSize('=', {-1, 0});
    result.adjustSize('*', {-1, 0});
    result.adjustSize('?', {-1, 0});

    result.adjustSize('.', {-3, 0});
    result.adjustSize(',', {-3, 0});
    result.adjustSize('\'', {-3, 0});
    result.adjustSize('"', {-1, 0});
    result.adjustSize('@', {-1, 0});
    result.adjustSize('$', {-1, 0});

    result.adjustSize('/', {-2, 0});

    return result;
}


////////////////////////////////////////////////////////////
struct [[nodiscard]] ParticleData
{
    sf::Vec2f position;
    sf::Vec2f velocity;

    float scale;
    float scaleDecay;

    float accelerationY;

    float opacity;
    float opacityDecay;

    float rotation;
    float torque;

    sf::Color color;

    float        radius;
    unsigned int pointCount;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] EarnedXPParticle // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    sf::Vec2f  startPosition;
    sf::Vec2f  targetPosition;
    PaletteIdx paletteIdx;

    float delay;
    float startRotation = 0.f;

    Timer progress{};
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawBlockOptions
{
    float opacity          = 1.f;
    float squishMult       = 0.f;
    float rotation         = 0.f;
    float scale            = 1.f;
    bool  drawText         = true;
    bool  applyYOffset     = true;
    bool  applyQuakeOffset = true;
    bool  drawTimer        = false;
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] QuakeSinEffect
{
    float timeRemaining = 0.f;
    float magnitude     = 0.f;
    float speed         = 0.f;

    ////////////////////////////////////////////////////////////
    void update(const sf::Time deltaTime)
    {
        if (timeRemaining <= 0.f)
            return;

        timeRemaining -= deltaTime.asSeconds() * speed;

        if (timeRemaining <= 0.f)
        {
            timeRemaining = 0.f;
            magnitude     = 0.f;
        }
    }

    ////////////////////////////////////////////////////////////
    void start(const float newMagnitude, const float newSpeed)
    {
        magnitude = sf::base::max(magnitude, newMagnitude);
        speed     = newSpeed;

        timeRemaining = 1.f;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getValue() const
    {
        return sf::base::sin(timeRemaining * sf::base::pi) * magnitude;
    }
};


////////////////////////////////////////////////////////////
struct Sounds
{
    LoadedSound landed{"Landed.wav"};
    LoadedSound newLevel{"NewLevel.wav"};
    LoadedSound rotate{"Rotate.wav"};
    LoadedSound single{"Single.wav"};
    LoadedSound exp{"Exp.wav"};
    LoadedSound place{"Place.wav"};
    LoadedSound hold{"Hold.wav"};
    LoadedSound hit{"Hit.wav"};
    LoadedSound bonus{"Bonus.wav"};
    LoadedSound strike{"Strike.wav"};
    LoadedSound drill{"Drill.wav"};
    LoadedSound error{"Error.wav"};
    LoadedSound laser{"Laser.wav"};
    LoadedSound bounce{"Bounce.wav"};
    LoadedSound menuSelect{"MenuSelect.ogg"};
    LoadedSound menuConfirm{"MenuConfirm.ogg"};
    LoadedSound menuReroll{"MenuReroll.ogg"};
};


////////////////////////////////////////////////////////////
struct EveryNCounter
{
    sf::base::SizeT requiredCount;
    sf::base::SizeT currentCount = 0u;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool incrementAndCheck(const sf::base::SizeT n)
    {
        currentCount += n;

        if (currentCount < requiredCount)
            return false;

        currentCount = 0u;
        return true;
    }
};


////////////////////////////////////////////////////////////
struct TriggerTetraminoPlaced
{
    sf::base::Optional<TetraminoType> requiredType;

    EveryNCounter counter;
};


////////////////////////////////////////////////////////////
struct TriggerLinesCleared
{
    EveryNCounter counter;

    // TODO: also detect doubles/triples? or different trigger
};


////////////////////////////////////////////////////////////
struct TriggerDrillHit
{
    EveryNCounter counter;
};


////////////////////////////////////////////////////////////
struct TriggerLaserHit
{
    EveryNCounter counter;
};


////////////////////////////////////////////////////////////
struct TriggerLightningHit
{
    EveryNCounter counter;
};


////////////////////////////////////////////////////////////
struct TriggerBlockDamaged
{
    EveryNCounter counter;
};


////////////////////////////////////////////////////////////
struct TriggerPowerUpCollected
{
    EveryNCounter counter;
};


////////////////////////////////////////////////////////////
using DynamicPerkTrigger = sf::base::Variant< //
    TriggerTetraminoPlaced,
    TriggerLinesCleared,
    TriggerDrillHit,
    TriggerLaserHit,
    TriggerLightningHit,
    TriggerBlockDamaged,
    TriggerPowerUpCollected>;


////////////////////////////////////////////////////////////
struct EffectGainXP
{
    sf::base::U64 amount;
    // TODO: effect spawn position
};


////////////////////////////////////////////////////////////
using DynamicPerkEffect = sf::base::Variant< //
    EffectGainXP>;


////////////////////////////////////////////////////////////
struct DynamicPerk
{
    DynamicPerkTrigger                  trigger;
    sf::base::Vector<DynamicPerkEffect> effects;

    sf::base::Vector<DynamicPerkEffect> pendingEffects{};

    ////////////////////////////////////////////////////////////
    void onTetraminoPlaced(const Tetramino& tetramino)
    {
        auto* t = trigger.getIf<TriggerTetraminoPlaced>();

        if (t == nullptr)
            return;

        if (t->requiredType.hasValue() && tetramino.tetraminoType != t->requiredType.value())
            return;

        if (!t->counter.incrementAndCheck(1u))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void onLinesCleared(const sf::base::SizeT nLinesCleared)
    {
        auto* t = trigger.getIf<TriggerLinesCleared>();

        if (t == nullptr)
            return;

        if (!t->counter.incrementAndCheck(nLinesCleared))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void onDrillHit()
    {
        auto* t = trigger.getIf<TriggerDrillHit>();

        if (t == nullptr)
            return;

        if (!t->counter.incrementAndCheck(1u))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void onLaserHit()
    {
        auto* t = trigger.getIf<TriggerLaserHit>();

        if (t == nullptr)
            return;

        if (!t->counter.incrementAndCheck(1u))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void onLightningHit()
    {
        auto* t = trigger.getIf<TriggerLightningHit>();

        if (t == nullptr)
            return;

        if (!t->counter.incrementAndCheck(1u))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void onBlockDamaged([[maybe_unused]] const sf::Vec2uz position, [[maybe_unused]] Block& block)
    {
        auto* t = trigger.getIf<TriggerBlockDamaged>();

        if (t == nullptr)
            return;

        if (!t->counter.incrementAndCheck(1u))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void onPowerupCollected([[maybe_unused]] Block& block)
    {
        auto* t = trigger.getIf<TriggerPowerUpCollected>();

        if (t == nullptr)
            return;

        if (!t->counter.incrementAndCheck(1u))
            return;

        apply();
    }

    ////////////////////////////////////////////////////////////
    void apply()
    {
        pendingEffects.emplaceRange(effects.begin(), effects.size());
    }
};


////////////////////////////////////////////////////////////
class Game
{
private:
    //////////////////////////////////////////////////////////////
    const unsigned int m_aaLevel = sf::base::min(16u, sf::RenderTexture::getMaximumAntiAliasingLevel());

    ////////////////////////////////////////////////////////////
    sf::RenderWindow m_window = makeDPIScaledRenderWindow(
        {.size            = resolution.toVec2u(),
         .title           = "Tetris Survivors",
         .resizable       = true,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = 0u}});

    ////////////////////////////////////////////////////////////
    sf::Shader m_shader{[]
    {
        auto result = sf::Shader::loadFromFile({
                                                   .vertexPath   = "resources/shader.vert",
                                                   .fragmentPath = "resources/shader.frag",
                                               })
                          .value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    ////////////////////////////////////////////////////////////
    sf::Shader m_shaderCRT{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/crt.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    bool m_useCRTShader = true;

    sf::Shader::UniformLocation m_ulInputSize        = m_shaderCRT.getUniformLocation("u_inputSize").value();
    sf::Shader::UniformLocation m_ulTime             = m_shaderCRT.getUniformLocation("u_time").value();
    sf::Shader::UniformLocation m_ulCurvature        = m_shaderCRT.getUniformLocation("u_curvature").value();
    sf::Shader::UniformLocation m_ulVignetteStrength = m_shaderCRT.getUniformLocation("u_vignetteStrength").value();
    sf::Shader::UniformLocation m_ulVignetteInnerRadius = m_shaderCRT.getUniformLocation("u_vignetteInnerRadius").value();
    sf::Shader::UniformLocation m_ulVignetteOuterRadius = m_shaderCRT.getUniformLocation("u_vignetteOuterRadius").value();
    sf::Shader::UniformLocation m_ulScanlineStrength = m_shaderCRT.getUniformLocation("u_scanlineStrength").value();
    sf::Shader::UniformLocation
        m_ulScanlineBrightnessModulation = m_shaderCRT.getUniformLocation("u_scanlineBrightnessModulation").value();
    sf::Shader::UniformLocation m_ulScanlineScrollSpeed = m_shaderCRT.getUniformLocation("u_scanlineScrollSpeed").value();
    sf::Shader::UniformLocation m_ulScanlineThickness = m_shaderCRT.getUniformLocation("u_scanlineThickness").value();
    sf::Shader::UniformLocation m_ulScanlineHeight    = m_shaderCRT.getUniformLocation("u_scanlineHeight").value();
    sf::Shader::UniformLocation m_ulNoiseStrength     = m_shaderCRT.getUniformLocation("u_noiseStrength").value();
    sf::Shader::UniformLocation m_ulMaskStrength      = m_shaderCRT.getUniformLocation("u_maskStrength").value();
    sf::Shader::UniformLocation m_ulMaskScale         = m_shaderCRT.getUniformLocation("u_maskScale").value();
    sf::Shader::UniformLocation m_ulBloomStrength     = m_shaderCRT.getUniformLocation("u_bloomStrength").value();
    sf::Shader::UniformLocation m_ulInputGamma        = m_shaderCRT.getUniformLocation("u_inputGamma").value();
    sf::Shader::UniformLocation m_ulOutputGamma       = m_shaderCRT.getUniformLocation("u_outputGamma").value();
    sf::Shader::UniformLocation m_ulSaturation        = m_shaderCRT.getUniformLocation("u_saturation").value();

    float m_fUlCurvature                    = -0.015f;
    float m_fUlVignetteStrength             = 0.2f;
    float m_fUlVignetteInnerRadius          = 0.5f;
    float m_fUlVignetteOuterRadius          = 1.f;
    float m_fUlScanlineStrength             = 0.25f;
    float m_fUlScanlineBrightnessModulation = 1.2f;
    float m_fUlScanlineScrollSpeed          = 1.f;
    float m_fUlScanlineThickness            = 4.f;
    float m_fUlScanlineHeight               = 2.f;
    float m_fUlNoiseStrength                = 2.5f;
    float m_fUlMaskStrength                 = 0.15f;
    float m_fUlMaskScale                    = 0.25f;
    float m_fUlBloomStrength                = 0.1f;
    float m_fUlInputGamma                   = 2.3f;
    float m_fUlOutputGamma                  = 2.6f;
    float m_fUlSaturation                   = 1.15f;

    bool m_mustSyncShaderUniforms = true;

    ////////////////////////////////////////////////////////////
    // Shader with post-processing effects
    sf::Shader m_shaderPostProcess{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/postprocess.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    sf::Shader::UniformLocation m_ulPPVibrance   = m_shaderPostProcess.getUniformLocation("u_vibrance").value();
    sf::Shader::UniformLocation m_ulPPSaturation = m_shaderPostProcess.getUniformLocation("u_saturation").value();
    sf::Shader::UniformLocation m_ulPPLightness  = m_shaderPostProcess.getUniformLocation("u_lightness").value();
    sf::Shader::UniformLocation m_ulPPSharpness  = m_shaderPostProcess.getUniformLocation("u_sharpness").value();

    float m_fUlPPVibrance   = 0.1f;
    float m_fUlPPSaturation = 0.85f;
    float m_fUlPPLightness  = 1.f;
    float m_fUlPPSharpness  = 0.1f;

    ////////////////////////////////////////////////////////////
    sf::Font m_font      = sf::Font::openFromFile("resources/monogram.ttf").value();
    sf::Font m_fontMago2 = sf::Font::openFromFile("resources/petty5.bdf").value();
    sf::Font m_font3     = sf::Font::openFromFile("resources/ChikareGo.ttf").value();
    sf::Font m_font4     = sf::Font::openFromFile("resources/TinyUnicode.ttf").value();

    ////////////////////////////////////////////////////////////
    sf::PlaybackDevice m_playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    ////////////////////////////////////////////////////////////
    Sounds       m_sounds;
    SoundManager m_soundManager;

    ////////////////////////////////////////////////////////////
    void playSound(const LoadedSound& ls, const sf::base::SizeT maxOverlap = 255u)
    {
        (void)m_playbackDevice.updateListener({.volume = 0.5f});
        m_soundManager.playPooled(m_playbackDevice, ls, maxOverlap);
    }

    ////////////////////////////////////////////////////////////
    sf::ImGuiContext m_imGuiContext;
    ImFont* const    m_imguiFont{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/monogram.ttf", 16.f)};
    ImFont* const    m_imguiFontBig{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/BoldPixels.ttf", 16.f)};

    ////////////////////////////////////////////////////////////
    sf::Clock m_tickClock;
    float     m_timeAccumulator = 0.f;
    float     m_totalTime       = 0.f;

    ////////////////////////////////////////////////////////////
    World m_world;

    ////////////////////////////////////////////////////////////
    sf::Vec2f                             m_currentTetraminoVisualCenter;
    AnimationTimeline<AnimationCommandP0> m_animationTimelineP0;
    AnimationTimeline<AnimationCommandP1> m_animationTimelineP1;
    AnimationTimeline<AnimationCommandP2> m_animationTimelineP2;
    sf::base::Vector<float>               m_rowYOffsets;

    ////////////////////////////////////////////////////////////
    bool            m_inLevelUpScreen      = false;
    sf::base::SizeT m_selectedPerk         = 0u;
    float           m_menuDelayProgress    = 0.f; // delay before accepting input in menus
    sf::base::SizeT m_rerollsLeftThisLevel = 0u;

    ////////////////////////////////////////////////////////////
    PerkChainLightning                  m_perkChainLightning;
    PerkPeekNextTetraminos              m_perkPeekNextTetraminos;
    PerkOnClearLightningStrike          m_perkOnClearLightningStrike;
    PerkHardDropDrillUnlock             m_perkHardDropDrillUnlock;
    PerkHardDropDrillPenetration        m_perkHardDropDrillPenetration;
    PerkHardDropDrillBluntForce         m_perkHardDropDrillBluntForce;
    PerkHoldSkipTetramino               m_perkHoldSkipTetramino;
    PerkXpPerTetraminoPlaced            m_perkXpPerTetraminoPlaced;
    PerkXpPerBlockDamaged               m_perkXpPerBlockDamaged;
    PerkDeleteFloorPerNTetraminos       m_perkDeleteFloorPerNTetraminos;
    PerkRndHitPerNTetraminos            m_perkRndHitPerNTetraminos;
    PerkExtraLinePieces                 m_perkExtraLinePieces;
    PerkHorizontalDrillLeftUnlock       m_perkHorizontalDrillLeftUnlock;
    PerkHorizontalDrillRightUnlock      m_perkHorizontalDrillRightUnlock;
    PerkHorizontalDrillLeftPenetration  m_perkHorizontalDrillLeftPenetration;
    PerkHorizontalDrillLeftCoverage     m_perkHorizontalDrillLeftCoverage;
    PerkHorizontalDrillRightPenetration m_perkHorizontalDrillRightPenetration;
    PerkHorizontalDrillRightCoverage    m_perkHorizontalDrillRightCoverage;
    PerkDiagonalLaserLeftUnlock         m_perkDiagonalLaserLeftUnlock;
    PerkDiagonalLaserRightUnlock        m_perkDiagonalLaserRightUnlock;
    PerkDiagonalLaserLeftPenetration    m_perkDiagonalLaserLeftPenetration;
    PerkDiagonalLaserRightPenetration   m_perkDiagonalLaserRightPenetration;
    PerkDiagonalLaserLeftBounce         m_perkDiagonalLaserLeftBounce;
    PerkDiagonalLaserRightBounce        m_perkDiagonalLaserRightBounce;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<const Perk*> m_perks{
        &m_perkChainLightning,
        &m_perkPeekNextTetraminos,
        &m_perkOnClearLightningStrike,
        &m_perkHardDropDrillUnlock,
        &m_perkHardDropDrillPenetration,
        &m_perkHardDropDrillBluntForce,
        &m_perkHoldSkipTetramino,
        &m_perkXpPerTetraminoPlaced,
        &m_perkXpPerBlockDamaged,
        &m_perkDeleteFloorPerNTetraminos,
        &m_perkRndHitPerNTetraminos,
        &m_perkExtraLinePieces,
        &m_perkHorizontalDrillLeftUnlock,
        &m_perkHorizontalDrillRightUnlock,
        &m_perkHorizontalDrillLeftPenetration,
        &m_perkHorizontalDrillLeftCoverage,
        &m_perkHorizontalDrillRightPenetration,
        &m_perkHorizontalDrillRightCoverage,
        &m_perkDiagonalLaserLeftUnlock,
        &m_perkDiagonalLaserRightUnlock,
        &m_perkDiagonalLaserLeftPenetration,
        &m_perkDiagonalLaserRightPenetration,
        &m_perkDiagonalLaserLeftBounce,
        &m_perkDiagonalLaserRightBounce,
    };

    sf::base::Vector<sf::base::SizeT> m_perkIndicesSelectedThisLevel;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<DynamicPerk> m_dynamicPerks; // TODO: to world

    ////////////////////////////////////////////////////////////
    sf::base::Vector<LightningBolt> m_lightningBolts;

    ////////////////////////////////////////////////////////////
    QuakeSinEffect m_quakeSinEffectLineClear;
    QuakeSinEffect m_quakeSinEffectHardDrop;

    ////////////////////////////////////////////////////////////
    sf::base::Optional<LaserBeam> m_optLaserBeam;

    ////////////////////////////////////////////////////////////
    // Screen shake effect state
    float m_screenShakeAmount{0.f};
    float m_screenShakeTimer{0.f};

    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] BlockEffect
    {
        static constexpr float squishDuration = 0.2f;
        float                  squishTime     = 0.f;
    };

    ankerl::unordered_dense::map<BlockId, BlockEffect> m_blockEffects;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<EarnedXPParticle> m_earnedXPParticles;
    sf::base::Vector<ParticleData>     m_hueColorCircleShapeParticles;
    sf::base::Vector<ParticleData>     m_fixedColorCircleShapeParticles;

    ////////////////////////////////////////////////////////////
    RNGFast m_rngFast{static_cast<RNGFast::SeedType>(
        sf::Clock::now().asMicroseconds())}; // very fast, low-quality, but good enough for VFXs

    //////////////////////////////////////////////////////////////
    sf::TextureAtlas m_textureAtlas{sf::Texture::create({512u, 512u}, {.smooth = false}).value()};

    ////////////////////////////////////////////////////////////
    const sf::Rect2f m_txrWhiteDotTrue   = m_textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value();
    const sf::Rect2f m_txrWhiteDot       = {{0.f, 0.f}, {1.f, 1.f}};
    const sf::Rect2f m_txrBlock0         = addImgResourceToAtlas("block0.png");
    const sf::Rect2f m_txrBlock1         = addImgResourceToAtlas("block1.png");
    const sf::Rect2f m_txrBlock2         = addImgResourceToAtlas("block2.png");
    const sf::Rect2f m_txrBlock3         = addImgResourceToAtlas("block3.png");
    const sf::Rect2f m_txrBlock4         = addImgResourceToAtlas("block4.png");
    const sf::Rect2f m_txrBlock5         = addImgResourceToAtlas("block5.png");
    const sf::Rect2f m_txrBlock6         = addImgResourceToAtlas("block6.png");
    const sf::Rect2f m_txrDivider        = addImgResourceToAtlas("divider.png");
    const sf::Rect2f m_txrDrill          = addImgResourceToAtlas("drill.png");
    const sf::Rect2f m_txrRedDot         = addImgResourceToAtlas("reddot.png");
    const sf::Rect2f m_txrEmitter        = addImgResourceToAtlas("emitter.png");
    const sf::Rect2f m_txrPowerupXP      = addImgResourceToAtlas("powerupxp.png");
    const sf::Rect2f m_txrPowerupBomb    = addImgResourceToAtlas("powerupbomb.png");
    const sf::Rect2f m_txrBFMinogram6x10 = addImgResourceToAtlas("minogram_6x10.png");
    const sf::Rect2f m_txrBFTiny5        = addImgResourceToAtlas("tiny5.png");

    //////////////////////////////////////////////////////////////
    BitmapFont m_bitmapFontMinogram{loadMinogramData()};
    BitmapFont m_bitmapFontTiny5{loadTiny5Data()};


    //////////////////////////////////////////////////////////////
    sf::RenderTexture m_rtGame{
        sf::RenderTexture::create(resolution.toVec2u(), {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};

    sf::RenderTexture m_rtPostProcess{
        sf::RenderTexture::create(resolution.toVec2u(), {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};


    //////////////////////////////////////////////////////////////
    sf::base::Vector<sf::Vertex>    m_textVerticesBuffer;
    sf::base::Vector<sf::IndexType> m_textIndicesBuffer;


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] inline constexpr sf::CircleShapeData particleToCircleData(const ParticleData& particle)
    {
        SFML_BASE_ASSERT(particle.opacity >= 0.f && particle.opacity <= 1.f);
        const auto opacityAsAlpha = static_cast<sf::base::U8>(particle.opacity * 255.f);

        return {
            .position    = floorVec2(particle.position),
            .scale       = {particle.scale, particle.scale},
            .rotation    = sf::radians(particle.rotation),
            .textureRect = m_txrRedDot, // No texture
            .fillColor   = particle.color.withAlpha(opacityAsAlpha),
            .radius      = particle.radius,
            .pointCount  = particle.pointCount,
        };
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f addImgResourceToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile("resources" / path).value(), /* padding */ {2u, 2u}).value();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] int calculateGhostY(const Tetramino& tetramino) const
    {
        sf::base::Optional<Tetramino> finalHardDropState;

        if (const auto* squish = m_animationTimelineP1.getIfPlaying<AnimSquish>())
            finalHardDropState.emplace(squish->tetramino);

        int ghostY = tetramino.position.y;

        while (true)
        {
            const sf::Vec2i nextGhostPos = {tetramino.position.x, ghostY + 1};

            if (!m_world.blockGrid.isValidMove(tetramino.shape, nextGhostPos))
                break;

            if (finalHardDropState.hasValue() &&
                blockMatricesIntersect(tetramino.shape, nextGhostPos, finalHardDropState->shape, finalHardDropState->position))
                break;

            ++ghostY;
        }

        return ghostY;
    }


    ////////////////////////////////////////////////////////////
    static inline constexpr sf::Vec2f drawBlockSize{11.f, 11.f};
    static inline constexpr sf::Vec2f drawOffset{drawBlockSize.x + 3.f, drawBlockSize.y - drawBlockSize.y* gridGraceY + 3.f};


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] sf::Vec2f toDrawCoordinates(const sf::Vec2<T> position) const noexcept
    {
        return floorVec2(drawOffset + position.toVec2f().componentWiseMul(drawBlockSize));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] sf::Vec2i toGridCoordinates(const sf::Vec2f drawPosition) const noexcept
    {
        return (drawPosition - drawOffset).componentWiseDiv(drawBlockSize).toVec2i();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] sf::Color hueColorFromPaletteIdxRotated(
        const PaletteIdx   paletteIdx,
        const sf::base::U8 alpha,
        const float        degrees)
    {
        const auto hue = sf::base::positiveRemainder(blockPalette[paletteIdx].toHSL().hue + degrees, 360.f);
        return hueColor(hue, 255u).withAlpha(alpha);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] sf::Color hueColorFromPaletteIdx(const PaletteIdx paletteIdx,
                                                                                                 const sf::base::U8 alpha)
    {
        return hueColorFromPaletteIdxRotated(paletteIdx, alpha, 0.f);
    }


    ////////////////////////////////////////////////////////////
    sf::Rect2f drawBlock(const Block& block, const sf::Vec2f position, const DrawBlockOptions& options = {})
    {
        const float timerProgress = block.getTimerProgress();

        sf::Vec2f mins{9999.f, 9999.f};
        sf::Vec2f maxs{-9999.f, -9999.f};

        float yOffset = 0.f;

        if (options.applyYOffset && !m_rowYOffsets.empty())
        {
            const float progress = m_animationTimelineP1.isPlaying<AnimCollapseGrid>()
                                       ? m_animationTimelineP1.getProgress()
                                       : 0.f;

            const auto gridPos = toGridCoordinates(position);

            if (gridPos.y >= 0 && static_cast<sf::base::SizeT>(gridPos.y) < m_rowYOffsets.size())
            {
                const float initialOffset = m_rowYOffsets[static_cast<sf::base::SizeT>(gridPos.y)];
                yOffset += initialOffset * (1.f - easeInBack(progress));
            }
        }

        if (options.applyQuakeOffset)
        {
            yOffset += -(m_quakeSinEffectHardDrop.getValue() + m_quakeSinEffectLineClear.getValue());
        }

        const auto alpha = static_cast<sf::base::U8>(options.opacity * 255.f);

        float finalSquishMult = 1.f + options.squishMult;
        if (const auto* it = m_blockEffects.find(block.blockId); it != m_blockEffects.end())
        {
            const BlockEffect& effect = it->second;

            const float progress = effect.squishTime / BlockEffect::squishDuration;
            finalSquishMult += easeInOutSine(bounce(progress)) * 0.5f;
        }

        const sf::RenderTarget::TextureDrawParams commonDrawParams{
            .position = floorVec2(position.addY(yOffset)).addX(1.f).addY(1.f),
            .scale    = sf::Vec2f{finalSquishMult, finalSquishMult} * options.scale,
            .origin   = floorVec2(drawBlockSize / 2.f),
            .color    = hueColorFromPaletteIdx(block.paletteIdx, alpha),
        };

        {
            const auto [sine, cosine] = sf::base::sinCosLookup(commonDrawParams.rotation.wrapUnsigned().asRadians());

            const auto transform = sf::Transform::fromPositionScaleOriginSinCos(commonDrawParams.position,
                                                                                commonDrawParams.scale,
                                                                                commonDrawParams.origin,
                                                                                sine,
                                                                                cosine);

            mins = transform.transformPoint({0.f, 0.f});
            maxs = transform.transformPoint(drawBlockSize);
        }

        const float shakeAmount = easeInOutSine(timerProgress) * 0.75f;
        const auto  shake       = m_rngFast.getVec2f({-shakeAmount, -shakeAmount}, {shakeAmount, shakeAmount});

        const bool hasTimer = block.tickTimerTarget != nullTickTimerTarget;

        const bool doesNotHaveTimerOrIsPowerup = !hasTimer || block.powerup != BlockPowerup::None;

        // const bool useNormalTexture  = block.health == 1u && !doesNotHaveTimerOrIsPowerup;
        const bool useDamagedTexture = block.health == 1u && hasTimer && block.powerup == BlockPowerup::None;
        const bool useArmoredTexture = block.health > 1u;

        const auto txr = useArmoredTexture ? m_txrBlock4 : useDamagedTexture ? m_txrBlock6 : m_txrBlock1;

        m_rtGame.draw(m_textureAtlas.getTexture(),
                      {
                          .position    = commonDrawParams.position + shake,
                          .scale       = commonDrawParams.scale,
                          .origin      = commonDrawParams.origin,
                          .rotation    = sf::degrees(options.rotation),
                          .textureRect = txr,
                          .color       = commonDrawParams.color,
                      },
                      {.shader = &m_shader});

        if (block.powerup != BlockPowerup::None && options.drawText)
        {
            if (block.powerup == BlockPowerup::XPBonus)
            {
                m_rtGame.draw(m_textureAtlas.getTexture(),
                              {
                                  .position    = commonDrawParams.position,
                                  .scale       = commonDrawParams.scale,
                                  .origin      = commonDrawParams.origin,
                                  .textureRect = m_txrPowerupXP,
                                  .color       = commonDrawParams.color,
                              },
                              {.shader = &m_shader});
            }
            else if (block.powerup == BlockPowerup::ColumnDrill)
            {
                // TODO
            }
            else if (block.powerup == BlockPowerup::ThreeRowDrill)
            {
                m_rtGame.draw(m_textureAtlas.getTexture(),
                              {
                                  .position    = commonDrawParams.position,
                                  .scale       = commonDrawParams.scale,
                                  .origin      = commonDrawParams.origin,
                                  .textureRect = m_txrPowerupBomb,
                                  .color       = commonDrawParams.color,
                              },
                              {.shader = &m_shader});
            }
        }

        if (block.health > 1u && options.drawText)
        {
            sf::Text text{m_fontMago2,
                          {
                              .origin        = floorVec2(drawBlockSize / 2.f),
                              .string        = std::to_string(static_cast<unsigned int>(block.health - 1u)),
                              .characterSize = 5u,
                              .fillColor     = sf::Color::blackMask(alpha),
                          }};

            text.setCenter(floorVec2(position.addY(yOffset) + sf::Vec2f{2.f, 3.f}));
            m_rtGame.draw(text);

            text.setFillColor(sf::Color::whiteMask(alpha));

            text.setCenter(floorVec2(position.addY(yOffset)) + sf::Vec2f{2.f, 2.f});
            m_rtGame.draw(text);
        }

        if (block.tickTimerTarget != nullTickTimerTarget && options.drawTimer)
        {
            const bool timerDirection = block.powerup == BlockPowerup::None;

            const float progress0 = sf::base::clamp(timerProgress * 4.f, 0.f, 1.f);
            const float progress1 = sf::base::clamp((timerProgress - 0.25f) * 4.f, 0.f, 1.f);
            const float progress2 = sf::base::clamp((timerProgress - 0.5f) * 4.f, 0.f, 1.f);
            const float progress3 = sf::base::clamp((timerProgress - 0.75f) * 4.f, 0.f, 1.f);

            const float p0 = timerDirection ? progress0 : (1.f - progress3);
            const float p1 = timerDirection ? progress1 : (1.f - progress2);
            const float p2 = timerDirection ? progress2 : (1.f - progress1);
            const float p3 = timerDirection ? progress3 : (1.f - progress0);

            sf::RectangleShapeData timerLine{
                .scale       = commonDrawParams.scale,
                .origin      = commonDrawParams.origin,
                .textureRect = m_txrWhiteDot,
                .fillColor   = sf::Color::White,
            };

            const auto drawTimerLines = [&](const float embed)
            {
                // top line
                timerLine.position = commonDrawParams.position + sf::Vec2f{embed, embed};
                timerLine.rotation = sf::degrees(0.f);
                timerLine.size     = {(drawBlockSize.x - embed * 2.f) * p0, 1.f};
                m_rtGame.draw(timerLine, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

                // right line
                timerLine.position = commonDrawParams.position + sf::Vec2f{drawBlockSize.x - embed - 1.f, embed};
                timerLine.rotation = sf::degrees(0.f);
                timerLine.size     = {1.f, (drawBlockSize.y - embed * 2.f) * p1};
                m_rtGame.draw(timerLine, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

                // bottom line
                timerLine.position = commonDrawParams.position + sf::Vec2f{1.f - embed, 1.f - embed};
                timerLine.rotation = sf::degrees(180.f);
                timerLine.size     = {(drawBlockSize.x - embed * 2.f) * p2, 1.f};
                m_rtGame.draw(timerLine, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

                // left line
                timerLine.position = commonDrawParams.position + sf::Vec2f{-drawBlockSize.x + embed + 2.f, 1.f - embed};
                timerLine.rotation = sf::degrees(180.f);
                timerLine.size     = {1.f, (drawBlockSize.y - embed * 2.f) * p3};
                m_rtGame.draw(timerLine, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});
            };

            if (useDamagedTexture)
            {
                timerLine.textureRect = m_txrWhiteDot;

                timerLine.fillColor = sf::Color::White;
                drawTimerLines(0.f);

                timerLine.fillColor = sf::Color::DarkGray;
                drawTimerLines(1.f);
            }
            else
            {
                timerLine.textureRect = m_txrRedDot;
                timerLine.fillColor   = useDamagedTexture ? sf::Color::White
                                                          : hueColorFromPaletteIdxRotated(block.paletteIdx, 255u, 90.f);

                drawTimerLines(0.f);
            }
        }

        return {mins, maxs - mins};
    }


    ////////////////////////////////////////////////////////////
    sf::Rect2f drawTetramino(const BlockMatrix& shape, const sf::Vec2f centerPosition, const DrawBlockOptions& options = {})
    {
        sf::Vec2f mins{9999.f, 9999.f};
        sf::Vec2f maxs{-9999.f, -9999.f};

        const float     scale    = options.scale;
        constexpr float rotation = 0.f;

        // 1. Define the pivot point in the tetramino's local, unscaled coordinate space.
        // This is the center of the 4x4 grid.
        const sf::Vec2f localPivot = floorVec2((drawBlockSize * static_cast<float>(shapeDimension)) / 2.f);

        for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
            for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            {
                const sf::base::Optional<Block>& optBlock = shape.at(x, y);

                if (!optBlock.hasValue())
                    continue;

                // 2. Calculate this block's local center position, relative to the top-left corner.
                const sf::Vec2f localBlockCenter = sf::Vec2uz{x, y}.toVec2f().componentWiseMul(drawBlockSize) +
                                                   floorVec2(drawBlockSize / 2.f);

                // 3. Get the block's position vector relative to the central pivot.
                sf::Vec2f positionRelativeToPivot = localBlockCenter - localPivot;

                // 4. Scale and rotate this relative vector.
                positionRelativeToPivot = positionRelativeToPivot.componentWiseMul({scale, scale});
                positionRelativeToPivot = positionRelativeToPivot.rotatedBy(sf::degrees(rotation));

                // 5. The final screen position is the tetramino's center plus the transformed relative vector.
                const sf::Vec2f finalDrawPosition = floorVec2(centerPosition + positionRelativeToPivot);

                // 6. Call drawBlock, passing all the necessary transform properties.
                const auto [pos,
                            size] = drawBlock(*optBlock,
                                              finalDrawPosition,
                                              {
                                                  .opacity    = options.opacity,
                                                  .squishMult = options.squishMult,
                                                  .rotation = rotation, // Pass rotation for the block's own orientation
                                                  .scale    = scale,
                                                  .drawText = options.drawText,
                                                  .applyYOffset     = false, // Usually false for UI elements
                                                  .applyQuakeOffset = options.applyQuakeOffset,
                                                  .drawTimer        = options.drawTimer,
                                              });

                mins.x = sf::base::min(mins.x, pos.x);
                mins.y = sf::base::min(mins.y, pos.y);
                maxs.x = sf::base::max(maxs.x, pos.x + size.x);
                maxs.y = sf::base::max(maxs.y, pos.y + size.y);
            }

        return {mins, maxs - mins};
    }


    ////////////////////////////////////////////////////////////
    void moveTetramino(Tetramino& tetramino, const sf::Vec2i delta) const
    {
        const auto newPosition = tetramino.position + delta;

        if (m_world.blockGrid.isValidMove(tetramino.shape, newPosition))
            tetramino.position = newPosition;
    }


    ////////////////////////////////////////////////////////////
    void rotateTetramino(Tetramino& tetramino, const bool clockwise) const
    {
        const auto nextRotationState = static_cast<RotationState>((tetramino.rotationState + (clockwise ? 1 : 3)) % 4u);

        const auto& targetShapeTemplate = srsTetraminoShapes[static_cast<sf::base::SizeT>(tetramino.tetraminoType)][nextRotationState];

        // Create the new stateful shape by re-mapping the blocks
        const BlockMatrix rotatedShape = mapBlocksToNewShape(tetramino, targetShapeTemplate);

        const auto& kickTable = (tetramino.tetraminoType == TetraminoType::I) ? kickDataI : kickDataJLSTZ;

        int kickTableIndex = clockwise ? tetramino.rotationState : nextRotationState;
        kickTableIndex     = kickTableIndex * 2 + (clockwise ? 0 : 1);

        for (const sf::Vec2i offset : kickTable[static_cast<sf::base::SizeT>(kickTableIndex)])
        {
            const sf::Vec2i testPosition = tetramino.position + offset;

            if (!m_world.blockGrid.isValidMove(rotatedShape, testPosition))
                continue;

            tetramino.shape         = rotatedShape;
            tetramino.position      = testPosition;
            tetramino.rotationState = nextRotationState;

            return;
        }
    }


    ////////////////////////////////////////////////////////////
    void moveCurrentTetraminoBy(const sf::Vec2i delta)
    {
        if (m_world.currentTetramino.hasValue())
            moveTetramino(*m_world.currentTetramino, delta);
    }


    ////////////////////////////////////////////////////////////
    void rotateCurrentTetramino(const bool clockwise)
    {
        if (m_world.currentTetramino.hasValue())
        {
            rotateTetramino(*m_world.currentTetramino, clockwise);
            playSound(m_sounds.rotate);
        }
    }


    ////////////////////////////////////////////////////////////
    void resetAndRedrawCurrentTetramino(const bool usedHold)
    {
        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        m_world.currentTetramino.reset();
        m_world.holdUsedThisTurn = usedHold;

        updateStepRefillBlockMatrixIfNeeded();
        initializeCurrentTetraminoFromBag();
    }


    ////////////////////////////////////////////////////////////
    void skipCurrentTetramino()
    {
        if (m_world.holdUsedThisTurn)
            return;

        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        playSound(m_sounds.hold);
        resetAndRedrawCurrentTetramino(/* usedHold */ true);
    }


    ////////////////////////////////////////////////////////////
    void holdCurrentTetramino()
    {
        if (m_world.holdUsedThisTurn)
            return;

        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        playSound(m_sounds.hold);

        const auto temp       = m_world.heldTetramino;
        m_world.heldTetramino = m_world.currentTetramino;

        if (temp.hasValue())
        {
            m_world.currentTetramino = temp;
            m_world.currentTetramino->position = sf::Vec2uz{(m_world.blockGrid.getWidth() - shapeDimension) / 2u, 0u}.toVec2i();

            m_world.holdUsedThisTurn = true;
        }
        else
        {
            resetAndRedrawCurrentTetramino(/* usedHold */ true);
        }
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] AnimClearLines::RowVector findFullRows()
    {
        AnimClearLines::RowVector fullRows;

        for (sf::base::SizeT y = gridGraceY; y < m_world.blockGrid.getHeight(); ++y)
        {
            bool isFull = true;

            for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
            {
                if (!m_world.blockGrid.at(sf::Vec2uz{x, y}).hasValue())
                {
                    isFull = false;
                    break;
                }
            }

            if (isFull)
                fullRows.pushBack(y);
        }

        return fullRows;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] ShapeBlockPositionVector findDrillBlocks(const Tetramino& tetramino, const DrillDirection::Enum direction) const
    {
        if (direction == DrillDirection::Down)
        {
            auto downmostBlocksXY = findDownmostBlocks(tetramino.shape);

            if (downmostBlocksXY.size() > 1u && m_world.perkDrill[DrillDirection::Down]->coverage == 1u)
                return {};

            return downmostBlocksXY;
        }

        return findHorizontalBlocks(tetramino.shape, static_cast<sf::base::SizeT>(m_world.perkDrill[direction]->coverage));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] ShapeBlockPositionVector findLaserBlocks(const Tetramino& tetramino, const LaserDirection::Enum direction) const
    {
        const auto outOfBoundsOrEmpty = [&](const sf::base::SizeT x, const sf::base::SizeT y) -> bool
        { return !tetramino.shape.isInBounds(x, y) || !tetramino.shape.at(x, y).hasValue(); };

        ShapeBlockPositionVector result;

        for (const auto bPos : findTopmostBlocks(tetramino.shape))
        {
            if (direction == LaserDirection::Left)
            {
                if (bPos.x == 0u) // to avoid unsigned wraparound shenanigans
                {
                    result.pushBack(bPos);
                }
                else
                {
                    const bool leftValid       = outOfBoundsOrEmpty(bPos.x - 1u, bPos.y);
                    const bool bottomLeftValid = outOfBoundsOrEmpty(bPos.x - 1u, bPos.y + 1u);

                    if (leftValid && bottomLeftValid)
                        result.pushBack(bPos);
                }
            }

            if (direction == LaserDirection::Right)
            {
                if (bPos.x == shapeDimension - 1u) // to avoid unsigned wraparound shenanigans
                {
                    result.pushBack(bPos);
                }
                else
                {
                    const bool rightValid       = outOfBoundsOrEmpty(bPos.x + 1u, bPos.y);
                    const bool bottomRightValid = outOfBoundsOrEmpty(bPos.x + 1u, bPos.y + 1u);

                    if (rightValid && bottomRightValid)
                        result.pushBack(bPos);
                }
            }
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] BlockAndPosition // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        Block*     block;
        sf::Vec2uz position;
    };


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Vector<BlockAndPosition> findDrillTargetBlocks(const Tetramino&           tetramino,
                                                                           const DrillDirection::Enum direction)
    {
        sf::base::Vector<BlockAndPosition> result;

        // 1. Guard Clause: Exit early if this drill perk isn't active.
        if (!m_world.perkDrill[direction].hasValue())
            return result;

        // 2. Get the number of blocks to penetrate.
        auto nToHit = static_cast<sf::base::SizeT>(m_world.perkDrill[direction]->maxPenetration);
        if (nToHit == 0)
            return result;

        // 3. Determine the iteration step based on direction.
        const sf::Vec2i step = drillDirectionToVec2i(direction);

        // 4. Iterate over the source blocks on the current tetramino.
        for (const auto bPos : findDrillBlocks(tetramino, direction))
        {
            // 5. Start probing from the block adjacent to the source block.
            sf::Vec2i probePos = tetramino.position + bPos.toVec2i() + step;

            // 6. Traverse the grid in the given direction until we hit a boundary.
            while (m_world.blockGrid.isInBounds(probePos))
            {
                const auto blockGridPos = probePos.toVec2uz();
                auto&      optBlock     = m_world.blockGrid.at(blockGridPos);

                // If we hit empty space, the drill path is blocked.
                if (!optBlock.hasValue())
                    break;

                // If the block is damageable (health > 1), add it to our list.
                if (optBlock->isDamageable())
                {
                    result.pushBack({optBlock.asPtr(), blockGridPos});
                    if (--nToHit == 0u)
                        break; // Stop probing if we've reached the penetration limit.
                }

                // Move to the next block in the line.
                probePos += step;
            }

            // If we've found all the blocks we can hit, we can stop checking other source blocks.
            if (nToHit == 0u)
                break;
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    void handleEffects()
    {
        const auto visitor = sf::base::OverloadSet{
            [&](const EffectGainXP& e) { addXP(e.amount); },
        };

        for (auto& dp : m_dynamicPerks)
        {
            for (auto& e : dp.pendingEffects)
                e.linearVisit(visitor);

            dp.pendingEffects.clear();
        }
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerTetraminoPlaced(const Tetramino& tetramino)
    {
        for (auto& dp : m_dynamicPerks)
            dp.onTetraminoPlaced(tetramino);

        handleEffects();

        if (m_world.perkXPPerTetraminoPlaced > 0)
        {
            addXP(static_cast<sf::base::U64>(m_world.perkXPPerTetraminoPlaced));

            const auto downmostBlocksXY = findDownmostBlocks(tetramino.shape);

            for (const auto& bPos : downmostBlocksXY)
            {
                const sf::base::Optional<Block>& optBlock = tetramino.shape.at(bPos.x, bPos.y);
                SFML_BASE_ASSERT(optBlock.hasValue());

                spawnXPEarnedParticle(toDrawCoordinates(tetramino.position + bPos.toVec2i()), optBlock->paletteIdx);
            }
        }

        if (auto* rndHitPerNTetraminos = m_world.perkRndHitPerNTetraminos.asPtr())
        {
            ++(rndHitPerNTetraminos->tetraminosPlacedCount);

            if (rndHitPerNTetraminos->tetraminosPlacedCount >= rndHitPerNTetraminos->nTetraminos)
            {
                m_animationTimelineP0.addInstantaneous(AnimLightningStrike{.numStrikes = 1u});
                rndHitPerNTetraminos->tetraminosPlacedCount = 0;
            }
        }

        if (auto* deleteFloorNTetraminos = m_world.perkDeleteFloorPerNTetraminos.asPtr())
        {
            ++(deleteFloorNTetraminos->tetraminosPlacedCount);

            if (deleteFloorNTetraminos->tetraminosPlacedCount >= deleteFloorNTetraminos->nTetraminos)
            {
                // Delete bottom line (must happen after regular clear)
                m_animationTimelineP2.addInstantaneous(AnimClearLines{
                    .rows       = {m_world.blockGrid.getHeight() - 1u},
                    .awardXP    = false,
                    .forceClear = true,
                });

                deleteFloorNTetraminos->tetraminosPlacedCount = 0;
            }
        }
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerLinesCleared(const sf::base::SizeT nLinesCleared)
    {
        for (auto& dp : m_dynamicPerks)
            dp.onLinesCleared(nLinesCleared);

        handleEffects();

        if (m_world.perkRndHitOnClear > 0)
        {
            m_animationTimelineP0.addInstantaneous(
                AnimLightningStrike{.numStrikes = static_cast<sf::base::SizeT>(m_world.perkRndHitOnClear)});
        }
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerDrillHit()
    {
        for (auto& dp : m_dynamicPerks)
            dp.onDrillHit();

        handleEffects();
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerLaserHit()
    {
        for (auto& dp : m_dynamicPerks)
            dp.onLaserHit();

        handleEffects();

        if (roll100(m_world.perkChainLightning))
            m_animationTimelineP0.addInstantaneous(AnimLightningStrike{.numStrikes = 1u});
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerLightningHit()
    {
        for (auto& dp : m_dynamicPerks)
            dp.onLightningHit();

        handleEffects();

        if (roll100(m_world.perkChainLightning))
            m_animationTimelineP0.addInstantaneous(AnimLightningStrike{.numStrikes = 1u});
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerBlockDamaged(const sf::Vec2uz position, Block& block)
    {
        for (auto& dp : m_dynamicPerks)
            dp.onBlockDamaged(position, block);

        handleEffects();

        if (m_world.perkXPPerBlockDamaged > 0)
        {
            addXP(static_cast<sf::base::U64>(m_world.perkXPPerBlockDamaged));
            playSound(m_sounds.exp);

            spawnXPEarnedParticle(toDrawCoordinates(position) + drawBlockSize / 2.f, block.paletteIdx);
        }
    }


    ////////////////////////////////////////////////////////////
    void handleTriggerPowerupCollected(Block& block)
    {
        for (auto& dp : m_dynamicPerks)
            dp.onPowerupCollected(block);

        handleEffects();
    }


    ////////////////////////////////////////////////////////////
    void embedTetraminoAndClearLines(const Tetramino& tetramino)
    {
        handleTriggerTetraminoPlaced(tetramino);

        m_world.blockGrid.embedTetramino(tetramino);
        m_world.graceDropMoves = 0u;

        ++m_world.tetaminosPlaced;

        if (const auto fullRows = findFullRows(); !fullRows.empty())
            m_animationTimelineP1.addInstantaneous(AnimClearLines{
                .rows       = fullRows,
                .awardXP    = true,
                .forceClear = false,
            });
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isInPlayableState() const
    {
        return !m_animationTimelineP0.anyAnimationPlaying() && !m_animationTimelineP1.anyAnimationPlaying() &&
               !m_animationTimelineP2.anyAnimationPlaying() && !m_inLevelUpScreen;
    }


    /////////////////////////////////////////////////////////////
    void addXP(const sf::base::U64 amount)
    {
        m_world.currentXP += amount;

        // Check for level up (can happen multiple times from one large XP gain)
        while (m_world.currentXP >= getXPNeededForLevelUp(m_world.playerLevel))
        {
            m_world.currentXP -= getXPNeededForLevelUp(m_world.playerLevel);
            ++m_world.playerLevel;
        }
    }


    /////////////////////////////////////////////////////////////
    void restartGame()
    {
        m_world = World{};

        updateStepRefillBlockMatrixIfNeeded();
        initializeCurrentTetraminoFromBag();

        m_dynamicPerks.clear();

        m_dynamicPerks.pushBack(DynamicPerk{
            .trigger = DynamicPerkTrigger{TriggerLightningHit{2u}},
            .effects =
                {
                    DynamicPerkEffect{EffectGainXP{10u}},
                },
        });
    }


    /////////////////////////////////////////////////////////////
    void handleMenuKeyPressedEvent(const sf::Event::KeyPressed& eKeyPressed)
    {
        SFML_BASE_ASSERT(m_inLevelUpScreen);

        if (m_menuDelayProgress < 1.f)
            return;

        const auto nPerks = m_perkIndicesSelectedThisLevel.size();

        if (eKeyPressed.code == sf::Keyboard::Key::Down)
        {
            ++m_selectedPerk;

            if (m_selectedPerk >= nPerks)
                m_selectedPerk = 0u;

            playSound(m_sounds.menuSelect);
        }
        else if (eKeyPressed.code == sf::Keyboard::Key::Up)
        {
            --m_selectedPerk;

            if (m_selectedPerk >= nPerks)
                m_selectedPerk = nPerks - 1u;

            playSound(m_sounds.menuSelect);
        }
        else if (eKeyPressed.code == sf::Keyboard::Key::Enter || eKeyPressed.code == sf::Keyboard::Key::Space)
        {
            m_inLevelUpScreen = false;

            ++m_world.committedPlayerLevel;

            m_perks[m_perkIndicesSelectedThisLevel[m_selectedPerk]]->apply(m_world);
            m_perkIndicesSelectedThisLevel.clear();

            playSound(m_sounds.menuConfirm);
        }
        else if (m_rerollsLeftThisLevel > 0 && eKeyPressed.code == sf::Keyboard::Key::LShift)
        {
            --m_rerollsLeftThisLevel;

            m_menuDelayProgress = 0.5f;
            rerollPerks();

            playSound(m_sounds.menuReroll);
        }
    }


    /////////////////////////////////////////////////////////////
    void handleKeyPressedEvent(const sf::Event::KeyPressed& eKeyPressed)
    {
        SFML_BASE_ASSERT(!m_inLevelUpScreen);

        const bool isP0TimelineBusy = m_animationTimelineP0.anyAnimationPlaying() &&
                                      !m_animationTimelineP0.isPlaying<AnimFadeAttachments>(); // skippable

        const bool isP1TimelineBusy = (m_animationTimelineP1.anyAnimationPlaying() &&
                                       !m_animationTimelineP1.isPlaying<AnimCollapseGrid>() && // skippable
                                       !m_animationTimelineP1.isPlaying<AnimFadeBlocks>()) ||  // skippable
                                      m_animationTimelineP1.isEnqueued<AnimHardDrop>() ||      // important
                                      m_animationTimelineP1.isEnqueued<AnimSquish>();          // important

        const bool isP2TimelineBusy = m_animationTimelineP2.anyAnimationPlaying();

        const bool inAnimation = isP0TimelineBusy || isP1TimelineBusy || isP2TimelineBusy;

        if (eKeyPressed.code == sf::Keyboard::Key::Right)
        {
            moveCurrentTetraminoBy({1, 0});
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Down)
        {
            if (inAnimation)
            {
                playSound(m_sounds.error);
                return;
            }

            moveCurrentTetraminoBy({0, 1});
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Left)
        {
            moveCurrentTetraminoBy({-1, 0});
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Up)
        {
            rotateCurrentTetramino(/* clockwise */ true);
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Z)
        {
            rotateCurrentTetramino(/* clockwise */ false);
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::X)
        {
            rotateCurrentTetramino(/* clockwise */ true);
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Space)
        {
            if (inAnimation)
            {
                playSound(m_sounds.error);
                return;
            }

            const int endY = calculateGhostY(*m_world.currentTetramino);

            m_animationTimelineP1.add(0.125f,
                                      AnimHardDrop{
                                          .tetramino = *m_world.currentTetramino,
                                          .endY      = endY,
                                      });

            resetAndRedrawCurrentTetramino(/* usedHold */ false);
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::LShift)
        {
            if (inAnimation)
            {
                playSound(m_sounds.error);
                return;
            }

            if (m_world.perkCanHoldTetramino == 1)
                holdCurrentTetramino();
            else if (m_world.perkCanHoldTetramino == 2)
                skipCurrentTetramino();

            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::R)
        {
            if (inAnimation)
            {
                playSound(m_sounds.error);
                return;
            }

            restartGame();
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Q)
        {
            ++m_world.playerLevel;
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::W)
        {
            m_animationTimelineP0.addInstantaneous(AnimLightningStrike{.numStrikes = 1u});
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::E)
        {
            m_world.tick += 2000;
            m_world.blockMatrixBag.clear();
            updateStepRefillBlockMatrixIfNeeded();

            return;
        }
    }


    /////////////////////////////////////////////////////////////
    void initializeCurrentTetraminoFromBag()
    {
        SFML_BASE_ASSERT(!m_world.currentTetramino.hasValue());
        SFML_BASE_ASSERT(!m_world.blockMatrixBag.empty());

        const TaggedBlockMatrix taggedBlockMatrix = m_world.blockMatrixBag.front();
        m_world.blockMatrixBag.eraseAt(0u);

        m_world.currentTetramino.emplace(Tetramino{
            .shape         = taggedBlockMatrix.blockMatrix,
            .position      = sf::Vec2uz{(m_world.blockGrid.getWidth() - shapeDimension) / 2u, 0u}.toVec2i(),
            .tetraminoType = taggedBlockMatrix.tetraminoType,
            .rotationState = RotationState{0u},
        });

        m_currentTetraminoVisualCenter = toDrawCoordinates(m_world.currentTetramino->position);
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] ControlFlow eventStep()
    {
        while (sf::base::Optional event = m_window.pollEvent())
        {
            m_imGuiContext.processEvent(m_window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return ControlFlow::Break;

            if (handleNonScalingResize(*event, resolution, m_window))
                continue;

            if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (m_inLevelUpScreen)
                    handleMenuKeyPressedEvent(*eKeyPressed);
                else
                    handleKeyPressedEvent(*eKeyPressed);
            }
        }

        return ControlFlow::Continue;
    }


    /////////////////////////////////////////////////////////////
    void damageBlock(const sf::Vec2uz position, Block& block)
    {
        block.applyDamage();

        playSound(m_sounds.hit);

        m_blockEffects[block.blockId] = BlockEffect{};
    }


    /////////////////////////////////////////////////////////////
    void rerollPerks()
    {
        m_perkIndicesSelectedThisLevel.clear();

        for (sf::base::SizeT i = 0u; i < m_perks.size(); ++i)
            if (m_perks[i]->meetsPrerequisites(m_world))
                m_perkIndicesSelectedThisLevel.pushBack(i);

        while (m_perkIndicesSelectedThisLevel.size() > m_world.nPerkChoicesPerLevel)
        {
            const auto removeIdx = m_rngFast.getI<sf::base::SizeT>(0u, m_perkIndicesSelectedThisLevel.size() - 1u);
            m_perkIndicesSelectedThisLevel.eraseAt(removeIdx);
        }

        shuffleBag(m_perkIndicesSelectedThisLevel, m_rngFast);
    }


    /////////////////////////////////////////////////////////////
    void spawnXPEarnedParticle(const sf::Vec2f startPosition, const PaletteIdx paletteIdx)
    {
        m_earnedXPParticles.pushBack(EarnedXPParticle{
            .startPosition  = startPosition,
            .targetPosition = sf::Vec2f{430.f, 64.f} + m_rngFast.getVec2f({-16.f, -16.f}, {16.f, 16.f}),
            .paletteIdx     = paletteIdx,
            .delay          = m_rngFast.getF(0.f, 0.2f),
            .startRotation  = m_rngFast.getF(0.f, sf::base::tau),
        });
    }

    /////////////////////////////////////////////////////////////
    struct [[nodiscard]] EligibleBlock // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        Block*     block;
        sf::Vec2uz position;
    };


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool roll100(const int chanceOutOf100)
    {
        const auto roll = m_rngFast.getI(0, 100);
        return roll < chanceOutOf100;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] Block* pickDamageableBlock()
    {
        sf::base::SizeT count    = 0u;
        Block*          selected = nullptr;

        for (auto& optBlock : m_world.blockGrid.getBlocks())
        {
            if (!optBlock.hasValue())
                continue;

            if (optBlock->health <= 1u)
                continue;

            ++count;

            // Select the current bubble with probability `1/count` (reservoir sampling)
            if (m_rngFast.getI<sf::base::SizeT>(0, count - 1) == 0)
                selected = optBlock.asPtr();
        }

        return (count == 0u) ? nullptr : selected;
    }


    /////////////////////////////////////////////////////////////
    void updateStepQuakeEffect(const sf::Time deltaTime)
    {
        m_quakeSinEffectLineClear.update(deltaTime);
        m_quakeSinEffectHardDrop.update(deltaTime);
    }


    /////////////////////////////////////////////////////////////
    void updateStepScreenShake(const sf::Time deltaTime)
    {
        if (m_screenShakeTimer <= 0.f)
            return;

        m_screenShakeTimer -= deltaTime.asSeconds();

        if (m_screenShakeTimer <= 0.f)
        {
            m_screenShakeTimer  = 0.f;
            m_screenShakeAmount = 0.f;
        }
    }


    /////////////////////////////////////////////////////////////
    void updateStepShowLevelUpScreenIfNeeded()
    {
        if (m_animationTimelineP0.anyAnimationPlaying() || m_animationTimelineP1.anyAnimationPlaying() ||
            !m_earnedXPParticles.empty())
            return;

        if (m_world.committedPlayerLevel < m_world.playerLevel && !m_inLevelUpScreen)
        {
            m_inLevelUpScreen      = true;
            m_selectedPerk         = 0u;
            m_menuDelayProgress    = 0.f;
            m_rerollsLeftThisLevel = m_world.rerollsPerLevel;

            rerollPerks();

            playSound(m_sounds.newLevel);
        }
    }


    /////////////////////////////////////////////////////////////
    void updateStepInterpolateVisualTetraminoPosition(const sf::base::Optional<Tetramino>& optTetramino,
                                                      sf::Vec2f&                           visualCenter,
                                                      const sf::Time                       deltaTime)
    {
        if (!optTetramino.hasValue())
            return;

        const sf::Vec2f targetPosition = getTetraminoCenterDrawPosition(optTetramino->position);

        const float interpolationSpeed = 50.f;
        const auto  deltaTimeMs        = static_cast<float>(deltaTime.asMicroseconds()) / 1000.f;

        visualCenter = exponentialApproach(visualCenter, targetPosition, deltaTimeMs, interpolationSpeed);
    }


    /////////////////////////////////////////////////////////////
    void updateStepAnimations(const sf::Time deltaTime)
    {
        if (m_inLevelUpScreen)
            return;

        const auto processTimeline = [&](auto& timeline)
        {
            auto& anim = timeline.commands.front();

            auto visitor = [&]<typename T>(T& innerAnim)
                requires(!sf::base::isConst<T>) { return updateAnimation(timeline, innerAnim); };

            if (anim.data.linearVisit(visitor))
            {
                timeline.popFrontCommand();
                return;
            }

            timeline.timeOnCurrentCommand += deltaTime.asSeconds() * 1.f;

            if (!timeline.commands.empty() && timeline.commands.front().duration == 0.f)
            {
                updateStepAnimations(deltaTime);
            }
        };

        if (m_animationTimelineP0.anyAnimationPlaying())
        {
            processTimeline(m_animationTimelineP0);
            return;
        }

        if (m_animationTimelineP1.anyAnimationPlaying())
        {
            processTimeline(m_animationTimelineP1);
            return;
        }

        if (m_animationTimelineP2.anyAnimationPlaying())
        {
            processTimeline(m_animationTimelineP2);
            return;
        }
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimWait&)
    {
        return timeline.getProgress() >= 1.f;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimHardDrop& hardDrop)
    {
        if (timeline.getProgress() < 1.f)
            return false;

        // hardDrop.tetramino.position.y = hardDrop.endY;
        hardDrop.tetramino.position.y = calculateGhostY(hardDrop.tetramino);

        m_animationTimelineP1.add(0.125f, AnimSquish{.tetramino = hardDrop.tetramino});

        m_quakeSinEffectHardDrop.start(4.f, 4.f);

        playSound(m_sounds.landed);

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimSquish& squish)
    {
        if (timeline.getProgress() < 1.f)
            return false;

        const auto processDrill = [&](const DrillDirection::Enum direction)
        {
            if (!m_world.perkDrill[direction].hasValue() || findDrillTargetBlocks(squish.tetramino, direction).empty())
                return;

            int maxDrilledBlocks = 0;

            for (const auto bPos : findDrillBlocks(squish.tetramino, direction))
            {
                const auto startPos                   = squish.tetramino.position + bPos.toVec2i();
                const auto [nDrillableBlocks, endPos] = countDrillableBlocks(startPos, direction);

                maxDrilledBlocks = sf::base::max(maxDrilledBlocks, nDrillableBlocks);
            }

            m_animationTimelineP0.add(0.3f + (0.1f * static_cast<float>(maxDrilledBlocks)),
                                      AnimDrill{
                                          .tetramino = squish.tetramino,
                                          .direction = direction,
                                      });
        };

        const auto processLaser = [&](const LaserDirection::Enum direction)
        {
            if (!m_world.perkLaser[direction].hasValue())
                return;

            const auto maxPenetration = static_cast<sf::base::SizeT>(m_world.perkLaser[direction]->maxPenetration);

            for (const auto bPos : findLaserBlocks(squish.tetramino, direction))
            {
                const auto startPos            = squish.tetramino.position + bPos.toVec2i();
                auto       laserableBlocksInfo = findLaserableBlocks(startPos, direction);

                if (laserableBlocksInfo.positions.empty() && laserableBlocksInfo.bouncePositions.empty())
                    continue;

                if (laserableBlocksInfo.positions.size() > maxPenetration)
                    laserableBlocksInfo.positions.resize(maxPenetration);

                for (const auto targetPos : laserableBlocksInfo.positions)
                    m_animationTimelineP0.add(0.175f,
                                              AnimLaser{
                                                  .tetramino     = squish.tetramino,
                                                  .direction     = direction,
                                                  .gridStartPos  = startPos,
                                                  .gridTargetPos = targetPos,
                                                  .onlyVisual    = false,
                                              });

                for (const auto targetPos : laserableBlocksInfo.bouncePositions)
                {
                    m_animationTimelineP0.add(0.175f,
                                              AnimLaser{
                                                  .tetramino     = squish.tetramino,
                                                  .direction     = direction,
                                                  .gridStartPos  = startPos,
                                                  .gridTargetPos = laserableBlocksInfo.bouncePos,
                                                  .onlyVisual    = true,
                                              });

                    m_animationTimelineP0.add(0.175f,
                                              AnimLaser{
                                                  .tetramino     = squish.tetramino,
                                                  .direction     = direction,
                                                  .gridStartPos  = laserableBlocksInfo.bouncePos,
                                                  .gridTargetPos = targetPos,
                                                  .onlyVisual    = false,
                                              });
                }
            }
        };

        processDrill(DrillDirection::Down);
        processDrill(DrillDirection::Left);
        processDrill(DrillDirection::Right);

        processLaser(LaserDirection::Left);
        processLaser(LaserDirection::Right);

        m_animationTimelineP0.add(0.125f, AnimFadeAttachments{.tetramino = squish.tetramino});

        embedTetraminoAndClearLines(squish.tetramino);

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimClearLines& clearLines)
    {
        AnimClearLines::RowVector         trulyClearedRows;
        AnimFadeBlocks::FadingBlockVector fadingBlocks;
        sf::base::Vector<sf::Vec2uz>      columnClearPositions;

        const auto addRowIfNotExistent = [&](const sf::base::SizeT row)
        {
            if (sf::base::find(trulyClearedRows.begin(), trulyClearedRows.end(), row) == trulyClearedRows.end())
                trulyClearedRows.pushBack(row);
        };

        for (sf::base::SizeT y : clearLines.rows)
        {
            bool rowIsFullyCleared = true;

            for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
            {
                if (auto& optBlock = m_world.blockGrid.at(sf::Vec2uz{x, y}); optBlock.hasValue())
                {
                    // TODO: powerup
                    if (!clearLines.forceClear && optBlock->isArmored())
                    {
                        damageBlock(sf::Vec2uz{x, y}, *optBlock);
                        rowIsFullyCleared = false;
                    }
                    else
                    {
                        fadingBlocks.pushBack(AnimFadeBlocks::FadingBlock{
                            .block    = *optBlock,
                            .position = {x, y},
                        });

                        if (optBlock->powerup != BlockPowerup::None)
                            handleTriggerPowerupCollected(*optBlock);

                        if (optBlock->powerup == BlockPowerup::XPBonus)
                        {
                            addXP(5u * m_world.playerLevel);
                            playSound(m_sounds.bonus);

                            spawnXPEarnedParticle(toDrawCoordinates(sf::Vec2uz{x, y}), optBlock->paletteIdx);
                        }
                        else if (optBlock->powerup == BlockPowerup::ColumnDrill)
                        {
                            columnClearPositions.emplaceBack(x, y);
                            playSound(m_sounds.bonus);
                        }
                        else if (optBlock->powerup == BlockPowerup::ThreeRowDrill)
                        {
                            addRowIfNotExistent(y);

                            if (y + 1u < m_world.blockGrid.getHeight())
                                addRowIfNotExistent(y + 1);

                            if (y > 0u)
                                addRowIfNotExistent(y - 1);

                            playSound(m_sounds.bonus);
                        }

                        optBlock.reset();
                    }
                }
            }

            if (rowIsFullyCleared)
                addRowIfNotExistent(y);
        }

        if (!fadingBlocks.empty())
            m_animationTimelineP1.add(0.125f, AnimFadeBlocks{.fadingBlocks = fadingBlocks});

        if (!trulyClearedRows.empty())
        {
            playSound(m_sounds.single);

            const sf::base::SizeT numCleared = trulyClearedRows.size();

            if (clearLines.awardXP)
            {
                m_world.linesCleared += numCleared;

                const sf::base::U64 amount = [&]
                {
                    if (numCleared == 1)
                        return 10u;

                    if (numCleared == 2)
                        return 25u;

                    if (numCleared == 3)
                        return 40u;

                    return 60u;
                }();

                const float quakeMagnitude = 8.f + static_cast<float>(numCleared) * 1.5f;
                const float quakeSpeed     = 4.f - static_cast<float>(numCleared) * 0.5f;

                m_quakeSinEffectLineClear.start(quakeMagnitude, quakeSpeed);

                addXP(amount);
                playSound(m_sounds.exp);

                for (sf::base::U64 i = 0u; i < fadingBlocks.size() * 4u; ++i)
                {
                    const auto& block = fadingBlocks[i % fadingBlocks.size()];

                    const auto startPosition = toDrawCoordinates(block.position) +
                                               m_rngFast.getVec2f(-drawBlockSize, drawBlockSize) / 2.f;

                    spawnXPEarnedParticle(startPosition, block.block.paletteIdx);
                }
            }

            sf::base::quickSort(trulyClearedRows.begin(),
                                trulyClearedRows.end(),
                                [](const sf::base::SizeT a, const sf::base::SizeT b) { return a < b; });

            const auto height = m_world.blockGrid.getHeight();
            m_rowYOffsets.resize(height);

            for (sf::base::SizeT y = 0u; y < height; ++y)
                m_rowYOffsets[y] = 0.f;

            sf::base::SizeT dropAmount = 0;

            // Iterate from the bottom of the grid upwards to calculate offsets.
            // This calculates the correct offset for each row's FINAL position.
            for (int y = static_cast<int>(height) - 1; y >= 0; --y)
            {
                // Check if the current row 'y' (from the original grid) is being cleared.
                const bool isCleared = sf::base::find(trulyClearedRows.begin(),
                                                      trulyClearedRows.end(),
                                                      static_cast<sf::base::SizeT>(y)) != trulyClearedRows.end();

                if (isCleared)
                {
                    // If it is, increment the number of lines that rows above this one must drop.
                    ++dropAmount;
                }
                else if (dropAmount > 0)
                {
                    // If this row is being kept, the row that lands at its *new* position
                    // (which is y + dropAmount) needs an initial offset.
                    const sf::base::SizeT finalY = static_cast<sf::base::SizeT>(y) + dropAmount;
                    if (finalY < height)
                        m_rowYOffsets[finalY] = -static_cast<float>(dropAmount) * drawBlockSize.y;
                }
            }

            for (const auto rowIndex : trulyClearedRows)
                m_world.blockGrid.shiftRowDown(rowIndex);

            m_animationTimelineP1.add(0.15f, AnimCollapseGrid{.clearedRows = trulyClearedRows});

            if (clearLines.awardXP)
                handleTriggerLinesCleared(numCleared);
        }
        else if (!fadingBlocks.empty())
        {
            const auto numPartiallyCleared = clearLines.rows.size();

            const sf::base::U64 amount = [&]
            {
                if (numPartiallyCleared == 1)
                    return 4u;

                if (numPartiallyCleared == 2)
                    return 10u;

                if (numPartiallyCleared == 3)
                    return 16u;

                return 20u;
            }();

            addXP(amount);
            playSound(m_sounds.exp);

            for (sf::base::U64 i = 0u; i < fadingBlocks.size() * 4u; ++i)
            {
                const auto& block = fadingBlocks[i % fadingBlocks.size()];

                const auto startPosition = toDrawCoordinates(block.position) +
                                           m_rngFast.getVec2f(-drawBlockSize, drawBlockSize) / 2.f;

                spawnXPEarnedParticle(startPosition, block.block.paletteIdx);
            }
        }

        for (const auto columnClearPos : columnClearPositions)
            m_animationTimelineP1.add(0.3f, AnimColumnClear{.position = columnClearPos.addY(1)});

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimFadeBlocks&)
    {
        return timeline.getProgress() >= 1.f;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimCollapseGrid&)
    {
        if (timeline.getProgress() < 1.f)
            return false;

        for (auto& rowOffset : m_rowYOffsets)
            rowOffset = 0.f;

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimDrill& drill)
    {
        if (timeline.justStarted())
            playSound(m_sounds.drill);

        m_screenShakeAmount = 0.85f;
        m_screenShakeTimer  = 0.05f;

        if (timeline.getProgress() < 1.f)
            return false;

        for (auto [blockPtr, position] : findDrillTargetBlocks(drill.tetramino, drill.direction))
        {
            damageBlock(position, *blockPtr);
            handleTriggerDrillHit();
        }

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimLaser& laser)
    {
        auto& optBlock = m_world.blockGrid.at(laser.gridTargetPos);

        if (!laser.onlyVisual && (!optBlock.hasValue() || !optBlock->isDamageable()))
        {
            // cancel animation if block is already at 1 health or destroyed
            // (e.g. drilled earlier in the same animation sequence)
            return true;
        }

        if (timeline.justStarted())
        {
            playSound(m_sounds.laser);

            const auto dir      = laserDirectionToVec2i(laser.direction).toVec2f();
            const auto startPos = toDrawCoordinates(laser.gridStartPos) + dir * 8.f + sf::Vec2f{0, 2.f};

            SFML_BASE_ASSERT(!m_optLaserBeam.hasValue());
            m_optLaserBeam.emplace(startPos, startPos, blockPalette[getTetraminoPaletteIdx(laser.tetramino)]);
        }

        m_screenShakeAmount = 0.65f;
        m_screenShakeTimer  = 0.05f;

        if (timeline.getProgress() < 1.f)
            return false;

        if (!laser.onlyVisual)
        {
            SFML_BASE_ASSERT(optBlock.hasValue());
            damageBlock(laser.gridTargetPos.toVec2uz(), *optBlock);

            handleTriggerLaserHit();
        }
        else
        {
            playSound(m_sounds.bounce);
        }

        SFML_BASE_ASSERT(m_optLaserBeam.hasValue());
        m_optLaserBeam.reset();

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimFadeAttachments&)
    {
        return timeline.getProgress() >= 1.f;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimColumnClear& columnClear)
    {
        if (timeline.getProgress() < 1.f)
            return false;

        AnimFadeBlocks::FadingBlockVector fadingBlocks;

        for (sf::base::SizeT y = columnClear.position.y; y < m_world.blockGrid.getHeight(); ++y)
            if (auto& optBlock = m_world.blockGrid.at(sf::Vec2uz{columnClear.position.x, y}); optBlock.hasValue())
            {
                fadingBlocks.pushBack(AnimFadeBlocks::FadingBlock{
                    .block    = *optBlock,
                    .position = sf::Vec2uz{columnClear.position.x, y},
                });

                optBlock.reset();
            }

        if (!fadingBlocks.empty())
            m_animationTimelineP1.add(0.125f, AnimFadeBlocks{.fadingBlocks = fadingBlocks});

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(auto& timeline, AnimLightningStrike& lightningStrike)
    {
        if (timeline.getProgress() < 1.f)
            return false;

        sf::base::Vector<EligibleBlock> eligibleBlocks;

        m_world.blockGrid.forBlocks([&](Block& block, const sf::Vec2uz position)
        {
            if (block.isDamageable())
                eligibleBlocks.pushBack({&block, position});

            return ControlFlow::Continue;
        });

        if (eligibleBlocks.empty())
            return true;

        const auto blockInfo = eligibleBlocks[0];

        m_lightningBolts.emplaceBack(m_rngFast,
                                     sf::Vec2f{9.f + m_rngFast.getF(0.f, drawBlockSize.x * m_world.blockGrid.getWidth()), 0.f},
                                     toDrawCoordinates(blockInfo.position));

        for (int i = 0; i < 16; ++i)
        {
            m_fixedColorCircleShapeParticles.emplaceBack(ParticleData{
                .position      = toDrawCoordinates(blockInfo.position),
                .velocity      = m_rngFast.getVec2f({-0.75f, -2.15f}, {0.75f, -0.25f}) * 0.25f,
                .scale         = m_rngFast.getF(0.08f, 0.27f) * 0.75f,
                .scaleDecay    = 0.f,
                .accelerationY = 0.0004f,
                .opacity       = 0.75f,
                .opacityDecay  = m_rngFast.getF(0.001f, 0.002f) * 0.7f,
                .rotation      = m_rngFast.getF(0.f, sf::base::tau),
                .torque        = m_rngFast.getF(-0.001f, 0.001f),
                .color         = sf::Color::White,
                .radius        = m_rngFast.getF(9.f, 16.f),
                .pointCount    = 5u,
            });

            m_fixedColorCircleShapeParticles.emplaceBack(ParticleData{
                .position      = toDrawCoordinates(blockInfo.position),
                .velocity      = m_rngFast.getVec2f({-0.75f, -2.15f}, {0.75f, -0.25f}) * 0.075f,
                .scale         = m_rngFast.getF(0.08f, 0.27f) * 0.25f,
                .scaleDecay    = 0.f,
                .accelerationY = 0.0004f,
                .opacity       = 0.75f,
                .opacityDecay  = m_rngFast.getF(0.001f, 0.002f) * 0.7f,
                .rotation      = m_rngFast.getF(0.f, sf::base::tau),
                .torque        = m_rngFast.getF(-0.001f, 0.001f),
                .color         = sf::Color::LightYellow,
                .radius        = m_rngFast.getF(8.f, 14.f),
                .pointCount    = 5u,
            });
        }

        playSound(m_sounds.strike);
        damageBlock(blockInfo.position, *blockInfo.block);

        m_screenShakeAmount = 2.5f;
        m_screenShakeTimer  = 0.2f;

        m_animationTimelineP0.add(0.15f, AnimWait{});

        handleTriggerLightningHit();

        if (lightningStrike.numStrikes > 1u)
            m_animationTimelineP0.addInstantaneous(AnimLightningStrike{.numStrikes = lightningStrike.numStrikes - 1u});

        return true;
    }


    /////////////////////////////////////////////////////////////
    void updateStepBlockEffects(const sf::Time deltaTime)
    {
        for (auto* it = m_blockEffects.begin(); it != m_blockEffects.end();)
        {
            it->second.squishTime += deltaTime.asSeconds();

            if (it->second.squishTime >= BlockEffect::squishDuration)
            {
                it = m_blockEffects.erase(it);
                continue;
            }

            ++it;
        }
    }


    /////////////////////////////////////////////////////////////
    void updateStepEarnedXPParticles(const sf::Time deltaTime)
    {
        const auto deltaTimeMs = static_cast<float>(deltaTime.asMicroseconds()) / 1000.f;

        for (auto& particle : m_earnedXPParticles)
        {
            if (particle.delay > 0.f)
            {
                particle.delay -= deltaTimeMs * 0.0015f;
                continue;
            }

            (void)particle.progress.updateForwardAndStop(deltaTimeMs * 0.0015f);
        }

        sf::base::vectorEraseIf(m_earnedXPParticles, [&](const auto& p) { return p.progress.isDoneForward(); });
    }


    /////////////////////////////////////////////////////////////
    void updateStepCircleDataParticles(auto& vec, const sf::Time deltaTime)
    {
        const auto deltaTimeMs = static_cast<float>(deltaTime.asMicroseconds()) / 1000.f;

        for (auto& p : vec)
        {
            p.velocity.y += p.accelerationY * deltaTimeMs;
            p.position += p.velocity * deltaTimeMs;

            p.rotation += p.torque * deltaTimeMs;
            p.rotation = sf::base::positiveRemainder(p.rotation, sf::base::tau);

            p.opacity = sf::base::clamp(p.opacity - p.opacityDecay * deltaTimeMs, 0.f, 1.f);
            p.scale   = sf::base::max(p.scale - p.scaleDecay * deltaTimeMs, 0.f);
        }

        sf::base::vectorEraseIf(vec, [](const auto& particleLike) { return particleLike.opacity <= 0.f; });
    }


    /////////////////////////////////////////////////////////////
    void updateStepRefillBlockMatrixIfNeeded()
    {
        if (m_world.blockMatrixBag.size() >= 3u) // TODO: adjust for peek?
            return;

        const auto oldBagSize = m_world.blockMatrixBag.size();

        constexpr sf::base::SizeT bagMult = 2u;

        const auto addToBag = [&](const TetraminoType j)
        {
            const Block block{
                .tetraminoId        = m_world.nextTetraminoId++,
                .blockId            = BlockId{0u}, // updated below
                .health             = Health{1u},
                .paletteIdx         = PaletteIdx::from(j),
                .shapeBlockSequence = ShapeBlockSequence::_, // set by `shapeMatrixToBlockMatrix`
                .powerup            = BlockPowerup::None,
                .tickTimer          = 0u,
                .tickTimerTarget    = nullTickTimerTarget,
            };

            auto& [blockMatrix, tetraminoType] = m_world.blockMatrixBag.pushBack({
                .blockMatrix = shapeMatrixToBlockMatrix(srsTetraminoShapes[static_cast<sf::base::SizeT>(j) /* pieceType */][0],
                                                        block),
                .tetraminoType = j,
            });

            const auto healthDist = generateTetraminoHealthDistribution(getDifficultyFactor(m_world.tick), m_rngFast);
            sf::base::SizeT nextHealthDistIdx = 0u;

            for (sf::base::Optional<Block>& b : blockMatrix.data)
            {
                if (!b.hasValue())
                    continue;

                b->blockId = m_world.nextBlockId++;

                const auto blockType = static_cast<sf::base::U8>(healthDist[nextHealthDistIdx++]);

                if (blockType > 2)
                {
                    b->health = blockType - 1u;
                }
                else if (blockType == 2)
                {
                    b->tickTimerTarget = static_cast<sf::base::U32>(secondsToTicks(20.f));
                }
                else if (roll100(1))
                {
                    b->health = 1u;

                    if (roll100(25))
                    {
                        b->powerup         = BlockPowerup::ThreeRowDrill;
                        b->tickTimerTarget = static_cast<sf::base::U32>(secondsToTicks(10.f));
                    }
                    else
                    {
                        b->powerup         = BlockPowerup::XPBonus;
                        b->tickTimerTarget = static_cast<sf::base::U32>(secondsToTicks(20.f));
                    }
                }
            }
        };

        for (sf::base::SizeT i = 0u; i < bagMult; ++i)
            for (sf::base::U8 j = 0u; j < static_cast<sf::base::U8>(tetraminoShapeCount); ++j)
                addToBag(static_cast<TetraminoType>(j));

        for (int i = 0; i < m_world.perkExtraLinePiecesInPool; ++i)
            addToBag(TetraminoType::I);

        // only shuffle the newly added items
        shuffleBag(m_world.blockMatrixBag.begin() + oldBagSize, m_world.blockMatrixBag.end(), m_rngFast);
    }


    /////////////////////////////////////////////////////////////
    void applyGravityToCurrentTetramino()
    {
        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        const auto newPosition = m_world.currentTetramino->position.addY(1);

        if (m_world.blockGrid.isValidMove(m_world.currentTetramino->shape, newPosition))
        {
            m_world.currentTetramino->position = newPosition;
            return;
        }

        if (m_world.graceDropMoves < m_world.maxGraceDropMoves)
        {
            ++m_world.graceDropMoves;
            return;
        }

        embedTetraminoAndClearLines(*m_world.currentTetramino);
        resetAndRedrawCurrentTetramino(/* usedHold */ false);

        playSound(m_sounds.place);
    }


    /////////////////////////////////////////////////////////////
    void updateStepProcessSimulation(const float xTicksPerSecond)
    {
        for (unsigned int i = 0; i < static_cast<unsigned int>(m_timeAccumulator * xTicksPerSecond); ++i)
        {
            m_timeAccumulator -= 1.f / xTicksPerSecond;

            ++m_world.tick;

            SFML_BASE_ASSERT(m_world.dropTickAccumulator < m_world.dropTickTarget);
            SFML_BASE_ASSERT(m_world.dropTickTarget > 0u);

            ++m_world.dropTickAccumulator;

            if (m_world.dropTickAccumulator == m_world.dropTickTarget)
            {
                m_world.dropTickAccumulator = 0u;
                applyGravityToCurrentTetramino();
            }

            for (auto& optBlock : m_world.blockGrid.getBlocks())
            {
                if (!optBlock.hasValue())
                    continue;

                if (optBlock->tickTimerTarget == nullTickTimerTarget)
                    continue;

                if (optBlock->tickTimer == optBlock->tickTimerTarget)
                    continue;

                ++optBlock->tickTimer;

                if (optBlock->tickTimer == optBlock->tickTimerTarget)
                {
                    optBlock->tickTimerTarget = nullTickTimerTarget;

                    if (optBlock->powerup != BlockPowerup::None)
                    {
                        // powerup expires
                        optBlock->powerup = BlockPowerup::None;
                    }
                    else
                    {
                        // health increases
                        ++optBlock->health;
                    }
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////
    void updateStepLightningBolts(const sf::Time deltaTime)
    {
        for (auto& lb : m_lightningBolts)
            lb.update(deltaTime);

        sf::base::vectorEraseIf(m_lightningBolts, [](const LightningBolt& lb) { return lb.isFinished(); });
    }


    /////////////////////////////////////////////////////////////
    void updateStepLaserBeam(const sf::Time deltaTime)
    {
        if (m_optLaserBeam.hasValue())
            m_optLaserBeam->update(deltaTime);
    }


    /////////////////////////////////////////////////////////////
    void updateStep(const sf::Time deltaTime, const float xTicksPerSecond)
    {
        SFEX_PROFILE_SCOPE("update");

        updateStepQuakeEffect(deltaTime);
        updateStepScreenShake(deltaTime);
        updateStepShowLevelUpScreenIfNeeded();
        updateStepInterpolateVisualTetraminoPosition(m_world.currentTetramino, m_currentTetraminoVisualCenter, deltaTime);
        updateStepAnimations(deltaTime);
        updateStepBlockEffects(deltaTime);
        updateStepEarnedXPParticles(deltaTime);
        updateStepCircleDataParticles(m_hueColorCircleShapeParticles, deltaTime);
        updateStepCircleDataParticles(m_fixedColorCircleShapeParticles, deltaTime);

        if (isInPlayableState())
            updateStepProcessSimulation(xTicksPerSecond);

        updateStepLightningBolts(deltaTime);
        updateStepLaserBeam(deltaTime);

        // TODO:
        if (m_menuDelayProgress < 1.f)
        {
            m_menuDelayProgress += deltaTime.asSeconds();
            m_menuDelayProgress = sf::base::min(m_menuDelayProgress, 1.f);
        }
    }


    /////////////////////////////////////////////////////////////
    void syncShaderUniforms()
    {
        if (!m_mustSyncShaderUniforms)
            return;

        m_mustSyncShaderUniforms = false;

        m_shaderCRT.setUniform(m_ulCurvature, m_fUlCurvature);
        m_shaderCRT.setUniform(m_ulVignetteStrength, m_fUlVignetteStrength);
        m_shaderCRT.setUniform(m_ulVignetteInnerRadius, m_fUlVignetteInnerRadius);
        m_shaderCRT.setUniform(m_ulVignetteOuterRadius, m_fUlVignetteOuterRadius);
        m_shaderCRT.setUniform(m_ulScanlineStrength, m_fUlScanlineStrength);
        m_shaderCRT.setUniform(m_ulScanlineBrightnessModulation, m_fUlScanlineBrightnessModulation);
        m_shaderCRT.setUniform(m_ulScanlineScrollSpeed, m_fUlScanlineScrollSpeed);
        m_shaderCRT.setUniform(m_ulScanlineThickness, m_fUlScanlineThickness);
        m_shaderCRT.setUniform(m_ulScanlineHeight, m_fUlScanlineHeight);
        m_shaderCRT.setUniform(m_ulNoiseStrength, m_fUlNoiseStrength);
        m_shaderCRT.setUniform(m_ulMaskStrength, m_fUlMaskStrength);
        m_shaderCRT.setUniform(m_ulMaskScale, m_fUlMaskScale);
        m_shaderCRT.setUniform(m_ulBloomStrength, m_fUlBloomStrength);
        m_shaderCRT.setUniform(m_ulInputGamma, m_fUlInputGamma);
        m_shaderCRT.setUniform(m_ulOutputGamma, m_fUlOutputGamma);
        m_shaderCRT.setUniform(m_ulSaturation, m_fUlSaturation);

        m_shaderPostProcess.setUniform(m_ulPPVibrance, m_fUlPPVibrance);
        m_shaderPostProcess.setUniform(m_ulPPSaturation, m_fUlPPSaturation);
        m_shaderPostProcess.setUniform(m_ulPPLightness, m_fUlPPLightness);
        m_shaderPostProcess.setUniform(m_ulPPSharpness, m_fUlPPSharpness);
    }


    /////////////////////////////////////////////////////////////
    void drawShop()
    {
        if (!m_inLevelUpScreen)
            return;

        SFML_BASE_ASSERT(m_world.committedPlayerLevel < m_world.playerLevel);

        const auto darkenAlpha = static_cast<sf::base::U8>((0.65f * m_menuDelayProgress) * 255.f);

        m_rtGame.draw(sf::RectangleShapeData{
            .position  = {0.f, 0.f},
            .fillColor = sf::Color::blackMask(darkenAlpha),
            .size      = resolution,
        });

        sf::Vec2f shopSize{280.f, 161.f};

        if (m_perkIndicesSelectedThisLevel.size() == 4u)
            shopSize.y += 53.f;

        const auto centeredShopPos = resolution * 0.5f;

        const float menuScale = easeInOutBack(m_menuDelayProgress);

        const auto menuTransform = sf::Transform::fromPositionScaleOrigin(centeredShopPos, {menuScale, menuScale}, shopSize * 0.5f);

        m_rtGame.draw(
            sf::RectangleShapeData{
                .position         = {0.f, 0.f},
                .origin           = {0.f, 0.f},
                .fillColor        = {0, 0, 0},
                .outlineColor     = {135, 135, 135},
                .outlineThickness = 1.f,
                .size             = shopSize,
            },
            {.transform = menuTransform});

        m_textVerticesBuffer.clear();
        m_textIndicesBuffer.clear();

        std::string levelUpString = "^bold[](^wobble[5,1.2,0.5](LEVEL UP)^)^";

        if (m_rerollsLeftThisLevel > 0u)
            levelUpString += std::format("^color[190,190,190]( - Press SHIFT to reroll ({} left))^", m_rerollsLeftThisLevel);

        const BitmapTextToVerticesOptions titleOpts = {
            .outVertices     = m_textVerticesBuffer,
            .outIndices      = m_textIndicesBuffer,
            .bitmapFont      = m_bitmapFontMinogram,
            .fontTextureRect = m_txrBFMinogram6x10,
            .alignment       = BitmapTextAlignment::Center,
            .baseColor       = sf::Color::White,
            .time            = m_totalTime,
            .string          = levelUpString,
        };

        auto            bounds       = bitmapTextToVertices<true>(titleOpts);
        const sf::Vec2f textPosition = {(shopSize.x - bounds.x) * 0.5f, -8.f};

        const auto titleGlobalBounds = bitmapTextToVerticesPretransformed(titleOpts,
                                                                          sf::Transform::fromPosition(textPosition));

        m_rtGame.draw(sf::RectangleShapeData{.position         = titleGlobalBounds.position - sf::Vec2f{8.f, 5.f},
                                             .origin           = {0.f, 0.f},
                                             .fillColor        = {0, 0, 0},
                                             .outlineColor     = {135, 135, 135},
                                             .outlineThickness = 1.f,
                                             .size             = titleGlobalBounds.size + sf::Vec2f{16.f, 8.f}},
                      {.transform = menuTransform});

        /*

        m_rtGame.draw(
            sf::RectangleShapeData{
                .position  = {4.f, titleGlobalBounds.getBottom() + 4.f},
                .origin    = {0.f, 0.f},
                .fillColor = {135, 135, 135},
                .size      = {shopSize.x - 8.f, 1.f},
            },
            {.transform = menuTransform});

        */

        auto perkDrawPos = sf::Vec2f{48.f, titleGlobalBounds.getBottom() + 8.f};

        sf::base::SizeT iPerk = 0u;

        for (const sf::base::SizeT psIndex : m_perkIndicesSelectedThisLevel)
        {
            const Perk& perk = *(m_perks[psIndex]);

            std::string perkName        = perk.getName();
            std::string perkDescription = wrapText(perk.getDescription(m_world), 38u);
            std::string perkProgression = wrapText(perk.getProgressionStr(m_world), 38u);

            const auto perkStr = std::format("^bold[]({})^\n^hspace[0](^color[190,190,190]({})^)^", perkName, perkDescription);

            const auto transform = sf::Transform::fromPosition(perkDrawPos);

            const auto globalBounds = bitmapTextToVerticesPretransformed(
                {
                    .outVertices     = m_textVerticesBuffer,
                    .outIndices      = m_textIndicesBuffer,
                    .bitmapFont      = m_bitmapFontMinogram,
                    .fontTextureRect = m_txrBFMinogram6x10,
                    .alignment       = BitmapTextAlignment::Left,
                    .baseColor       = sf::Color::White,
                    .time            = m_totalTime,
                    .string          = perkStr,
                },
                transform);

            m_rtGame.draw(sf::RectangleShapeData{.position  = globalBounds.position.withX(8),
                                                 .origin    = {0.f, 0.f},
                                                 .fillColor = sf::Color::VeryDarkBrown.withAlpha(100),
                                                 .size      = {32.f, 32.f}},
                          {.transform = menuTransform});

            m_rtGame.draw(sf::RectangleShapeData{.position  = globalBounds.position - sf::Vec2f{1.f, 1.f},
                                                 .origin    = {0.f, 0.f},
                                                 .fillColor = sf::Color::VeryDarkBrown.withAlpha(100),
                                                 .size = globalBounds.size.withY(sf::base::max(40.f, globalBounds.size.y)) +
                                                         sf::Vec2f{2.f, 2.f}},
                          {.transform = menuTransform});

            if (m_selectedPerk == iPerk)
            {
                m_rtGame.draw(sf::RectangleShapeData{.position  = globalBounds.position - sf::Vec2f{1.f, 1.f},
                                                     .origin    = {0.f, 0.f},
                                                     .fillColor = sf::Color::VeryDarkGreen,
                                                     .size      = globalBounds.size + sf::Vec2f{2.f, 2.f}},
                              {.transform = menuTransform});
            }

            perkDrawPos.y += sf::base::max(40.f, globalBounds.size.y) + 12.f;

            ++iPerk;
        }

        m_rtGame.drawIndexedVertices({
            .vertexData    = m_textVerticesBuffer.data(),
            .vertexCount   = m_textVerticesBuffer.size(),
            .indexData     = m_textIndicesBuffer.data(),
            .indexCount    = m_textIndicesBuffer.size(),
            .primitiveType = sf::PrimitiveType::Triangles,
            .renderStates =
                {
                    .transform = menuTransform,
                    .texture   = &m_textureAtlas.getTexture(),
                },
        });
    }


    /////////////////////////////////////////////////////////////
    void imguiStep(const sf::Time deltaTime)
    {
        SFEX_PROFILE_SCOPE("imgui");

        m_imGuiContext.update(m_window, deltaTime);
        return;

        {
            ImGui::Begin("Graphics settings", nullptr);

            {
                ImGui::Checkbox("CRT Shader", &m_useCRTShader);

                bool textureFiltering = m_rtGame.isSmooth();
                if (ImGui::Checkbox("Texture Filtering", &textureFiltering))
                    m_rtGame.setSmooth(textureFiltering);

                bool textureFilteringPost = m_rtPostProcess.isSmooth();
                if (ImGui::Checkbox("Texture Filtering (post)", &textureFilteringPost))
                    m_rtPostProcess.setSmooth(textureFilteringPost);

                const auto makeSlider = [&](float& value, const char* label, const float min, const float max)
                {
                    if (ImGui::SliderFloat(label, &value, min, max))
                    {
                        m_mustSyncShaderUniforms = true;
                        return true;
                    }

                    return false;
                };

                makeSlider(m_fUlCurvature, "Curvature", -0.5, 0.5);
                makeSlider(m_fUlVignetteStrength, "Vignette Strength", 0.f, 1.f);
                makeSlider(m_fUlVignetteInnerRadius, "Vignette Inner Radius", -1.f, 2.f);
                makeSlider(m_fUlVignetteOuterRadius, "Vignette Outer Radius", -1.f, 2.f);
                makeSlider(m_fUlScanlineStrength, "Scanline Strength", -1.f, 10.f);
                makeSlider(m_fUlScanlineBrightnessModulation, "Scanline Brightness Modulation", -5.f, 5.f);
                makeSlider(m_fUlScanlineScrollSpeed, "Scanline Scroll Speed", -100.f, 100.f);
                makeSlider(m_fUlScanlineThickness, "Scanline Thickness", -40.f, 40.f);
                makeSlider(m_fUlScanlineHeight, "Scanline Height", -10.f, 10.f);
                makeSlider(m_fUlNoiseStrength, "Noise Strength", -100.f, 100.f);
                makeSlider(m_fUlMaskStrength, "Mask Strength", -5.f, 5.f);
                makeSlider(m_fUlMaskScale, "Mask Scale", -5.f, 5.f);
                makeSlider(m_fUlBloomStrength, "Bloom Strength", -5.f, 5.f);
                makeSlider(m_fUlInputGamma, "Input Gamma", -5.f, 5.f);
                makeSlider(m_fUlOutputGamma, "Output Gamma", -5.f, 5.f);
                makeSlider(m_fUlSaturation, "Saturation", -5.f, 5.f);

                makeSlider(m_fUlPPVibrance, "Vibrance (post)", -3.f, 3.f);
                makeSlider(m_fUlPPSaturation, "Saturation (post)", -3.f, 3.f);
                makeSlider(m_fUlPPLightness, "Lightness (post)", -3.f, 3.f);
                makeSlider(m_fUlPPSharpness, "Sharpness (post)", -3.f, 3.f);
            }

            ImGui::End();
        }

        const auto  windowSize = m_window.getSize().toVec2f();
        const float scale      = getPixelPerfectScale(windowSize, resolution);

        const auto setFontScale = [&](const float x) { ImGui::SetWindowFontScale(x * scale / 2.f); };

        const auto textCentered = [&](const std::string& text)
        {
            const auto windowWidth = ImGui::GetWindowSize().x;
            const auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text("%s", text.c_str());
        };

        if (m_inLevelUpScreen)
        {
            SFML_BASE_ASSERT(m_world.committedPlayerLevel < m_world.playerLevel);

            ImGui::SetNextWindowBgAlpha(0.95f);
            ImGui::PushFont(m_imguiFont);

            ImGui::Begin("Level Up!", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

            const ImVec2 menuSize{100.f * scale, 200.f * scale};
            const ImVec2 menuPos{windowSize.x / 2.f - menuSize.x / 2.f, windowSize.y / 2.f - menuSize.y / 2.f};

            ImGui::SetWindowPos(menuPos);
            ImGui::SetWindowSize(menuSize);

            setFontScale(1.f);
            ImGui::PushFont(m_imguiFontBig);
            textCentered("*** LEVEL UP ***");
            ImGui::PopFont();
            setFontScale(1.f);
            textCentered("CHOOSE A PERK");
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();

            static int  selectedPerk = -1;
            static bool sep          = false;

            for (const sf::base::SizeT psIndex : m_perkIndicesSelectedThisLevel)
            {
                const Perk& perk = *(m_perks[psIndex]);

                std::string perkName        = perk.getName();
                std::string perkDescription = perk.getDescription(m_world);
                std::string perkProgression = perk.getProgressionStr(m_world);

                if (sep)
                    ImGui::Separator();

                // setFontScale(2.f);
                ImGui::PushFont(m_imguiFontBig);

                if (ImGui::Selectable(perkName.c_str(), selectedPerk == static_cast<int>(psIndex)))
                    selectedPerk = static_cast<int>(psIndex);

                ImGui::PopFont();

                setFontScale(0.5f);
                if (!perkProgression.empty())
                    ImGui::Text("(%s)\n", perkProgression.c_str());
                else
                    ImGui::Text("\n");
                setFontScale(1.f);
                ImGui::TextWrapped("%s", perkDescription.c_str());

                sep = true;
            }

            auto buttonCenteredOnLine = [&](const char* label, float alignment = 0.5f)
            {
                ImGuiStyle& style = ImGui::GetStyle();

                float size  = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.f;
                float avail = ImGui::GetContentRegionAvail().x;

                float off = (avail - size) * alignment;
                if (off > 0.f)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

                return ImGui::Button(label);
            };

            {
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::PushFont(m_imguiFontBig);
                setFontScale(1.f);

                if (selectedPerk == -1)
                    ImGui::BeginDisabled();

                if (buttonCenteredOnLine("Confirm"))
                {
                    m_inLevelUpScreen = false;
                    m_perkIndicesSelectedThisLevel.clear();

                    ++m_world.committedPlayerLevel;

                    m_perks[static_cast<sf::base::SizeT>(selectedPerk)]->apply(m_world);
                }

                if (selectedPerk == -1)
                    ImGui::EndDisabled();

                ImGui::PopFont();
            }

            ImGui::PopFont();
            ImGui::End();
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepBackground()
    {
        const sf::Vec2uz gridSize{m_world.blockGrid.getWidth(), m_world.blockGrid.getHeight() - gridGraceY};

        m_rtGame.draw(sf::RectangleShapeData{
            .position         = toDrawCoordinates(sf::Vec2uz{0, gridGraceY}),
            .origin           = floorVec2(drawBlockSize / 2.f),
            .fillColor        = {30, 30, 30},
            .outlineColor     = {35, 35, 35},
            .outlineThickness = 1.f,
            .size             = gridSize.toVec2f().componentWiseMul(drawBlockSize).addX(2.f).addY(2.f),
        });

        const auto dividerStartPos = toDrawCoordinates(sf::Vec2uz{0, gridGraceY});

        for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth() + 1u; ++x)
            for (sf::base::SizeT y = 0u; y < m_world.blockGrid.getHeight() - gridGraceY + 1u; ++y)
            {
                m_rtGame.draw(
                    sf::Sprite{
                        .position = dividerStartPos - drawBlockSize + sf::Vec2f{3.f, 3.f} +
                                    sf::Vec2uz{x, y}.toVec2f().componentWiseMul(drawBlockSize),
                        .textureRect = m_txrDivider,
                    },
                    {
                        .texture = &m_textureAtlas.getTexture(),
                        .shader  = &m_shader,
                    });
            }

        m_rtGame.draw(sf::RectangleShapeData{
            .position         = toDrawCoordinates(sf::Vec2uz{0, gridGraceY}) - sf::Vec2f{1.f, 1.f},
            .origin           = floorVec2(drawBlockSize / 2.f),
            .fillColor        = sf::Color::Transparent,
            .outlineColor     = {35, 35, 35},
            .outlineThickness = 1.f,
            .size             = gridSize.toVec2f().componentWiseMul(drawBlockSize).addX(4.f).addY(4.f),
        });

        m_rtGame.draw(sf::RectangleShapeData{
            .position         = toDrawCoordinates(sf::Vec2uz{0, gridGraceY}) - sf::Vec2f{3.f, 3.f},
            .origin           = floorVec2(drawBlockSize / 2.f),
            .fillColor        = sf::Color::Transparent,
            .outlineColor     = {135, 135, 135},
            .outlineThickness = 1.f,
            .size             = gridSize.toVec2f().componentWiseMul(drawBlockSize).addX(8.f).addY(8.f),
        });
    }


    /////////////////////////////////////////////////////////////
    void drawStepEmbeddedBlocks()
    {
        for (sf::base::SizeT y = gridGraceY; y < m_world.blockGrid.getHeight(); ++y)
            for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
            {
                const sf::Vec2uz gridPosition{x, y};
                const auto&      optBlock = m_world.blockGrid.at(gridPosition);

                if (!optBlock.hasValue())
                    continue;

                (void)drawBlock(*optBlock,
                                toDrawCoordinates(gridPosition),
                                {
                                    .opacity          = 1.f,
                                    .applyQuakeOffset = false,
                                    .drawTimer        = true,
                                });
            }
    }


    /////////////////////////////////////////////////////////////
    void drawStepFadingBlocks()
    {
        auto* fadeBlocks = m_animationTimelineP1.getIfPlaying<AnimFadeBlocks>();
        if (fadeBlocks == nullptr)
            return;

        const float progress = m_animationTimelineP1.getProgress();

        for (const auto& fadingBlock : fadeBlocks->fadingBlocks)
        {
            (void)drawBlock(fadingBlock.block,
                            toDrawCoordinates(fadingBlock.position),
                            {
                                .opacity          = 1.f,
                                .squishMult       = -easeInBackWithCustomOvershoot(progress, 6.f),
                                .applyYOffset     = false,
                                .applyQuakeOffset = false,
                                .drawTimer        = false,
                            });
        }
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] PaletteIdx getTetraminoPaletteIdx(const Tetramino& tetramino) const
    {
        for (const auto& b : tetramino.shape.data)
            if (b.hasValue())
                return b->paletteIdx;

        SFML_BASE_UNREACHABLE();
    }


    /////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrillableBlocksInfo
    {
        int       count;
        sf::Vec2i endPos;
    };


    /////////////////////////////////////////////////////////////
    [[nodiscard]] DrillableBlocksInfo countDrillableBlocks(const sf::Vec2i startPos, const DrillDirection::Enum direction)
    {
        DrillableBlocksInfo info{
            .count  = 0,
            .endPos = startPos + drillDirectionToVec2i(direction),
        };

        const auto horizontalIteration = [&](const int iX) -> ControlFlow
        {
            auto& optBlock = m_world.blockGrid.at(sf::Vec2i{iX, info.endPos.y});

            if (!optBlock.hasValue())
                return ControlFlow::Break;

            if (!optBlock->isDamageable())
                return ControlFlow::Continue;

            info.endPos.x = iX;

            if (++info.count >= m_world.perkDrill[direction]->maxPenetration)
                return ControlFlow::Break;

            return ControlFlow::Continue;
        };

        if (direction == DrillDirection::Left)
        {
            info.endPos.x = sf::base::max(info.endPos.x, 0);

            for (int iX = info.endPos.x; iX >= 0; --iX)
                if (horizontalIteration(iX) == ControlFlow::Break)
                    break;
        }
        else if (direction == DrillDirection::Right)
        {
            const auto gridWidth = static_cast<int>(m_world.blockGrid.getWidth());
            info.endPos.x        = sf::base::min(info.endPos.x, gridWidth - 1);

            for (int iX = info.endPos.x; iX < gridWidth; ++iX)
                if (horizontalIteration(iX) == ControlFlow::Break)
                    break;
        }
        else if (direction == DrillDirection::Down)
        {
            const auto gridHeight = static_cast<int>(m_world.blockGrid.getHeight());
            info.endPos.y         = sf::base::min(info.endPos.y, gridHeight - 1);

            for (int iY = info.endPos.y; iY < gridHeight; ++iY)
            {
                auto& optBlock = m_world.blockGrid.at(sf::Vec2i{info.endPos.x, iY});

                if (!optBlock.hasValue())
                    break;

                if (!optBlock->isDamageable())
                    continue;

                info.endPos.y = iY;

                if (++info.count >= m_world.perkDrill[DrillDirection::Down]->maxPenetration)
                    break;
            }
        }

        return info;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] LaserableBlocksInfo findLaserableBlocks(const sf::Vec2i startPos, const LaserDirection::Enum direction)
    {
        LaserableBlocksInfo result;

        const auto dir = laserDirectionToVec2i(direction);
        sf::Vec2i  pos = startPos + dir;

        while (m_world.blockGrid.isInBounds(pos))
        {
            auto& optBlock = m_world.blockGrid.at(pos);

            if (optBlock.hasValue() && optBlock->isDamageable())
                result.positions.emplaceBack(pos);

            pos += dir;
        }

        if (m_world.perkLaser[direction]->bounce)
        {
            const auto bounceDir = sf::Vec2i{-dir.x, dir.y};

            pos += sf::Vec2i{-dir.x, 0};
            result.bouncePos = pos;
            pos += bounceDir;

            while (m_world.blockGrid.isInBounds(pos))
            {
                auto& optBlock = m_world.blockGrid.at(pos);

                if (optBlock.hasValue() && optBlock->isDamageable())
                    result.bouncePositions.emplaceBack(pos);

                pos += bounceDir;
            }
        }

        return result;
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimDrill()
    {
        auto* drillAnim = m_animationTimelineP0.getIfPlaying<AnimDrill>();

        if (drillAnim == nullptr)
            return;

        drawStepActiveTetramino({
            .tetramino     = drillAnim->tetramino,
            .visualCenter  = toDrawCoordinates(drillAnim->tetramino.position) + drawBlockSize.toVec2f() * 2.f,
            .squishMult    = 0.f,
            .drawTetramino = false,
            .drawGhost     = false,
        });

        const sf::Vec2f offsetByDirection[] = {
            {-drawBlockSize.x / 2.f - 3.f + 2.f, 1.f}, // Left
            {drawBlockSize.x - 1.f, 2.f},              // Right
            {1.f, drawBlockSize.y - 1.f},              // Down
        };

        const sf::Angle arrayByDirection[] = {
            sf::degrees(90.f),  // Left
            sf::degrees(270.f), // Right
            sf::degrees(0.f),   // Down
        };

        const auto drillDrawOffset = offsetByDirection[static_cast<sf::base::SizeT>(drillAnim->direction)];
        const auto rotation        = arrayByDirection[static_cast<sf::base::SizeT>(drillAnim->direction)];

        const auto& tetramino = drillAnim->tetramino;

        const float progress   = m_animationTimelineP0.getProgress();
        const auto  paletteIdx = getTetraminoPaletteIdx(tetramino);

        for (const auto bPos : findDrillBlocks(tetramino, drillAnim->direction))
        {
            const auto startPos = tetramino.position + bPos.toVec2i();

            const auto [nDrillableBlocks, endPos] = countDrillableBlocks(startPos, drillAnim->direction);

            if (nDrillableBlocks == 0)
                continue;

            const auto startDrawPos = toDrawCoordinates(startPos.toVec2uz());
            const auto endDrawPos   = toDrawCoordinates(endPos.toVec2uz());
            const auto diff         = endDrawPos - startDrawPos;

            const int  nDrills = 1 + static_cast<int>(diff.length() / 2.5f);
            const auto radius  = drawBlockSize.x;

            const auto getDrawPosition = [&](int i)
            {
                return drillDrawOffset +
                       blend(startDrawPos,
                             startDrawPos + ((diff / static_cast<float>(nDrills)) * static_cast<float>(i + 1)),
                             easeInOutSine(bounce(progress)));
            };

            const auto gridWidth  = m_world.blockGrid.getWidth();
            const auto gridHeight = m_world.blockGrid.getHeight();

            const auto lastDrawPos = getDrawPosition(nDrills - 1);
            const auto lastGridPos = toGridCoordinates(floorVec2(lastDrawPos + sf::Vec2f{radius / 2.f, radius / 2.f}))
                                         .componentWiseClamp({0, 0}, sf::Vec2uz{gridWidth - 1, gridHeight - 1}.toVec2i());

            const auto& optBlock = m_world.blockGrid.at(lastGridPos);


            if (optBlock.hasValue())
            {
                for (int i = 0; i < 2; ++i)
                    m_hueColorCircleShapeParticles.emplaceBack(ParticleData{
                        .position = lastDrawPos - drillDrawOffset +
                                    drillDirectionToVec2i(drillAnim->direction).toVec2f() * (radius / 2.f) +
                                    m_rngFast.getVec2f({-3.f, -3.f}, {3.f, 3.f}),
                        .velocity      = m_rngFast.getVec2f({-0.75f, -2.15f}, {0.75f, -0.25f}) * 0.05f,
                        .scale         = m_rngFast.getF(0.08f, 0.27f) * 0.95f,
                        .scaleDecay    = 0.f,
                        .accelerationY = 0.0004f,
                        .opacity       = 0.95f,
                        .opacityDecay  = m_rngFast.getF(0.001f, 0.002f) * 0.5f,
                        .rotation      = m_rngFast.getF(0.f, sf::base::tau),
                        .torque        = m_rngFast.getF(-0.001f, 0.001f),
                        .color         = hueColorFromPaletteIdx(optBlock->paletteIdx, 255u),
                        .radius        = m_rngFast.getF(6.f, 12.f),
                        .pointCount    = 3u,
                    });
            }

            for (int i = 0; i < nDrills; ++i)
            {
                m_rtGame.draw(m_textureAtlas.getTexture(),
                              {
                                  .position    = floorVec2(getDrawPosition(i)),
                                  .origin      = floorVec2(sf::Vec2f{radius / 2.f, radius / 2.f}),
                                  .rotation    = rotation,
                                  .textureRect = m_txrDrill,
                                  .color       = hueColorFromPaletteIdx(paletteIdx, 255u),
                              },
                              {.shader = &m_shader});
            }
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimLaser()
    {
        auto* laserAnim = m_animationTimelineP0.getIfPlaying<AnimLaser>();

        if (laserAnim == nullptr)
            return;

        if (!m_optLaserBeam.hasValue())
            return;

        drawStepActiveTetramino({
            .tetramino     = laserAnim->tetramino,
            .visualCenter  = toDrawCoordinates(laserAnim->tetramino.position) + drawBlockSize.toVec2f() * 2.f,
            .squishMult    = 0.f,
            .drawTetramino = false,
            .drawGhost     = false,
        });

        const auto targetPos = toDrawCoordinates(laserAnim->gridTargetPos) +
                               m_rngFast.getVec2f(-drawBlockSize, drawBlockSize) / 4.f;

        const auto progress = m_animationTimelineP0.getProgress();
        m_optLaserBeam->end = m_optLaserBeam->start + (targetPos - m_optLaserBeam->start) * easeInOutBack(progress);

        for (int i = 0; i < 3; ++i)
            m_fixedColorCircleShapeParticles.emplaceBack(ParticleData{
                .position      = m_optLaserBeam->end,
                .velocity      = m_rngFast.getVec2f({-0.75f, -2.15f}, {0.75f, -0.25f}) * 0.25f,
                .scale         = m_rngFast.getF(0.08f, 0.27f) * 0.75f,
                .scaleDecay    = 0.f,
                .accelerationY = 0.0004f,
                .opacity       = 0.75f,
                .opacityDecay  = m_rngFast.getF(0.001f, 0.002f) * 0.7f,
                .rotation      = m_rngFast.getF(0.f, sf::base::tau),
                .torque        = m_rngFast.getF(-0.001f, 0.001f),
                .color         = blockPalette[getTetraminoPaletteIdx(laserAnim->tetramino)],
                .radius        = m_rngFast.getF(4.f, 7.f),
                .pointCount    = 5u,
            });
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimFadeAttachments()
    {
        auto* fadeAttachments = m_animationTimelineP0.getIfPlaying<AnimFadeAttachments>();

        if (fadeAttachments == nullptr)
            return;

        drawStepActiveTetramino({
            .tetramino     = fadeAttachments->tetramino,
            .visualCenter  = toDrawCoordinates(fadeAttachments->tetramino.position) + drawBlockSize.toVec2f() * 2.f,
            .squishMult    = 0.f - m_animationTimelineP0.getProgress(),
            .drawTetramino = false,
            .drawGhost     = false,
        });
    }


    /////////////////////////////////////////////////////////////
    static inline constexpr sf::Vec2f tetraminoVisualCenterOffset = (drawBlockSize * static_cast<float>(shapeDimension)) / 2.f;


    /////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] sf::Vec2f getTetraminoCenterDrawPosition(const sf::Vec2<T> tetraminoGridPosition) const
    {
        // 1. Get the screen position of the top-left corner of the tetramino's grid cell.
        const sf::Vec2f topLeftDrawPosition = toDrawCoordinates(tetraminoGridPosition);

        // 2. The pivot is the center of the 4x4 shape. Find the offset from the top-left to this pivot.
        // 3. The final center is the top-left position plus the offset.
        return topLeftDrawPosition + tetraminoVisualCenterOffset;
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] sf::Vec2f getDrawPositionOfLocalBlock(const sf::Vec2<T>& localBlockGridPos,
                                                        const sf::Vec2f    tetraminoCenter) const
    {
        // 1. The pivot is the center of the 4x4 shape in its own local space.
        const sf::Vec2f localPivot = tetraminoVisualCenterOffset;

        // 2. Find the center of the specific block in local space.
        const sf::Vec2f localBlockCenter = localBlockGridPos.toVec2f().componentWiseMul(drawBlockSize) +
                                           (drawBlockSize / 2.f);

        // 3. Get the block's position vector relative to the tetramino's pivot.
        const sf::Vec2f positionRelativeToPivot = localBlockCenter - localPivot;

        // 4. The final world position is the tetramino's world center plus this relative vector.
        return floorVec2(tetraminoCenter + positionRelativeToPivot);
    }


    ////////////////////////////////////////////////////////////
    void drawDrillSpikesForPerk(
        const ShapeBlockPositionVector& localBlockPositions,
        const sf::Vec2f                 offset,
        const sf::Color                 color,
        const sf::Angle                 rotation,
        const sf::Vec2f                 mainTetraminoCenter,
        const sf::Vec2f                 ghostTetraminoCenter,
        const bool                      drawGhost,
        const float                     squishMult)
    {
        const auto mainColor  = color;
        const auto ghostColor = mainColor.withAlpha(64);

        float finalSquishMult = 1.f + squishMult;

        sf::Sprite spike{
            .scale       = sf::Vec2f{finalSquishMult, finalSquishMult},
            .origin      = floorVec2(sf::Vec2f{drawBlockSize.x / 2.f, drawBlockSize.y / 2.f}),
            .rotation    = rotation,
            .textureRect = m_txrDrill,
        };

        for (const auto& bPos : localBlockPositions)
        {
            const sf::Vec2f mainBlockDrawPos  = getDrawPositionOfLocalBlock(bPos, mainTetraminoCenter);
            const sf::Vec2f ghostBlockDrawPos = getDrawPositionOfLocalBlock(bPos, ghostTetraminoCenter);

            // Draw main spike
            spike.position = floorVec2(offset + mainBlockDrawPos.addX(sf::base::floor(-drawBlockSize.x / 2.f)));
            spike.color    = mainColor;
            m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

            // Draw ghost spike
            if (drawGhost)
            {
                spike.position = floorVec2(offset + ghostBlockDrawPos.addY(sf::base::floor(drawBlockSize.y / 2.f))) -
                                 sf::Vec2f{1.f, 1.f};
                spike.color = ghostColor;
                m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});
            }
        }
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f calculateLaserGridIntersection(const sf::Vec2f startPos, const LaserDirection::Enum direction) const
    {
        // 1. Define grid boundaries in draw coordinates. The playable grid area starts below gridGraceY.
        const float left  = toDrawCoordinates(sf::Vec2f{0.f, 0.f}).x;
        const float right = toDrawCoordinates(sf::Vec2f{static_cast<float>(m_world.blockGrid.getWidth() - 1), 0.f}).x;
        const float top   = toDrawCoordinates(sf::Vec2f{0.f, static_cast<float>(gridGraceY)}).y;
        const float bottom = toDrawCoordinates(sf::Vec2f{0.f, static_cast<float>(m_world.blockGrid.getHeight() - 1)}).y + 1.f;

        // 2. Define the ray using its starting position and direction vector in draw space.
        const sf::Vec2f rayDirGrid = laserDirectionToVec2i(direction).toVec2f();
        const sf::Vec2f rayDirDraw = rayDirGrid.componentWiseMul(drawBlockSize);

        // Avoid division by zero if direction is a zero vector.
        if (rayDirDraw.x == 0.f && rayDirDraw.y == 0.f)
            return startPos;

        float minT = 10000.f;

        // 3. Calculate the parametric 't' value for intersection with each of the four boundary lines.
        // A valid intersection occurs if t > 0 (in front of the ray) and the intersection
        // point lies on the boundary segment. The smallest valid 't' is the first point of impact.

        // Check intersection with vertical walls (left/right).
        if (rayDirDraw.x != 0.f)
        {
            if (direction == LaserDirection::Left)
            {
                const float tLeft = (left - startPos.x) / rayDirDraw.x;
                if (tLeft > 0.f)
                {
                    const float y = startPos.y + tLeft * rayDirDraw.y;
                    if (y >= top && y <= bottom)
                        minT = sf::base::min(minT, tLeft);
                }
            }
            else
            {
                const float tRight = (right - startPos.x) / rayDirDraw.x;
                if (tRight > 0.f)
                {
                    const float y = startPos.y + tRight * rayDirDraw.y;
                    if (y >= top && y <= bottom)
                        minT = sf::base::min(minT, tRight);
                }
            }
        }

        // Check intersection with horizontal walls (top/bottom).
        if (rayDirDraw.y != 0.f)
        {
            const float tBottom = (bottom - startPos.y) / rayDirDraw.y;
            if (tBottom > 0.f)
            {
                const float x = startPos.x + tBottom * rayDirDraw.x;
                if (x >= left && x <= right)
                    minT = sf::base::min(minT, tBottom);
            }
        }

        // 4. If a valid intersection was found (minT is not infinity), calculate the precise point.
        if (minT < 10000.f)
            return startPos + minT * rayDirDraw;

        // Fallback: If no intersection is found (e.g., ray points away from grid),
        // return a point far along the ray's direction.
        return startPos + 5.f * rayDirDraw;
    }


    ////////////////////////////////////////////////////////////
    void drawLaserEmittersForPerk(
        const ShapeBlockPositionVector& localBlockPositions,
        const sf::Vec2f                 offset,
        const sf::Color                 color,
        const sf::Angle                 rotation,
        const sf::Vec2f                 mainTetraminoCenter,
        const sf::Vec2f                 ghostTetraminoCenter,
        const bool                      drawGhost,
        const float                     squishMult)
    {
        const auto mainColor  = color;
        const auto ghostColor = mainColor.withAlpha(64);

        const auto laserDirection = rotation == sf::degrees(45.f) ? LaserDirection::Left : LaserDirection::Right;
        const auto laserDir       = laserDirectionToVec2i(laserDirection);

        float finalSquishMult = 1.f + squishMult;

        sf::Sprite spike{
            .scale       = sf::Vec2f{finalSquishMult, finalSquishMult},
            .origin      = floorVec2(sf::Vec2f{drawBlockSize.x / 2.f, drawBlockSize.y / 2.f} - sf::Vec2f{2.f, 2.f}),
            .rotation    = rotation + sf::degrees(180.f + 45.f),
            .textureRect = m_txrEmitter,
        };

        const auto drawGuide = [this, &mainColor](const sf::Vec2f            startPos,
                                                  const LaserDirection::Enum guideLaserDirection,
                                                  const sf::Angle            guideRotation)
        {
            const auto guideLaserDir = laserDirectionToVec2i(guideLaserDirection);
            const auto guideOffset   = guideLaserDir.toVec2f() * 2.f;

            auto endPos = calculateLaserGridIntersection(startPos - guideLaserDir.toVec2f() * 6.f, guideLaserDirection) +
                          guideLaserDir.toVec2f() * 5.f - guideOffset;

            if (guideLaserDirection == LaserDirection::Right)
                endPos += guideLaserDir.toVec2f() * 3.f;

            sf::RectangleShape guide{{
                .rotation    = guideRotation.wrapUnsigned(),
                .textureRect = m_txrRedDot,
                .size        = sf::Vec2f{1.f, (endPos - startPos).length()},
            }};

            const auto guideAnchorPoint = guideLaserDirection == LaserDirection::Left ? sf::Vec2f{1.f, 0.f} // top-right
                                                                                      : sf::Vec2f{0.f, 0.f}; // top-left

            guide.setFillColor(mainColor.withAlpha(32));
            guide.setSize({1.f, (endPos - startPos).length()});
            guide.setAnchorPoint(guideAnchorPoint, startPos + guideOffset);
            m_rtGame.draw(guide, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

            guide.setFillColor(mainColor.withAlpha(16));
            guide.setSize({2.f, (endPos - startPos).length()});
            guide.setAnchorPoint(guideAnchorPoint, startPos + guideOffset);
            m_rtGame.draw(guide, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

            return endPos;
        };

        for (const auto& bPos : localBlockPositions)
        {
            const sf::Vec2f mainBlockDrawPos  = getDrawPositionOfLocalBlock(bPos, mainTetraminoCenter);
            const sf::Vec2f ghostBlockDrawPos = getDrawPositionOfLocalBlock(bPos, ghostTetraminoCenter);

            const auto mainSpikePos = floorVec2(offset + mainBlockDrawPos.addX(sf::base::floor(-drawBlockSize.x / 2.f)));

            // Draw main spike
            spike.position = mainSpikePos + (laserDir * 4).toVec2f();
            spike.color    = mainColor;
            m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

            if (!drawGhost)
                continue;

            const auto ghostSpikePos = floorVec2(offset + ghostBlockDrawPos.addY(sf::base::floor(drawBlockSize.y / 2.f))) -
                                       sf::Vec2f{1.f, 1.f};

            // Draw ghost spike
            spike.position = ghostSpikePos + (laserDir * 4).toVec2f();
            spike.color    = ghostColor;
            m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

            const auto endPos = drawGuide(ghostSpikePos, laserDirection, rotation);

            if (m_world.perkLaser[laserDirection]->bounce)
            {
                const auto bounceOffset = laserDirection == LaserDirection::Left ? sf::Vec2f{-4.f, -2.f}
                                                                                 : sf::Vec2f{4.f, -2.f};

                (void)drawGuide(endPos + bounceOffset,
                                laserDirection == LaserDirection::Left ? LaserDirection::Right : LaserDirection::Left,
                                sf::degrees(180.f) - rotation);
            }
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimHardDrop()
    {
        auto* hardDrop = m_animationTimelineP1.getIfPlaying<AnimHardDrop>();

        if (hardDrop == nullptr)
            return;

        const float progress = m_animationTimelineP1.getProgress();

        const float targetVisualY = toDrawCoordinates(hardDrop->tetramino.position.withY(hardDrop->endY)).y +
                                    drawBlockSize.y / 2.f;

        const float startVisualY = toDrawCoordinates(hardDrop->tetramino.position).y + drawBlockSize.y * 2.f;

        const auto visualCenterY = startVisualY + (targetVisualY - startVisualY) * easeInBack(progress);
        const auto visualCenterX = toDrawCoordinates(hardDrop->tetramino.position).x + drawBlockSize.x * 2.f;

        drawStepActiveTetramino({
            .tetramino     = hardDrop->tetramino,
            .visualCenter  = {visualCenterX, visualCenterY},
            .squishMult    = 0.f,
            .drawTetramino = true,
            .drawGhost     = false,
        });
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimSquish()
    {
        auto* squish = m_animationTimelineP1.getIfPlaying<AnimSquish>();

        if (squish == nullptr)
            return;

        drawStepActiveTetramino({
            .tetramino     = squish->tetramino,
            .visualCenter  = toDrawCoordinates(squish->tetramino.position) + drawBlockSize.toVec2f() * 2.f,
            .squishMult    = easeInOutSine(bounce(m_animationTimelineP1.getProgress())) * 0.5f,
            .drawTetramino = true,
            .drawGhost     = false,
        });
    }


    /////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawActiveTetraminoOptions // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        Tetramino& tetramino;
        sf::Vec2f  visualCenter;
        float      squishMult;
        bool       drawTetramino;
        bool       drawGhost;
    };


    /////////////////////////////////////////////////////////////
    void drawStepActiveTetramino(const DrawActiveTetraminoOptions& options)
    {
        const auto& [tetramino, visualCenter, squishMult, mustDrawTetramino, mustDrawGhost] = options;

        const auto color = hueColorFromPaletteIdx(getTetraminoPaletteIdx(tetramino), 255u);

        auto tetraminoDrawPosition = floorVec2(visualCenter - drawBlockSize / 2.f) + sf::Vec2f{1, 1};

        const sf::Vec2f ghostGridPosition = tetramino.position.toVec2f().withY(
            static_cast<float>(calculateGhostY(tetramino)));

        const sf::Vec2f ghostCenterDrawPosition = getTetraminoCenterDrawPosition(ghostGridPosition)
                                                      .withX(tetraminoDrawPosition.x)
                                                      .addY(-drawBlockSize.y / 2.f)
                                                      .addY(1.f);

        if (m_world.perkDrill[DrillDirection::Down].hasValue())
            drawDrillSpikesForPerk(findDrillBlocks(tetramino, DrillDirection::Down),
                                   floorVec2(sf::Vec2f{0.f + 2.f, drawBlockSize.y / 2.f + 2.f}),
                                   color,
                                   sf::degrees(0.f),
                                   visualCenter,
                                   ghostCenterDrawPosition,
                                   mustDrawGhost,
                                   squishMult);

        if (m_world.perkDrill[DrillDirection::Left].hasValue())
            drawDrillSpikesForPerk(findDrillBlocks(tetramino, DrillDirection::Left),
                                   floorVec2(sf::Vec2f{-drawBlockSize.x / 2.f - 2.f, -drawBlockSize.y / 2.f + 2.f}),
                                   color,
                                   sf::degrees(90.f),
                                   visualCenter,
                                   ghostCenterDrawPosition,
                                   mustDrawGhost,
                                   squishMult);

        if (m_world.perkDrill[DrillDirection::Right].hasValue())
            drawDrillSpikesForPerk(findDrillBlocks(tetramino, DrillDirection::Right),
                                   floorVec2(sf::Vec2f{drawBlockSize.x + 2.f, -1.f - 2.f}),
                                   color,
                                   sf::degrees(270.f),
                                   visualCenter,
                                   ghostCenterDrawPosition,
                                   mustDrawGhost,
                                   squishMult);

        if (mustDrawTetramino)
            (void)drawTetramino(tetramino.shape,
                                tetraminoDrawPosition,
                                {
                                    .squishMult = squishMult,
                                    .drawTimer  = true,
                                });

        if (mustDrawGhost)
            (void)drawTetramino(tetramino.shape,
                                ghostCenterDrawPosition,
                                {
                                    .opacity    = 0.25f,
                                    .squishMult = squishMult,
                                    .drawTimer  = false,
                                });


        if (m_world.perkLaser[LaserDirection::Left].hasValue())
            drawLaserEmittersForPerk(findLaserBlocks(tetramino, LaserDirection::Left),
                                     {0.f, -1.f},
                                     color,
                                     sf::degrees(45.f),
                                     visualCenter,
                                     ghostCenterDrawPosition,
                                     mustDrawGhost,
                                     squishMult);

        if (m_world.perkLaser[LaserDirection::Right].hasValue())
            drawLaserEmittersForPerk(findLaserBlocks(tetramino, LaserDirection::Right),
                                     floorVec2(sf::Vec2f{drawBlockSize.x / 2.f, -1.f}),
                                     color,
                                     sf::degrees(315.f),
                                     visualCenter,
                                     ghostCenterDrawPosition,
                                     mustDrawGhost,
                                     squishMult);
    }


    /////////////////////////////////////////////////////////////
    void drawStepUINextTetraminos()
    {
        const sf::base::SizeT nPeek = sf::base::min(static_cast<sf::base::SizeT>(m_world.perkNPeek),
                                                    m_world.blockMatrixBag.size());

        constexpr float uiTetraminoScale = 9.f / drawBlockSize.x;

        const auto hudPos = getHudPos();

        sf::Vec2f uiBoxCenter = sf::Vec2f{hudPos.x + 12.f, hudPos.y + 68.f + 24.f} + sf::Vec2f{16.f, 16.f};

        for (sf::base::SizeT iPeek = 0u; iPeek < nPeek; ++iPeek)
        {
            const auto& shape = m_world.blockMatrixBag[iPeek].blockMatrix;

            const auto globalBounds = drawTetramino(shape,
                                                    uiBoxCenter,
                                                    {
                                                        .opacity = 1.f - static_cast<float>(iPeek) * 0.1f,
                                                        .scale   = uiTetraminoScale - static_cast<float>(iPeek) * 0.05f,
                                                        .drawText         = true,
                                                        .applyYOffset     = false,
                                                        .applyQuakeOffset = false,
                                                        .drawTimer        = false,
                                                    });

            uiBoxCenter.x = globalBounds.getCenterRight().x + 24.f;
            uiBoxCenter.y -= static_cast<float>(iPeek) * 0.1f;
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepUIHeldTetramino()
    {
        if (!m_world.heldTetramino.hasValue())
            return;

        const auto&     shape       = m_world.heldTetramino->shape;
        const auto      hudPos      = getHudPos();
        const sf::Vec2f uiBoxCenter = {hudPos.x + 128.f + 10.f, hudPos.y};

        (void)drawTetramino(shape, uiBoxCenter + sf::Vec2f{20.f, 48.f});
    }


    /////////////////////////////////////////////////////////////
    void drawStepLightningBolts()
    {
        for (auto& lb : m_lightningBolts)
            lb.draw(m_rtGame, {.blendMode = sf::BlendAdd});

        if (m_optLaserBeam.hasValue())
        {
            m_optLaserBeam->draw(m_rtGame, {.blendMode = sf::BlendAdd});
            m_optLaserBeam->draw(m_rtGame, {.blendMode = sf::BlendAdd});
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepEarnedXPParticles()
    {
        const auto bezier = [](const sf::Vec2f start, const sf::Vec2f end, const float t)
        {
            const sf::Vec2f control(start.x, end.y);
            const float     u = 1.f - t;

            return u * u * start + 2.f * u * t * control + t * t * end;
        };

        for (const auto& particle : m_earnedXPParticles)
        {
            const auto newPos = bezier(particle.startPosition, particle.targetPosition, easeInOutSine(particle.progress.value));

            const auto newPos2 = bezier(particle.startPosition,
                                        particle.targetPosition,
                                        easeInOutBack(particle.progress.value));

            const auto alpha = static_cast<sf::base::U8>((particle.progress.remapBouncedEased(easeInOutQuint, 64.f, 255.f)));

            m_rtGame.draw(
                sf::CircleShapeData{
                    .position = {blend(newPos2.x, newPos.x, 0.5f), newPos.y},
                    .scale    = sf::Vec2f{0.25f, 0.25f} * particle.progress.remapBounced(0.6f, 2.f),
                    .origin   = {12.f, 12.f},
                    .rotation = sf::radians(
                        sf::base::fmod(particle.startRotation + particle.progress.remap(0.f, sf::base::tau * 2.f),
                                       sf::base::tau)),
                    .textureRect = m_txrRedDot,
                    .fillColor   = hueColorFromPaletteIdx(particle.paletteIdx, alpha),
                    .radius      = 12.f,
                    .pointCount  = 3u,
                },
                {
                    .texture = &m_textureAtlas.getTexture(),
                    .shader  = &m_shader,
                });
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepCircleDataParticles()
    {
        for (const auto& particle : m_hueColorCircleShapeParticles)
            m_rtGame.draw(particleToCircleData(particle), {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

        for (const auto& particle : m_fixedColorCircleShapeParticles)
            m_rtGame.draw(particleToCircleData(particle), {});
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f getHudPos()
    {
        const sf::Vec2uz gridSize{m_world.blockGrid.getWidth(), m_world.blockGrid.getHeight() - gridGraceY};
        const auto       bgPosition = toDrawCoordinates(sf::Vec2uz{0, gridGraceY}) - sf::Vec2f{1.f, 1.f};
        const auto       bgSize     = gridSize.toVec2f().componentWiseMul(drawBlockSize).addX(4.f).addY(4.f);

        const auto hudStart = bgPosition + sf::Vec2f{bgSize.x + 4.f, -11.f};

        return hudStart;
    }


    /////////////////////////////////////////////////////////////
    void drawStepStatsText()
    {
        sf::RectangleShape statsBorder{{
            .position         = getHudPos().addY(4.f).addX(-1.f),
            .fillColor        = sf::Color::Transparent,
            .outlineColor     = {135, 135, 135},
            .outlineThickness = 1.f,
            .size             = {128.f, 64.f},
        }};

        m_rtGame.draw(statsBorder);

        m_textVerticesBuffer.clear();
        m_textIndicesBuffer.clear();

        auto statsStr = std::format(
            "^bold[](Level)^: {}\n"
            "^bold[](XP)^: {} / {}\n"
            "^bold[](Clock)^: {}s\n"
            "^bold[](Lines Cleared)^: {}\n"
            "^bold[](Pieces Placed)^: {}\n"
            "^bold[](Difficulty)^: {}",

            m_world.playerLevel,
            m_world.currentXP,
            getXPNeededForLevelUp(m_world.playerLevel),
            getElapsedSeconds(m_world.tick),
            m_world.linesCleared,
            m_world.tetaminosPlaced,
            getDifficultyFactor(m_world.tick));


        (void)bitmapTextToVerticesPretransformed(
            {
                .outVertices     = m_textVerticesBuffer,
                .outIndices      = m_textIndicesBuffer,
                .bitmapFont      = m_bitmapFontMinogram,
                .fontTextureRect = m_txrBFMinogram6x10,
                .alignment       = BitmapTextAlignment::Left,
                .baseColor       = sf::Color::White,
                .time            = m_totalTime,
                .string          = statsStr,
            },
            sf::Transform::fromPosition(statsBorder.getTopLeft() + sf::Vec2f{4.f, 2.f}));

        m_rtGame.drawIndexedVertices({
            .vertexData    = m_textVerticesBuffer.data(),
            .vertexCount   = m_textVerticesBuffer.size(),
            .indexData     = m_textIndicesBuffer.data(),
            .indexCount    = m_textIndicesBuffer.size(),
            .primitiveType = sf::PrimitiveType::Triangles,
            .renderStates  = {.texture = &m_textureAtlas.getTexture()},
        });
    }


    /////////////////////////////////////////////////////////////
    void drawStepPerksText()
    {
        sf::RectangleShape statsBorder{{
            .position         = getHudPos().addY(168.f - 48.f).addX(-1.f),
            .fillColor        = sf::Color::Transparent,
            .outlineColor     = {135, 135, 135},
            .outlineThickness = 1.f,
            .size             = {184.f, 64.f + 48.f},
        }};

        m_rtGame.draw(statsBorder);

        std::string perksStr;

        for (const auto& perk : m_perks)
            if (perk->isActive(m_world))
                perksStr += std::format("- {} {}\n", perk->getName(), perk->getInventoryStr(m_world));

        m_textVerticesBuffer.clear();
        m_textIndicesBuffer.clear();

        (void)bitmapTextToVerticesPretransformed(
            {
                .outVertices     = m_textVerticesBuffer,
                .outIndices      = m_textIndicesBuffer,
                .bitmapFont      = m_bitmapFontTiny5,
                .fontTextureRect = m_txrBFTiny5,
                .alignment       = BitmapTextAlignment::Left,
                .baseColor       = sf::Color::White,
                .time            = m_totalTime,
                .string          = perksStr,
            },
            sf::Transform::fromPosition(statsBorder.getTopLeft() + sf::Vec2f{4.f, 2.f}));

        m_rtGame.drawIndexedVertices({
            .vertexData    = m_textVerticesBuffer.data(),
            .vertexCount   = m_textVerticesBuffer.size(),
            .indexData     = m_textIndicesBuffer.data(),
            .indexCount    = m_textIndicesBuffer.size(),
            .primitiveType = sf::PrimitiveType::Triangles,
            .renderStates  = {.texture = &m_textureAtlas.getTexture()},
        });
    }


    /////////////////////////////////////////////////////////////
    void drawStep()
    {
        SFEX_PROFILE_SCOPE("draw");

        syncShaderUniforms();

        m_rtGame.clear(sf::Color{9, 9, 9});

        {
            SFEX_PROFILE_SCOPE("rtGame");

            drawStepBackground();
            drawStepEmbeddedBlocks();
            drawStepFadingBlocks();

            drawStepAnimHardDrop();
            drawStepAnimSquish();

            if (m_world.currentTetramino.hasValue())
                drawStepActiveTetramino({
                    .tetramino     = *m_world.currentTetramino,
                    .visualCenter  = m_currentTetraminoVisualCenter,
                    .squishMult    = 0.f,
                    .drawTetramino = true,
                    .drawGhost     = true,
                });

            drawStepEarnedXPParticles();
            drawStepCircleDataParticles();

            drawStepAnimDrill();
            drawStepAnimLaser();
            drawStepAnimFadeAttachments();

            drawStepUINextTetraminos();
            drawStepUIHeldTetramino();
            drawStepLightningBolts();

            drawStepStatsText();
            drawStepPerksText();

            if (m_world.perkNPeek > 0)
                m_rtGame.draw(m_font,
                              sf::TextData{
                                  .position      = getHudPos().addY(68.f).addX(4.f),
                                  .string        = "Next:",
                                  .characterSize = 16u,
                                  .outlineColor  = sf::Color::White,
                              });

            if (m_world.perkCanHoldTetramino == 1)
                m_rtGame.draw(m_font,
                              sf::TextData{
                                  .position      = getHudPos().addX(128.f + 10.f),
                                  .string        = "Held:",
                                  .characterSize = 16u,
                                  .outlineColor  = sf::Color::White,
                              });

            drawShop();
        }

        m_rtGame.display();


        const auto screenShake = m_rngFast.getVec2f({-m_screenShakeAmount, -m_screenShakeAmount},
                                                    {m_screenShakeAmount, m_screenShakeAmount});


        float scale = 1.f;

        // figure out scale that fits in the window (pixel-perfect)
        {
            const sf::Vec2f windowSize = m_window.getSize().toVec2f();
            const sf::Vec2f rtGameSize = m_rtGame.getSize().toVec2f();

            scale = sf::base::floor(sf::base::min(windowSize.x / rtGameSize.x, windowSize.y / rtGameSize.y));
        }

        // figure out position to center the rtGame texture in the window
        const sf::Vec2f windowSize = m_window.getSize().toVec2f();
        const sf::Vec2f rtGameSize = m_rtGame.getSize().toVec2f() * scale;

        m_shaderCRT.setUniform(m_ulInputSize, m_rtGame.getSize().toVec2f());
        m_shaderCRT.setUniform(m_ulTime, m_totalTime);

        if (m_rtPostProcess.getSize() != rtGameSize.toVec2u())
            m_rtPostProcess = sf::RenderTexture::create(rtGameSize.toVec2u()).value();

        m_rtPostProcess.clear();
        m_rtPostProcess.draw(m_rtGame.getTexture(), {.shader = m_useCRTShader ? &m_shaderCRT : nullptr});
        m_rtPostProcess.display();

        const sf::Vec2f centeredPosition = (windowSize - rtGameSize) / 2.f;
        const float     quakeYOffset     = m_quakeSinEffectHardDrop.getValue() + m_quakeSinEffectLineClear.getValue();
        const sf::Vec2f finalPosition    = floorVec2(centeredPosition + screenShake.addY(quakeYOffset));

        m_window.clear();
        m_window.draw(m_rtPostProcess.getTexture(), {.position = finalPosition}, {.shader = &m_shaderPostProcess});
        m_imGuiContext.render(m_window);
        m_window.display();
    }


public:
    ////////////////////////////////////////////////////////////
    Game()
    {
        m_rtGame.setSmooth(false);
        m_rtPostProcess.setSmooth(true);
        m_font.setSmooth(false);
        m_fontMago2.setSmooth(false);

        float scale = 4.f;

        m_window.setSize((resolution * scale).toVec2u());
        m_window.setPosition(
            (sf::VideoModeUtils::getDesktopMode().size / 2u - (resolution * (scale * 0.5f)).toVec2u()).toVec2i());

        restartGame();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        while (true)
        {
            if (eventStep() == ControlFlow::Break)
                return false;

            const auto deltaTime = m_tickClock.restart();

            m_totalTime += deltaTime.asSeconds();

            if (isInPlayableState())
                m_timeAccumulator += deltaTime.asSeconds();

            updateStep(deltaTime, ticksPerSecond);
            imguiStep(deltaTime);
            drawStep();
        }

        return true;
    }
};

} // namespace tsurv


////////////////////////////////////////////////////////////
int main()
{
    auto audioContext    = sf::AudioContext::create().value();
    auto graphicsContext = sf::GraphicsContext::create().value();

    auto game = sf::base::makeUnique<tsurv::Game>();

    if (!game->run())
        return 1;

    return 0;
}


// TODO:
// - combo system for line clears
// - random powerups
// - bombs?
// - every X damage deal, remove line from bottom
// - rerolls
// - perk to choose one more perk
// - lightning = random block
// - fire = random row
// - water = random column
// - earth = janitor?
// - make everything more chance based? roll func with luck parameter?
// - every time drill, lightning% chance
// - max perks
// - remove CC
// - damaging a block damages a random adjacent block
// - diagonal left/right lasers + bounce perk
// - separate weapon perks from utility perks
// - change of drill block, change of lightning block, chance of laser block
// - count combo clears and related perks
// - bag manipulation perks (e.g. never get same piece twice in a row)
// - multi hold?
// - dynamic perk system?
//      - triggers:
//            - every N ...
//                - any/specific tetromino place
//                - 1/2/3/4 lines clear
//                - drill hit
//                - laser hit
//                - block destroy
//                - block damage
//                - combo
//                - lightning
//                - power up collected
//                - gap filled
//      - effects
//           - X% chance of...
//               - lightning bolt
//               - fill a gap
//               - gain xp
//               - remove a block from last line
//               - remove last line
//               - turn random block into power up
//               - shift line l/r
//               - compact random column
// - concrete filler weapon instead of drill?
// - separate weapons and items, items must be bought with gold and are RNG based
// - finalize main mechanics, then work on items/perks/synergies
// - timed block +1hp
// - timed powerups
// - perks that affect the timers
// - tutorial modals
// - active perks/items with cooldowns or restrictions (e.g. one-time undo)
