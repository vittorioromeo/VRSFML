#include "../bubble_idle/ControlFlow.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Countdown.hpp"   // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"    // TODO P1: avoid the relative path...?
#include "../bubble_idle/MathUtils.hpp"   // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp"     // TODO P1: avoid the relative path...?
#include "../bubble_idle/Timer.hpp"       // TODO P1: avoid the relative path...?

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"

#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Math/Atan2.hpp"
#include "SFML/Base/Math/Fmod.hpp"

#include <algorithm>
#include <string>

#define SFEX_PROFILER_ENABLED
#include "Profiler.hpp"
#include "ProfilerImGui.hpp"

//
#include "AnimationCommands.hpp"
#include "AnimationTimeline.hpp"
#include "Block.hpp"
#include "BlockGrid.hpp"
#include "BlockMatrix.hpp"
#include "Constants.hpp"
#include "RandomBag.hpp"
#include "ShapeDimension.hpp"
#include "Tetramino.hpp"
#include "TetraminoShapes.hpp"
#include "World.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
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
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Sort.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>


namespace tsurv
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1024.f, 768.f};


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] static constexpr float bounce(const float value) noexcept
{
    // return 4.f * value * (1.f - value);
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
};


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline sf::Sprite particleToSprite(const ParticleData&  particle,
                                                                     const sf::FloatRect& textureRect,
                                                                     const sf::Color&     color)
{
    const auto opacityAsAlpha = static_cast<sf::base::U8>(particle.opacity * 255.f);

    return {
        .position    = particle.position,
        .scale       = {particle.scale, particle.scale},
        .origin      = textureRect.size / 2.f,
        .rotation    = sf::radians(particle.rotation),
        .textureRect = textureRect,
        .color       = color.withAlpha(opacityAsAlpha),
    };
}


////////////////////////////////////////////////////////////
struct EarnedXPParticle
{
    sf::Vec2f    startPosition;
    sf::Vec2f    targetPosition;
    sf::base::U8 paletteIdx;
    float        delay;
    float        startRotation = 0.f;

    Timer progress{};
};


////////////////////////////////////////////////////////////
struct [[nodiscard]] DrawBlockOptions
{
    float opacity      = 1.f;
    float squishMult   = 0.f;
    float rotation     = 0.f;
    float scale        = 1.f;
    bool  drawText     = true;
    bool  applyYOffset = true;
};


////////////////////////////////////////////////////////////
class LightningBolt
{
public:
    ////////////////////////////////////////////////////////////
    struct Segment
    {
        sf::Vec2f start;
        sf::Vec2f end;
    };

    ////////////////////////////////////////////////////////////
    explicit LightningBolt(auto&&          rng,
                           const sf::Vec2f start,
                           const sf::Vec2f end,
                           const sf::Color color = sf::Color(200, 220, 255)) :
        m_color(color),
        m_duration(sf::seconds(0.27f))
    {
        // --- Tuning Parameters ---
        const float mainBoltJaggedness = 0.2f; // Percentage of segment length
        const float mainBoltThickness  = 4.0f;
        const float branchChance       = 0.3f; // 30% chance to branch
        const float branchLengthMin    = 0.3f; // 30% of original segment
        const float branchLengthMax    = 0.7f; // 70% of original segment

        // Create the main bolt line segments
        sf::base::Vector<Segment> segments;
        segments.pushBack({start, end});

        // Generate the jagged points for the main bolt
        generateSegments(rng, segments, mainBoltJaggedness);

        // From the main bolt segments, generate branches
        for (auto& segment : segments)
        {
            if (rng.getF(0.f, 1.f) < branchChance)
            {
                sf::Vec2f dir          = segment.end - segment.start;
                float     length       = dir.length();
                float     branchLength = length * rng.getF(branchLengthMin, branchLengthMax);

                // Add a random angle to the branch direction
                float angle = sf::base::atan2(dir.y, dir.x);
                angle += rng.getF(-0.8f, 0.8f); // Radians

                sf::Vec2f branchEnd = segment.start + sf::Vec2f(sf::base::cos(angle), sf::base::sin(angle)) * branchLength;

                m_branches.emplaceBack(rng, segment.start, branchEnd, color);
            }
        }


        // Convert the final list of points into a thick vertex array for drawing
        createVertexArray(segments, mainBoltThickness);
    }

