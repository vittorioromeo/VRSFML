#include "../bubble_idle/ControlFlow.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"    // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/Texture.hpp"

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
    bool      fixed      = false;
    bool      locked     = false;
};


////////////////////////////////////////////////////////////
using ObjectId = sf::base::SizeT;


////////////////////////////////////////////////////////////
struct TGravityRotator
{
    bool clockwise{false};
};


////////////////////////////////////////////////////////////
using TileType = sfvr::tinyvariant<TGravityRotator>;


////////////////////////////////////////////////////////////
struct Tile
{
    sf::Vec2i position;
    TileType  type;
};


////////////////////////////////////////////////////////////
using Object = sfvr::tinyvariant<Block, Tile>;


////////////////////////////////////////////////////////////
struct World
{
private:
    sf::base::Vector<sf::base::Optional<Object>> m_objects;

public:
    ////////////////////////////////////////////////////////////
    void addWall(const sf::Vec2i position)
    {
        m_objects.emplaceBack(sf::base::inPlace,
                              Block{
                                  .position = position,
                                  .type     = BlockType{BWall{}},
                              });
    }

    ////////////////////////////////////////////////////////////
    void addColored(const sf::Vec2i position, const sf::Color color, const sf::Vec2i gravityDir = {0, 0})
    {
        m_objects.emplaceBack(sf::base::inPlace,
                              Block{
                                  .position   = position,
                                  .type       = BlockType{BColored{.color = color}},
                                  .gravityDir = gravityDir,
                              });
    }

    ////////////////////////////////////////////////////////////
    void addGravityRotator(const sf::Vec2i position, const bool clockwise)
    {
        m_objects.emplaceBack(sf::base::inPlace,
                              Tile{
                                  .position = position,
                                  .type     = TileType{TGravityRotator{.clockwise = clockwise}},
                              });
    }

    ////////////////////////////////////////////////////////////
    void forTiles(auto&& f)
    {
        for (ObjectId i = 0u; i < m_objects.size(); ++i)
            if (m_objects[i].hasValue() && m_objects[i]->is<Tile>())
                if (f(i, m_objects[i]->as<Tile>()) == ControlFlow::Break)
                    break;
    }

