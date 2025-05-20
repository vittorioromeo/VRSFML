#include "../bubble_idle/Easing.hpp"  // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/CircleShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/EllipseShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/PieSliceShapeData.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RingPieSliceShapeData.hpp"
#include "SFML/Graphics/RingShapeData.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StarShapeData.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#include <iostream>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <initializer_list>


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1016.f, 1016.f};


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] constexpr sf::base::SizeT convert2DIndexTo1D(
    const sf::base::SizeT x,
    const sf::base::SizeT y,
    const sf::base::SizeT width)
{
    SFML_BASE_ASSERT(width > 0);
    SFML_BASE_ASSERT(x < width);

    return x + y * width;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr float bounce(const float value) noexcept
{
    return 1.f - sf::base::fabs(value - 0.5f) * 2.f;
}


////////////////////////////////////////////////////////////
constexpr sf::base::SizeT worldSizeX = 16u;
constexpr sf::base::SizeT worldSizeY = 16u;


////////////////////////////////////////////////////////////
enum class BlockType : sf::base::U8
{
    Wall,
    Red,
    Blue,
    Green,
};


////////////////////////////////////////////////////////////
enum class GravityType : sf::base::U8
{
    None,
    Down,
    Up,
    Left,
    Right,
};


////////////////////////////////////////////////////////////
struct Block
{
    sf::Vec2i   position;
    BlockType   type;
    GravityType gravityType;
};


////////////////////////////////////////////////////////////
using BlockId = sf::base::SizeT;

////////////////////////////////////////////////////////////
struct World
{
private:
    sf::base::Vector<sf::base::Optional<Block>> m_blocks;

public:
    ////////////////////////////////////////////////////////////
    void addBlock(const Block& block)
    {
        m_blocks.pushBack(block);
    }

    ////////////////////////////////////////////////////////////
    void forBlocks(auto&& f)
    {
        for (BlockId i = 0u; i < m_blocks.size(); ++i)
            if (m_blocks[i].hasValue())
                f(i, *m_blocks[i]);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<Block>& getBlockById(const BlockId blockId)
    {
        SFML_BASE_ASSERT(blockId < m_blocks.size());
        return m_blocks[blockId];
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isBlocked(const sf::Vec2i position) const
    {
        for (const sf::base::Optional<Block>& block : m_blocks)
        {
            if (!block.hasValue())
                continue;

            if (block->position == position)
                return true;
        }

        return false;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<BlockId> sameBlockTypeNearby(const BlockId blockId) const
    {
        const sf::base::Optional<Block>& block = m_blocks[blockId];
        if (!block.hasValue())
            return sf::base::nullOpt;

        for (BlockId i = 0u; i < m_blocks.size(); ++i)
        {
            const auto& otherBlock = m_blocks[i];

            if (!otherBlock.hasValue())
                continue;

            if (std::abs(otherBlock->position.x - block->position.x) == 1 &&
                std::abs(otherBlock->position.y - block->position.y) == 1)
                continue;

            if (otherBlock->position == block->position || std::abs(otherBlock->position.x - block->position.x) > 1 ||
                std::abs(otherBlock->position.y - block->position.y) > 1)
                continue;

            if (otherBlock->type == block->type)
                return sf::base::makeOptional<BlockId>(i);
        }

        return sf::base::nullOpt;
    }

    ////////////////////////////////////////////////////////////
    void killBlock(const BlockId blockId)
    {
        SFML_BASE_ASSERT(blockId < m_blocks.size());
        m_blocks[blockId].reset();
    }
};


////////////////////////////////////////////////////////////
struct TEMoveBlock
{
    BlockId   blockId;
    sf::Vec2i newPosition;
    float     progress{0.f};
};


////////////////////////////////////////////////////////////
struct TESquishBlock
{
    BlockId   blockId;
    sf::Vec2i dir;
    float     progress{0.f};
};


////////////////////////////////////////////////////////////
struct TEMerge
{
    BlockId blockIdA;
    BlockId blockIdB;
    float   progress{0.f};
};


////////////////////////////////////////////////////////////
using TurnEvent = sfvr::tinyvariant<TEMoveBlock, TESquishBlock, TEMerge>;


////////////////////////////////////////////////////////////
class Game
{
private:
    ////////////////////////////////////////////////////////////
    sf::RenderWindow m_window = makeDPIScaledRenderWindow(
        {.size            = resolution.toVec2u(),
         .title           = "Block Puzzle",
         .resizable       = true,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = 8u}});

    ////////////////////////////////////////////////////////////
    const sf::Font m_font = sf::Font::openFromFile("resources/Born2bSportyFS.ttf").value();

    ////////////////////////////////////////////////////////////
    sf::ImGuiContext m_imGuiContext;

    ////////////////////////////////////////////////////////////
    sf::Clock m_fpsClock;

    ////////////////////////////////////////////////////////////
    World                               m_world;
    sf::base::Optional<sf::base::SizeT> m_grabbedBlockId;
    sf::base::Vector<TurnEvent>         m_turnEvents;

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        m_world.addBlock({.position = {0u, 0u}, .type = BlockType::Wall, .gravityType = GravityType::None});
        m_world.addBlock({.position = {1u, 0u}, .type = BlockType::Wall, .gravityType = GravityType::None});
        m_world.addBlock({.position = {2u, 0u}, .type = BlockType::Wall, .gravityType = GravityType::None});
        m_world.addBlock({.position = {0u, 1u}, .type = BlockType::Wall, .gravityType = GravityType::None});
        m_world.addBlock({.position = {0u, 2u}, .type = BlockType::Wall, .gravityType = GravityType::None});
        m_world.addBlock({.position = {6u, 6u}, .type = BlockType::Wall, .gravityType = GravityType::None});

        m_world.addBlock({.position = {4u, 4u}, .type = BlockType::Red, .gravityType = GravityType::None});
        m_world.addBlock({.position = {6u, 4u}, .type = BlockType::Red, .gravityType = GravityType::None});
        m_world.addBlock({.position = {7u, 6u}, .type = BlockType::Red, .gravityType = GravityType::None});
        m_world.addBlock({.position = {3u, 3u}, .type = BlockType::Red, .gravityType = GravityType::None});

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
            }
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Update step
            ////////////////////////////////////////////////////////////
            // ---
            const auto  deltaTime   = m_fpsClock.restart();
            const float deltaTimeMs = deltaTime.asSeconds() * 1000.f;

            const bool lmbPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            const auto mousePosition = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_window.getView()).toVec2f();
            const auto adjustedMousePosition = mousePosition + sf::Vec2f{32.f, 32.f};
            const auto worldMousePosition    = (mousePosition / 64.f).toVec2i();

            if (!lmbPressed)
                m_grabbedBlockId.reset();

            m_world.forBlocks([&](const BlockId blockId, const Block& block)
            {
                const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 64.f, block.position.y * 64.f} +
                                               sf::Vec2f{32.f, 32.f};

                const bool hoveredByMouse = adjustedMousePosition.x > drawPosition.x &&
                                            adjustedMousePosition.x < drawPosition.x + 64.f &&
                                            adjustedMousePosition.y > drawPosition.y &&
                                            adjustedMousePosition.y < drawPosition.y + 64.f;

                if (hoveredByMouse && lmbPressed && !m_grabbedBlockId.hasValue())
                    m_grabbedBlockId.emplace(blockId);
            });

            if (m_turnEvents.empty())
            {
                if (m_grabbedBlockId.hasValue())
                {
                    Block& grabbedBlock = *m_world.getBlockById(*m_grabbedBlockId);

                    auto diff = worldMousePosition - grabbedBlock.position;
                    const sf::Vec2i dir = {diff.x < 0 ? -1 : (diff.x > 0 ? 1 : 0), diff.y < 0 ? -1 : (diff.y > 0 ? 1 : 0)};

                    const auto newPos = grabbedBlock.position + dir;

                    if (!m_world.isBlocked(newPos))
                        m_turnEvents.pushBack(
                            TEMoveBlock{.blockId = *m_grabbedBlockId, .newPosition = grabbedBlock.position + dir});
                    else if (diff != sf::Vec2i{0, 0})
                        m_turnEvents.pushBack(TESquishBlock{.blockId = *m_grabbedBlockId, .dir = dir});
                }
            }
            else
            {
                auto& turnEvent = m_turnEvents.front();
                if (auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
                {
                    if (moveBlock->progress < 1.f)
                        moveBlock->progress += deltaTimeMs * 0.0075f;

                    if (moveBlock->progress >= 1.f)
                    {
                        Block& grabbedBlock   = *m_world.getBlockById(moveBlock->blockId);
                        grabbedBlock.position = moveBlock->newPosition;
                        const auto savedId    = moveBlock->blockId;
                        m_turnEvents.erase(m_turnEvents.begin());

                        if (const auto blockIdB = m_world.sameBlockTypeNearby(savedId))
                            m_turnEvents.pushBack(TEMerge{.blockIdA = savedId, .blockIdB = *blockIdB});
                    }
                }
                else if (auto* squishBlock = turnEvent.get_if<TESquishBlock>())
                {
                    if (squishBlock->progress < 1.f)
                        squishBlock->progress += deltaTimeMs * 0.0025f;

                    if (squishBlock->progress >= 1.f)
                        m_turnEvents.erase(m_turnEvents.begin());
                }
                else if (auto* merge = turnEvent.get_if<TEMerge>())
                {
                    if (merge->progress < 1.f)
                        merge->progress += deltaTimeMs * 0.001f;

                    if (merge->progress >= 1.f)
                    {
                        m_world.killBlock(merge->blockIdA);
                        m_world.killBlock(merge->blockIdB);
                        m_grabbedBlockId.reset();

                        m_turnEvents.erase(m_turnEvents.begin());
                    }
                }
            }
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // ImGui step
            ////////////////////////////////////////////////////////////
            // ---
            m_imGuiContext.update(m_window, deltaTime);

            // ImGui::Begin("What", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            // ImGui::End();
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Draw step
            ////////////////////////////////////////////////////////////
            // ---
            m_window.clear();

            m_world.forBlocks([&](const BlockId blockId, const Block& block)
            {
                sf::Vec2f drawPositionOffset = {0.f, 0.f};
                sf::Vec2f scaleMultiplier    = {1.f, 1.f};

                if (!m_turnEvents.empty())
                {
                    const auto& turnEvent = m_turnEvents.front();

                    if (const auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
                    {
                        if (moveBlock->blockId == blockId)
                        {
                            const auto worldOffset = moveBlock->newPosition.toVec2f() - block.position.toVec2f();
                            drawPositionOffset     = easeInOutBack(moveBlock->progress) * worldOffset * 64.f;
                        }
                    }
                    else if (const auto* squishBlock = turnEvent.get_if<TESquishBlock>())
                    {
                        if (squishBlock->blockId == blockId)
                        {
                            const auto absDir = sf::Vec2f{sf::base::fabs(static_cast<float>(squishBlock->dir.x)),
                                                          sf::base::fabs(static_cast<float>(squishBlock->dir.y))};

                            const float progress = easeInOutBack(bounce(squishBlock->progress));

                            scaleMultiplier += (progress * absDir * -0.35f);
                            scaleMultiplier += (progress * sf::Vec2f{absDir.y, absDir.x} * 0.35f);

                            drawPositionOffset = progress * squishBlock->dir.toVec2f() * 32.f;
                        }
                    }
                    else if (const auto* merge = turnEvent.get_if<TEMerge>())
                    {
                        if (merge->blockIdA == blockId || merge->blockIdB == blockId)
                            scaleMultiplier *= 1.f - easeInOutBack(merge->progress);
                    }
                }

                const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 64.f, block.position.y * 64.f} +
                                               drawPositionOffset + sf::Vec2f{32.f, 32.f};

                const bool hoveredByMouse = adjustedMousePosition.x > drawPosition.x &&
                                            adjustedMousePosition.x < drawPosition.x + 64.f &&
                                            adjustedMousePosition.y > drawPosition.y &&
                                            adjustedMousePosition.y < drawPosition.y + 64;

                switch (block.type)
                {
                    case BlockType::Wall:
                        m_window.draw(sf::RectangleShapeData{
                            .position         = drawPosition,
                            .origin           = {32.f, 32.f},
                            .fillColor        = sf::Color::Gray,
                            .outlineColor     = sf::Color::LightGray,
                            .outlineThickness = 4.f,
                            .size             = {64.f, 64.f},
                        });
                        break;

                    case BlockType::Red:
                        m_window.draw(sf::RoundedRectangleShapeData{
                            .position         = drawPosition,
                            .scale            = scaleMultiplier,
                            .origin           = {32.f, 32.f},
                            .fillColor        = sf::Color::Red,
                            .outlineColor     = hoveredByMouse ? sf::Color::White : sf::Color::LightRed,
                            .outlineThickness = (hoveredByMouse ? -8.f : -4.f) *
                                                scaleMultiplier.x, // TODO P0: sign inconsistency with normal rectangle
                            .size         = sf::Vec2f{64.f, 64.f},
                            .cornerRadius = 8.f,
                        });
                        break;

                    default:
                        break;
                }

                if (true)
                    m_window.draw(m_font,
                                  sf::TextData{
                                      .position         = drawPosition + sf::Vec2f{8.f, 2.f} - sf::Vec2f{32.f, 32.f},
                                      .string           = std::to_string(blockId),
                                      .characterSize    = 18u,
                                      .fillColor        = sf::Color::White,
                                      .outlineColor     = sf::Color::Black,
                                      .outlineThickness = 2.f,
                                  });
            });

            if (m_grabbedBlockId.hasValue())
                m_window.draw(m_font,
                              sf::TextData{
                                  .position         = {100, 100},
                                  .string           = std::to_string(*m_grabbedBlockId),
                                  .characterSize    = 18u,
                                  .fillColor        = sf::Color::White,
                                  .outlineColor     = sf::Color::Black,
                                  .outlineThickness = 2.f,
                              });

            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Display step
            ////////////////////////////////////////////////////////////
            // ---
            m_imGuiContext.render(m_window);
            m_window.display();
            // ---
            ////////////////////////////////////////////////////////////
        }

        return true;
    }
};

} // namespace


////////////////////////////////////////////////////////////
int main()
{
    auto audioContext    = sf::AudioContext::create().value();
    auto graphicsContext = sf::GraphicsContext::create().value();

    Game game;

    if (!game.run())
        return 1;

    return 0;
}
