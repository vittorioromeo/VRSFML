#include "../bubble_idle/Easing.hpp"  // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Sampler.hpp" // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Music.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Window/EventUtils.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#include <algorithm>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <initializer_list>


namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1024.f, 1024.f};


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
struct BWall
{
};


////////////////////////////////////////////////////////////
struct BColored
{
    sf::Color color;
};


////////////////////////////////////////////////////////////
using BlockType = sfvr::tinyvariant<BWall, BColored>;


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
    sf::Vec2i position;
    BlockType type;
    sf::Vec2i gravityDir = {0, 0};
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
    [[nodiscard]] sf::base::Optional<BlockId> getBlockByPosition(const sf::Vec2i position) const
    {
        for (BlockId i = 0u; i < m_blocks.size(); ++i)
        {
            const auto& block = m_blocks[i];

            if (block.hasValue() && block->position == position)
                return sf::base::makeOptional<BlockId>(i);
        }

        return sf::base::nullOpt;
    }

    ////////////////////////////////////////////////////////////
    void forOrthogonalNeighbors(const BlockId blockId, auto&& f) const
    {
        const sf::base::Optional<Block>& block = m_blocks[blockId];
        if (!block.hasValue())
            return;

        if (const auto e = getBlockByPosition(block->position + sf::Vec2i{0, 1}); e.hasValue())
            f(*e);

        if (const auto w = getBlockByPosition(block->position + sf::Vec2i{1, 0}); w.hasValue())
            f(*w);

        if (const auto n = getBlockByPosition(block->position + sf::Vec2i{0, -1}); n.hasValue())
            f(*n);

        if (const auto s = getBlockByPosition(block->position + sf::Vec2i{-1, 0}); s.hasValue())
            f(*s);
    }

    ////////////////////////////////////////////////////////////
    void killBlock(const BlockId blockId)
    {
        SFML_BASE_ASSERT(blockId < m_blocks.size());
        SFML_BASE_ASSERT(m_blocks[blockId].hasValue());
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
struct TEFallBlock
{
    BlockId   blockId;
    sf::Vec2i dir;
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
struct TEKill
{
    BlockId blockId;
    float   progress{0.f};
};


////////////////////////////////////////////////////////////
using TurnEvent = sfvr::tinyvariant<TEMoveBlock, TEFallBlock, TESquishBlock, TEKill>;


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

    ////////////////////////////////////////////////////////////
    void checkForKill()
    {
        sf::base::Vector<BlockId> blocksToKill;

        m_world.forBlocks([&](const BlockId blockId, const Block& block)
        {
            const auto* blockColored = block.type.get_if<BColored>();
            if (blockColored == nullptr)
                return;

            m_world.forOrthogonalNeighbors(blockId,
                                           [&](const BlockId neighborBlockId)
            {
                if (neighborBlockId >= blockId)
                    return;

                const Block& neighborBlock        = *m_world.getBlockById(neighborBlockId);
                const auto*  neighborBlockColored = neighborBlock.type.get_if<BColored>();

                if (neighborBlockColored == nullptr)
                    return;

                if (blockColored->color == neighborBlockColored->color)
                {
                    blocksToKill.pushBack(blockId);
                    blocksToKill.pushBack(neighborBlockId);
                }
            });
        });

        std::sort(blocksToKill.begin(), blocksToKill.end());
        blocksToKill.erase(sf::base::unique(blocksToKill.begin(), blocksToKill.end()), blocksToKill.end());

        for (const BlockId blockId : blocksToKill)
            m_turnEvents.pushBack(TEKill{.blockId = blockId});
    }

    ////////////////////////////////////////////////////////////
    void forTurnEventsToProcess(auto&& f)
    {
        if (m_turnEvents.empty())
            return;

        auto&      turnEvent  = m_turnEvents.front();
        const bool mustDelete = f(turnEvent);

        if (mustDelete)
            m_turnEvents.erase(m_turnEvents.begin());

        if (!turnEvent.is<TEKill>())
            return;

        for (BlockId i = mustDelete ? 0u : 1u; i < m_turnEvents.size(); ++i)
        {
            if (!m_turnEvents[i].is<TEKill>())
                break;

            const bool mustDelete2 = f(m_turnEvents[i]);
            if (mustDelete2)
            {
                m_turnEvents.erase(m_turnEvents.begin() + i);
                --i;
            }
        }
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {

        for (int iX = 0; iX < 16; ++iX)
            for (int iY = 0; iY < 16; ++iY)
                if (iX == 0 || iX == 15 || iY == 0 || iY == 15)
                    m_world.addBlock({.position = {iX, iY}, .type = BlockType{BWall{}}});

        m_world.addBlock({.position = {4, 4}, .type = BlockType{BColored{.color = sf::Color::Red}}});

        m_world.addBlock({
            .position   = {6, 4},
            .type       = BlockType{BColored{.color = sf::Color::Red}},
            .gravityDir = {0, 1},
        });

        m_world.addBlock({
            .position   = {6, 6},
            .type       = BlockType{BColored{.color = sf::Color::Red}},
            .gravityDir = {0, -1},
        });

        m_world.addBlock({.position = {7, 6}, .type = BlockType{BColored{.color = sf::Color::Red}}});
        m_world.addBlock({.position = {3, 3}, .type = BlockType{BColored{.color = sf::Color::Red}}});
        m_world.addBlock({.position = {8, 8}, .type = BlockType{BColored{.color = sf::Color::Blue}}});
        m_world.addBlock({.position = {9, 9}, .type = BlockType{BColored{.color = sf::Color::Blue}}});
        m_world.addBlock({.position = {10, 10}, .type = BlockType{BColored{.color = sf::Color::Green}}});
        m_world.addBlock({
            .position   = {12, 10},
            .type       = BlockType{BColored{.color = sf::Color::Green}},
            .gravityDir = {-1, 0},
        });

        m_world.addBlock({.position = {6, 5}, .type = BlockType{BWall{}}});
        m_world.addBlock({.position = {7, 5}, .type = BlockType{BWall{}}});
        m_world.addBlock({.position = {8, 5}, .type = BlockType{BWall{}}});


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

            if (m_turnEvents.empty())
                m_world.forBlocks([&](const BlockId blockId, const Block& block)
                {
                    if (block.gravityDir == sf::Vec2i{0, 0})
                        return;

                    sf::Vec2i targetPosition = block.position;
                    while (!m_world.isBlocked(targetPosition + block.gravityDir))
                        targetPosition += block.gravityDir;

                    if (targetPosition != block.position)
                        m_turnEvents.pushBack(
                            TEFallBlock{.blockId = blockId, .dir = block.gravityDir, .newPosition = targetPosition});
                });


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
                forTurnEventsToProcess([&](TurnEvent& turnEvent)
                {
                    if (auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
                    {
                        if (moveBlock->progress < 1.f)
                            moveBlock->progress += deltaTimeMs * 0.0075f;

                        if (moveBlock->progress >= 1.f)
                        {
                            Block& grabbedBlock   = *m_world.getBlockById(moveBlock->blockId);
                            grabbedBlock.position = moveBlock->newPosition;

                            checkForKill();

                            return true;
                        }
                    }
                    else if (auto* fallBlock = turnEvent.get_if<TEFallBlock>())
                    {
                        if (fallBlock->progress < 1.f)
                            fallBlock->progress += deltaTimeMs * 0.0015f;

                        if (fallBlock->progress >= 1.f)
                        {
                            Block& grabbedBlock   = *m_world.getBlockById(fallBlock->blockId);
                            grabbedBlock.position = fallBlock->newPosition;
                            m_turnEvents.pushBack(TESquishBlock{.blockId = fallBlock->blockId, .dir = fallBlock->dir});

                            checkForKill();

                            return true;
                        }
                    }
                    else if (auto* squishBlock = turnEvent.get_if<TESquishBlock>())
                    {
                        if (squishBlock->progress < 1.f)
                            squishBlock->progress += deltaTimeMs * 0.0020f;

                        if (squishBlock->progress >= 1.f)
                            return true;
                    }
                    else if (auto* kill = turnEvent.get_if<TEKill>())
                    {
                        if (kill->progress < 1.f)
                            kill->progress += deltaTimeMs * 0.0015f;

                        if (kill->progress >= 1.f)
                        {
                            m_world.killBlock(kill->blockId);
                            m_grabbedBlockId.reset();

                            return true;
                        }
                    }

                    return false;
                });
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
                float     rotationRadians    = 0.f;

                forTurnEventsToProcess([&](TurnEvent& turnEvent)
                {
                    if (const auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
                    {
                        if (moveBlock->blockId == blockId)
                        {
                            const auto worldOffset = moveBlock->newPosition.toVec2f() - block.position.toVec2f();
                            drawPositionOffset     = easeInOutBack(moveBlock->progress) * worldOffset * 64.f;
                        }
                    }
                    else if (const auto* fallBlock = turnEvent.get_if<TEFallBlock>())
                    {
                        if (fallBlock->blockId == blockId)
                        {
                            const auto worldOffset = fallBlock->newPosition.toVec2f() - block.position.toVec2f();
                            drawPositionOffset     = easeInBack(fallBlock->progress) * worldOffset * 64.f;
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
                    else if (const auto* kill = turnEvent.get_if<TEKill>())
                    {
                        if (kill->blockId == blockId)
                        {
                            scaleMultiplier *= 1.f - easeInOutBack(kill->progress);
                            rotationRadians = easeInOutSine(kill->progress) * sf::base::tau;
                        }
                    }

                    return false;
                });

                const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 64.f, block.position.y * 64.f} +
                                               drawPositionOffset + sf::Vec2f{32.f, 32.f};

                const bool hoveredByMouse = adjustedMousePosition.x > drawPosition.x &&
                                            adjustedMousePosition.x < drawPosition.x + 64.f &&
                                            adjustedMousePosition.y > drawPosition.y &&
                                            adjustedMousePosition.y < drawPosition.y + 64.f;

                block.type.linear_match(
                    [&](const BWall&)
                {
                    m_window.draw(sf::RectangleShapeData{
                        .position         = drawPosition,
                        .origin           = {32.f, 32.f},
                        .fillColor        = sf::Color::Gray,
                        .outlineColor     = sf::Color::LightGray,
                        .outlineThickness = 4.f,
                        .size             = {64.f, 64.f},
                    });
                },
                    [&](const BColored& bColored)
                {
                    m_window.draw(sf::RoundedRectangleShapeData{
                        .position         = drawPosition,
                        .scale            = scaleMultiplier,
                        .origin           = {32.f, 32.f},
                        .rotation         = sf::radians(rotationRadians).wrapUnsigned(),
                        .fillColor        = bColored.color,
                        .outlineColor     = hoveredByMouse ? sf::Color::White : bColored.color.withLightness(0.75f),
                        .outlineThickness = (hoveredByMouse ? -8.f : -4.f) *
                                            scaleMultiplier.x, // TODO P0: sign inconsistency with normal rectangle
                        .size         = sf::Vec2f{64.f, 64.f},
                        .cornerRadius = 8.f,
                    });
                });

                if (block.gravityDir != sf::Vec2i{0, 0})
                    m_window.draw(sf::ArrowShapeData{
                        .position = drawPosition,
                        .scale    = scaleMultiplier,
                        .origin   = {},
                        .rotation = sf::radians(block.gravityDir.toVec2f().angle().asRadians() + rotationRadians).wrapUnsigned(),
                        .fillColor        = sf::Color::LightRed,
                        .outlineColor     = sf::Color::White,
                        .outlineThickness = 2.f * sf::base::fabs(scaleMultiplier.x),
                        .shaftLength      = 12.f * sf::base::fabs(scaleMultiplier.x),
                        .shaftWidth       = 12.f * sf::base::fabs(scaleMultiplier.x),
                        .headLength       = 12.f * sf::base::fabs(scaleMultiplier.x),
                        .headWidth        = 24.f * sf::base::fabs(scaleMultiplier.x),
                    });

                if (false)
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