    ////////////////////////////////////////////////////////////
    void forBlocks(auto&& f)
    {
        for (ObjectId i = 0u; i < m_objects.size(); ++i)
            if (m_objects[i].hasValue() && m_objects[i]->is<Block>())
                if (f(i, m_objects[i]->as<Block>()) == ControlFlow::Break)
                    break;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Block& getBlockById(const ObjectId objectId)
    {
        SFML_BASE_ASSERT(objectId < m_objects.size());
        SFML_BASE_ASSERT(m_objects[objectId].hasValue());
        SFML_BASE_ASSERT(m_objects[objectId]->is<Block>());

        return m_objects[objectId]->as<Block>();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isBlocked(const sf::Vec2i position) const
    {
        for (const sf::base::Optional<Object>& object : m_objects)
        {
            if (!object.hasValue() || !object->is<Block>())
                continue;

            if (object->as<Block>().position == position)
                return true;
        }

        return false;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] TGravityRotator* tryGetGravityRotator(const sf::Vec2i position)
    {
        for (sf::base::Optional<Object>& object : m_objects)
        {
            if (!object.hasValue() || !object->is<Tile>())
                continue;

            if (object->as<Tile>().position == position)
                return object->as<Tile>().type.get_if<TGravityRotator>();
        }

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::base::Optional<ObjectId> getBlockByPosition(const sf::Vec2i position) const
    {
        for (ObjectId i = 0u; i < m_objects.size(); ++i)
        {
            const auto& object = m_objects[i];

            if (object.hasValue() && object->is<Block>() && object->as<Block>().position == position)
                return sf::base::makeOptional<ObjectId>(i);
        }

        return sf::base::nullOpt;
    }

    ////////////////////////////////////////////////////////////
    void forOrthogonalNeighbors(const ObjectId objectId, auto&& f) const
    {
        const sf::base::Optional<Object>& object = m_objects[objectId];

        if (!object.hasValue() || !object->is<Block>())
            return;

        if (const auto e = getBlockByPosition(object->as<Block>().position + sf::Vec2i{0, 1}); e.hasValue())
            if (f(*e) == ControlFlow::Break)
                return;

        if (const auto w = getBlockByPosition(object->as<Block>().position + sf::Vec2i{1, 0}); w.hasValue())
            if (f(*w) == ControlFlow::Break)
                return;

        if (const auto n = getBlockByPosition(object->as<Block>().position + sf::Vec2i{0, -1}); n.hasValue())
            if (f(*n) == ControlFlow::Break)
                return;

        if (const auto s = getBlockByPosition(object->as<Block>().position + sf::Vec2i{-1, 0}); s.hasValue())
            if (f(*s) == ControlFlow::Break)
                return;
    }

    ////////////////////////////////////////////////////////////
    void killBlock(const ObjectId objectId)
    {
        SFML_BASE_ASSERT(objectId < m_objects.size());
        SFML_BASE_ASSERT(m_objects[objectId].hasValue());
        m_objects[objectId].reset();
    }
};


////////////////////////////////////////////////////////////
struct TEMoveBlock
{
    ObjectId  objectId;
    sf::Vec2i newPosition;
    float     progress{0.f};
};


////////////////////////////////////////////////////////////
struct TEFallBlock
{
    ObjectId  objectId;
    sf::Vec2i dir;
    sf::Vec2i newPosition;
    bool      squish;
    float     progress{0.f};
};


////////////////////////////////////////////////////////////
struct TESquishBlock
{
    ObjectId  objectId;
    sf::Vec2i dir;
    float     progress{0.f};
};


////////////////////////////////////////////////////////////
struct TERotateGravityDir
{
    ObjectId objectId;
    bool     clockwise;
    float    progress{0.f};
};


////////////////////////////////////////////////////////////
struct TEKill
{
    ObjectId objectId;
    float    progress{0.f};
};


////////////////////////////////////////////////////////////
using TurnEvent = sfvr::tinyvariant<TEMoveBlock, TEFallBlock, TESquishBlock, TEKill, TERotateGravityDir>;


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
    sf::Shader m_shader{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/shader.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    ////////////////////////////////////////////////////////////
    const sf::Font m_font = sf::Font::openFromFile("resources/Born2bSportyFS.ttf").value();

    ////////////////////////////////////////////////////////////
    const sf::Texture m_txFixed  = sf::Texture::loadFromFile("resources/fixed.png", {.smooth = true}).value();
    const sf::Texture m_txLocked = sf::Texture::loadFromFile("resources/locked.png", {.smooth = true}).value();

    ////////////////////////////////////////////////////////////
    sf::ImGuiContext m_imGuiContext;

    ////////////////////////////////////////////////////////////
    sf::Clock m_fpsClock;

    ////////////////////////////////////////////////////////////
    World                               m_world;
    sf::base::Optional<sf::base::SizeT> m_grabbedObjectId;
    sf::base::Vector<TurnEvent>         m_turnEvents;

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    ////////////////////////////////////////////////////////////
    void checkForKill()
    {
        sf::base::Vector<ObjectId> blocksToKill;

        m_world.forBlocks([&](const ObjectId objectId, const Block& block)
        {
            const auto* blockColored = block.type.get_if<BColored>();
            if (blockColored == nullptr)
                return ControlFlow::Continue;

            m_world.forOrthogonalNeighbors(objectId,
                                           [&](const ObjectId neighborObjectId)
            {
                if (neighborObjectId >= objectId)
                    return ControlFlow::Continue;

                const Block& neighborBlock        = m_world.getBlockById(neighborObjectId);
                const auto*  neighborBlockColored = neighborBlock.type.get_if<BColored>();

                if (neighborBlockColored == nullptr)
                    return ControlFlow::Continue;

                if (blockColored->color == neighborBlockColored->color)
                {
                    blocksToKill.pushBack(objectId);
                    blocksToKill.pushBack(neighborObjectId);
                }

                return ControlFlow::Continue;
            });

            return ControlFlow::Continue;
        });

        std::sort(blocksToKill.begin(), blocksToKill.end());
        blocksToKill.erase(sf::base::unique(blocksToKill.begin(), blocksToKill.end()), blocksToKill.end());

        for (const ObjectId objectId : blocksToKill)
            m_turnEvents.pushBack(TEKill{.objectId = objectId});
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

        for (ObjectId i = mustDelete ? 0u : 1u; i < m_turnEvents.size(); ++i)
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
                    m_world.addWall({iX, iY});

        m_world.addColored({4, 4}, sf::Color::Red);

        m_world.addColored({7, 4}, sf::Color::Red, {0, 1});
        m_world.addColored({6, 6}, sf::Color::Red, {0, -1});
        m_world.addColored({7, 6}, sf::Color::Red);
        m_world.addColored({3, 3}, sf::Color::Red);
        m_world.addColored({8, 8}, sf::Color::Blue);
        m_world.addColored({9, 9}, sf::Color::Blue);

        m_world.addGravityRotator({9, 10}, /* clockwise */ true);
        m_world.addGravityRotator({2, 10}, /* clockwise */ true);
        m_world.addGravityRotator({7, 1}, /* clockwise */ true);

        m_world.addColored({10, 10}, sf::Color::Green);
        m_world.addColored({12, 11}, sf::Color::Green, {-1, 0});

        m_world.addWall({6, 4});
        m_world.addWall({6, 5});
        m_world.addWall({7, 5});
        m_world.addWall({8, 5});

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

            m_time += deltaTimeMs;

            const bool lmbPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            const auto mousePosition = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_window.getView()).toVec2f();
            const auto adjustedMousePosition = mousePosition + sf::Vec2f{32.f, 32.f};
            const auto worldMousePosition    = (mousePosition / 64.f).toVec2i();

            if (!lmbPressed)
                m_grabbedObjectId.reset();

            if (m_turnEvents.empty())
                m_world.forBlocks([&](const ObjectId objectId, Block& block)
                {
                    if (block.gravityDir == sf::Vec2i{0, 0})
                        return ControlFlow::Continue;

                    sf::Vec2i targetPosition = block.position;
                    bool      mustFall       = false;

                    while (true)
                    {
                        if (targetPosition == block.position)
                        {
                            targetPosition += block.gravityDir;
                            continue;
                        }

                        if (m_world.isBlocked(targetPosition))
                        {
                            mustFall = targetPosition != block.position + block.gravityDir;
                            break;
                        }

                        if (auto* gr = m_world.tryGetGravityRotator(targetPosition))
                        {
                            m_turnEvents.pushBack(TEFallBlock{
                                .objectId    = objectId,
                                .dir         = block.gravityDir,
                                .newPosition = targetPosition,
                                .squish      = false,
                            });

                            m_turnEvents.pushBack(TERotateGravityDir{
                                .objectId  = objectId,
                                .clockwise = gr->clockwise,
                            });

                            return ControlFlow::Continue;
                        }

                        targetPosition += block.gravityDir;
                    }

                    if (mustFall)
                        m_turnEvents.pushBack(TEFallBlock{.objectId    = objectId,
                                                          .dir         = block.gravityDir,
                                                          .newPosition = targetPosition - block.gravityDir,
                                                          .squish      = true});

                    return ControlFlow::Continue;
                });

            m_world.forBlocks([&](const ObjectId objectId, const Block& block)
            {
                const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 64.f, block.position.y * 64.f} +
                                               sf::Vec2f{32.f, 32.f};

                const bool hoveredByMouse = adjustedMousePosition.x > drawPosition.x &&
                                            adjustedMousePosition.x < drawPosition.x + 64.f &&
                                            adjustedMousePosition.y > drawPosition.y &&
                                            adjustedMousePosition.y < drawPosition.y + 64.f;

                if (hoveredByMouse && lmbPressed && !m_grabbedObjectId.hasValue())
                {
                    m_grabbedObjectId.emplace(objectId);
                    return ControlFlow::Break;
                }

                return ControlFlow::Continue;
            });

            if (m_turnEvents.empty())
            {
                if (m_grabbedObjectId.hasValue())
                {
                    Block& grabbedBlock = m_world.getBlockById(*m_grabbedObjectId);

                    auto diff = worldMousePosition - grabbedBlock.position;
                    const sf::Vec2i dir = {diff.x < 0 ? -1 : (diff.x > 0 ? 1 : 0), diff.y < 0 ? -1 : (diff.y > 0 ? 1 : 0)};

                    const auto newPos = grabbedBlock.position + dir;

                    const bool movingAgainstGravity = grabbedBlock.gravityDir != sf::Vec2i{0, 0} &&
                                                      grabbedBlock.gravityDir == -dir;

                    if (!m_world.isBlocked(newPos))
                    {
                        if (!movingAgainstGravity)
                            m_turnEvents.pushBack(
                                TEMoveBlock{.objectId = *m_grabbedObjectId, .newPosition = grabbedBlock.position + dir});
                    }
                    else if (diff != sf::Vec2i{0, 0})
                        m_turnEvents.pushBack(TESquishBlock{.objectId = *m_grabbedObjectId, .dir = dir});
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
                            Block& grabbedBlock   = m_world.getBlockById(moveBlock->objectId);
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
                            Block& grabbedBlock   = m_world.getBlockById(fallBlock->objectId);
                            grabbedBlock.position = fallBlock->newPosition;

                            if (fallBlock->squish)
                                m_turnEvents.pushBack(TESquishBlock{.objectId = fallBlock->objectId, .dir = fallBlock->dir});

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
                            m_world.killBlock(kill->objectId);
                            m_grabbedObjectId.reset();

                            return true;
                        }
                    }
                    else if (auto* rotateGravityDir = turnEvent.get_if<TERotateGravityDir>())
                    {
                        if (rotateGravityDir->progress < 1.f)
                            rotateGravityDir->progress += deltaTimeMs * 0.0020f;

                        if (rotateGravityDir->progress >= 1.f)
                        {
                            if (rotateGravityDir->clockwise)
                                m_world.getBlockById(rotateGravityDir->objectId)
                                    .gravityDir = sf::Vec2i{-m_world.getBlockById(rotateGravityDir->objectId).gravityDir.y,
                                                            m_world.getBlockById(rotateGravityDir->objectId).gravityDir.x};
                            else
                                m_world.getBlockById(rotateGravityDir->objectId)
                                    .gravityDir = sf::Vec2i{m_world.getBlockById(rotateGravityDir->objectId).gravityDir.y,
                                                            -m_world.getBlockById(rotateGravityDir->objectId).gravityDir.x};

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

            m_world.forTiles([&](const ObjectId objectId, const Tile& tile)
            {
                const sf::Vec2f drawPosition = sf::Vec2f{tile.position.x * 64.f, tile.position.y * 64.f} +
                                               sf::Vec2f{32.f, 32.f};

                tile.type.linear_match([&](const TGravityRotator& gravityRotator)
                {
                    m_window.draw(
                        sf::CurvedArrowShapeData{
                            .position         = drawPosition,
                            .scale            = {0.6f, 0.6f},
                            .origin           = {32.f, 32.f},
                            .rotation         = sf::radians(m_time * 0.002f).wrapUnsigned(),
                            .fillColor        = sf::Color::LightYellow,
                            .outlineColor     = sf::Color::Yellow,
                            .outlineThickness = 2.f,
                            .outerRadius      = 32.f,
                            .innerRadius      = 24.f,
                            .startAngle       = sf::degrees(0.f),
                            .sweepAngle       = sf::degrees(270.f),
                            .headLength       = 16.f,
                            .headWidth        = 24.f,
                        },
                        {.shader = &m_shader});
                });

                return ControlFlow::Continue;
            });

            m_world.forBlocks([&](const ObjectId objectId, const Block& block)
            {
                sf::Vec2f drawPositionOffset   = {0.f, 0.f};
                sf::Vec2f scaleMultiplier      = {1.f, 1.f};
                float     rotationRadians      = 0.f;
                float     arrowRotationRadians = 0.f;

                forTurnEventsToProcess([&](TurnEvent& turnEvent)
                {
                    if (const auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
                    {
                        if (moveBlock->objectId == objectId)
                        {
                            const auto worldOffset = moveBlock->newPosition.toVec2f() - block.position.toVec2f();
                            drawPositionOffset     = easeInOutBack(moveBlock->progress) * worldOffset * 64.f;
                        }
                    }
                    else if (const auto* fallBlock = turnEvent.get_if<TEFallBlock>())
                    {
                        if (fallBlock->objectId == objectId)
                        {
                            const auto worldOffset = fallBlock->newPosition.toVec2f() - block.position.toVec2f();
                            drawPositionOffset     = easeInBack(fallBlock->progress) * worldOffset * 64.f;
                        }
                    }
                    else if (const auto* squishBlock = turnEvent.get_if<TESquishBlock>())
                    {
                        if (squishBlock->objectId == objectId)
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
                        if (kill->objectId == objectId)
                        {
                            scaleMultiplier *= 1.f - easeInOutBack(kill->progress);
                            rotationRadians = easeInOutSine(kill->progress) * sf::base::tau;
                        }
                    }
                    else if (const auto* rotateGravityDir = turnEvent.get_if<TERotateGravityDir>())
                    {
                        if (rotateGravityDir->objectId == objectId)
                        {
                            arrowRotationRadians = easeInOutSine(rotateGravityDir->progress) * sf::base::halfPi;

                            scaleMultiplier.x += 0.35f * easeInOutSine(bounce(rotateGravityDir->progress));
                            scaleMultiplier.y += 0.35f * easeInOutSine(bounce(rotateGravityDir->progress));
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
                    m_window.draw(
                        sf::RectangleShapeData{
                            .position         = drawPosition,
                            .origin           = {32.f, 32.f},
                            .fillColor        = sf::Color::Gray,
                            .outlineColor     = sf::Color::LightGray,
                            .outlineThickness = 4.f,
                            .size             = {64.f, 64.f},
                        },
                        {.shader = &m_shader});
                },
                    [&](const BColored& bColored)
                {
                    m_window.draw(
                        sf::RoundedRectangleShapeData{
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
                        },
                        {.shader = &m_shader});


                    if (true || block.fixed)
                        m_window.draw(m_txFixed,
                                      {
                                          .position = drawPosition + sf::Vec2f{16.f, -16.f},
                                          .scale    = scaleMultiplier * 0.3f,
                                          .origin   = {32.f, 32.f},
                                          .rotation = sf::radians(rotationRadians).wrapUnsigned(),
                                          .color    = hueColor(bColored.color.toHSL().hue, 255u),
                                      },
                                      {.shader = &m_shader});

                    if (true || block.locked)
                        m_window.draw(m_txLocked,
                                      {
                                          .position = drawPosition + sf::Vec2f{16.f, 16.f},
                                          .scale    = scaleMultiplier * 0.3f,
                                          .origin   = {32.f, 32.f},
                                          .rotation = sf::radians(rotationRadians).wrapUnsigned(),
                                          .color    = hueColor(bColored.color.toHSL().hue, 255u),
                                      },
                                      {.shader = &m_shader});

                    if (block.gravityDir != sf::Vec2i{0, 0})
                        m_window.draw(
                            sf::ArrowShapeData{
                                .position = drawPosition + sf::Vec2f{-16.f, -16.f},
                                .scale    = scaleMultiplier.rotatedBy(sf::degrees(90.f)) * 0.55f,
                                .origin   = {12.f * sf::base::fabs(scaleMultiplier.x), 0.f},
                                .rotation = sf::radians(block.gravityDir.toVec2f().angle().asRadians() +
                                                        rotationRadians + arrowRotationRadians + sf::base::pi)
                                                .wrapUnsigned(),
                                .fillColor        = bColored.color.withLightness(0.35f),
                                .outlineColor     = sf::Color::White,
                                .outlineThickness = 1.f * sf::base::fabs(scaleMultiplier.x),
                                .shaftLength      = 12.f * sf::base::fabs(scaleMultiplier.x),
                                .shaftWidth       = 10.f * sf::base::fabs(scaleMultiplier.x),
                                .headLength       = 12.f * sf::base::fabs(scaleMultiplier.x),
                                .headWidth        = 20.f * sf::base::fabs(scaleMultiplier.x),
                            },
                            {.shader = &m_shader});
                });


                if (false)
                    m_window.draw(m_font,
                                  sf::TextData{
                                      .position         = drawPosition + sf::Vec2f{8.f, 2.f} - sf::Vec2f{32.f, 32.f},
                                      .string           = std::to_string(objectId),
                                      .characterSize    = 18u,
                                      .fillColor        = sf::Color::White,
                                      .outlineColor     = sf::Color::Black,
                                      .outlineThickness = 2.f,
                                  },
                                  {.shader = &m_shader});

                return ControlFlow::Continue;
            });

            if (m_grabbedObjectId.hasValue())
                m_window.draw(m_font,
                              sf::TextData{
                                  .position         = {100, 100},
                                  .string           = std::to_string(*m_grabbedObjectId),
                                  .characterSize    = 18u,
                                  .fillColor        = sf::Color::White,
                                  .outlineColor     = sf::Color::Black,
                                  .outlineThickness = 2.f,
                              },
                              {.shader = &m_shader});

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
