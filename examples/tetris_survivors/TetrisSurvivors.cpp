#include "../bubble_idle/ControlFlow.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"    // TODO P1: avoid the relative path...?
#include "../bubble_idle/MathUtils.hpp"   // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp"     // TODO P1: avoid the relative path...?
#include "BlockMatrix.hpp"
#include "Tetramino.hpp"

#include <iostream>

#define SFEX_PROFILER_ENABLED
#include "Profiler.hpp"
#include "ProfilerImGui.hpp"

//
#include "Block.hpp"
#include "BlockGrid.hpp"
#include "RandomBag.hpp"
#include "ShapeMatrix.hpp"
#include "TetraminoShapes.hpp"

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Glsl.hpp"
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

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/Algorithm/Unique.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/Math/Ceil.hpp"
#include "SFML/Base/Math/Fmod.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Sort.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <initializer_list>


namespace tsurv
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1024.f, 768.f};


////////////////////////////////////////////////////////////
struct World
{
    BlockGrid                     blockGrid{10, 20};
    sf::base::Optional<Tetramino> currentTetramino;
    sf::base::Vector<BlockMatrix> blockMatrixBag;
    sf::base::SizeT               tick = 0u;
};


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

    ////////////////////////////////////////////////////////////
    sf::Clock m_tickClock;
    float     m_timeAccumulator = 0.f;

    ////////////////////////////////////////////////////////////
    World m_world;

    ////////////////////////////////////////////////////////////
    RNGFast m_rngFast; // very fast, low-quality, but good enough for VFXs

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
    [[nodiscard]] int calculateGhostY()
    {
        SFML_BASE_ASSERT(m_world.currentTetramino.hasValue());

        int ghostY = m_world.currentTetramino->position.y;
        while (m_world.blockGrid.isValidMove(m_world.currentTetramino->shape,
                                             {m_world.currentTetramino->position.x, ghostY + 1}))
            ++ghostY;

        return ghostY;
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        while (true)
        {
            ////////////////////////////////////////////////////////////
            // Event handling
            ////////////////////////////////////////////////////////////
            // ---
            while (sf::base::Optional event = m_window.pollEvent())
            {
                m_imGuiContext.processEvent(m_window, *event);

                if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                    return true;

                if (handleAspectRatioAwareResize(*event, resolution, m_window))
                    continue;

                if (auto* eKeyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    const auto moveCurrentTetraminoBy = [&](const sf::Vec2i delta)
                    {
                        if (!m_world.currentTetramino.hasValue())
                            return;

                        const auto newPosition = m_world.currentTetramino->position + delta;

                        if (m_world.blockGrid.isValidMove(m_world.currentTetramino->shape, newPosition))
                            m_world.currentTetramino->position = newPosition;
                    };

                    const auto rotateCurrentTetramino = [&](const bool clockwise)
                    {
                        if (!m_world.currentTetramino.hasValue())
                            return;

                        const auto& originalShape = m_world.currentTetramino->shape;

                        const BlockMatrix rotatedShape = clockwise ? rotateBlockMatrixClockwise(originalShape)
                                                                   : rotateBlockMatrixCounterClockwise(originalShape);

                        const auto testPosition = m_world.currentTetramino->position;

                        if (m_world.blockGrid.isValidMove(rotatedShape, testPosition))
                        {
                            m_world.currentTetramino->shape = rotatedShape;
                        }
                        else if (m_world.blockGrid.isValidMove(rotatedShape, testPosition.addX(1)))
                        {
                            m_world.currentTetramino->shape = rotatedShape;
                            ++m_world.currentTetramino->position.x;
                        }
                        else if (m_world.blockGrid.isValidMove(rotatedShape, testPosition.addX(-1)))
                        {
                            m_world.currentTetramino->shape = rotatedShape;
                            --m_world.currentTetramino->position.x;
                        }
                    };

                    if (eKeyPressed->code == sf::Keyboard::Key::Right)
                    {
                        moveCurrentTetraminoBy({1, 0});
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::Down)
                    {
                        moveCurrentTetraminoBy({0, 1});
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::Left)
                    {
                        moveCurrentTetraminoBy({-1, 0});
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::Up)
                    {
                        rotateCurrentTetramino(/* clockwise */ true);
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::Space)
                    {
                        const int endY = calculateGhostY();

                        m_world.currentTetramino->position.y = endY;

                        m_world.blockGrid.embedTetramino(*m_world.currentTetramino);
                        m_world.currentTetramino.reset();

                        continue;
                    }
                }
            }
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Update step
            ////////////////////////////////////////////////////////////
            // ---
            const auto      deltaTime      = m_tickClock.restart();
            constexpr float ticksPerSecond = 240.f;
            m_timeAccumulator += deltaTime.asSeconds();

            {
                SFEX_PROFILE_SCOPE("update");

                const auto shapeMatrixToBlockMatrix = [](const ShapeMatrix& shapeMatrix, const Block& block) -> BlockMatrix
                {
                    BlockMatrix blockMatrix;

                    for (sf::base::SizeT y = 0; y < shapeDimension; ++y)
                        for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
                            if (shapeMatrix[y * shapeDimension + x])
                                blockMatrix[y * shapeDimension + x].emplace(block);

                    return blockMatrix;
                };

                // Refill block matrix bag if needed
                if (m_world.blockMatrixBag.empty())
                {
                    constexpr sf::base::SizeT bagMult = 2u;

                    sf::base::Vector<ShapeMatrix> sourceShapes;
                    sourceShapes.reserve(tetraminoShapes.size() * bagMult);

                    for (sf::base::SizeT i = 0u; i < bagMult; ++i)
                        for (const auto& shape : tetraminoShapes)
                            sourceShapes.pushBack(shape);

                    sf::base::SizeT paletteIdx = 0u;

                    for (const auto& shape : sourceShapes)
                    {
                        const Block block{
                            .health     = 1,
                            .paletteIdx = paletteIdx++ % blockPalette.size(),
                        };

                        m_world.blockMatrixBag.pushBack(shapeMatrixToBlockMatrix(shape, block));
                    }
                }

                // Pick next tetramino if there is none
                if (!m_world.currentTetramino.hasValue())
                {
                    const auto blockMatrix = drawFromBag(m_world.blockMatrixBag);

                    m_world.currentTetramino.emplace(Tetramino{
                        .shape = blockMatrix,
                        .position = sf::Vec2i{static_cast<int>(m_world.blockGrid.getWidth() / 2u - shapeDimension / 2u), 0u},
                    });
                }

                for (unsigned int i = 0; i < static_cast<unsigned int>(m_timeAccumulator * ticksPerSecond); ++i)
                {
                    ++m_world.tick;
                    m_timeAccumulator -= 1.f / ticksPerSecond;

                    if (m_world.tick % 60 == 0)
                    {
                        if (m_world.currentTetramino.hasValue())
                        {
                            const auto newPosition = m_world.currentTetramino->position + sf::Vec2u{0u, 1u};

                            if (m_world.blockGrid.isValidMove(m_world.currentTetramino->shape, newPosition))
                            {
                                m_world.currentTetramino->position = newPosition;
                            }
                            else
                            {
                                m_world.blockGrid.embedTetramino(*m_world.currentTetramino);
                                m_world.currentTetramino.reset();
                            }
                        }
                    }
                }
            }
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // ImGui step
            ////////////////////////////////////////////////////////////
            // ---
            {
                SFEX_PROFILE_SCOPE("imgui");
                m_imGuiContext.update(m_window, deltaTime);
            }
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Draw step
            ////////////////////////////////////////////////////////////
            // ---
            {
                SFEX_PROFILE_SCOPE("draw");

                m_rtGame.clear();

                {
                    SFEX_PROFILE_SCOPE("rtGame");

                    // Draw grid background and lines
                    for (sf::base::SizeT x = 0; x < m_world.blockGrid.getWidth(); ++x)
                        for (sf::base::SizeT y = 0; y < m_world.blockGrid.getHeight(); ++y)
                        {
                            const sf::Vec2f position = {static_cast<float>(x * 32), static_cast<float>(y * 32)};

                            m_rtGame.draw(sf::RectangleShapeData{
                                .position         = position,
                                .fillColor        = sf::Color(30, 30, 30),
                                .outlineColor     = sf::Color(50, 50, 50),
                                .outlineThickness = 2.f,
                                .size             = {32.f, 32.f},
                            });
                        }

                    // Draw pieces embedded in grid
                    for (sf::base::SizeT x = 0; x < m_world.blockGrid.getWidth(); ++x)
                        for (sf::base::SizeT y = 0; y < m_world.blockGrid.getHeight(); ++y)
                        {
                            const sf::base::Optional<Block>& blockOpt = m_world.blockGrid.at(sf::Vec2uz{x, y}.toVec2u());

                            if (blockOpt.hasValue())
                            {
                                const Block& block = *blockOpt;

                                const sf::Vec2f position = {static_cast<float>(x * 32), static_cast<float>(y * 32)};

                                m_rtGame.draw(sf::RectangleShapeData{
                                    .position         = position,
                                    .fillColor        = blockPalette[block.paletteIdx],
                                    .outlineColor     = blockPalette[block.paletteIdx].withLightness(0.3f),
                                    .outlineThickness = 2.f,
                                    .size             = {32.f, 32.f},
                                });
                            }
                        }

                    // Draw current piece
                    if (m_world.currentTetramino.hasValue())
                    {
                        const Tetramino& tetramino = *m_world.currentTetramino;

                        for (sf::base::SizeT y = 0; y < shapeDimension; ++y)
                            for (sf::base::SizeT x = 0; x < shapeDimension; ++x)
                            {
                                const sf::base::Optional<Block>& blockOpt = tetramino.shape[y * shapeDimension + x];

                                if (blockOpt.hasValue())
                                {
                                    const Block& block = *blockOpt;

                                    const auto position = (tetramino.position + sf::Vec2uz{x, y}.toVec2i()).toVec2f() * 32.f;

                                    m_rtGame.draw(sf::RectangleShapeData{
                                        .position         = position,
                                        .fillColor        = blockPalette[block.paletteIdx],
                                        .outlineColor     = blockPalette[block.paletteIdx].withLightness(0.3f),
                                        .outlineThickness = 2.f,
                                        .size             = {32.f, 32.f},
                                    });
                                }
                            }
                    }
                }

                m_rtGame.display();

                m_window.clear();
                m_window.draw(m_rtGame.getTexture(), {.scale = {1.f, 1.f}});

                m_imGuiContext.render(m_window);
            }

            m_window.display();
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
