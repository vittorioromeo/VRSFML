#include "../bubble_idle/ControlFlow.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"    // TODO P1: avoid the relative path...?
#include "../bubble_idle/MathUtils.hpp"   // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp"     // TODO P1: avoid the relative path...?
#include "../bubble_idle/Timer.hpp"       // TODO P1: avoid the relative path...?

#include "SFML/Graphics/CircleShapeData.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Count.hpp"
#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/FixedFunction.hpp"
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
    sf::ImGuiContext m_imGuiContext;
    ImFont* const    m_imguiFont{ImGui::GetIO().Fonts->AddFontFromFileTTF("resources/Born2bSportyFS.ttf", 32.f)};

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
    static inline constexpr auto drawOffset    = sf::Vec2f{32.f, 32.f};
    static inline constexpr auto drawBlockSize = sf::Vec2f{32.f, 32.f};


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] sf::Vec2f toDrawCoordinates(const sf::Vec2<T>& position) const noexcept
    {
        return sf::Vec2f{static_cast<float>(position.x), static_cast<float>(position.y)}.componentWiseMul(drawBlockSize);
    }


    ////////////////////////////////////////////////////////////
    void drawBlock(const Block&     block,
                   const sf::Vec2f& position,
                   const float      opacity,
                   const float      squishMult = 0.f,
                   const float      rotation   = 0.f,
                   const float      scale      = 1.f,
                   const bool       drawText   = true)
    {
        float yOffset = 0.f;

        if (m_animationTimeline.isPlaying<AnimCollapseGrid>() && !m_rowYOffsets.empty())
            yOffset = m_rowYOffsets[static_cast<sf::base::SizeT>(position.y / drawBlockSize.y)] *
                      easeInBack(m_animationTimeline.getProgress());

        const auto alpha = static_cast<sf::base::U8>(opacity * 255.f);

        float finalSquishMult = 1.f + squishMult;
        if (const auto* it = m_blockEffects.find(block.blockId); it != m_blockEffects.end())
        {
            const BlockEffect& effect = it->second;

            const float progress = effect.squishTime / BlockEffect::squishDuration;
            finalSquishMult += easeInOutSine(bounce(progress)) * 0.5f;
        }

        m_rtGame.draw(sf::RectangleShapeData{
            .position         = drawOffset + position.addY(yOffset),
            .scale            = sf::Vec2f{finalSquishMult, finalSquishMult} * scale,
            .origin           = drawBlockSize / 2.f,
            .rotation         = sf::degrees(rotation),
            .fillColor        = blockPalette[block.paletteIdx].withAlpha(alpha),
            .outlineColor     = blockPalette[block.paletteIdx].withAlpha(alpha).withLightness(0.3f),
            .outlineThickness = 2.f,
            .size             = drawBlockSize,
        });

        if (block.health > 1u && drawText)
        {
            sf::Text text{m_font,
                          {
                              .scale            = sf::Vec2f{finalSquishMult, finalSquishMult} * scale,
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
            rotateTetramino(*m_world.currentTetramino, clockwise);
    }


    ////////////////////////////////////////////////////////////
    void skipCurrentTetramino()
    {
        if (m_world.holdUsedThisTurn)
            return;

        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

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

        const auto temp       = m_world.heldTetramino;
        m_world.heldTetramino = m_world.currentTetramino;

        if (temp.hasValue())
            m_world.currentTetramino = temp;
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

        for (sf::base::SizeT y = 0u; y < m_world.blockGrid.getHeight(); ++y)
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
    sf::base::InPlaceVector<sf::Vec2uz, shapeDimension> findHorizontalDrillBlocks() const
    {
        return findHorizontalBlocks(m_world.currentTetramino->shape,
                                    static_cast<sf::base::SizeT>(m_world.perkHorizontalDrill->maxBlocks));
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
    sf::base::Vector<BlockAndPosition> findHorizontalDrillTargetBlocks()
    {
        sf::base::Vector<BlockAndPosition> result;

        auto nToHitLeft  = static_cast<sf::base::SizeT>(m_world.perkHorizontalDrill->maxPenetration);
        auto nToHitRight = nToHitLeft;

        for (const auto bPos : findHorizontalDrillBlocks())
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

        m_blockEffects[block.blockId] = BlockEffect{};

        if (m_world.perkXPPerBlockDamaged > 0)
        {
            addXP(static_cast<sf::base::U64>(m_world.perkXPPerBlockDamaged));
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
    void updateStep(const sf::Time deltaTime, const float xTicksPerSecond)
    {
        SFEX_PROFILE_SCOPE("update");

        // Show level up screen if needed
        if (m_world.committedPlayerLevel < m_world.playerLevel && !m_inLevelUpScreen)
        {
            m_inLevelUpScreen = true;
            rerollPerks();
        }

        // Interpolate visual position
        if (m_world.currentTetramino.hasValue())
        {
            const sf::Vec2f targetPosition = toDrawCoordinates(m_world.currentTetramino->position);

            const float interpolationSpeed = 15.f;

            m_currentTetraminoVisualPosition += (targetPosition - m_currentTetraminoVisualPosition) *
                                                sf::base::clamp(interpolationSpeed * deltaTime.asSeconds(), 0.f, 1.f);
        }

        // Deal with animations
        if (m_animationTimeline.anyAnimationPlaying())
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
                }
            }
            else if (auto* squish = currentCmd.getIf<AnimSquish>())
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
                    else if (m_world.perkHorizontalDrill.hasValue() && !findHorizontalDrillTargetBlocks().empty())
                    {
                        m_animationTimeline.add(AnimHorizontalDrill{
                            .duration = 0.3f,
                        });
                    }
                    else
                    {
                        embedCurrentTetraminoAndClearLines();
                    }

                    m_animationTimeline.popFrontCommand();
                }
            }
            else if (auto* clearLines = currentCmd.getIf<AnimClearLines>())
            {
                sf::base::Vector<sf::base::SizeT>             trulyClearedRows;
                sf::base::Vector<AnimFadeBlocks::FadingBlock> fadingBlocks;

                for (sf::base::SizeT y : clearLines->rows)
                {
                    bool rowIsFullyCleared = true;

                    for (sf::base::SizeT x = 0u; x < m_world.blockGrid.getWidth(); ++x)
                    {
                        if (auto& blockOpt = m_world.blockGrid.at(sf::Vec2uz{x, y}); blockOpt.hasValue())
                        {
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

                                blockOpt.reset();
                            }
                        }
                    }

                    if (rowIsFullyCleared)
                        trulyClearedRows.pushBack(y);
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

                            SFML_BASE_ASSERT(numCleared == 4);
                            return 60u;
                        }();

                        addXP(amount);

                        for (sf::base::U64 i = 0u; i < fadingBlocks.size() * 4u; ++i)
                        {
                            const auto& block = fadingBlocks[i % fadingBlocks.size()];

                            const auto startPosition = block.position + drawBlockSize / 2.f +
                                                       m_rngFast.getVec2f({-16.f, -16.f}, {16.f, 16.f});

                            spawnXPEarnedParticle(startPosition, block.block.paletteIdx);
                        }

                        // Random block hit perk
                        {
                            const sf::base::SizeT nBlocksToHit = static_cast<sf::base::SizeT>(m_world.perkRandomBlockHit) *
                                                                 numCleared;

                            struct EligibleBlock // NOLINT(cppcoreguidelines-pro-type-member-init)
                            {
                                Block*     block;
                                sf::Vec2uz position;
                            };

                            sf::base::Vector<EligibleBlock> eligibleBlocks;
                            m_world.blockGrid.forBlocks([&](Block& block, const sf::Vec2uz position)
                            {
                                if (block.health > 1u)
                                    eligibleBlocks.pushBack({&block, position});

                                return ControlFlow::Continue;
                            });

                            auto actualNBlocksToHit = sf::base::min(nBlocksToHit, eligibleBlocks.size());

                            while (actualNBlocksToHit > 0u)
                            {
                                const auto randomIndex = m_rngFast.getI<sf::base::SizeT>(0u, eligibleBlocks.size() - 1u);

                                damageBlock(eligibleBlocks[randomIndex].position, *eligibleBlocks[randomIndex].block);
                                eligibleBlocks.erase(eligibleBlocks.begin() + randomIndex);

                                --actualNBlocksToHit;
                            }
                        }
                    }

                    sf::base::quickSort(trulyClearedRows.begin(),
                                        trulyClearedRows.end(),
                                        [](const sf::base::SizeT a, const sf::base::SizeT b) { return a < b; });

                    m_animationTimeline.add(AnimCollapseGrid{
                        .clearedRows = trulyClearedRows,
                        .duration    = 0.1f,
                    });
                }

                m_animationTimeline.popFrontCommand();
            }
            else if (auto* fadeBlocks = currentCmd.getIf<AnimFadeBlocks>())
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

                const auto height = m_world.blockGrid.getHeight();

                m_rowYOffsets.resize(height);

                for (sf::base::SizeT y = 0u; y < height; ++y)
                    m_rowYOffsets[y] = 0.f;

                int clearedCount = 0;

                // iterate in reverse
                for (int y = static_cast<int>(height) - 1; y >= 0; --y)
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
            else if (auto* vDrill = currentCmd.getIf<AnimVerticalDrill>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    m_animationTimeline.popFrontCommand();

                    for (auto [blockPtr, position] : findVerticalDrillTargetBlocks())
                        damageBlock(position, *blockPtr);

                    embedCurrentTetraminoAndClearLines();
                }
            }
            else if (auto* hDrill = currentCmd.getIf<AnimHorizontalDrill>())
            {
                if (m_animationTimeline.getProgress() >= 1.f)
                {
                    m_animationTimeline.popFrontCommand();

                    for (auto [blockPtr, position] : findHorizontalDrillTargetBlocks())
                        damageBlock(position, *blockPtr);

                    embedCurrentTetraminoAndClearLines();
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
                    };

                    auto& [blockMatrix, tetraminoType] = m_world.blockMatrixBag.pushBack({
                        .blockMatrix   = shapeMatrixToBlockMatrix(srsTetraminoShapes[static_cast<sf::base::SizeT>(
                                                                    j) /* pieceType */][0],
                                                                block),
                        .tetraminoType = j,
                    });

                    const auto healthDist = generateTetrominoHealthDistribution(getDifficultyFactor(m_world.tick), m_rngFast);
                    sf::base::SizeT nextHealthDistIdx = 0u;

                    for (sf::base::Optional<Block>& b : blockMatrix)
                    {
                        if (!b.hasValue())
                            continue;

                        b->blockId = m_world.nextBlockId++;
                        b->health  = static_cast<sf::base::U8>(healthDist[nextHealthDistIdx++]);
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
                                embedCurrentTetraminoAndClearLines();
                        }
                    }
                }
            }
        }
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
            .fnName = [&] { return makeTitle("Random Block Hit", m_world.perkRandomBlockHit, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Each time you clear a line, randomly damage " +
                           std::to_string(m_world.perkRandomBlockHit + 1) + " additional block(s)."};
    },
            .fnPrerequisites = [&] { return true; },
            .fnApply         = [&] { ++m_world.perkRandomBlockHit; },
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
            .fnDescription = [&] { return "The entire surface of the tetromino acts as a drill when hard-dropping."; },
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
            .fnName = [&] { return makeTitle("XP per Tetramino Placed", m_world.perkXPPerTetraminoPlaced, 2); },
            .fnDescription =
                [&]
    {
        return std::string{"Gain "} + std::to_string(m_world.perkXPPerTetraminoPlaced + 2) +
               " XP for each tetramino you place.";
    },
            .fnPrerequisites = [&] { return true; },
            .fnApply         = [&] { m_world.perkXPPerTetraminoPlaced += 2; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("XP per Block Damaged", m_world.perkXPPerBlockDamaged, 8); },
            .fnDescription =
                [&]
    {
        return std::string{"Gain "} + std::to_string(m_world.perkXPPerBlockDamaged + 8) +
               " XP for each block you damage.";
    },
            .fnPrerequisites = [&] { return true; },
            .fnApply         = [&] { m_world.perkXPPerBlockDamaged += 8; },
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
            .fnName          = [&] { return "Horizontal Drill"; },
            .fnDescription   = [&] { return "Damage all blocks directly adjacent horizontally to the tetramino."; },
            .fnPrerequisites = [&] { return !m_world.perkHorizontalDrill.hasValue(); },
            .fnApply         = [&] { m_world.perkHorizontalDrill.emplace(); },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&]
    { return makeTitle("Horizontal Drill - Penetration", m_world.perkHorizontalDrill->maxPenetration, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum number of blocks damaged to "} +
               std::to_string(m_world.perkHorizontalDrill->maxPenetration + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkHorizontalDrill.hasValue() && m_world.perkHorizontalDrill->maxPenetration < 4; },
            .fnApply = [&] { ++m_world.perkHorizontalDrill->maxPenetration; },
        },

        /////////////////////////////////////////////////////////////
        PerkSelector{
            .fnName = [&] { return makeTitle("Horizontal Drill - Length", m_world.perkHorizontalDrill->maxBlocks, 1); },
            .fnDescription =
                [&]
    {
        return std::string{"Increase the maximum length of the drill to "} +
               std::to_string(m_world.perkHorizontalDrill->maxBlocks + 1) + ".";
    },
            .fnPrerequisites = [&]
    { return m_world.perkHorizontalDrill.hasValue() && m_world.perkHorizontalDrill->maxBlocks < 4; },
            .fnApply = [&] { ++m_world.perkHorizontalDrill->maxBlocks; },
        },
    };


    /////////////////////////////////////////////////////////////
    void imguiStep(const sf::Time deltaTime)
    {
        SFEX_PROFILE_SCOPE("imgui");
        m_imGuiContext.update(m_window, deltaTime);


        if (m_inLevelUpScreen)
        {
            SFML_BASE_ASSERT(m_world.committedPlayerLevel < m_world.playerLevel);

            ImGui::SetNextWindowBgAlpha(0.75f);

            ImGui::PushFont(m_imguiFont);

            ImGui::Begin("Level Up!", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

            ImGui::SetWindowPos(ImVec2{resolution.x / 2.f - 300.f, resolution.y / 2.f - 150.f});
            ImGui::SetWindowSize(ImVec2{600.f, 300.f});

            ImGui::Text("Choose a perk:");

            static int selectedPerk = -1;

            for (const sf::base::SizeT psIndex : m_perkIndicesSelectedThisLevel)
            {
                PerkSelector& perkSelector = m_perkSelectors[psIndex];

                std::string perkName        = perkSelector.fnName();
                std::string perkDescription = perkSelector.fnDescription();

                if (ImGui::Selectable(perkName.c_str(), selectedPerk == static_cast<int>(psIndex)))
                    selectedPerk = static_cast<int>(psIndex);

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", perkDescription.c_str());
            }

            if (selectedPerk != -1)
            {
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
                for (sf::base::SizeT y = 0u; y < m_world.blockGrid.getHeight(); ++y)
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
                for (sf::base::SizeT y = 0u; y < m_world.blockGrid.getHeight(); ++y)
                {
                    const sf::Vec2uz                 gridPosition{x, y};
                    const sf::base::Optional<Block>& blockOpt = m_world.blockGrid.at(gridPosition);

                    if (!blockOpt.hasValue())
                        continue;

                    drawBlock(*blockOpt, toDrawCoordinates(gridPosition), /* opacity */ 1.f);
                }

            // Draw fading blocks
            if (auto* fadeBlocks = m_animationTimeline.getIfPlaying<AnimFadeBlocks>())
            {
                const float progress = m_animationTimeline.getProgress();

                for (const auto& fadingBlock : fadeBlocks->fadingBlocks)
                {
                    drawBlock(fadingBlock.block,
                              fadingBlock.position,
                              1.f - easeInOutSine(progress),
                              /* squishMult */ 0.f);
                }
            }

            // Draw vertical drill
            if (auto* vDrill = m_animationTimeline.getIfPlaying<AnimVerticalDrill>())
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
                                .position         = {blendVec(startDrawPos,
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

            // Draw horizontal drill
            if (auto* hDrill = m_animationTimeline.getIfPlaying<AnimHorizontalDrill>())
            {
                const float progress = m_animationTimeline.getProgress();

                const auto paletteIdx = [&]
                {
                    for (const auto& b : m_world.currentTetramino->shape)
                        if (b.hasValue())
                            return b->paletteIdx;

                    SFML_BASE_UNREACHABLE();
                }();

                for (const auto bPos : findHorizontalDrillBlocks())
                {
                    const auto startPos    = m_world.currentTetramino->position + bPos.toVec2i();
                    auto       endPosLeft  = startPos.addX(-1);
                    auto       endPosRight = startPos.addX(1);

                    endPosLeft.x  = sf::base::max(endPosLeft.x, 0);
                    endPosRight.x = sf::base::min(endPosRight.x, static_cast<int>(m_world.blockGrid.getWidth() - 1));

                    while (endPosLeft.x > 0)
                    {
                        if (!m_world.blockGrid.at(endPosLeft.toVec2uz()).hasValue())
                            break;

                        --endPosLeft.x;
                    }

                    while (endPosRight.x < static_cast<int>(m_world.blockGrid.getWidth()))
                    {
                        if (!m_world.blockGrid.at(endPosRight.toVec2uz()).hasValue())
                            break;

                        ++endPosRight.x;
                    }

                    const auto startDrawPos    = toDrawCoordinates(startPos.toVec2uz()) + drawOffset;
                    const auto endDrawPosLeft  = toDrawCoordinates(endPosLeft.toVec2uz()).addX(32.f) + drawOffset;
                    const auto endDrawPosRight = toDrawCoordinates(endPosRight.toVec2uz()).addX(-32.f) + drawOffset;

                    const auto diffLeft  = endDrawPosLeft - startDrawPos;
                    const auto diffRight = endDrawPosRight - startDrawPos;

                    for (int i = 0; i < 32; ++i)
                    {
                        m_rtGame.draw(sf::CircleShapeData{
                            .position = sf::Vec2f{blendVec(startDrawPos,
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

                        m_rtGame.draw(sf::CircleShapeData{
                            .position = sf::Vec2f{blendVec(startDrawPos,
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

                // Draw drill arrow
                if (m_world.perkVerticalDrill.hasValue())
                    for (const auto bPos : findVerticalDrillBlocks())
                    {
                        const sf::base::Optional<Block>& blockOpt = tetramino.shape[bPos.y * shapeDimension + bPos.x];

                        drawBlock(*blockOpt,
                                  m_currentTetraminoVisualPosition + toDrawCoordinates(bPos).addY(16.f),
                                  /* opacity */ 1.f,
                                  m_squishMult,
                                  /* rotation */ 45.f,
                                  /* scale */ 0.75f,
                                  /* drawText */ false);
                    }

                // Draw horizontal drill arrows
                if (m_world.perkHorizontalDrill.hasValue())
                    for (const auto& bPos : findHorizontalDrillBlocks())
                    {
                        const sf::base::Optional<Block>& blockOpt = tetramino.shape[bPos.y * shapeDimension + bPos.x];

                        drawBlock(*blockOpt,
                                  m_currentTetraminoVisualPosition + toDrawCoordinates(bPos).addX(16.f),
                                  /* opacity */ 1.f,
                                  m_squishMult,
                                  /* rotation */ 45.f,
                                  /* scale */ 0.75f,
                                  /* drawText */ false);

                        drawBlock(*blockOpt,
                                  m_currentTetraminoVisualPosition + toDrawCoordinates(bPos).addX(-16.f),
                                  /* opacity */ 1.f,
                                  m_squishMult,
                                  /* rotation */ 45.f,
                                  /* scale */ 0.75f,
                                  /* drawText */ false);
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

                        drawBlock(*blockOpt, drawPosition, /* opacity */ 1.f, m_squishMult);

                        const auto gridGhostPosition = gridPosition.withY(
                            static_cast<sf::base::SizeT>(calculateGhostY(tetramino)) + y);

                        drawBlock(*blockOpt,
                                  toDrawCoordinates(gridGhostPosition).withX(drawPosition.x),
                                  /* opacity */ 0.25f,
                                  m_squishMult);
                    }
            }

            const sf::base::SizeT nPeek = 2u;

            // Draw next piece(s) (TODO: make perk?)
            if (m_world.blockMatrixBag.size() >= nPeek) // TODO: adjust for peek
            {
                for (sf::base::SizeT iPeek = 0u; iPeek < nPeek; ++iPeek)
                {
                    const auto& shape = m_world.blockMatrixBag[m_world.blockMatrixBag.size() - nPeek + iPeek].blockMatrix;

                    for (sf::base::SizeT x = 0u; x < shapeDimension; ++x)
                        for (sf::base::SizeT y = 0u; y < shapeDimension; ++y)
                        {
                            const sf::base::Optional<Block>& blockOpt = shape[y * shapeDimension + x];

                            if (!blockOpt.hasValue())
                                continue;

                            const auto drawPosition = sf::Vec2f{386.f, 456.f - static_cast<float>(iPeek) * 96.f} +
                                                      toDrawCoordinates(sf::Vec2uz{x, y});

                            drawBlock(*blockOpt, drawPosition, /* opacity */ 1.f, 0.f);
                        }
                }
            }

            // Draw held piece (TODO: make perk?)
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

                        drawBlock(*blockOpt, drawPosition, /* opacity */ 1.f, 0.f);
                    }
            }
        }

        m_rtGame.display();

        // m_window.setAutoBatchMode(sf::RenderTarget::AutoBatchMode::Disabled);
        m_window.clear();
        m_window.draw(m_rtGame.getTexture(), {.scale = {1.f, 1.f}});

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

        if (m_world.perkRandomBlockHit > 0)
        {
            stats += "- Random Block Hit (";
            stats += std::to_string(m_world.perkRandomBlockHit);
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

        if (m_world.perkExtraLinePiecesInPool > 0)
        {
            stats += "- Extra Line Pieces in Pool (";
            stats += std::to_string(m_world.perkExtraLinePiecesInPool);
            stats += "x)\n";
        }

        if (m_world.perkHorizontalDrill.hasValue())
        {
            stats += "- Horizontal Drill (Penetration: ";
            stats += std::to_string(m_world.perkHorizontalDrill->maxPenetration);
            stats += "x; Length: ";
            stats += std::to_string(m_world.perkHorizontalDrill->maxBlocks);
            stats += ")\n";
        }

        m_window.draw(m_font,
                      sf::TextData{
                          .position      = {400.f, 32.f},
                          .string        = stats,
                          .characterSize = 32,
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