    ////////////////////////////////////////////////////////////
    void update(sf::Time dt)
    {
        m_lifetime += dt;
        for (auto& branch : m_branches)
            branch.update(dt);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isFinished() const
    {
        return m_lifetime >= m_duration;
    }

    ////////////////////////////////////////////////////////////
    void draw(sf::RenderTarget& target, const sf::RenderStates states) const
    {
        if (isFinished())
            return;

        // Calculate the current alpha based on lifetime
        float progress = m_lifetime.asSeconds() / m_duration.asSeconds();
        auto  alpha    = static_cast<sf::base::U8>(255 * (1.f - progress));

        // Draw the outer glow
        sf::base::Vector<sf::Vertex> glow = m_verticesGlow;
        for (auto& i : glow)
            i.color.a = alpha / 3;
        target.draw(glow, sf::PrimitiveType::Triangles, states);

        // Draw the inner core
        sf::base::Vector<sf::Vertex> core = m_verticesCore;
        for (auto& i : core)
            i.color.a = alpha;
        target.draw(core, sf::PrimitiveType::Triangles, states);

        // Draw the branches
        for (const auto& branch : m_branches)
            branch.draw(target, states);
    }

private:
    ////////////////////////////////////////////////////////////
    void generateSegments(auto&& rng, sf::base::Vector<Segment>& segments, float jaggedness, int recursionDepth = 5)
    {
        if (recursionDepth <= 0)
            return;

        sf::base::Vector<Segment> newSegments;

        for (const auto& segment : segments)
        {
            sf::Vec2f midpoint = (segment.start + segment.end) * 0.5f;

            // Displace the midpoint perpendicularly

            if ((segment.end - segment.start).length() < 1.f)
                continue;

            sf::Vec2f dir    = (segment.end - segment.start).normalized();
            sf::Vec2f normal = {-dir.y, dir.x}; // Perpendicular vector

            float offset = rng.getF(-1.f, 1.f) * (segment.end - segment.start).length() * jaggedness;
            midpoint += normal * offset;

            newSegments.pushBack({segment.start, midpoint});
            newSegments.pushBack({midpoint, segment.end});
        }

        segments = newSegments;
        generateSegments(rng, segments, jaggedness, recursionDepth - 1);
    }

    ////////////////////////////////////////////////////////////
    void createVertexArray(const sf::base::Vector<Segment>& segments, float thickness)
    {
        float glowThickness = thickness * 4.f;

        for (const auto& segment : segments)
        {
            sf::Vec2f dir    = (segment.end - segment.start).normalized();
            sf::Vec2f normal = {-dir.y, dir.x};

            // Core vertices
            m_verticesCore.pushBack({segment.start - normal * thickness * 0.5f, m_color}); // A
            m_verticesCore.pushBack({segment.end - normal * thickness * 0.5f, m_color});   // B
            m_verticesCore.pushBack({segment.end + normal * thickness * 0.5f, m_color});   // C
            m_verticesCore.pushBack({segment.start - normal * thickness * 0.5f, m_color}); // A
            m_verticesCore.pushBack({segment.start + normal * thickness * 0.5f, m_color}); // D
            m_verticesCore.pushBack({segment.end + normal * thickness * 0.5f, m_color});   // C

            // Glow vertices
            m_verticesGlow.pushBack({segment.start - normal * glowThickness * 0.5f, m_color}); // A
            m_verticesGlow.pushBack({segment.end - normal * glowThickness * 0.5f, m_color});   // B
            m_verticesGlow.pushBack({segment.end + normal * glowThickness * 0.5f, m_color});   // C
            m_verticesGlow.pushBack({segment.start - normal * glowThickness * 0.5f, m_color}); // A
            m_verticesGlow.pushBack({segment.start + normal * glowThickness * 0.5f, m_color}); // D
            m_verticesGlow.pushBack({segment.end + normal * glowThickness * 0.5f, m_color});   // C
        }
    }

    ////////////////////////////////////////////////////////////
    sf::Color m_color;
    sf::Time  m_lifetime;
    sf::Time  m_duration;

    sf::base::Vector<sf::Vertex> m_verticesCore;
    sf::base::Vector<sf::Vertex> m_verticesGlow;

    sf::base::Vector<LightningBolt> m_branches;
};


////////////////////////////////////////////////////////////
// Delayed actions
struct DelayedAction
{
    Countdown                            delayCountdown;
    sf::base::FixedFunction<void(), 128> action;
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
         .contextSettings = {.antiAliasingLevel = m_aaLevel}});

    ////////////////////////////////////////////////////////////
    const sf::Font m_font = sf::Font::openFromFile("resources/Born2bSportyFS.ttf").value();

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

    ////////////////////////////////////////////////////////////
    sf::base::Array<sf::base::Optional<sf::Sound>, 8> m_soundPool;

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
    ImFont* const    m_imguiFont{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/Born2bSportyFS.ttf", 64.f)};

    ////////////////////////////////////////////////////////////
    sf::Clock m_tickClock;
    float     m_timeAccumulator = 0.f;

    ////////////////////////////////////////////////////////////
    World m_world;

    ////////////////////////////////////////////////////////////
    sf::Vec2f                         m_currentTetraminoVisualPosition;
    float                             m_squishMult = 0.f;
    AnimationTimeline                 m_animationTimeline;
    sf::base::Vector<float>           m_rowYOffsets;
    bool                              m_inLevelUpScreen = false;
    sf::base::Vector<sf::base::SizeT> m_perkIndicesSelectedThisLevel;
    sf::base::Vector<LightningBolt>   m_lightningBolts;
    sf::base::Vector<DelayedAction>   m_delayedActions;

    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] QuakeSinEffect
    {
        float timeRemaining = 0.f;
        float magnitude     = 0.f;
        float speed         = 0.f;

        ////////////////////////////////////////////////////////////
        void update(const sf::Time deltaTime)
        {
            if (timeRemaining > 0.f)
            {
                timeRemaining -= deltaTime.asSeconds() * speed;

                if (timeRemaining <= 0.f)
                {
                    timeRemaining = 0.f;
                    magnitude     = 0.f;
                }
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

    QuakeSinEffect m_quakeSinEffectLineClear;
    QuakeSinEffect m_quakeSinEffectHardDrop;

    ////////////////////////////////////////////////////////////
    struct BlockEffect
    {
        static constexpr float squishDuration = 0.2f;
        float                  squishTime     = 0.f;
    };

    ankerl::unordered_dense::map<sf::base::U32, BlockEffect> m_blockEffects;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<EarnedXPParticle> m_earnedXPParticles;

    ////////////////////////////////////////////////////////////
    RNGFast m_rngFast{static_cast<RNGFast::SeedType>(
        sf::Clock::now().asMicroseconds())}; // very fast, low-quality, but good enough for VFXs

    /////////////////////////////////////////////////////////////
    const sf::Cursor m_cursorArrow      = sf::Cursor::loadFromSystem(sf::Cursor::Type::Arrow).value();
    const sf::Cursor m_cursorHand       = sf::Cursor::loadFromSystem(sf::Cursor::Type::Hand).value();
    const sf::Cursor m_cursorNotAllowed = sf::Cursor::loadFromSystem(sf::Cursor::Type::NotAllowed).value();

    //////////////////////////////////////////////////////////////
    sf::TextureAtlas m_textureAtlas{sf::Texture::create({4096u, 4096u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    const sf::FloatRect m_txrWhiteDotTrue = m_textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value();
    const sf::FloatRect m_txrWhiteDot = {{0.f, 0.f}, {1.f, 1.f}};

    //////////////////////////////////////////////////////////////
    sf::RenderTexture m_rtGame{
        sf::RenderTexture::create(resolution.toVec2u(), {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};


    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgResourceToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile("resources" / path).value(), /* padding */ {2u, 2u}).value();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isLMBPressed() const noexcept
    {
        return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] sf::Vec2f getMousePos() const noexcept
    {
        return m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_window.getView()) * 2.f;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] int calculateGhostY(const Tetramino& tetramino) const
    {
        int ghostY = tetramino.position.y;

        while (m_world.blockGrid.isValidMove(tetramino.shape, {tetramino.position.x, ghostY + 1}))
            ++ghostY;

        return ghostY;
    }


    ////////////////////////////////////////////////////////////
    static inline constexpr auto drawOffset    = sf::Vec2f{32.f, 32.f - 32.f * gridGraceY};
    static inline constexpr auto drawBlockSize = sf::Vec2f{32.f, 32.f};


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] sf::Vec2f toDrawCoordinates(const sf::Vec2<T>& position) const noexcept
    {
        return sf::Vec2f{static_cast<float>(position.x), static_cast<float>(position.y)}.componentWiseMul(drawBlockSize);
    }


    ////////////////////////////////////////////////////////////
    void drawBlock(const Block& block, const sf::Vec2f& position, const DrawBlockOptions& options = {})
    {
        float yOffset = 0.f;

        if (options.applyYOffset)
        {
            if (m_animationTimeline.isPlaying<AnimCollapseGrid>() && !m_rowYOffsets.empty())
                yOffset = m_rowYOffsets[static_cast<sf::base::SizeT>(position.y / drawBlockSize.y)] *
                          easeInBack(m_animationTimeline.getProgress());
        }
        else
        {
            yOffset = -(m_quakeSinEffectHardDrop.getValue() + m_quakeSinEffectLineClear.getValue());
        }

        const auto alpha = static_cast<sf::base::U8>(options.opacity * 255.f);

        float finalSquishMult = 1.f + options.squishMult;
        if (const auto* it = m_blockEffects.find(block.blockId); it != m_blockEffects.end())
        {
            const BlockEffect& effect = it->second;

            const float progress = effect.squishTime / BlockEffect::squishDuration;
            finalSquishMult += easeInOutSine(bounce(progress)) * 0.5f;
        }

        m_rtGame.draw(sf::RectangleShapeData{
            .position         = drawOffset + position.addY(yOffset),
            .scale            = sf::Vec2f{finalSquishMult, finalSquishMult} * options.scale,
            .origin           = drawBlockSize / 2.f,
            .rotation         = sf::degrees(options.rotation),
            .fillColor        = blockPalette[block.paletteIdx].withAlpha(alpha),
            .outlineColor     = blockPalette[block.paletteIdx].withAlpha(alpha).withLightness(0.3f),
            .outlineThickness = 2.f,
            .size             = drawBlockSize,
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

            sf::Text text{m_font,
                          {
                              .scale            = sf::Vec2f{finalSquishMult, finalSquishMult} * options.scale * 0.75f,
                              .origin           = drawBlockSize / 2.f,
                              .string           = txt,
                              .fillColor        = sf::Color::Black.withAlpha(alpha),
                              .outlineColor     = sf::Color::White.withAlpha(alpha),
                              .outlineThickness = 2.f,
                          }};

            text.setCenter(drawOffset + position.addY(yOffset));
            m_rtGame.draw(text);
        }

        if (block.health > 1u && options.drawText)
        {
            sf::Text text{m_font,
                          {
                              .scale            = sf::Vec2f{finalSquishMult, finalSquishMult} * options.scale,
                              .origin           = drawBlockSize / 2.f,
                              .string           = std::to_string(static_cast<unsigned int>(block.health - 1u)),
                              .fillColor        = sf::Color::White.withAlpha(alpha),
                              .outlineColor     = sf::Color::Black.withAlpha(alpha),
                              .outlineThickness = 2.f,
                          }};

            text.setCenter(drawOffset + position.addY(yOffset));
            m_rtGame.draw(text);
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

        const bool  isIPiece  = (tetramino.tetraminoType == TetraminoType::I);
        const auto& kickTable = isIPiece ? kickDataI : kickDataJLSTZ;

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
                .duration   = 0.3f,
                .awardXP    = true,
                .forceClear = false,
            });
    }


    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findVerticalDrillBlocks() const
    {
        auto downmostBlocksXY = findDownmostBlocks(m_world.currentTetramino->shape);

        if (downmostBlocksXY.size() > 1u && !m_world.perkVerticalDrill->multiHit)
            return {};

        return downmostBlocksXY;
    }


    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findHorizontalDrillLeftBlocks() const
    {
        return findHorizontalBlocks(m_world.currentTetramino->shape,
                                    static_cast<sf::base::SizeT>(m_world.perkHorizontalDrillLeft->maxBlocks));
    }


    ////////////////////////////////////////////////////////////
    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findHorizontalDrillRightBlocks() const
    {
        return findHorizontalBlocks(m_world.currentTetramino->shape,
                                    static_cast<sf::base::SizeT>(m_world.perkHorizontalDrillRight->maxBlocks));
    }


    ////////////////////////////////////////////////////////////
    struct BlockAndPosition
    {
        Block*     block;
        sf::Vec2uz position;
    };


    ////////////////////////////////////////////////////////////
    sf::base::Vector<BlockAndPosition> findVerticalDrillTargetBlocks()
    {
        sf::base::Vector<BlockAndPosition> result;

        auto nToHit = static_cast<sf::base::SizeT>(m_world.perkVerticalDrill->maxPenetration);

        for (const auto bPos : findVerticalDrillBlocks())
        {
            const auto currPos = m_world.currentTetramino->position + bPos.toVec2i().addY(1);

            if (currPos.y == static_cast<int>(m_world.blockGrid.getHeight()))
                continue;

            SFML_BASE_ASSERT(m_world.blockGrid.isInBounds(currPos));

            for (auto y = currPos.y; y < static_cast<int>(m_world.blockGrid.getHeight()); ++y)
            {
                const auto blockPos = sf::Vec2i{currPos.x, y}.toVec2uz();

                auto& optBlock = m_world.blockGrid.at(blockPos);

                if (!optBlock.hasValue())
                    break;

                if (optBlock->health == 1u)
                    continue;

                result.pushBack({optBlock.asPtr(), blockPos});

                if (--nToHit == 0u)
                    break;
            }
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    sf::base::Vector<BlockAndPosition> findHorizontalDrillTargetBlocksLeft()
    {
        sf::base::Vector<BlockAndPosition> result;

        auto nToHitLeft = static_cast<sf::base::SizeT>(m_world.perkHorizontalDrillLeft->maxPenetration);

        for (const auto bPos : findHorizontalDrillLeftBlocks())
        {
            const auto currPos = m_world.currentTetramino->position + bPos.toVec2i();

            for (auto x = currPos.x - 1; x >= 0; --x)
            {
                const auto blockPos = sf::Vec2i{x, currPos.y}.toVec2uz();
                auto&      optBlock = m_world.blockGrid.at(blockPos);

                if (!optBlock.hasValue())
                    break;

                if (optBlock->health == 1u)
                    continue;

                result.pushBack({optBlock.asPtr(), blockPos});

                if (--nToHitLeft == 0u)
                    break;
            }
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    sf::base::Vector<BlockAndPosition> findHorizontalDrillTargetBlocksRight()
    {
        sf::base::Vector<BlockAndPosition> result;

        auto nToHitRight = static_cast<sf::base::SizeT>(m_world.perkHorizontalDrillRight->maxPenetration);

        for (const auto bPos : findHorizontalDrillRightBlocks())
        {
            const auto currPos = m_world.currentTetramino->position + bPos.toVec2i();

            for (auto x = currPos.x + 1; x < static_cast<int>(m_world.blockGrid.getWidth()); ++x)
            {
                const auto blockPos = sf::Vec2i{x, currPos.y}.toVec2uz();
                auto&      optBlock = m_world.blockGrid.at(blockPos);

                if (!optBlock.hasValue())
                    break;

                if (optBlock->health == 1u)
                    continue;

                result.pushBack({optBlock.asPtr(), blockPos});

                if (--nToHitRight == 0u)
                    break;
            }
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    void embedCurrentTetraminoAndClearLines()
    {
        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        if (m_world.perkXPPerTetraminoPlaced > 0)
        {
            addXP(static_cast<sf::base::U64>(m_world.perkXPPerTetraminoPlaced));

            const auto downmostBlocksXY = findDownmostBlocks(m_world.currentTetramino->shape);

            for (const auto& bPos : downmostBlocksXY)
            {
                const sf::base::Optional<Block>& blockOpt = m_world.currentTetramino->shape[bPos.y * shapeDimension + bPos.x];
                SFML_BASE_ASSERT(blockOpt.hasValue());

                spawnXPEarnedParticle(toDrawCoordinates(m_world.currentTetramino->position + bPos.toVec2i()) +
                                          drawBlockSize / 2.f,
                                      blockOpt->paletteIdx);
            }
        }

        m_world.blockGrid.embedTetramino(*m_world.currentTetramino);
        m_world.currentTetramino.reset();
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
                    .duration   = 0.3f,
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
        if (eKeyPressed.code == sf::Keyboard::Key::Right)
        {
            moveCurrentTetraminoBy({1, 0});
            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::Down)
        {
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
            const int endY = calculateGhostY(*m_world.currentTetramino);

            m_animationTimeline.add(AnimHardDrop{
                .duration = 0.1f,
                .endY     = endY,
            });

            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::LShift)
        {
            if (m_world.perkCanHoldTetramino == 1)
                holdCurrentTetramino();
            else if (m_world.perkCanHoldTetramino == 2)
                skipCurrentTetramino();

            return;
        }

        if (eKeyPressed.code == sf::Keyboard::Key::R)
        {
            // Restart the game
            m_world = World{};
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

        m_currentTetraminoVisualPosition = toDrawCoordinates(m_world.currentTetramino->position);
        m_squishMult                     = 0.f;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool eventStep()
    {
        while (sf::base::Optional event = m_window.pollEvent())
        {
            m_imGuiContext.processEvent(m_window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return false;

            if (handleAspectRatioAwareResize(*event, resolution, m_window))
                continue;

            if (isInPlayableState())
            {
                if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                    handleKeyPressedEvent(*eKeyPressed);
            }
        }

        return true;
    }


    /////////////////////////////////////////////////////////////
    void damageBlock(const sf::Vec2uz position, Block& block)
    {
        SFML_BASE_ASSERT(block.health > 1u);

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

        for (sf::base::SizeT i = 0u; i < m_perkSelectors.size(); ++i)
            if (m_perkSelectors[i].fnPrerequisites())
                m_perkIndicesSelectedThisLevel.pushBack(i);

        while (m_perkIndicesSelectedThisLevel.size() > 4u)
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
    struct EligibleBlock // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        Block*     block;
        sf::Vec2uz position;
    };


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
                    [this, pos = blockInfo.position]
            {
                m_lightningBolts.emplaceBack(m_rngFast,
                                             sf::Vec2f{m_rngFast.getF(0.f, 400.f), 0.f},
                                             toDrawCoordinates(pos) + drawOffset);

                playSound(m_sbStrike, 0.75f);
            },
                .duration = 0.2f,
            });

            m_animationTimeline.add(AnimAction{
                .action   = [this, pos = blockInfo.position, &block = *blockInfo.block] { damageBlock(pos, block); },
                .duration = 0.1f,
            });

            eligibleBlocks.erase(eligibleBlocks.begin() + randomIndex);
        }

        if (m_world.perkChainLightning > 0)
        {
            const auto roll = m_rngFast.getI(0, 100);

            if (roll < m_world.perkChainLightning)
            {
                m_animationTimeline.add(AnimAction{
                    .action   = [this, n = actualNBlocksToHit] { strikeNRandomBlocks(n); },
                    .duration = 0.01f,
                });
            }
        }
    }


    /////////////////////////////////////////////////////////////
    void updateStep(const sf::Time deltaTime, const float xTicksPerSecond)
    {
        SFEX_PROFILE_SCOPE("update");

        // Quake effect
        {
            m_quakeSinEffectLineClear.update(deltaTime);
            m_quakeSinEffectHardDrop.update(deltaTime);
        }

        // Show level up screen if needed
        if (m_world.committedPlayerLevel < m_world.playerLevel && !m_inLevelUpScreen)
        {
            m_inLevelUpScreen = true;
            rerollPerks();

            playSound(m_sbNewLevel);
        }

        // Interpolate visual position
        if (m_world.currentTetramino.hasValue())
        {
            const sf::Vec2f targetPosition = toDrawCoordinates(m_world.currentTetramino->position);

            const float interpolationSpeed = 0.0005f;

            m_currentTetraminoVisualPosition = exponentialApproach(m_currentTetraminoVisualPosition,
                                                                   targetPosition,
                                                                   interpolationSpeed,
                                                                   deltaTime.asSeconds());
        }

        // Deal with animations
        if (!m_inLevelUpScreen && m_animationTimeline.anyAnimationPlaying())
        {
            m_animationTimeline.timeOnCurrentCommand += deltaTime.asSeconds();

            auto& currentCmd = m_animationTimeline.commands.front();

            if (auto* wait = currentCmd.getIf<AnimWait>())
            {
                if (m_animationTimeline.timeOnCurrentCommand >= wait->duration)
                    m_animationTimeline.popFrontCommand();
            }
            else if (auto* hardDrop = currentCmd.getIf<AnimHardDrop>())
            {
                SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

                const float progress = m_animationTimeline.getProgress();

                const float targetVisualY = static_cast<float>(hardDrop->endY) * drawBlockSize.y;
                const float startVisualY  = static_cast<float>(m_world.currentTetramino->position.y) * drawBlockSize.y;

                m_currentTetraminoVisualPosition.y = startVisualY + (targetVisualY - startVisualY) * easeInBack(progress);

                if (progress >= 1.f)
                {
                    m_world.currentTetramino->position.y = hardDrop->endY;

                    m_animationTimeline.popFrontCommand();
                    m_animationTimeline.add(AnimSquish{
                        .duration = 0.1f,
                    });

                    m_quakeSinEffectHardDrop.start(8.f, 4.f);

                    playSound(m_sbLanded);
                }
            }
            else if (currentCmd.is<AnimSquish>())
            {
                SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

                const float progress = m_animationTimeline.getProgress();

                m_squishMult = easeInOutSine(bounce(progress)) * 0.5f;

                if (progress >= 1.f)
                {
                    if (m_world.perkVerticalDrill.hasValue() && !findVerticalDrillTargetBlocks().empty())
                    {
                        m_animationTimeline.add(AnimVerticalDrill{
                            .duration = 0.3f,
                        });
                    }

                    if (m_world.perkHorizontalDrillLeft.hasValue() && !findHorizontalDrillTargetBlocksLeft().empty())
                    {
                        m_animationTimeline.add(AnimHorizontalDrillLeft{
                            .duration = 0.3f,
                        });
                    }

                    if (m_world.perkHorizontalDrillRight.hasValue() && !findHorizontalDrillTargetBlocksRight().empty())
                    {
                        m_animationTimeline.add(AnimHorizontalDrillRight{
                            .duration = 0.3f,
                        });
                    }

                    m_animationTimeline.add(AnimEmbed{
                        .duration = 0.01f,
                    });

                    m_animationTimeline.popFrontCommand();
                }
            }
            else if (currentCmd.is<AnimEmbed>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    embedCurrentTetraminoAndClearLines();
                    m_animationTimeline.popFrontCommand();
                }
            }
            else if (auto* clearLines = currentCmd.getIf<AnimClearLines>())
            {
                sf::base::Vector<sf::base::SizeT>             trulyClearedRows;
                sf::base::Vector<AnimFadeBlocks::FadingBlock> fadingBlocks;
                sf::base::Vector<sf::Vec2uz>                  columnClearPositions;

                const auto addRowIfNotExistent = [&](const sf::base::SizeT row)
                {
                    if (sf::base::find(trulyClearedRows.begin(), trulyClearedRows.end(), row) == trulyClearedRows.end())
                        trulyClearedRows.pushBack(row);
                };

                for (sf::base::SizeT y : clearLines->rows)
                {
                    bool rowIsFullyCleared = true;

                    for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
                    {
                        if (auto& blockOpt = m_world.blockGrid.at(sf::Vec2uz{x, y}); blockOpt.hasValue())
                        {
                            // TODO: powerup
                            if (!clearLines->forceClear && blockOpt->health > 1u)
                            {
                                damageBlock(sf::Vec2uz{x, y}, *blockOpt);
                                rowIsFullyCleared = false;
                            }
                            else
                            {
                                fadingBlocks.pushBack(AnimFadeBlocks::FadingBlock{
                                    .block    = *blockOpt,
                                    .position = sf::Vec2uz{x, y}.toVec2f().componentWiseMul(drawBlockSize),
                                });

                                if (blockOpt->powerup == BlockPowerup::XPBonus)
                                {
                                    addXP(5u * m_world.playerLevel);
                                    playSound(m_sbBonus, 0.5f);

                                    spawnXPEarnedParticle(toDrawCoordinates(sf::Vec2uz{x, y}) + drawBlockSize / 2.f -
                                                              sf::Vec2f{0.f, 32.f * gridGraceY},
                                                          blockOpt->paletteIdx);
                                }
                                else if (blockOpt->powerup == BlockPowerup::ColumnDrill)
                                {
                                    columnClearPositions.emplaceBack(x, y);
                                    playSound(m_sbBonus, 0.5f);
                                }
                                else if (blockOpt->powerup == BlockPowerup::ThreeRowDrill)
                                {
                                    addRowIfNotExistent(y);

                                    if (y + 1u < m_world.blockGrid.getHeight())
                                        addRowIfNotExistent(y + 1);

                                    if (y > 0u)
                                        addRowIfNotExistent(y - 1);

                                    playSound(m_sbBonus, 0.5f);
                                }

                                blockOpt.reset();
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
                        .duration     = clearLines->duration,
                    });
                }

                if (!trulyClearedRows.empty())
                {
                    playSound(m_sbSingle, 0.85f);

                    const sf::base::SizeT numCleared = trulyClearedRows.size();

                    if (clearLines->awardXP)
                    {
                        m_world.linesCleared += numCleared;

                        const sf::base::U64 amount = [&]() noexcept
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

                            const auto startPosition = block.position + drawBlockSize / 2.f +
                                                       m_rngFast.getVec2f({-16.f, -16.f}, {16.f, 16.f});

                            spawnXPEarnedParticle(startPosition, block.block.paletteIdx);
                        }

                        // Random block hit perk
                        if (m_world.perkRndHitOnClear > 0 && numCleared > 1u)
                            strikeNRandomBlocks(static_cast<sf::base::SizeT>(m_world.perkRndHitOnClear));
                    }

                    sf::base::quickSort(trulyClearedRows.begin(),
                                        trulyClearedRows.end(),
                                        [](const sf::base::SizeT a, const sf::base::SizeT b) { return a < b; });

                    m_animationTimeline.add(AnimCollapseGrid{
                        .clearedRows = trulyClearedRows,
                        .duration    = 0.1f,
                    });
                }

                for (const auto columnClearPos : columnClearPositions)
                {
                    m_animationTimeline.add(AnimColumnClear{
                        .position = columnClearPos.addY(1),
                        .duration = clearLines->duration,
                    });
                }

                m_animationTimeline.popFrontCommand();
            }
            else if (currentCmd.is<AnimFadeBlocks>())
            {
                const float progress = m_animationTimeline.getProgress();

                if (progress >= 1.f)
                    m_animationTimeline.popFrontCommand();
            }
            else if (auto* collapseGrid = currentCmd.getIf<AnimCollapseGrid>())
            {
                const float progress = m_animationTimeline.getProgress();

                if (progress >= 1.f)
                {
                    for (const auto rowIndex : collapseGrid->clearedRows)
                        m_world.blockGrid.shiftRowDown(rowIndex);

                    m_animationTimeline.popFrontCommand();
                }
                else
                {
                    const auto height = m_world.blockGrid.getHeight();

                    m_rowYOffsets.resize(height);

                    for (sf::base::SizeT y = 0u; y < height; ++y)
                        m_rowYOffsets[y] = 0.f;

                    int clearedCount = 0;

                    // iterate in reverse
                    for (int y = static_cast<int>(height) - 1; y >= static_cast<int>(gridGraceY); --y)
                    {
                        const bool wasCleared = sf::base::find(collapseGrid->clearedRows.begin(),
                                                               collapseGrid->clearedRows.end(),
                                                               static_cast<sf::base::SizeT>(y)) !=
                                                collapseGrid->clearedRows.end();

                        if (wasCleared)
                            ++clearedCount;
                        else if (clearedCount > 0)
                            m_rowYOffsets[static_cast<sf::base::SizeT>(y)] = static_cast<float>(clearedCount) *
                                                                             drawBlockSize.y;
                    }
                }
            }
            else if (currentCmd.is<AnimVerticalDrill>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    m_animationTimeline.popFrontCommand();

                    for (auto [blockPtr, position] : findVerticalDrillTargetBlocks())
                        damageBlock(position, *blockPtr);
                }
            }
            else if (currentCmd.is<AnimHorizontalDrillLeft>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    m_animationTimeline.popFrontCommand();

                    for (auto [blockPtr, position] : findHorizontalDrillTargetBlocksLeft())
                        damageBlock(position, *blockPtr);
                }
            }
            else if (currentCmd.is<AnimHorizontalDrillRight>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    m_animationTimeline.popFrontCommand();

                    for (auto [blockPtr, position] : findHorizontalDrillTargetBlocksRight())
                        damageBlock(position, *blockPtr);
                }
            }
            else if (auto* columnClear = currentCmd.getIf<AnimColumnClear>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    sf::base::Vector<AnimFadeBlocks::FadingBlock> fadingBlocks;

                    for (sf::base::SizeT y = columnClear->position.y; y < m_world.blockGrid.getHeight(); ++y)
                        if (auto& blockOpt = m_world.blockGrid.at(sf::Vec2uz{columnClear->position.x, y});
                            blockOpt.hasValue())
                        {
                            fadingBlocks.pushBack(AnimFadeBlocks::FadingBlock{
                                .block = *blockOpt,
                                .position = sf::Vec2uz{columnClear->position.x, y}.toVec2f().componentWiseMul(drawBlockSize),
                            });

                            blockOpt.reset();
                        }

                    if (!fadingBlocks.empty())
                        m_animationTimeline.add(AnimFadeBlocks{
                            .fadingBlocks = fadingBlocks,
                            .duration     = 0.2f,
                        });

                    m_animationTimeline.popFrontCommand();
                }
            }
            else if (auto* action = currentCmd.getIf<AnimAction>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    action->action();
                    m_animationTimeline.popFrontCommand();
                }
            }
        }

        // Deal with block effects
        for (auto* it = m_blockEffects.begin(); it != m_blockEffects.end();)
        {
            it->second.squishTime += deltaTime.asSeconds();

            if (it->second.squishTime >= BlockEffect::squishDuration)
            {
                it = m_blockEffects.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Earned XP particles
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

        if (isInPlayableState())
        {
            // Refill block matrix bag if needed
            if (m_world.blockMatrixBag.size() < 2u) // TODO: adjust for peek
            {
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
                        .blockMatrix   = shapeMatrixToBlockMatrix(srsTetraminoShapes[static_cast<sf::base::SizeT>(
                                                                    j) /* pieceType */][0],
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
                                if (m_rngFast.getI(0, 100) > 50)
                                    b->powerup = BlockPowerup::ColumnDrill;
                                else
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

            // Pick next tetramino if there is none
            if (!m_world.currentTetramino.hasValue())
            {
                initializeCurrentTetraminoFromBag();
                m_world.holdUsedThisTurn = false;
            }

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
                                embedCurrentTetraminoAndClearLines();
                                playSound(m_sbPlace, 0.5f);
                            }
                        }
                    }
                }
            }
        }

        //
        // Delayed actions
        {
            const auto deltaTimeMs = static_cast<float>(deltaTime.asMicroseconds()) / 1000.f;

            for (auto& [delayCountdown, func] : m_delayedActions)
                if (delayCountdown.updateAndStop(deltaTimeMs) == CountdownStatusStop::JustFinished)
                    func();

            sf::base::vectorEraseIf(m_delayedActions, [](const auto& delayedAction) {
                return delayedAction.delayCountdown.isDone();
            });
        }

        //
        // Lightning bolts
        for (auto& lb : m_lightningBolts)
            lb.update(deltaTime);

        sf::base::vectorEraseIf(m_lightningBolts, [](const LightningBolt& lb) { return lb.isFinished(); });
    }


    /////////////////////////////////////////////////////////////
    struct [[nodiscard]] PerkSelector
    {
        sf::base::FixedFunction<std::string(), 32> fnName;
        sf::base::FixedFunction<std::string(), 32> fnDescription;
        sf::base::FixedFunction<bool(), 32>        fnPrerequisites;
        sf::base::FixedFunction<void(), 32>        fnApply;
    };


    /////////////////////////////////////////////////////////////
    [[nodiscard]] std::string makeTitle(const std::string& prefix, const auto& value, const auto inc)
    {
        return prefix + " (" + std::to_string(value) + " -> " + std::to_string(value + inc) + ")";
    }


    /////////////////////////////////////////////////////////////
    sf::base::Vector<PerkSelector> m_perkSelectors = {

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("Chain Lightning", m_world.perkChainLightning, 10); },
            .fnDescription =
                [&]
    {
        return std::string{"Each time lightning strikes, add a " + std::to_string(m_world.perkChainLightning + 10) +
                           "% chance to hit an additional block."};
    },
            .fnPrerequisites =
                [&]
    {
        return (m_world.perkRndHitPerNTetraminos.hasValue() || m_world.perkRndHitOnClear > 0) &&
               m_world.perkChainLightning < 50;
    },
            .fnApply = [&] { m_world.perkChainLightning += 10; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName        = [&] { return makeTitle("Peek Next Tetraminos", m_world.perkNPeek, 1); },
            .fnDescription = [&]
    { return std::string{"See the next " + std::to_string(m_world.perkNPeek + 1) + " upcoming tetraminos."}; },
            .fnPrerequisites = [&] { return m_world.perkNPeek < 3; },
            .fnApply         = [&] { ++m_world.perkNPeek; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("On-Double Lightning Strike", m_world.perkRndHitOnClear, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Each time you full-clear two lines or more, randomly damage " +
                           std::to_string(m_world.perkRndHitOnClear + 1) + " block(s) with a lightning strike."};
    },
            .fnPrerequisites = [&] { return true; },
            .fnApply         = [&] { ++m_world.perkRndHitOnClear; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName        = [&] { return "Hard-Drop Drill"; },
            .fnDescription = [&]
    { return "When hard dropping, automatically damage blocks directly connected below the tetramino's sharp edges."; },
            .fnPrerequisites = [&] { return !m_world.perkVerticalDrill.hasValue(); },
            .fnApply         = [&] { m_world.perkVerticalDrill.emplace(); },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]
    { return makeTitle("Hard-Drop Drill - Penetration", m_world.perkVerticalDrill->maxPenetration, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum number of blocks damaged to "} +
               std::to_string(m_world.perkVerticalDrill->maxPenetration + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkVerticalDrill.hasValue() && m_world.perkVerticalDrill->maxPenetration < 10; },
            .fnApply = [&] { ++m_world.perkVerticalDrill->maxPenetration; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName        = [&] { return "Hard-Drop Drill - Blunt Force"; },
            .fnDescription = [&] { return "The entire surface of the tetramino acts as a drill when hard-dropping."; },
            .fnPrerequisites = [&]
    { return m_world.perkVerticalDrill.hasValue() && !m_world.perkVerticalDrill->multiHit; },
            .fnApply = [&] { m_world.perkVerticalDrill->multiHit = true; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName =
                [&]
    {
        if (m_world.perkCanHoldTetramino == 0)
            return "Hold Tetramino";

        return "Skip Tetramino (Hold -> Skip)";
    },
            .fnDescription =
                [&]
    {
        if (m_world.perkCanHoldTetramino == 0)
            return "Gain the ability to hold your current tetramino. Can later be updated to skip a tetramino.";

        return "Gain the ability to completely skip your current tetramino.";
    },
            .fnPrerequisites = [&] { return m_world.perkCanHoldTetramino < 2; },
            .fnApply         = [this] { ++m_world.perkCanHoldTetramino; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("XP per Tetramino Placed", m_world.perkXPPerTetraminoPlaced, 3); },
            .fnDescription =
                [&]
    {
        return std::string{"Gain "} + std::to_string(m_world.perkXPPerTetraminoPlaced + 3) +
               " XP for each tetramino you place.";
    },
            .fnPrerequisites = [&] { return true; },
            .fnApply         = [&] { m_world.perkXPPerTetraminoPlaced += 3; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("XP per Block Damaged", m_world.perkXPPerBlockDamaged, 10); },
            .fnDescription =
                [&]
    {
        return std::string{"Gain "} + std::to_string(m_world.perkXPPerBlockDamaged + 10) +
               " XP for each block you damage.";
    },
            .fnPrerequisites = [&] { return true; },
            .fnApply         = [&] { m_world.perkXPPerBlockDamaged += 10; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]() -> std::string
    {
        if (!m_world.perkDeleteFloorPerNTetraminos.hasValue())
            return "Delete Floor per 30 Tetramino Placed";

        return std::string{"Delete Floor per Tetramino Placed"} + " (" +
               std::to_string(m_world.perkDeleteFloorPerNTetraminos->nTetraminos) + " -> " +
               std::to_string(m_world.perkDeleteFloorPerNTetraminos->nTetraminos - 1) + ")";
    },
            .fnDescription = [&]
    { return "Each time you place a certain amount of tetraminos, delete the bottom row of the grid."; },
            .fnPrerequisites =
                [&]
    {
        return !m_world.perkDeleteFloorPerNTetraminos.hasValue() || m_world.perkDeleteFloorPerNTetraminos->nTetraminos > 10;
    },
            .fnApply =
                [&]
    {
        if (!m_world.perkDeleteFloorPerNTetraminos.hasValue())
            m_world.perkDeleteFloorPerNTetraminos.emplace(30);
        else
            --(m_world.perkDeleteFloorPerNTetraminos->nTetraminos);
    },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]() -> std::string
    {
        if (!m_world.perkRndHitPerNTetraminos.hasValue())
            return "On-N-Placed Lightning Strike";

        return std::string{"On-N-Placed Lightning Strike"} + " (" +
               std::to_string(m_world.perkRndHitPerNTetraminos->nTetraminos) + " -> " +
               std::to_string(m_world.perkRndHitPerNTetraminos->nTetraminos - 1) + ")";
    },
            .fnDescription = [&]
    { return "Each time you place a certain amount of tetraminos, randomly damage a block with a lightning strike."; },
            .fnPrerequisites = [&]
    { return !m_world.perkRndHitPerNTetraminos.hasValue() || m_world.perkRndHitPerNTetraminos->nTetraminos > 10; },
            .fnApply =
                [&]
    {
        if (!m_world.perkRndHitPerNTetraminos.hasValue())
            m_world.perkRndHitPerNTetraminos.emplace(20);
        else
            --(m_world.perkRndHitPerNTetraminos->nTetraminos);
    },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("Extra Line Pieces in Pool", m_world.perkExtraLinePiecesInPool, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the number of line pieces in the tetramino pool by "} +
               std::to_string(m_world.perkExtraLinePiecesInPool + 1) + ".";
    },
            .fnPrerequisites = [&] { return m_world.perkExtraLinePiecesInPool < 3; },
            .fnApply         = [&] { ++m_world.perkExtraLinePiecesInPool; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName          = [&] { return "Left Horizontal Drill"; },
            .fnDescription   = [&] { return "Damage all blocks directly adjacent left to the tetramino."; },
            .fnPrerequisites = [&] { return !m_world.perkHorizontalDrillLeft.hasValue(); },
            .fnApply         = [&] { m_world.perkHorizontalDrillLeft.emplace(); },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName          = [&] { return "Right Horizontal Drill"; },
            .fnDescription   = [&] { return "Damage all blocks directly adjacent right to the tetramino."; },
            .fnPrerequisites = [&] { return !m_world.perkHorizontalDrillRight.hasValue(); },
            .fnApply         = [&] { m_world.perkHorizontalDrillRight.emplace(); },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]
    { return makeTitle("Left Horizontal Drill - Penetration", m_world.perkHorizontalDrillLeft->maxPenetration, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum number of blocks damaged to "} +
               std::to_string(m_world.perkHorizontalDrillLeft->maxPenetration + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkHorizontalDrillLeft.hasValue() && m_world.perkHorizontalDrillLeft->maxPenetration < 4; },
            .fnApply = [&] { ++m_world.perkHorizontalDrillLeft->maxPenetration; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]
    { return makeTitle("Left Horizontal Drill - Length", m_world.perkHorizontalDrillLeft->maxBlocks, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum length of the drill to "} +
               std::to_string(m_world.perkHorizontalDrillLeft->maxBlocks + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkHorizontalDrillLeft.hasValue() && m_world.perkHorizontalDrillLeft->maxBlocks < 4; },
            .fnApply = [&] { ++m_world.perkHorizontalDrillLeft->maxBlocks; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]
    { return makeTitle("Right Horizontal Drill - Penetration", m_world.perkHorizontalDrillRight->maxPenetration, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum number of blocks damaged to "} +
               std::to_string(m_world.perkHorizontalDrillRight->maxPenetration + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkHorizontalDrillRight.hasValue() && m_world.perkHorizontalDrillRight->maxPenetration < 4; },
            .fnApply = [&] { ++m_world.perkHorizontalDrillRight->maxPenetration; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]
    { return makeTitle("Right Horizontal Drill - Length", m_world.perkHorizontalDrillRight->maxBlocks, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum length of the drill to "} +
               std::to_string(m_world.perkHorizontalDrillRight->maxBlocks + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkHorizontalDrillRight.hasValue() && m_world.perkHorizontalDrillRight->maxBlocks < 4; },
            .fnApply = [&] { ++m_world.perkHorizontalDrillRight->maxBlocks; },
        },
    };


    /////////////////////////////////////////////////////////////
    void imguiStep(const sf::Time deltaTime)
    {
        SFEX_PROFILE_SCOPE("imgui");
        m_imGuiContext.update(m_window, deltaTime);

        const auto setFontScale = [&](const float x) { ImGui::SetWindowFontScale(x * 0.75f); };

        const auto textCentered = [&](const std::string& text)
        {
            auto windowWidth = ImGui::GetWindowSize().x;
            auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text("%s", text.c_str());
        };

        if (m_inLevelUpScreen)
        {
            SFML_BASE_ASSERT(m_world.committedPlayerLevel < m_world.playerLevel);

            ImGui::SetNextWindowBgAlpha(0.95f);

            ImGui::PushFont(m_imguiFont);

            ImGui::Begin("Level Up!", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

            ImGui::SetWindowPos(ImVec2{resolution.x / 2.f - 400.f, resolution.y / 2.f - 300.f});
            ImGui::SetWindowSize(ImVec2{800.f, 600.f});

            setFontScale(0.85f);
            textCentered("*** LEVEL UP ***");
            setFontScale(0.65f);
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
                PerkSelector& perkSelector = m_perkSelectors[psIndex];

                std::string perkName        = perkSelector.fnName();
                std::string perkDescription = perkSelector.fnDescription();

                if (sep)
                    ImGui::Separator();

                setFontScale(0.8f);
                if (ImGui::Selectable(perkName.c_str(), selectedPerk == static_cast<int>(psIndex)))
                    selectedPerk = static_cast<int>(psIndex);

                setFontScale(0.55f);
                ImGui::TextWrapped("%s", perkDescription.c_str());

                sep = true;
            }

            if (selectedPerk != -1)
            {
                ImGui::Spacing();
                ImGui::Spacing();
                setFontScale(0.65f);
                if (ImGui::Button("Confirm"))
                {
                    m_inLevelUpScreen = false;
                    m_perkIndicesSelectedThisLevel.clear();

                    ++m_world.committedPlayerLevel;

                    m_perkSelectors[static_cast<sf::base::SizeT>(selectedPerk)].fnApply();
                }
            }

            ImGui::PopFont();
            ImGui::End();
        }
    }


    /////////////////////////////////////////////////////////////
    void drawStep()
    {
        SFEX_PROFILE_SCOPE("draw");

        m_rtGame.clear();

        {
            SFEX_PROFILE_SCOPE("rtGame");

            // Draw grid background and lines
            for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
                for (sf::base::SizeT y = gridGraceY; y < m_world.blockGrid.getHeight(); ++y)
                {
                    const auto position = toDrawCoordinates(sf::Vec2uz{x, y});

                    m_rtGame.draw(sf::RectangleShapeData{
                        .position         = drawOffset + position,
                        .origin           = drawBlockSize / 2.f,
                        .fillColor        = sf::Color(30, 30, 30),
                        .outlineColor     = sf::Color(50, 50, 50),
                        .outlineThickness = 2.f,
                        .size             = drawBlockSize,
                    });
                }

            // Draw pieces embedded in grid
            for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
                for (sf::base::SizeT y = gridGraceY; y < m_world.blockGrid.getHeight(); ++y)
                {
                    const sf::Vec2uz                 gridPosition{x, y};
                    const sf::base::Optional<Block>& blockOpt = m_world.blockGrid.at(gridPosition);

                    if (!blockOpt.hasValue())
                        continue;

                    drawBlock(*blockOpt, toDrawCoordinates(gridPosition), {.opacity = 1.f});
                }

            // Draw fading blocks
            if (auto* fadeBlocks = m_animationTimeline.getIfPlaying<AnimFadeBlocks>())
            {
                const float progress = m_animationTimeline.getProgress();

                for (const auto& fadingBlock : fadeBlocks->fadingBlocks)
                {
                    drawBlock(fadingBlock.block,
                              fadingBlock.position,
                              {
                                  .opacity    = 1.f - easeInOutSine(progress),
                                  .squishMult = 0.f,
                              });
                }
            }

            // Draw vertical drill
            if (m_animationTimeline.isPlaying<AnimVerticalDrill>())
            {
                const float progress = m_animationTimeline.getProgress();

                const auto downmostBlocksXY = findDownmostBlocks(m_world.currentTetramino->shape);
                // auto nToHit = static_cast<sf::base::SizeT>(m_world.perkVerticalDrill->maxPenetration);

                const auto paletteIdx = [&]
                {
                    for (const auto& b : m_world.currentTetramino->shape)
                        if (b.hasValue())
                            return b->paletteIdx;

                    SFML_BASE_UNREACHABLE();
                }();

                if (downmostBlocksXY.size() == 1u || m_world.perkVerticalDrill->multiHit)
                    for (const auto bPos : downmostBlocksXY)
                    {
                        const auto startPos = m_world.currentTetramino->position + bPos.toVec2i();
                        auto       endPos   = startPos.addY(1);

                        while (endPos.y < static_cast<int>(m_world.blockGrid.getHeight()))
                        {
                            if (!m_world.blockGrid.at(endPos.toVec2uz()).hasValue())
                                break;

                            ++endPos.y;
                        }

                        const auto startDrawPos = toDrawCoordinates(startPos.toVec2uz()).addY(16.f).addX(-8.f) + drawOffset;
                        const auto endDrawPos = toDrawCoordinates(endPos.toVec2uz()).addY(-32.f).addX(-8.f) + drawOffset;

                        const auto diff = endDrawPos - startDrawPos;

                        for (int i = 0; i < 32; ++i)
                        {
                            m_rtGame.draw(sf::CircleShapeData{
                                .position         = {blend(startDrawPos,
                                                   startDrawPos + ((diff / 32.f) * static_cast<float>(i + 1)),
                                                   easeInOutSine(bounce(progress)))},
                                .scale            = sf::Vec2f{0.5f, 0.5f},
                                .origin           = {16.f, 16.f},
                                .rotation         = sf::degrees(0),
                                .fillColor        = blockPalette[paletteIdx],
                                .outlineColor     = blockPalette[paletteIdx].withLightness(0.3f),
                                .outlineThickness = 1.f,
                                .radius           = 32.f,
                                .pointCount       = 3u,
                            });
                        }
                    }
            }

            // Draw horizontal drill (left)
            if (m_animationTimeline.isPlaying<AnimHorizontalDrillLeft>())
            {
                const float progress = m_animationTimeline.getProgress();

                const auto paletteIdx = [&]
                {
                    for (const auto& b : m_world.currentTetramino->shape)
                        if (b.hasValue())
                            return b->paletteIdx;

                    SFML_BASE_UNREACHABLE();
                }();

                for (const auto bPos : findHorizontalDrillLeftBlocks())
                {
                    const auto startPos   = m_world.currentTetramino->position + bPos.toVec2i();
                    auto       endPosLeft = startPos.addX(-1);

                    endPosLeft.x = sf::base::max(endPosLeft.x, 0);

                    while (endPosLeft.x > 0)
                    {
                        if (!m_world.blockGrid.at(endPosLeft.toVec2uz()).hasValue())
                            break;

                        --endPosLeft.x;
                    }

                    const auto startDrawPos   = toDrawCoordinates(startPos.toVec2uz()) + drawOffset;
                    const auto endDrawPosLeft = toDrawCoordinates(endPosLeft.toVec2uz()).addX(32.f) + drawOffset;

                    const auto diffLeft = endDrawPosLeft - startDrawPos;

                    for (int i = 0; i < 32; ++i)
                    {
                        m_rtGame.draw(sf::CircleShapeData{
                            .position = sf::Vec2f{blend(startDrawPos,
                                                        startDrawPos + ((diffLeft / 32.f) * static_cast<float>(i + 1)),
                                                        easeInOutSine(bounce(progress)))}
                                            .addY(-8.f),
                            .scale            = sf::Vec2f{0.5f, 0.5f},
                            .origin           = {16.f, 16.f},
                            .rotation         = sf::degrees(90),
                            .fillColor        = blockPalette[paletteIdx],
                            .outlineColor     = blockPalette[paletteIdx].withLightness(0.3f),
                            .outlineThickness = 1.f,
                            .radius           = 32.f,
                            .pointCount       = 3u,
                        });
                    }
                }
            }

            // Draw horizontal drill (right)
            if (m_animationTimeline.isPlaying<AnimHorizontalDrillRight>())
            {
                const float progress = m_animationTimeline.getProgress();

                const auto paletteIdx = [&]
                {
                    for (const auto& b : m_world.currentTetramino->shape)
                        if (b.hasValue())
                            return b->paletteIdx;

                    SFML_BASE_UNREACHABLE();
                }();

                for (const auto bPos : findHorizontalDrillRightBlocks())
                {
                    const auto startPos    = m_world.currentTetramino->position + bPos.toVec2i();
                    auto       endPosRight = startPos.addX(1);

                    endPosRight.x = sf::base::min(endPosRight.x, static_cast<int>(m_world.blockGrid.getWidth() - 1));

                    while (endPosRight.x < static_cast<int>(m_world.blockGrid.getWidth()))
                    {
                        if (!m_world.blockGrid.at(endPosRight.toVec2uz()).hasValue())
                            break;

                        ++endPosRight.x;
                    }

                    const auto startDrawPos    = toDrawCoordinates(startPos.toVec2uz()) + drawOffset;
                    const auto endDrawPosRight = toDrawCoordinates(endPosRight.toVec2uz()).addX(-32.f) + drawOffset;

                    const auto diffRight = endDrawPosRight - startDrawPos;

                    for (int i = 0; i < 32; ++i)
                    {
                        m_rtGame.draw(sf::CircleShapeData{
                            .position = sf::Vec2f{blend(startDrawPos,
                                                        startDrawPos + ((diffRight / 32.f) * static_cast<float>(i + 1)),
                                                        easeInOutSine(bounce(progress)))}
                                            .addY(8.f),
                            .scale            = sf::Vec2f{0.5f, 0.5f},
                            .origin           = {16.f, 16.f},
                            .rotation         = sf::degrees(270),
                            .fillColor        = blockPalette[paletteIdx],
                            .outlineColor     = blockPalette[paletteIdx].withLightness(0.3f),
                            .outlineThickness = 1.f,
                            .radius           = 32.f,
                            .pointCount       = 3u,
                        });
                    }
                }
            }


            // Draw current piece
            if (m_world.currentTetramino.hasValue())
            {
                const Tetramino& tetramino = *m_world.currentTetramino;

                // Draw vertical drill arrow
                if (m_world.perkVerticalDrill.hasValue())
                    for (const auto bPos : findVerticalDrillBlocks())
                    {
                        const sf::base::Optional<Block>& blockOpt = tetramino.shape[bPos.y * shapeDimension + bPos.x];

                        const auto drawPosition = m_currentTetraminoVisualPosition +
                                                  toDrawCoordinates(bPos).addY(16.f) + drawOffset;

                        sf::CircleShape spike{{
                            .scale            = sf::Vec2f{0.5f, 0.25f},
                            .fillColor        = blockPalette[blockOpt->paletteIdx],
                            .outlineColor     = blockPalette[blockOpt->paletteIdx].withLightness(0.3f),
                            .outlineThickness = 2.f,
                            .radius           = 32.f,
                            .pointCount       = 3u,
                        }};

                        spike.setTopCenter(drawPosition);
                        m_rtGame.draw(spike);

                        const auto gridPosition      = (tetramino.position + bPos.toVec2i()).toVec2uz();
                        const auto gridGhostPosition = gridPosition.withY(
                            static_cast<sf::base::SizeT>(calculateGhostY(tetramino)) + bPos.y);

                        const auto ghostDrawPos = toDrawCoordinates(gridGhostPosition).withX(drawPosition.x - 32.f).addY(16.f) +
                                                  drawOffset;

                        spike.setTopCenter(ghostDrawPos);
                        spike.setFillColor(blockPalette[blockOpt->paletteIdx].withAlpha(64));
                        spike.setOutlineColor(blockPalette[blockOpt->paletteIdx].withLightness(0.3f).withAlpha(64));

                        m_rtGame.draw(spike);
                    }

                // Draw horizontal drill arrows (left)
                if (m_world.perkHorizontalDrillLeft.hasValue())
                    for (const auto& bPos : findHorizontalDrillLeftBlocks())
                    {
                        const sf::base::Optional<Block>& blockOpt = tetramino.shape[bPos.y * shapeDimension + bPos.x];

                        sf::CircleShape spike{{
                            .scale            = sf::Vec2f{0.5f, 0.25f},
                            .fillColor        = blockPalette[blockOpt->paletteIdx],
                            .outlineColor     = blockPalette[blockOpt->paletteIdx].withLightness(0.3f),
                            .outlineThickness = 2.f,
                            .radius           = 32.f,
                            .pointCount       = 3u,
                        }};

                        const auto drawPosLeft = m_currentTetraminoVisualPosition +
                                                 toDrawCoordinates(bPos).addX(-16.f) + drawOffset;

                        spike.rotation = sf::degrees(90);
                        spike.setCenterRight(drawPosLeft);
                        m_rtGame.draw(spike);

                        const auto gridPosition      = (tetramino.position + bPos.toVec2i()).toVec2uz();
                        const auto gridGhostPosition = gridPosition.withY(
                            static_cast<sf::base::SizeT>(calculateGhostY(tetramino)) + bPos.y);

                        const auto ghostDrawPosLeft = toDrawCoordinates(gridGhostPosition).withX(drawPosLeft.x - 32.f) +
                                                      drawOffset;

                        spike.setFillColor(blockPalette[blockOpt->paletteIdx].withAlpha(64));
                        spike.setOutlineColor(blockPalette[blockOpt->paletteIdx].withLightness(0.3f).withAlpha(64));

                        spike.rotation = sf::degrees(90);
                        spike.setCenterRight(ghostDrawPosLeft);
                        m_rtGame.draw(spike);
                    }

                // Draw horizontal drill arrows (right)
                if (m_world.perkHorizontalDrillRight.hasValue())
                    for (const auto& bPos : findHorizontalDrillRightBlocks())
                    {
                        const sf::base::Optional<Block>& blockOpt = tetramino.shape[bPos.y * shapeDimension + bPos.x];

                        sf::CircleShape spike{{
                            .scale            = sf::Vec2f{0.5f, 0.25f},
                            .fillColor        = blockPalette[blockOpt->paletteIdx],
                            .outlineColor     = blockPalette[blockOpt->paletteIdx].withLightness(0.3f),
                            .outlineThickness = 2.f,
                            .radius           = 32.f,
                            .pointCount       = 3u,
                        }};

                        const auto drawPosRight = m_currentTetraminoVisualPosition +
                                                  toDrawCoordinates(bPos).addX(16.f) + drawOffset;

                        spike.rotation = sf::degrees(270);
                        spike.setCenterLeft(drawPosRight);
                        m_rtGame.draw(spike);

                        const auto gridPosition      = (tetramino.position + bPos.toVec2i()).toVec2uz();
                        const auto gridGhostPosition = gridPosition.withY(
                            static_cast<sf::base::SizeT>(calculateGhostY(tetramino)) + bPos.y);

                        const auto ghostDrawPosRight = toDrawCoordinates(gridGhostPosition).withX(drawPosRight.x - 32.f) +
                                                       drawOffset;

                        spike.setFillColor(blockPalette[blockOpt->paletteIdx].withAlpha(64));
                        spike.setOutlineColor(blockPalette[blockOpt->paletteIdx].withLightness(0.3f).withAlpha(64));

                        spike.rotation = sf::degrees(270);
                        spike.setCenterLeft(ghostDrawPosRight);
                        m_rtGame.draw(spike);
                    }

                // Draw blocks
                for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
                    for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
                    {
                        const sf::base::Optional<Block>& blockOpt = tetramino.shape[y * shapeDimension + x];

                        if (!blockOpt.hasValue())
                            continue;

                        const auto gridPosition = (tetramino.position + sf::Vec2uz{x, y}.toVec2i()).toVec2uz();
                        const auto drawPosition = m_currentTetraminoVisualPosition + toDrawCoordinates(sf::Vec2uz{x, y});

                        drawBlock(*blockOpt,
                                  drawPosition,
                                  {
                                      .squishMult = m_squishMult,
                                  });

                        const auto gridGhostPosition = gridPosition.withY(
                            static_cast<sf::base::SizeT>(calculateGhostY(tetramino)) + y);

                        drawBlock(*blockOpt,
                                  toDrawCoordinates(gridGhostPosition).withX(drawPosition.x),
                                  {
                                      .opacity    = 0.25f,
                                      .squishMult = m_squishMult,
                                  });
                    }
            }


            // Draw next piece(s)
            {
                const sf::base::SizeT nPeek = sf::base::min(static_cast<sf::base::SizeT>(m_world.perkNPeek),
                                                            m_world.blockMatrixBag.size());

                for (sf::base::SizeT iPeek = 0u; iPeek < nPeek; ++iPeek)
                {
                    const auto& shape = m_world.blockMatrixBag[m_world.blockMatrixBag.size() - nPeek + iPeek].blockMatrix;

                    for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
                        for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
                        {
                            const sf::base::Optional<Block>& blockOpt = shape[y * shapeDimension + x];

                            if (!blockOpt.hasValue())
                                continue;

                            const auto drawPosition = sf::Vec2f{386.f, 700.f - static_cast<float>(iPeek) * 96.f} +
                                                      toDrawCoordinates(sf::Vec2uz{x, y});

                            drawBlock(*blockOpt,
                                      drawPosition,
                                      {
                                          .applyYOffset = false,
                                      });
                        }
                }
            }

            // Draw held piece
            if (m_world.heldTetramino.hasValue())
            {
                const auto& shape = m_world.heldTetramino->shape;

                for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
                    for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
                    {
                        const sf::base::Optional<Block>& blockOpt = shape[y * shapeDimension + x];

                        if (!blockOpt.hasValue())
                            continue;

                        const auto drawPosition = sf::Vec2f{606.f, 456.f} + toDrawCoordinates(sf::Vec2uz{x, y});

                        drawBlock(*blockOpt, drawPosition);
                    }
            }
        }

        for (auto& lb : m_lightningBolts)
            lb.draw(m_rtGame, {});

        m_rtGame.display();

        // m_window.setAutoBatchMode(sf::RenderTarget::AutoBatchMode::Disabled);
        m_window.clear();

        m_window.draw(m_rtGame.getTexture(),
                      {
                          .position = {0.f, m_quakeSinEffectHardDrop.getValue() + m_quakeSinEffectLineClear.getValue()},
                          .scale    = {1.f, 1.f},
                      });

        {
            const sf::base::SizeT nPeek = sf::base::min(static_cast<sf::base::SizeT>(m_world.perkNPeek),
                                                        m_world.blockMatrixBag.size());
            m_window.draw(m_font,
                          sf::TextData{
                              .position      = {386.f, 700.f - static_cast<float>(nPeek + 1) * 96.f + 32.f},
                              .string        = "Next:",
                              .characterSize = 26,
                              .outlineColor  = sf::Color::White,
                          });
        }

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

        m_window.draw(m_font,
                      sf::TextData{
                          .position      = {400.f, 32.f},
                          .string        = stats,
                          .characterSize = 26,
                          .outlineColor  = sf::Color::White,
                      });

        // Draw earned XP
        {
            const auto bezier = [](const sf::Vec2f& start, const sf::Vec2f& end, const float t)
            {
                const sf::Vec2f control(start.x, end.y);
                const float     u = 1.f - t;

                return u * u * start + 2.f * u * t * control + t * t * end;
            };

            for (const auto& particle : m_earnedXPParticles)
            {
                const auto newPos = bezier(particle.startPosition,
                                           particle.targetPosition,
                                           easeInOutSine(particle.progress.value));

                const auto newPos2 = bezier(particle.startPosition,
                                            particle.targetPosition,
                                            easeInOutBack(particle.progress.value));

                // const float opacityScale = sf::base::clamp(particle.progress.value, 0.f, 0.15f) / 0.15f;
                const auto alpha = static_cast<sf::base::U8>(
                    (particle.progress.remapBouncedEased(easeInOutQuint, 64.f, 255.f)));

                m_window.draw(sf::CircleShapeData{
                    .position = {blend(newPos2.x, newPos.x, 0.5f), newPos.y},
                    .scale    = sf::Vec2f{0.25f, 0.25f} * particle.progress.remapBounced(0.5f, 2.f),
                    .origin   = {12.f, 12.f},
                    .rotation = sf::radians(
                        sf::base::fmod(particle.startRotation + particle.progress.remap(0.f, sf::base::tau * 2.f),
                                       sf::base::tau)),
                    .fillColor        = blockPalette[particle.paletteIdx].withAlpha(alpha),
                    .outlineColor     = blockPalette[particle.paletteIdx].withAlpha(alpha).withLightness(0.3f),
                    .outlineThickness = 1.f,
                    .radius           = 24.f,
                    .pointCount       = 3u,
                });
            }
        }

        m_imGuiContext.render(m_window);


        m_window.display();
    }


public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        while (true)
        {
            //
            // Event handling
            if (!eventStep())
                return false;

            //
            // Update step
            const auto deltaTime = m_tickClock.restart();

            if (isInPlayableState())
                m_timeAccumulator += deltaTime.asSeconds();

            updateStep(deltaTime, ticksPerSecond);

            //
            // ImGui step
            imguiStep(deltaTime);

            //
            // Draw step
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

    tsurv::Game game;

    if (!game.run())
        return 1;

    return 0;
}


// TODO:
// - combo system for line clears
// - random powerups
// - bombs?
