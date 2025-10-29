#include "../bubble_idle/Countdown.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"    // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"  // TODO P1: avoid the relative path...?
#include "../bubble_idle/MathUtils.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp"   // TODO P1: avoid the relative path...?
#include "../bubble_idle/Timer.hpp"     // TODO P1: avoid the relative path...?


#define SFEX_PROFILER_ENABLED
#include "Profiler.hpp"

//
#include "AnimationCommands.hpp"
#include "AnimationTimeline.hpp"
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
#include "Tetramino.hpp"
#include "TetraminoShapes.hpp"
#include "World.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
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
#include "SFML/System/Rect.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/Algorithm/Unique.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Math/Fmod.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Remainder.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsConst.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <string>


namespace tsurv
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{320.f, 240.f};


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] static constexpr sf::Vec2f floorVec2(const sf::Vec2<T> vec) noexcept
{
    return sf::Vec2f{sf::base::floor(vec.x), sf::base::floor(vec.y)};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] static constexpr float bounce(const float value) noexcept
{
    return 1.f - sf::base::fabs(value - 0.5f) * 2.f;
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
    sf::Vec2f    startPosition;
    sf::Vec2f    targetPosition;
    sf::base::U8 paletteIdx;

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
};


////////////////////////////////////////////////////////////
// Delayed actions
struct [[nodiscard]] DelayedAction
{
    Countdown                            delayCountdown;
    sf::base::FixedFunction<void(), 128> action;
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
         .contextSettings = {.antiAliasingLevel = 0}});

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
    sf::Font m_font      = sf::Font::openFromFile("resources/monogram.ttf").value();
    sf::Font m_fontMago2 = sf::Font::openFromFile("resources/petty5.bdf").value();

    ////////////////////////////////////////////////////////////
    sf::PlaybackDevice m_playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    ////////////////////////////////////////////////////////////
    sf::SoundBuffer m_sbLanded   = sf::SoundBuffer::loadFromFile("resources/Landed.wav").value();
    sf::SoundBuffer m_sbNewLevel = sf::SoundBuffer::loadFromFile("resources/NewLevel.wav").value();
    sf::SoundBuffer m_sbRotate   = sf::SoundBuffer::loadFromFile("resources/Rotate.wav").value();
    sf::SoundBuffer m_sbSingle   = sf::SoundBuffer::loadFromFile("resources/Single.wav").value();
    sf::SoundBuffer m_sbExp      = sf::SoundBuffer::loadFromFile("resources/Exp.wav").value();
    sf::SoundBuffer m_sbPlace    = sf::SoundBuffer::loadFromFile("resources/Place.wav").value();
    sf::SoundBuffer m_sbHold     = sf::SoundBuffer::loadFromFile("resources/Hold.wav").value();
    sf::SoundBuffer m_sbHit      = sf::SoundBuffer::loadFromFile("resources/Hit.wav").value();
    sf::SoundBuffer m_sbBonus    = sf::SoundBuffer::loadFromFile("resources/Bonus.wav").value();
    sf::SoundBuffer m_sbStrike   = sf::SoundBuffer::loadFromFile("resources/Strike.wav").value();
    sf::SoundBuffer m_sbDrill    = sf::SoundBuffer::loadFromFile("resources/Drill.wav").value();
    sf::SoundBuffer m_sbError    = sf::SoundBuffer::loadFromFile("resources/Error.wav").value();
    sf::SoundBuffer m_sbLaser    = sf::SoundBuffer::loadFromFile("resources/Laser.wav").value();

    ////////////////////////////////////////////////////////////
    sf::base::Array<sf::base::Optional<sf::Sound>, 8> m_soundPool;

    ////////////////////////////////////////////////////////////
    void playSound(const sf::SoundBuffer& soundBuffer, const float volumeMult = 1.f)
    {
        for (auto& soundOpt : m_soundPool)
            if (!soundOpt.hasValue() || !soundOpt->isPlaying())
            {
                auto& s = soundOpt.emplace(m_playbackDevice, soundBuffer);

                s.setVolume(0.5f * volumeMult);
                s.play();

                return;
            }
    }

    ////////////////////////////////////////////////////////////
    sf::ImGuiContext m_imGuiContext;
    ImFont* const    m_imguiFont{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/monogram.ttf", 16.f)};
    ImFont* const    m_imguiFontBig{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/BoldPixels.ttf", 16.f)};

    ////////////////////////////////////////////////////////////
    sf::Clock m_tickClock;
    float     m_timeAccumulator = 0.f;

    ////////////////////////////////////////////////////////////
    World m_world;

    ////////////////////////////////////////////////////////////
    sf::Vec2f               m_currentTetraminoVisualCenter;
    AnimationTimeline       m_animationTimeline;
    sf::base::Vector<float> m_rowYOffsets;

    ////////////////////////////////////////////////////////////
    bool m_inLevelUpScreen = false;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<sf::base::UniquePtr<const Perk>> m_perks;
    sf::base::Vector<sf::base::SizeT>                 m_perkIndicesSelectedThisLevel;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<LightningBolt> m_lightningBolts;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<DelayedAction> m_delayedActions;

    ////////////////////////////////////////////////////////////
    QuakeSinEffect m_quakeSinEffectLineClear;
    QuakeSinEffect m_quakeSinEffectHardDrop;

    ////////////////////////////////////////////////////////////
    sf::base::Optional<LaserBeam> m_testBeam;

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

    ankerl::unordered_dense::map<sf::base::U32, BlockEffect> m_blockEffects;

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
    const sf::FloatRect m_txrWhiteDotTrue = m_textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value();
    const sf::FloatRect m_txrWhiteDot = {{0.f, 0.f}, {1.f, 1.f}};
    const sf::FloatRect m_txrBlock0   = addImgResourceToAtlas("block0.png");
    const sf::FloatRect m_txrBlock1   = addImgResourceToAtlas("block1.png");
    const sf::FloatRect m_txrDivider  = addImgResourceToAtlas("divider.png");
    const sf::FloatRect m_txrDrill    = addImgResourceToAtlas("drill.png");
    const sf::FloatRect m_txrRedDot   = addImgResourceToAtlas("reddot.png");
    const sf::FloatRect m_txrEmitter  = addImgResourceToAtlas("emitter.png");

    //////////////////////////////////////////////////////////////
    sf::RenderTexture m_rtGame{
        sf::RenderTexture::create(resolution.toVec2u(), {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};


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
    [[nodiscard]] sf::FloatRect addImgResourceToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile("resources" / path).value(), /* padding */ {2u, 2u}).value();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool tetraminosIntersect(const BlockMatrix& shape1,
                                                  const sf::Vec2i&   pos1,
                                                  const BlockMatrix& shape2,
                                                  const sf::Vec2i&   pos2)
    {
        for (sf::base::SizeT y1 = 0; y1 < shapeDimension; ++y1)
            for (sf::base::SizeT x1 = 0; x1 < shapeDimension; ++x1)
            {
                if (!shape1[y1 * shapeDimension + x1].hasValue())
                    continue;

                const sf::Vec2i worldPos1 = pos1 + sf::Vec2uz{x1, y1}.toVec2i();

                for (sf::base::SizeT y2 = 0; y2 < shapeDimension; ++y2)
                    for (sf::base::SizeT x2 = 0; x2 < shapeDimension; ++x2)
                    {
                        if (!shape2[y2 * shapeDimension + x2].hasValue())
                            continue;

                        const sf::Vec2i worldPos2 = pos2 + sf::Vec2uz{x2, y2}.toVec2i();

                        if (worldPos1 == worldPos2)
                            return true; // Found an overlap!
                    }
            }

        return false; // No overlap found
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] int calculateGhostY(const Tetramino& tetramino) const
    {
        sf::base::Optional<Tetramino> finalHardDropState;

        if (const auto* hardDrop = m_animationTimeline.getIfPlaying<AnimHardDrop>())
        {
            finalHardDropState.emplace(hardDrop->tetramino);
            finalHardDropState->position.y = hardDrop->endY;
        }
        else if (const auto* squish = m_animationTimeline.getIfPlaying<AnimSquish>())
            finalHardDropState.emplace(squish->tetramino);

        int ghostY = tetramino.position.y;

        while (true)
        {
            const sf::Vec2i nextGhostPos = {tetramino.position.x, ghostY + 1};

            if (!m_world.blockGrid.isValidMove(tetramino.shape, nextGhostPos))
                break;

            if (finalHardDropState.hasValue() &&
                tetraminosIntersect(tetramino.shape, nextGhostPos, finalHardDropState->shape, finalHardDropState->position))
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
    [[nodiscard]] sf::Vec2f toDrawCoordinates(const sf::Vec2<T> position) const noexcept
    {
        return floorVec2(drawOffset + position.toVec2f().componentWiseMul(drawBlockSize));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2i toGridCoordinates(const sf::Vec2f drawPosition) const noexcept
    {
        return (drawPosition - drawOffset).componentWiseDiv(drawBlockSize).toVec2i();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Color hueColorFromPaletteIdx(const sf::base::SizeT paletteIdx, const sf::base::U8 alpha)
    {
        const auto hue = sf::base::positiveRemainder(blockPalette[paletteIdx].toHSL().hue, 360.f);
        return hueColor(hue, 255u).withAlpha(alpha);
    }


    ////////////////////////////////////////////////////////////
    void drawBlock(const Block& block, const sf::Vec2f& position, const DrawBlockOptions& options = {})
    {
        float yOffset = 0.f;

        if (options.applyYOffset)
        {
            if (!m_rowYOffsets.empty())
            {
                float progress = 0.f;

                if (m_animationTimeline.isPlaying<AnimCollapseGrid>())
                    progress = m_animationTimeline.getProgress();

                const auto gridPos = toGridCoordinates(position);

                if (gridPos.y >= 0 && static_cast<sf::base::SizeT>(gridPos.y) < m_rowYOffsets.size())
                {
                    const float initialOffset = m_rowYOffsets[static_cast<sf::base::SizeT>(gridPos.y)];
                    yOffset += initialOffset * (1.f - easeInBack(progress));
                }
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

        m_rtGame.draw(
            sf::Sprite{
                .position    = floorVec2(position.addY(yOffset)).addX(1.f).addY(1.f),
                .scale       = sf::Vec2f{finalSquishMult, finalSquishMult} * options.scale,
                .origin      = floorVec2(drawBlockSize / 2.f),
                .rotation    = sf::degrees(options.rotation),
                .textureRect = m_txrBlock1,
                .color       = hueColorFromPaletteIdx(block.paletteIdx, alpha),
            },
            {
                .texture = &m_textureAtlas.getTexture(),
                .shader  = &m_shader,
            });

        if (block.powerup != BlockPowerup::None && options.drawText)
        {
            std::string txt;

            if (block.powerup == BlockPowerup::XPBonus)
                txt = "XP";
            else if (block.powerup == BlockPowerup::ColumnDrill)
                txt = "CC";
            else if (block.powerup == BlockPowerup::ThreeRowDrill)
                txt = "TR";

            sf::Text text{m_fontMago2,
                          {
                              .scale         = sf::Vec2f{finalSquishMult, finalSquishMult} * options.scale,
                              .origin        = drawBlockSize / 2.f,
                              .string        = txt,
                              .characterSize = 5u,
                              .fillColor     = sf::Color::blackMask(alpha),
                          }};

            text.setCenter(position.addY(yOffset));
            m_rtGame.draw(text);
        }

        if (block.health > 1u && options.drawText)
        {
            sf::Text text{m_fontMago2,
                          {
                              .origin        = floorVec2(drawBlockSize / 2.f),
                              .string        = std::to_string(static_cast<unsigned int>(block.health - 1u)),
                              .characterSize = 5u,
                              .fillColor     = sf::Color::whiteMask(alpha),
                          }};

            sf::Text text2{m_fontMago2,
                           {
                               .origin        = floorVec2(drawBlockSize / 2.f),
                               .string        = std::to_string(static_cast<unsigned int>(block.health - 1u)),
                               .characterSize = 5u,
                               .fillColor     = sf::Color::blackMask(alpha),
                           }};

            text2.setCenter(floorVec2(position.addY(yOffset) + sf::Vec2f{2.f, 3.f}));
            m_rtGame.draw(text2);

            text.setCenter(floorVec2(position.addY(yOffset)) + sf::Vec2f{2.f, 2.f});
            m_rtGame.draw(text);
        }
    }


    ////////////////////////////////////////////////////////////
    void drawTetramino(const BlockMatrix& shape, const sf::Vec2f& centerPosition, const DrawBlockOptions& options = {})
    {
        const float     scale    = options.scale;
        constexpr float rotation = 0.f;

        // 1. Define the pivot point in the tetramino's local, unscaled coordinate space.
        // This is the center of the 4x4 grid.
        const sf::Vec2f localPivot = floorVec2((drawBlockSize * static_cast<float>(shapeDimension)) / 2.f);

        for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
            for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
            {
                const sf::base::Optional<Block>& optBlock = shape[y * shapeDimension + x];

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
                drawBlock(*optBlock,
                          finalDrawPosition,
                          {
                              .opacity      = options.opacity,
                              .squishMult   = options.squishMult,
                              .rotation     = rotation, // Pass rotation for the block's own orientation
                              .scale        = scale,
                              .drawText     = options.drawText,
                              .applyYOffset = false, // Usually false for UI elements
                          });
            }
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
        const auto nextRotationState = static_cast<sf::base::U8>((tetramino.rotationState + (clockwise ? 1 : 3)) % 4u);

        const auto& targetShapeTemplate = srsTetraminoShapes[static_cast<sf::base::SizeT>(tetramino.tetraminoType)][nextRotationState];

        // Create the new stateful shape by re-mapping the blocks
        const BlockMatrix rotatedShape = mapBlocksToNewShape(tetramino, targetShapeTemplate);

        const auto& kickTable = (tetramino.tetraminoType == TetraminoType::I) ? kickDataI : kickDataJLSTZ;

        int kickTableIndex = clockwise ? tetramino.rotationState : nextRotationState;
        kickTableIndex     = kickTableIndex * 2 + (clockwise ? 0 : 1);

        for (const sf::Vec2i& offset : kickTable[static_cast<sf::base::SizeT>(kickTableIndex)])
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
            playSound(m_sbRotate, 0.75f);
        }
    }


    ////////////////////////////////////////////////////////////
    void skipCurrentTetramino()
    {
        if (m_world.holdUsedThisTurn)
            return;

        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        playSound(m_sbHold, 0.75f);

        m_world.currentTetramino.reset();
        m_world.holdUsedThisTurn = true;

        initializeCurrentTetraminoFromBag();
    }


    ////////////////////////////////////////////////////////////
    void holdCurrentTetramino()
    {
        if (m_world.holdUsedThisTurn)
            return;

        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        playSound(m_sbHold, 0.75f);

        const auto temp       = m_world.heldTetramino;
        m_world.heldTetramino = m_world.currentTetramino;

        if (temp.hasValue())
        {
            m_world.currentTetramino = temp;
            m_world.currentTetramino->position = sf::Vec2uz{(m_world.blockGrid.getWidth() - shapeDimension) / 2u, 0u}.toVec2i();
        }
        else
        {
            m_world.currentTetramino.reset();
            initializeCurrentTetraminoFromBag();
        }

        m_world.holdUsedThisTurn = true;
    }


    ////////////////////////////////////////////////////////////
    void findAndClearLines()
    {
        sf::base::Vector<sf::base::SizeT> fullRows;

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

        if (!fullRows.empty())
            m_animationTimeline.add(AnimClearLines{
                .rows       = fullRows,
                .duration   = 0.01f,
                .awardXP    = true,
                .forceClear = false,
            });
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findDrillBlocks(const Tetramino& tetramino,
                                                                                      const DrillDirection::Enum direction) const
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
    [[nodiscard]] sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findLaserBlocks(const Tetramino& tetramino,
                                                                                      const LaserDirection::Enum direction) const
    {
        const auto inBounds = [](const sf::base::SizeT idx) { return idx < shapeDimension * shapeDimension; };

        sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> result;

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
                    const auto bottomLeftIdx   = getIndex2Dto1D({bPos.x - 1u, bPos.y + 1u}, shapeDimension);
                    const auto leftIdx         = getIndex2Dto1D({bPos.x - 1u, bPos.y}, shapeDimension);
                    const bool leftValid       = !inBounds(leftIdx) || !tetramino.shape[leftIdx].hasValue();
                    const bool bottomLeftValid = !inBounds(bottomLeftIdx) || !tetramino.shape[bottomLeftIdx].hasValue();

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
                    const auto rightIdx       = getIndex2Dto1D({bPos.x + 1u, bPos.y}, shapeDimension);
                    const auto bottomRightIdx = getIndex2Dto1D({bPos.x + 1u, bPos.y + 1u}, shapeDimension);
                    const bool rightValid     = !inBounds(rightIdx) || !tetramino.shape[rightIdx].hasValue();
                    const bool bottomRightValid = !inBounds(bottomRightIdx) || !tetramino.shape[bottomRightIdx].hasValue();

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
                if (optBlock->health > 1u)
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
    void embedTetraminoAndClearLines(const Tetramino& tetramino)
    {
        if (m_world.perkXPPerTetraminoPlaced > 0)
        {
            addXP(static_cast<sf::base::U64>(m_world.perkXPPerTetraminoPlaced));

            const auto downmostBlocksXY = findDownmostBlocks(tetramino.shape);

            for (const auto& bPos : downmostBlocksXY)
            {
                const sf::base::Optional<Block>& optBlock = tetramino.shape[bPos.y * shapeDimension + bPos.x];
                SFML_BASE_ASSERT(optBlock.hasValue());

                spawnXPEarnedParticle(toDrawCoordinates(tetramino.position + bPos.toVec2i()), optBlock->paletteIdx);
            }
        }

        m_world.blockGrid.embedTetramino(tetramino);
        m_world.graceDropMoves = 0u;

        ++m_world.tetaminosPlaced;

        if (auto* rndHitPerNTetraminos = m_world.perkRndHitPerNTetraminos.asPtr())
        {
            ++(rndHitPerNTetraminos->tetraminosPlacedCount);

            if (rndHitPerNTetraminos->tetraminosPlacedCount >= rndHitPerNTetraminos->nTetraminos)
            {
                strikeNRandomBlocks(1u);
                rndHitPerNTetraminos->tetraminosPlacedCount = 0;
            }
        }

        findAndClearLines();

        if (auto* deleteFloorNTetraminos = m_world.perkDeleteFloorPerNTetraminos.asPtr())
        {
            ++(deleteFloorNTetraminos->tetraminosPlacedCount);

            if (deleteFloorNTetraminos->tetraminosPlacedCount >= deleteFloorNTetraminos->nTetraminos)
            {
                // Delete bottom line
                m_animationTimeline.add(AnimClearLines{
                    .rows       = {m_world.blockGrid.getHeight() - 1u},
                    .duration   = 0.01f,
                    .awardXP    = false,
                    .forceClear = true,
                });

                deleteFloorNTetraminos->tetraminosPlacedCount = 0;
            }
        }
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isInPlayableState() const
    {
        return !m_animationTimeline.anyAnimationPlaying() && !m_inLevelUpScreen;
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

            // TODO: Pause game and show perk selection UI.
        }
    }


    /////////////////////////////////////////////////////////////
    void handleKeyPressedEvent(const sf::Event::KeyPressed& eKeyPressed)
    {
        const bool inMenu = m_inLevelUpScreen;

        if (inMenu)
            return;

        const bool inAnimation = (m_animationTimeline.anyAnimationPlaying() &&
                                  !m_animationTimeline.isPlaying<AnimFadeBlocks>() &&
                                  !m_animationTimeline.isPlaying<AnimCollapseGrid>()) ||
                                 m_animationTimeline.isEnqueued<AnimHardDrop>() ||
                                 m_animationTimeline.isEnqueued<AnimSquish>();

        if (eKeyPressed.code == sf::Keyboard::Key::Right)
        {
            moveCurrentTetraminoBy({1, 0});
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Down)
        {
            if (inAnimation)
            {
                playSound(m_sbError, 0.75f);
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
                playSound(m_sbError, 1.f);
                return;
            }

            const int endY = calculateGhostY(*m_world.currentTetramino);

            m_animationTimeline.add(AnimHardDrop{
                .tetramino = *m_world.currentTetramino,
                .duration  = 0.125f,
                .endY      = endY,
            });

            m_world.currentTetramino.reset();
            initializeCurrentTetraminoFromBag();
            m_world.holdUsedThisTurn = false;

            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::LShift)
        {
            if (inAnimation)
            {
                playSound(m_sbError, 0.75f);
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
                playSound(m_sbError, 0.75f);
                return;
            }

            // Restart the game
            m_world = World{};
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Q)
        {
            ++m_world.playerLevel;
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::W)
        {
            enqueueLightningStrikeRandomBlocks(1);
            return;
        }
    }


    /////////////////////////////////////////////////////////////
    void initializeCurrentTetraminoFromBag()
    {
        SFML_BASE_ASSERT(!m_world.currentTetramino.hasValue());

        const TaggedBlockMatrix taggedBlockMatrix = drawFromBag(m_world.blockMatrixBag);

        m_world.currentTetramino.emplace(Tetramino{
            .shape         = taggedBlockMatrix.blockMatrix,
            .position      = sf::Vec2uz{(m_world.blockGrid.getWidth() - shapeDimension) / 2u, 0u}.toVec2i(),
            .tetraminoType = taggedBlockMatrix.tetraminoType,
            .rotationState = 0u,
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

            if (handlePixelPerfectResize(*event, resolution, m_window))
                continue;

            if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                handleKeyPressedEvent(*eKeyPressed);
        }

        return ControlFlow::Continue;
    }


    /////////////////////////////////////////////////////////////
    void damageBlock(const sf::Vec2uz position, Block& block)
    {
        SFML_BASE_ASSERT(block.health > 1u); // TODO: crashed right after janitor for last line

        --block.health;

        playSound(m_sbHit, 0.75f);

        m_blockEffects[block.blockId] = BlockEffect{};

        if (m_world.perkXPPerBlockDamaged > 0)
        {
            addXP(static_cast<sf::base::U64>(m_world.perkXPPerBlockDamaged));
            playSound(m_sbExp, 0.25f);

            spawnXPEarnedParticle(toDrawCoordinates(position) + drawBlockSize / 2.f, block.paletteIdx);
        }
    }


    /////////////////////////////////////////////////////////////
    void rerollPerks()
    {
        m_perkIndicesSelectedThisLevel.clear();

        for (sf::base::SizeT i = 0u; i < m_perks.size(); ++i)
            if (m_perks[i]->meetsPrerequisites(m_world))
                m_perkIndicesSelectedThisLevel.pushBack(i);

        while (m_perkIndicesSelectedThisLevel.size() > 3u)
        {
            const auto removeIdx = m_rngFast.getI<sf::base::SizeT>(0u, m_perkIndicesSelectedThisLevel.size() - 1u);
            m_perkIndicesSelectedThisLevel.erase(m_perkIndicesSelectedThisLevel.begin() + removeIdx);
        }

        shuffleBag(m_perkIndicesSelectedThisLevel, m_rngFast);
    }


    /////////////////////////////////////////////////////////////
    void spawnXPEarnedParticle(const sf::Vec2f startPosition, const sf::base::U8 paletteIdx)
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
    void strikeNRandomBlocks(const sf::base::SizeT n)
    {
        sf::base::Vector<EligibleBlock> eligibleBlocks;

        m_world.blockGrid.forBlocks([&](Block& block, const sf::Vec2uz position)
        {
            if (block.health > 1u)
                eligibleBlocks.pushBack({&block, position});

            return ControlFlow::Continue;
        });

        const auto actualNBlocksToHit = sf::base::min(n, eligibleBlocks.size());

        for (sf::base::SizeT i = 0u; i < actualNBlocksToHit; ++i)
        {
            const auto randomIndex = m_rngFast.getI<sf::base::SizeT>(0u, eligibleBlocks.size() - 1u);
            auto&      blockInfo   = eligibleBlocks[randomIndex];

            if (i > 0u)
                m_animationTimeline.add(AnimWait{0.15f});

            m_animationTimeline.add(AnimAction{
                .action =
                    [this, blockInfo]
            {
                m_lightningBolts
                    .emplaceBack(m_rngFast,
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

                playSound(m_sbStrike, 0.85f);

                m_screenShakeAmount = 2.5f;
                m_screenShakeTimer  = 0.2f;
            },
                .duration = 0.2f,
            });

            m_animationTimeline.add(AnimAction{
                .action   = [this, blockInfo] { damageBlock(blockInfo.position, *blockInfo.block); },
                .duration = 0.1f,
            });

            eligibleBlocks.erase(eligibleBlocks.begin() + randomIndex);
        }

        if (roll100(m_world.perkChainLightning))
        {
            m_animationTimeline.add(AnimAction{
                .action   = [this, actualNBlocksToHit] { strikeNRandomBlocks(actualNBlocksToHit); },
                .duration = 0.01f,
            });
        }
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
        if (m_animationTimeline.anyAnimationPlaying() || !m_earnedXPParticles.empty())
            return;

        if (m_world.committedPlayerLevel < m_world.playerLevel && !m_inLevelUpScreen)
        {
            m_inLevelUpScreen = true;
            rerollPerks();

            playSound(m_sbNewLevel);
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
        if (m_inLevelUpScreen || !m_animationTimeline.anyAnimationPlaying())
            return;

        auto& anim = m_animationTimeline.commands.front();

        if (anim.linearVisit([&]<typename T>(T& innerAnim)
                                 requires(!sf::base::isConst<T>) { return updateAnimation(innerAnim); }))
        {
            m_animationTimeline.popFrontCommand();
            return;
        }

        m_animationTimeline.timeOnCurrentCommand += deltaTime.asSeconds();
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimWait&)
    {
        return m_animationTimeline.getProgress() >= 1.f;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimHardDrop& hardDrop)
    {
        if (m_animationTimeline.getProgress() < 1.f)
            return false;

        hardDrop.tetramino.position.y = hardDrop.endY;

        m_animationTimeline.add(AnimSquish{
            .tetramino = hardDrop.tetramino,
            .duration  = 0.15f,
        });

        m_quakeSinEffectHardDrop.start(4.f, 4.f);

        playSound(m_sbLanded);

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimSquish& squish)
    {
        if (m_animationTimeline.getProgress() < 1.f)
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

            m_animationTimeline.add(AnimDrill{
                .tetramino = squish.tetramino,
                .direction = direction,
                .duration  = 0.3f + (0.1f * static_cast<float>(maxDrilledBlocks)),
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

                if (laserableBlocksInfo.positions.empty())
                    continue;

                if (laserableBlocksInfo.positions.size() > maxPenetration)
                    laserableBlocksInfo.positions.resize(maxPenetration);

                for (const auto targetPos : laserableBlocksInfo.positions)
                    m_animationTimeline.add(AnimLaser{
                        .tetramino     = squish.tetramino,
                        .direction     = direction,
                        .gridStartPos  = startPos,
                        .gridTargetPos = targetPos,
                        .duration      = 0.175f,
                    });
            }
        };

        processDrill(DrillDirection::Down);
        processDrill(DrillDirection::Left);
        processDrill(DrillDirection::Right);

        processLaser(LaserDirection::Left);
        processLaser(LaserDirection::Right);

        embedTetraminoAndClearLines(squish.tetramino);

        return true;
    }


    /////////////////////////////////////////////////////////////
    void enqueueLightningStrikeRandomBlocks(const sf::base::SizeT n)
    {
        m_animationTimeline.add(AnimAction{
            .action   = [this, n] { strikeNRandomBlocks(n); },
            .duration = 0.1f,
        });
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimClearLines& clearLines)
    {
        sf::base::Vector<sf::base::SizeT>             trulyClearedRows;
        sf::base::Vector<AnimFadeBlocks::FadingBlock> fadingBlocks;
        sf::base::Vector<sf::Vec2uz>                  columnClearPositions;

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
                    if (!clearLines.forceClear && optBlock->health > 1u)
                    {
                        damageBlock(sf::Vec2uz{x, y}, *optBlock);
                        rowIsFullyCleared = false;
                    }
                    else
                    {
                        fadingBlocks.pushBack(AnimFadeBlocks::FadingBlock{
                            .block    = *optBlock,
                            .position = sf::Vec2uz{x, y},
                        });

                        if (optBlock->powerup == BlockPowerup::XPBonus)
                        {
                            addXP(5u * m_world.playerLevel);
                            playSound(m_sbBonus, 0.5f);

                            spawnXPEarnedParticle(toDrawCoordinates(sf::Vec2uz{x, y}), optBlock->paletteIdx);
                        }
                        else if (optBlock->powerup == BlockPowerup::ColumnDrill)
                        {
                            columnClearPositions.emplaceBack(x, y);
                            playSound(m_sbBonus, 0.5f);
                        }
                        else if (optBlock->powerup == BlockPowerup::ThreeRowDrill)
                        {
                            addRowIfNotExistent(y);

                            if (y + 1u < m_world.blockGrid.getHeight())
                                addRowIfNotExistent(y + 1);

                            if (y > 0u)
                                addRowIfNotExistent(y - 1);

                            playSound(m_sbBonus, 0.5f);
                        }

                        optBlock.reset();
                    }
                }
            }

            if (rowIsFullyCleared)
                addRowIfNotExistent(y);
        }

        if (!fadingBlocks.empty())
        {
            m_animationTimeline.add(AnimFadeBlocks{
                .fadingBlocks = fadingBlocks,
                .duration     = 0.15f,
            });
        }

        if (!trulyClearedRows.empty())
        {
            playSound(m_sbSingle, 0.85f);

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
                playSound(m_sbExp, 0.5f);

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

            trulyClearedRows.erase(sf::base::unique(trulyClearedRows.begin(), trulyClearedRows.end()),
                                   trulyClearedRows.end());

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

            m_animationTimeline.add(AnimCollapseGrid{
                .clearedRows = trulyClearedRows,
                .duration    = 0.15f,
            });

            // Random block hit perk
            if (clearLines.awardXP && m_world.perkRndHitOnClear > 0)
                enqueueLightningStrikeRandomBlocks(static_cast<sf::base::SizeT>(m_world.perkRndHitOnClear));
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
            playSound(m_sbExp, 0.5f);

            for (sf::base::U64 i = 0u; i < fadingBlocks.size() * 4u; ++i)
            {
                const auto& block = fadingBlocks[i % fadingBlocks.size()];

                const auto startPosition = toDrawCoordinates(block.position) +
                                           m_rngFast.getVec2f(-drawBlockSize, drawBlockSize) / 2.f;

                spawnXPEarnedParticle(startPosition, block.block.paletteIdx);
            }
        }

        for (const auto columnClearPos : columnClearPositions)
        {
            m_animationTimeline.add(AnimColumnClear{
                .position = columnClearPos.addY(1),
                .duration = 0.3f,
            });
        }

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimFadeBlocks&)
    {
        return m_animationTimeline.getProgress() >= 1.f;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimCollapseGrid&)
    {
        if (m_animationTimeline.getProgress() < 1.f)
            return false;

        for (auto& rowOffset : m_rowYOffsets)
            rowOffset = 0.f;

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimDrill& drill)
    {
        if (m_animationTimeline.justStarted())
            playSound(m_sbDrill, 0.75f);

        m_screenShakeAmount = 0.85f;
        m_screenShakeTimer  = 0.05f;

        if (m_animationTimeline.getProgress() < 1.f)
            return false;

        for (auto [blockPtr, position] : findDrillTargetBlocks(drill.tetramino, drill.direction))
            damageBlock(position, *blockPtr);

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimLaser& laser)
    {
        if (m_animationTimeline.justStarted())
        {
            playSound(m_sbLaser, 0.75f);

            const auto startPos = toDrawCoordinates(laser.gridStartPos);

            SFML_BASE_ASSERT(!m_testBeam.hasValue());
            m_testBeam.emplace(startPos, startPos, blockPalette[getTetraminoPaletteIdx(laser.tetramino)]);
        }

        m_screenShakeAmount = 0.65f;
        m_screenShakeTimer  = 0.05f;

        if (m_animationTimeline.getProgress() < 1.f)
            return false;

        auto& optBlock = m_world.blockGrid.at(laser.gridTargetPos);
        SFML_BASE_ASSERT(optBlock.hasValue());

        damageBlock(laser.gridTargetPos.toVec2uz(), *optBlock);

        SFML_BASE_ASSERT(m_testBeam.hasValue());
        m_testBeam.reset();

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimColumnClear& columnClear)
    {
        if (m_animationTimeline.getProgress() < 1.f)
            return false;

        sf::base::Vector<AnimFadeBlocks::FadingBlock> fadingBlocks;

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
            m_animationTimeline.add(AnimFadeBlocks{
                .fadingBlocks = fadingBlocks,
                .duration     = 0.15f,
            });

        return true;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateAnimation(AnimAction& action)
    {
        if (m_animationTimeline.getProgress() < 1.f)
            return false;

        action.action();

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

        constexpr sf::base::SizeT bagMult = 2u;

        const auto addToBag = [&](const TetraminoType j)
        {
            const Block block{
                .tetraminoId        = m_world.nextTetraminoId++,
                .blockId            = 0u, // updated below
                .health             = 1u,
                .paletteIdx         = static_cast<sf::base::U8>(j),
                .shapeBlockSequence = ShapeBlockSequence::_, // set by `shapeMatrixToBlockMatrix`
                .powerup            = BlockPowerup::None,
            };

            auto& [blockMatrix, tetraminoType] = m_world.blockMatrixBag.pushBack({
                .blockMatrix = shapeMatrixToBlockMatrix(srsTetraminoShapes[static_cast<sf::base::SizeT>(j) /* pieceType */][0],
                                                        block),
                .tetraminoType = j,
            });

            const auto healthDist = generateTetraminoHealthDistribution(getDifficultyFactor(m_world.tick), m_rngFast);
            sf::base::SizeT nextHealthDistIdx = 0u;

            for (sf::base::Optional<Block>& b : blockMatrix)
            {
                if (!b.hasValue())
                    continue;

                b->blockId = m_world.nextBlockId++;
                b->health  = static_cast<sf::base::U8>(healthDist[nextHealthDistIdx++]);

                if (m_rngFast.getI(0, 200) > 198)
                {
                    b->health = 1u;

                    if (m_rngFast.getI(0, 100) > 75)
                        b->powerup = BlockPowerup::ThreeRowDrill;
                    else
                        b->powerup = BlockPowerup::XPBonus;
                }
            }
        };

        for (sf::base::SizeT i = 0u; i < bagMult; ++i)
            for (sf::base::U8 j = 0u; j < tetraminoShapeCount; ++j)
                addToBag(static_cast<TetraminoType>(j));

        for (int i = 0; i < m_world.perkExtraLinePiecesInPool; ++i)
            addToBag(TetraminoType::I);

        shuffleBag(m_world.blockMatrixBag, m_rngFast);
    }


    /////////////////////////////////////////////////////////////
    void updateStepProcessSimulation(const float xTicksPerSecond)
    {
        for (unsigned int i = 0; i < static_cast<unsigned int>(m_timeAccumulator * xTicksPerSecond); ++i)
        {
            ++m_world.tick;
            m_timeAccumulator -= 1.f / xTicksPerSecond;

            if (m_world.tick % 60 == 0)
            {
                if (m_world.currentTetramino.hasValue())
                {
                    const auto newPosition = m_world.currentTetramino->position + sf::Vec2i{0, 1};

                    if (m_world.blockGrid.isValidMove(m_world.currentTetramino->shape, newPosition))
                    {
                        m_world.currentTetramino->position = newPosition;
                    }
                    else
                    {
                        if (m_world.graceDropMoves < m_world.maxGraceDropMoves)
                            ++m_world.graceDropMoves;
                        else
                        {
                            embedTetraminoAndClearLines(*m_world.currentTetramino);
                            m_world.currentTetramino.reset();
                            playSound(m_sbPlace, 0.5f);
                        }
                    }
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////
    void updateStepDelayedActions(const sf::Time deltaTime)
    {
        const auto deltaTimeMs = static_cast<float>(deltaTime.asMicroseconds()) / 1000.f;

        for (auto& [delayCountdown, func] : m_delayedActions)
            if (delayCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                func();

        sf::base::vectorEraseIf(m_delayedActions, [](const auto& delayedAction) {
            return delayedAction.delayCountdown.isDone();
        });
    }


    /////////////////////////////////////////////////////////////
    void updateStepLightningBolts(const sf::Time deltaTime)
    {
        for (auto& lb : m_lightningBolts)
            lb.update(deltaTime);

        sf::base::vectorEraseIf(m_lightningBolts, [](const LightningBolt& lb) { return lb.isFinished(); });

        if (m_testBeam.hasValue())
            m_testBeam->update(deltaTime);
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
        {
            updateStepRefillBlockMatrixIfNeeded();

            // Pick next tetramino if there is none
            if (!m_world.currentTetramino.hasValue())
            {
                initializeCurrentTetraminoFromBag();
                m_world.holdUsedThisTurn = false;
            }

            updateStepProcessSimulation(xTicksPerSecond);
        }

        updateStepDelayedActions(deltaTime);
        updateStepLightningBolts(deltaTime);
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string makeTitle(const std::string& prefix, const auto& value, const auto inc)
    {
        return prefix + " (" + std::to_string(value) + " -> " + std::to_string(value + inc) + ")";
    }


    /////////////////////////////////////////////////////////////
    void imguiStep(const sf::Time deltaTime)
    {
        SFEX_PROFILE_SCOPE("imgui");

        m_imGuiContext.update(m_window, deltaTime);

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

                float size  = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
                float avail = ImGui::GetContentRegionAvail().x;

                float off = (avail - size) * alignment;
                if (off > 0.0f)
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
            .fillColor        = sf::Color(30, 30, 30),
            .outlineColor     = sf::Color(35, 35, 35),
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
                                    sf::Vec2f{static_cast<float>(x), static_cast<float>(y)}.componentWiseMul(drawBlockSize),
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
            .outlineColor     = sf::Color(35, 35, 35),
            .outlineThickness = 1.f,
            .size             = gridSize.toVec2f().componentWiseMul(drawBlockSize).addX(4.f).addY(4.f),
        });
    }


    /////////////////////////////////////////////////////////////
    void drawStepEmbeddedBlocks()
    {
        for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
            for (sf::base::SizeT y = gridGraceY; y < m_world.blockGrid.getHeight(); ++y)
            {
                const sf::Vec2uz                 gridPosition{x, y};
                const sf::base::Optional<Block>& optBlock = m_world.blockGrid.at(gridPosition);

                if (!optBlock.hasValue())
                    continue;

                drawBlock(*optBlock,
                          toDrawCoordinates(gridPosition),
                          {
                              .opacity          = 1.f,
                              .applyQuakeOffset = false,
                          });
            }
    }


    /////////////////////////////////////////////////////////////
    void drawStepFadingBlocks()
    {
        auto* fadeBlocks = m_animationTimeline.getIfPlaying<AnimFadeBlocks>();
        if (fadeBlocks == nullptr)
            return;

        const float progress = m_animationTimeline.getProgress();

        for (const auto& fadingBlock : fadeBlocks->fadingBlocks)
        {
            drawBlock(fadingBlock.block,
                      toDrawCoordinates(fadingBlock.position),
                      {
                          .opacity          = 1.f - easeInOutSine(progress),
                          .squishMult       = 0.f,
                          .applyYOffset     = false,
                          .applyQuakeOffset = false,
                      });
        }
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::U8 getTetraminoPaletteIdx(const Tetramino& tetramino) const
    {
        for (const auto& b : tetramino.shape)
            if (b.hasValue())
                return b->paletteIdx;

        SFML_BASE_UNREACHABLE();
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2i drillDirectionToVec2i(const DrillDirection::Enum direction)
    {
        switch (direction)
        {
            case DrillDirection::Left:
                return sf::Vec2i{-1, 0};
            case DrillDirection::Right:
                return sf::Vec2i{1, 0};
            case DrillDirection::Down:
                return sf::Vec2i{0, 1};
            default:
                SFML_BASE_UNREACHABLE();
        }
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

            if (optBlock->health == 1u)
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

                if (optBlock->health == 1u)
                    continue;

                info.endPos.y = iY;

                if (++info.count >= m_world.perkDrill[DrillDirection::Down]->maxPenetration)
                    break;
            }
        }

        return info;
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2i laserDirectionToVec2i(const LaserDirection::Enum direction) const
    {
        switch (direction)
        {
            case LaserDirection::Left:
                return sf::Vec2i{-1, 1};
            case LaserDirection::Right:
                return sf::Vec2i{1, 1};
            default:
                SFML_BASE_UNREACHABLE();
        }
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

            if (optBlock.hasValue() && optBlock->health > 1u)
                result.positions.emplaceBack(pos);

            pos += dir;
        }

        return result;
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimDrill()
    {
        auto* drillAnim = m_animationTimeline.getIfPlaying<AnimDrill>();

        if (drillAnim == nullptr)
            return;

        const sf::Vec2f offsetByDirection[] = {
            {-drawBlockSize.x / 2.f - 3.f, 1.f}, // Left
            {drawBlockSize.x, 2.f},              // Right
            {1.f, drawBlockSize.y},              // Down
        };

        const sf::Angle arrayByDirection[] = {
            sf::degrees(90.f),  // Left
            sf::degrees(270.f), // Right
            sf::degrees(0.f),   // Down
        };

        const auto drillDrawOffset = offsetByDirection[static_cast<sf::base::SizeT>(drillAnim->direction)];
        const auto rotation        = arrayByDirection[static_cast<sf::base::SizeT>(drillAnim->direction)];

        const auto& tetramino = drillAnim->tetramino;

        const float progress   = m_animationTimeline.getProgress();
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
                sf::Sprite spike{
                    .position    = floorVec2(getDrawPosition(i)),
                    .origin      = floorVec2(sf::Vec2f{radius / 2.f, radius / 2.f}),
                    .rotation    = rotation,
                    .textureRect = m_txrDrill,
                    .color       = hueColorFromPaletteIdx(paletteIdx, 255u),
                };

                m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});
            }
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimLaser()
    {
        auto* laserAnim = m_animationTimeline.getIfPlaying<AnimLaser>();

        if (laserAnim == nullptr)
            return;

        if (!m_testBeam.hasValue())
            return;

        const auto targetPos = toDrawCoordinates(laserAnim->gridTargetPos) +
                               m_rngFast.getVec2f(-drawBlockSize, drawBlockSize) / 4.f;

        const auto progress = m_animationTimeline.getProgress();
        m_testBeam->end     = m_testBeam->start + (targetPos - m_testBeam->start) * easeInOutBack(progress);

        for (int i = 0; i < 3; ++i)
            m_fixedColorCircleShapeParticles.emplaceBack(ParticleData{
                .position      = m_testBeam->end,
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
                                                        const sf::Vec2f&   tetraminoCenter) const
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
    void drawDrillSpikesForPerk(const sf::base::InPlaceVector<sf::Vec2uz, shapeDimension>& localBlockPositions,
                                const sf::Vec2f                                            offset,
                                const sf::Color                                            color,
                                const sf::Angle                                            rotation,
                                const sf::Vec2f                                            mainTetraminoCenter,
                                const sf::Vec2f                                            ghostTetraminoCenter)
    {
        const auto mainColor  = color;
        const auto ghostColor = mainColor.withAlpha(64);

        sf::Sprite spike{
            .origin      = floorVec2(sf::Vec2f{drawBlockSize.x / 2.f, drawBlockSize.y / 2.f} - sf::Vec2f{2.f, 2.f}),
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
            spike.position = floorVec2(offset + ghostBlockDrawPos.addY(sf::base::floor(drawBlockSize.y / 2.f))) -
                             sf::Vec2f{1.f, 1.f};
            spike.color = ghostColor;
            m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});
        }
    }


    /////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Vec2f calculateLaserGridIntersection(const sf::Vec2f& startPos, const LaserDirection::Enum direction) const
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
    void drawLaserEmittersForPerk(const sf::base::InPlaceVector<sf::Vec2uz, shapeDimension>& localBlockPositions,
                                  const sf::Vec2f                                            offset,
                                  const sf::Color                                            color,
                                  const sf::Angle                                            rotation,
                                  const sf::Vec2f                                            mainTetraminoCenter,
                                  const sf::Vec2f                                            ghostTetraminoCenter)
    {
        const auto mainColor  = color;
        const auto ghostColor = mainColor.withAlpha(64);

        const auto laserDirection = rotation == sf::degrees(45.f) ? LaserDirection::Left : LaserDirection::Right;
        const auto laserDir       = laserDirectionToVec2i(laserDirection);

        sf::Sprite spike{
            .origin      = floorVec2(sf::Vec2f{drawBlockSize.x / 2.f, drawBlockSize.y / 2.f} - sf::Vec2f{2.f, 2.f}),
            .rotation    = rotation + sf::degrees(180.f + 45.f),
            .textureRect = m_txrEmitter,
        };

        const auto drawGuide = [&](const sf::Vec2f anchor)
        {
            const auto guideOffset = laserDir.toVec2f() * 2.f;

            auto endPos = calculateLaserGridIntersection(anchor - laserDir.toVec2f() * 6.f, laserDirection) +
                          laserDir.toVec2f() * 5.f - guideOffset;

            {
                sf::RectangleShape guide{{
                    .rotation    = (rotation).wrapUnsigned(),
                    .textureRect = m_txrRedDot,
                    .fillColor   = mainColor.withAlpha(32),
                    .size        = sf::Vec2f{1.f, (endPos - anchor).length()},
                }};

                if (laserDirection == LaserDirection::Left)
                    guide.setTopRight(anchor + guideOffset);
                else
                    guide.setTopLeft(anchor + guideOffset);

                m_rtGame.draw(guide, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});
            }

            {
                sf::RectangleShape guide{{
                    .rotation    = (rotation).wrapUnsigned(),
                    .textureRect = m_txrRedDot,
                    .fillColor   = mainColor.withAlpha(16),
                    .size        = sf::Vec2f{2.f, (endPos - anchor).length()},
                }};

                if (laserDirection == LaserDirection::Left)
                    guide.setTopRight(anchor + guideOffset);
                else
                    guide.setTopLeft(anchor + guideOffset);

                m_rtGame.draw(guide, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});
            }
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

            drawGuide(mainSpikePos);

            const auto ghostSpikePos = floorVec2(offset + ghostBlockDrawPos.addY(sf::base::floor(drawBlockSize.y / 2.f))) -
                                       sf::Vec2f{1.f, 1.f};

            // Draw ghost spike
            spike.position = ghostSpikePos + (laserDir * 4).toVec2f();
            spike.color    = ghostColor;
            m_rtGame.draw(spike, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shader});

            drawGuide(ghostSpikePos);
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimHardDrop()
    {
        auto* hardDrop = m_animationTimeline.getIfPlaying<AnimHardDrop>();

        if (hardDrop == nullptr)
            return;

        const float progress = m_animationTimeline.getProgress();

        const float targetVisualY = toDrawCoordinates(hardDrop->tetramino.position.withY(hardDrop->endY)).y +
                                    drawBlockSize.y / 2.f;

        const float startVisualY = toDrawCoordinates(hardDrop->tetramino.position).y + drawBlockSize.y * 2.f;

        const auto visualCenterY = startVisualY + (targetVisualY - startVisualY) * easeInBack(progress);
        const auto visualCenterX = toDrawCoordinates(hardDrop->tetramino.position).x + drawBlockSize.x * 2.f;

        drawStepActiveTetramino(hardDrop->tetramino,
                                sf::Vec2f{visualCenterX, visualCenterY},
                                /* squishMult */ 0.f,
                                /* drawGhost */ false);
    }


    /////////////////////////////////////////////////////////////
    void drawStepAnimSquish()
    {
        auto* squish = m_animationTimeline.getIfPlaying<AnimSquish>();

        if (squish == nullptr)
            return;

        const float progress = m_animationTimeline.getProgress();

        const float squishMult = easeInOutSine(bounce(progress)) * 0.5f;

        drawStepActiveTetramino(squish->tetramino,
                                toDrawCoordinates(squish->tetramino.position) +
                                    sf::Vec2f{drawBlockSize.x * 2.f, drawBlockSize.y * 2.f},
                                /* squishMult */ squishMult,
                                /* drawGhost */ false);
    }


    /////////////////////////////////////////////////////////////
    void drawStepActiveTetramino(const Tetramino& tetramino, const sf::Vec2f visualCenter, const float squishMult, const bool drawGhost)
    {
        const auto color = hueColorFromPaletteIdx(getTetraminoPaletteIdx(tetramino), 255u);

        auto tetraminoDrawPosition = floorVec2(visualCenter - drawBlockSize / 2.f) + sf::Vec2f{1, 1};
        tetraminoDrawPosition.y += m_quakeSinEffectHardDrop.getValue();

        const sf::Vec2f ghostGridPosition = tetramino.position.toVec2f().withY(
            static_cast<float>(calculateGhostY(tetramino)));

        sf::Vec2f ghostCenterDrawPosition = getTetraminoCenterDrawPosition(ghostGridPosition)
                                                .withX(tetraminoDrawPosition.x)
                                                .addY(-drawBlockSize.y / 2.f)
                                                .addY(1.f);

        ghostCenterDrawPosition.y += m_quakeSinEffectHardDrop.getValue();

        if (m_world.perkDrill[DrillDirection::Down].hasValue())
            drawDrillSpikesForPerk(findDrillBlocks(tetramino, DrillDirection::Down),
                                   floorVec2(sf::Vec2f{0.f, drawBlockSize.y / 2.f}),
                                   color,
                                   sf::degrees(0.f),
                                   visualCenter,
                                   ghostCenterDrawPosition);

        if (m_world.perkDrill[DrillDirection::Left].hasValue())
            drawDrillSpikesForPerk(findDrillBlocks(tetramino, DrillDirection::Left),
                                   {},
                                   color,
                                   sf::degrees(90.f),
                                   visualCenter,
                                   ghostCenterDrawPosition);

        if (m_world.perkDrill[DrillDirection::Right].hasValue())
            drawDrillSpikesForPerk(findDrillBlocks(tetramino, DrillDirection::Right),
                                   floorVec2(sf::Vec2f{drawBlockSize.x, -1.f}),
                                   color,
                                   sf::degrees(270.f),
                                   visualCenter,
                                   ghostCenterDrawPosition);

        drawTetramino(tetramino.shape,
                      tetraminoDrawPosition,
                      {
                          .squishMult = squishMult,
                      });

        if (drawGhost)
            drawTetramino(tetramino.shape,
                          ghostCenterDrawPosition,
                          {
                              .opacity    = 0.25f,
                              .squishMult = squishMult,
                          });


        if (m_world.perkLaser[LaserDirection::Left].hasValue())
            drawLaserEmittersForPerk(findLaserBlocks(tetramino, LaserDirection::Left),
                                     {0.f, -1.f},
                                     color,
                                     sf::degrees(45.f),
                                     visualCenter,
                                     ghostCenterDrawPosition);

        if (m_world.perkLaser[LaserDirection::Right].hasValue())
            drawLaserEmittersForPerk(findLaserBlocks(tetramino, LaserDirection::Right),
                                     floorVec2(sf::Vec2f{drawBlockSize.x / 2.f, -1.f}),
                                     color,
                                     sf::degrees(315.f),
                                     visualCenter,
                                     ghostCenterDrawPosition);
    }


    /////////////////////////////////////////////////////////////
    void drawStepUINextTetraminos()
    {
        const sf::base::SizeT nPeek = sf::base::min(static_cast<sf::base::SizeT>(m_world.perkNPeek),
                                                    m_world.blockMatrixBag.size());

        constexpr float uiTetraminoScale = 9.f / drawBlockSize.x;

        for (sf::base::SizeT iPeek = 0u; iPeek < nPeek; ++iPeek)
        {
            const auto&     shape  = m_world.blockMatrixBag[m_world.blockMatrixBag.size() - nPeek + iPeek].blockMatrix;
            const auto      hudPos = getHudPos();
            const sf::Vec2f uiBoxCenter = {hudPos.x, hudPos.y + 100.f + static_cast<float>(nPeek - iPeek) * 24.f};

            drawTetramino(shape,
                          uiBoxCenter + sf::Vec2f{16.f, 16.f},
                          {
                              .scale            = uiTetraminoScale,
                              .drawText         = true,
                              .applyYOffset     = false,
                              .applyQuakeOffset = false,
                          });
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStepUIHeldTetramino()
    {
        if (!m_world.heldTetramino.hasValue())
            return;

        const auto&     shape       = m_world.heldTetramino->shape;
        const auto      hudPos      = getHudPos();
        const sf::Vec2f uiBoxCenter = {hudPos.x, hudPos.y + 100.f};

        drawTetramino(shape, uiBoxCenter + sf::Vec2f{16.f + 64.f, 16.f + 24.f + 2.f});
    }


    /////////////////////////////////////////////////////////////
    void drawStepLightningBolts()
    {
        for (auto& lb : m_lightningBolts)
            lb.draw(m_rtGame, {.blendMode = sf::BlendAdd});

        if (m_testBeam.hasValue())
            m_testBeam->draw(m_rtGame, {.blendMode = sf::BlendAdd});
    }


    /////////////////////////////////////////////////////////////
    void drawStepEarnedXPParticles()
    {
        const auto bezier = [](const sf::Vec2f& start, const sf::Vec2f& end, const float t)
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
        std::string stats;

        stats += "Level ";
        stats += std::to_string(m_world.playerLevel);
        stats += "\nXP: ";
        stats += std::to_string(m_world.currentXP);
        stats += " / ";
        stats += std::to_string(getXPNeededForLevelUp(m_world.playerLevel));
        stats += "\nTime: ";
        stats += std::to_string(getElapsedSeconds(m_world.tick));
        stats += "s\n";
        stats += "Lines Cleared: ";
        stats += std::to_string(m_world.linesCleared);
        stats += "\nTetraminos Placed: ";
        stats += std::to_string(m_world.tetaminosPlaced);
        stats += "\nDifficulty: ";
        stats += std::to_string(getDifficultyFactor(m_world.tick));
        stats += "\n";

        /*

        if (m_world.perkRndHitOnClear > 0)
        {
            stats += "- Random Block Hit (";
            stats += std::to_string(m_world.perkRndHitOnClear);
            stats += "x)\n";
        }

        if (m_world.perkVerticalDrill.hasValue())
        {
            stats += "- Hard Drill (Penetration: ";
            stats += std::to_string(m_world.perkVerticalDrill->maxPenetration);
            stats += "x; MultiHit: ";
            stats += m_world.perkVerticalDrill->multiHit ? "Yes" : "No";
            stats += ")\n";
        }

        if (m_world.perkCanHoldTetramino == 1)
            stats += "- Hold Tetramino\n";
        else if (m_world.perkCanHoldTetramino == 2)
            stats += "- Skip Tetramino\n";

        if (m_world.perkXPPerTetraminoPlaced > 0)
        {
            stats += "- XP per Block Placed (";
            stats += std::to_string(m_world.perkXPPerTetraminoPlaced);
            stats += "x)\n";
        }

        if (m_world.perkXPPerBlockDamaged > 0)
        {
            stats += "- XP per Block Damaged (";
            stats += std::to_string(m_world.perkXPPerBlockDamaged);
            stats += "x)\n";
        }

        if (m_world.perkDeleteFloorPerNTetraminos.hasValue())
        {
            stats += "- Delete Floor per ";
            stats += std::to_string(m_world.perkDeleteFloorPerNTetraminos->nTetraminos);
            stats += " Tetramino Placed (";
            stats += std::to_string(m_world.perkDeleteFloorPerNTetraminos->tetraminosPlacedCount);
            stats += " / ";
            stats += std::to_string(m_world.perkDeleteFloorPerNTetraminos->nTetraminos);
            stats += ")\n";
        }

        if (m_world.perkRndHitPerNTetraminos.hasValue())
        {
            stats += "- Random Hit per ";
            stats += std::to_string(m_world.perkRndHitPerNTetraminos->nTetraminos);
            stats += " Tetramino Placed (";
            stats += std::to_string(m_world.perkRndHitPerNTetraminos->tetraminosPlacedCount);
            stats += " / ";
            stats += std::to_string(m_world.perkRndHitPerNTetraminos->nTetraminos);
            stats += ")\n";
        }

        if (m_world.perkExtraLinePiecesInPool > 0)
        {
            stats += "- Extra Line Pieces in Pool (";
            stats += std::to_string(m_world.perkExtraLinePiecesInPool);
            stats += "x)\n";
        }

        if (m_world.perkHorizontalDrillLeft.hasValue())
        {
            stats += "- Left Horizontal Drill (Penetration: ";
            stats += std::to_string(m_world.perkHorizontalDrillLeft->maxPenetration);
            stats += "x; Length: ";
            stats += std::to_string(m_world.perkHorizontalDrillLeft->maxBlocks);
            stats += ")\n";
        }

        if (m_world.perkHorizontalDrillRight.hasValue())
        {
            stats += "- Right Horizontal Drill (Penetration: ";
            stats += std::to_string(m_world.perkHorizontalDrillRight->maxPenetration);
            stats += "x; Length: ";
            stats += std::to_string(m_world.perkHorizontalDrillRight->maxBlocks);
            stats += ")\n";
        }

        if (m_world.perkChainLightning > 0)
        {
            stats += "- Chain Lightning (";
            stats += std::to_string(m_world.perkChainLightning);
            stats += "% chance)\n";
        }

        */

        m_window.draw(m_font,
                      sf::TextData{
                          .position      = getHudPos(),
                          .string        = stats,
                          .characterSize = 16u,
                          .outlineColor  = sf::Color::White,
                      });
    }


    /////////////////////////////////////////////////////////////
    void drawStep()
    {
        SFEX_PROFILE_SCOPE("draw");

        m_rtGame.clear();

        {
            SFEX_PROFILE_SCOPE("rtGame");

            drawStepBackground();
            drawStepEmbeddedBlocks();
            drawStepFadingBlocks();

            drawStepAnimHardDrop();
            drawStepAnimSquish();

            if (m_world.currentTetramino.hasValue())
                drawStepActiveTetramino(*m_world.currentTetramino,
                                        m_currentTetraminoVisualCenter,
                                        /* squishMult */ 0.f,
                                        /* drawGhost */ true);

            drawStepEarnedXPParticles();
            drawStepCircleDataParticles();

            drawStepAnimDrill();
            drawStepAnimLaser();

            drawStepUINextTetraminos();
            drawStepUIHeldTetramino();
            drawStepLightningBolts();
        }


        m_rtGame.display();

        m_window.clear();

        const auto screenShake = m_rngFast.getVec2f({-m_screenShakeAmount, -m_screenShakeAmount},
                                                    {m_screenShakeAmount, m_screenShakeAmount});

        m_window.draw(m_rtGame.getTexture(),
                      {
                          .position = screenShake.addY(
                              m_quakeSinEffectHardDrop.getValue() + m_quakeSinEffectLineClear.getValue()),

                          .scale = {1.f, 1.f},
                      });

        if (m_world.perkNPeek > 0)
            m_window.draw(m_font,
                          sf::TextData{
                              .position      = getHudPos().addY(100.f),
                              .string        = "Next:",
                              .characterSize = 16u,
                              .outlineColor  = sf::Color::White,
                          });

        if (m_world.perkCanHoldTetramino == 1)
            m_window.draw(m_font,
                          sf::TextData{
                              .position      = getHudPos().addY(100.f).addX(64.f),
                              .string        = "Held:",
                              .characterSize = 16u,
                              .outlineColor  = sf::Color::White,
                          });

        drawStepStatsText();

        m_imGuiContext.render(m_window);
        m_window.display();
    }


public:
    ////////////////////////////////////////////////////////////
    Game()
    {
        m_rtGame.setSmooth(false);
        m_font.setSmooth(false);
        m_fontMago2.setSmooth(false);

        float scale = 4.f;

        m_window.setSize((resolution * scale).toVec2u());
        m_window.setPosition(
            (sf::VideoModeUtils::getDesktopMode().size / 2u - (resolution * (scale * 0.5f)).toVec2u()).toVec2i());

        m_perks.emplaceBack(sf::base::makeUnique<PerkChainLightning>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkPeekNextTetraminos>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkOnClearLightningStrike>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHardDropDrillUnlock>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHardDropDrillPenetration>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHardDropDrillBluntForce>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHoldSkipTetramino>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkXpPerTetraminoPlaced>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkXpPerBlockDamaged>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkDeleteFloorPerNTetraminos>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkRndHitPerNTetraminos>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkExtraLinePieces>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHorizontalDrillLeftUnlock>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHorizontalDrillRightUnlock>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHorizontalDrillLeftPenetration>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHorizontalDrillLeftCoverage>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHorizontalDrillRightPenetration>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkHorizontalDrillRightCoverage>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkDiagonalLaserLeftUnlock>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkDiagonalLaserRightUnlock>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkDiagonalLaserLeftPenetration>());
        m_perks.emplaceBack(sf::base::makeUnique<PerkDiagonalLaserRightPenetration>());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        while (true)
        {
            if (eventStep() == ControlFlow::Break)
                return false;

            const auto deltaTime = m_tickClock.restart();

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
