#include "../bubble_idle/ControlFlow.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"    // TODO P1: avoid the relative path...?

#include "SFML/ImGui/ImGuiContext.hpp"

#include "SFML/Graphics/ArrowShapeData.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/CurvedArrowShapeData.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RoundedRectangleShapeData.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/TextData.hpp"
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

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Fmod.hpp"
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
constexpr sf::Color colorRed   = sf::Color::Red;
constexpr sf::Color colorGreen = sf::Color::Mint;
constexpr sf::Color colorBlue  = sf::Color::Blue;


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
struct BKey
{
    sf::Color color;
};


////////////////////////////////////////////////////////////
using BlockType = sfvr::tinyvariant<BWall, BColored, BKey>;


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
    sf::Vec2i                     position;
    BlockType                     type;
    sf::Vec2i                     gravityDir = {0, 0};
    bool                          fixed      = false;
    sf::base::Optional<sf::Color> locked;
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
    Object& addWall(const sf::Vec2i position)
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Block{
                                          .position = position,
                                          .type     = BlockType{BWall{}},
                                      });
    }

    ////////////////////////////////////////////////////////////
    Object& addColored(const sf::Vec2i position, const sf::Color color, const sf::Vec2i gravityDir = {0, 0})
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Block{
                                          .position   = position,
                                          .type       = BlockType{BColored{.color = color}},
                                          .gravityDir = gravityDir,
                                      });
    }

    ////////////////////////////////////////////////////////////
    Object& addKey(const sf::Vec2i position, const sf::Color color, const sf::Vec2i gravityDir = {0, 0})
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Block{
                                          .position   = position,
                                          .type       = BlockType{BKey{.color = color}},
                                          .gravityDir = gravityDir,
                                      });
    }

    ////////////////////////////////////////////////////////////
    Object& addGravityRotator(const sf::Vec2i position, const bool clockwise)
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
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
    [[nodiscard]] bool isWall(const sf::Vec2i position) const
    {
        for (const sf::base::Optional<Object>& object : m_objects)
        {
            if (!object.hasValue() || !object->is<Block>() || !object->as<Block>().type.is<BWall>())
                continue;

            if (object->as<Block>().position == position)
                return true;
        }

        return false;
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
    [[nodiscard]] sf::base::Optional<ObjectId> getTileByPosition(const sf::Vec2i position) const
    {
        for (ObjectId i = 0u; i < m_objects.size(); ++i)
        {
            const auto& object = m_objects[i];

            if (object.hasValue() && object->is<Tile>() && object->as<Tile>().position == position)
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
    void killObject(const ObjectId objectId)
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
struct TEUnlock
{
    ObjectId objectId;
    float    progress{0.f};
};


////////////////////////////////////////////////////////////
using TurnEvent = sfvr::tinyvariant<TEMoveBlock, TEFallBlock, TESquishBlock, TEKill, TERotateGravityDir, TEUnlock>;


////////////////////////////////////////////////////////////
class Game
{
private:
    //////////////////////////////////////////////////////////////
    const unsigned int m_aaLevel = sf::base::min(16u, sf::RenderTexture::getMaximumAntiAliasingLevel());

    ////////////////////////////////////////////////////////////
    sf::RenderWindow m_window = makeDPIScaledRenderWindow(
        {.size            = resolution.toVec2u(),
         .title           = "Block Puzzle",
         .resizable       = true,
         .vsync           = true,
         .frametimeLimit  = 144u,
         .contextSettings = {.antiAliasingLevel = m_aaLevel}});

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
    sf::ImGuiContext m_imGuiContext;

    ////////////////////////////////////////////////////////////
    sf::Clock m_fpsClock;

    ////////////////////////////////////////////////////////////
    World                               m_world;
    sf::base::Optional<sf::base::SizeT> m_grabbedObjectId;
    sf::base::Vector<TurnEvent>         m_turnEvents;

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    /////////////////////////////////////////////////////////////
    const sf::Cursor m_cursorArrow      = sf::Cursor::loadFromSystem(sf::Cursor::Type::Arrow).value();
    const sf::Cursor m_cursorHand       = sf::Cursor::loadFromSystem(sf::Cursor::Type::Hand).value();
    const sf::Cursor m_cursorNotAllowed = sf::Cursor::loadFromSystem(sf::Cursor::Type::NotAllowed).value();

    //////////////////////////////////////////////////////////////
    int       m_editorSelectedObjectIdx      = 0;
    int       m_editorSelectedColorIdx       = 0;
    int       m_editorSelectedLockedColorIdx = 0;
    sf::Vec2i m_editorGravity;
    bool      m_editorObjectFixed     = false;
    bool      m_editorObjectLocked    = false;
    bool      m_editorObjectClockwise = false;

    //////////////////////////////////////////////////////////////
    sf::TextureAtlas m_textureAtlas{sf::Texture::create({4096u, 4096u}, {.smooth = true}).value()};

    ////////////////////////////////////////////////////////////
    const sf::FloatRect m_txrWhiteDotTrue = m_textureAtlas.add(sf::GraphicsContext::getBuiltInWhiteDotTexture()).value();
    const sf::FloatRect m_txrWhiteDot   = {{0.f, 0.f}, {1.f, 1.f}};
    const sf::FloatRect m_txrGrid       = addImgResourceToAtlas("grid.png");
    const sf::FloatRect m_txrFixed      = addImgResourceToAtlas("fixed.png");
    const sf::FloatRect m_txrLocked     = addImgResourceToAtlas("locked.png");
    const sf::FloatRect m_txrGravArrow  = addImgResourceToAtlas("gravarrow.png");
    const sf::FloatRect m_txrKey0       = addImgResourceToAtlas("key0.png");
    const sf::FloatRect m_txrKey1       = addImgResourceToAtlas("key1.png");
    const sf::FloatRect m_txrKey2       = addImgResourceToAtlas("key2.png");
    const sf::FloatRect m_txrKeyBg      = addImgResourceToAtlas("keybg.png");
    const sf::FloatRect m_txrBlock0     = addImgResourceToAtlas("block0.png");
    const sf::FloatRect m_txrBlock1     = addImgResourceToAtlas("block1.png");
    const sf::FloatRect m_txrBlock2     = addImgResourceToAtlas("block2.png");
    const sf::FloatRect m_txrBlockBg    = addImgResourceToAtlas("blockbg.png");
    const sf::FloatRect m_txrWall       = addImgResourceToAtlas("wall.png");
    const sf::FloatRect m_txrWallCorner = addImgResourceToAtlas("wallcorner.png");
    const sf::FloatRect m_txrWallH      = addImgResourceToAtlas("wallh.png");
    const sf::FloatRect m_txrWallV      = addImgResourceToAtlas("wallv.png");
    const sf::FloatRect m_txrWallBg     = addImgResourceToAtlas("wallbg.png");
    const sf::FloatRect m_txrWallSet    = addImgResourceToAtlas("wallset.png");
    const sf::FloatRect m_txrWallBits   = addImgResourceToAtlas("wallbits.png");

    //////////////////////////////////////////////////////////////
    sf::CPUDrawableBatch m_dbBackground;
    sf::CPUDrawableBatch m_dbTile;
    sf::CPUDrawableBatch m_dbWall;
    sf::CPUDrawableBatch m_dbObjectBg;
    sf::CPUDrawableBatch m_dbObject;

    //////////////////////////////////////////////////////////////
    sf::RenderTexture m_rtGame{
        sf::RenderTexture::create(m_window.getSize() * 2.f, {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::FloatRect addImgResourceToAtlas(const sf::Path& path)
    {
        return m_textureAtlas.add(sf::Image::loadFromFile("resources" / path).value(), /* padding */ {2u, 2u}).value();
    }

    ////////////////////////////////////////////////////////////
    void checkForKill()
    {
        sf::base::Vector<ObjectId> blocksToKill;

        m_world.forBlocks([&](const ObjectId objectId, const Block& block)
        {
            const auto* blockColored = block.type.get_if<BColored>();
            if (blockColored == nullptr || block.locked.hasValue())
                return ControlFlow::Continue;

            m_world.forOrthogonalNeighbors(objectId,
                                           [&](const ObjectId neighborObjectId)
            {
                if (neighborObjectId >= objectId)
                    return ControlFlow::Continue;

                const Block& neighborBlock        = m_world.getBlockById(neighborObjectId);
                const auto*  neighborBlockColored = neighborBlock.type.get_if<BColored>();

                if (neighborBlockColored == nullptr || neighborBlock.locked.hasValue())
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
    void checkForUnlock()
    {
        sf::base::Vector<ObjectId> blocksToKill;

        m_world.forBlocks([&](const ObjectId objectId, const Block& block)
        {
            const auto* blockKey = block.type.get_if<BKey>();
            if (blockKey == nullptr)
                return ControlFlow::Continue;

            m_world.forOrthogonalNeighbors(objectId,
                                           [&](const ObjectId neighborObjectId)
            {
                Block& neighborBlock = m_world.getBlockById(neighborObjectId);
                if (!neighborBlock.locked.hasValue())
                    return ControlFlow::Continue;

                if (neighborBlock.locked.value() != blockKey->color)
                    return ControlFlow::Continue;

                const auto* neighborBlockColored = neighborBlock.type.get_if<BColored>();
                const auto* neighborBlockKey     = neighborBlock.type.get_if<BKey>();

                if (neighborBlockColored != nullptr || neighborBlockKey != nullptr)
                {
                    m_turnEvents.pushBack(TEUnlock{.objectId = neighborObjectId});
                    blocksToKill.pushBack(objectId);
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
    void checkForFall()
    {
        for (const auto& turnEvent : m_turnEvents)
        {
            if (turnEvent.is<TEFallBlock>())
                return; // already falling, no need to check again
        }

        m_world.forBlocks([&](const ObjectId objectId, Block& block)
        {
            if (block.gravityDir == sf::Vec2i{0, 0} || block.locked.hasValue())
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

                    return ControlFlow::Break;
                }

                targetPosition += block.gravityDir;
            }

            if (mustFall)
            {
                m_turnEvents.pushBack(TEFallBlock{.objectId    = objectId,
                                                  .dir         = block.gravityDir,
                                                  .newPosition = targetPosition - block.gravityDir,
                                                  .squish      = true});

                return ControlFlow::Break;
            }

            return ControlFlow::Continue;
        });
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

    ////////////////////////////////////////////////////////////
    void resetLevel()
    {
        m_world = World{};
        m_turnEvents.clear();
        m_grabbedObjectId.reset();
        m_time = 0.f;

        for (int iX = 0; iX < 16; ++iX)
            for (int iY = 0; iY < 16; ++iY)
                if (iX == 0 || iX == 15 || iY == 0 || iY == 15)
                    m_world.addWall({iX, iY});

        m_world.addColored({4, 4}, colorRed);

        m_world.addColored({7, 4}, colorRed, {0, 1});
        m_world.addColored({6, 6}, colorRed, {0, -1});
        m_world.addColored({7, 6}, colorRed);
        m_world.addColored({3, 3}, colorRed);
        m_world.addColored({8, 8}, colorBlue);
        m_world.addColored({9, 9}, colorBlue);

        m_world.addColored({3, 8}, colorBlue).as<Block>().fixed = true;

        m_world.addColored({4, 10}, colorBlue).as<Block>().locked.emplace(colorRed);
        m_world.addKey({6, 11}, colorBlue);

        // m_world.addGravityRotator({9, 10}, /* clockwise */ true);
        // m_world.addGravityRotator({2, 10}, /* clockwise */ true);
        // m_world.addGravityRotator({7, 1}, /* clockwise */ false);

        m_world.addColored({10, 10}, colorGreen);
        m_world.addColored({12, 11}, colorGreen, {-1, 0});

        m_world.addWall({6, 4});
        m_world.addWall({6, 5});
        m_world.addWall({7, 5});
        m_world.addWall({8, 5});
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool makeProgress(auto& turnEvent, const float speed)
    {
        turnEvent->progress += speed;
        return turnEvent->progress >= 1.f;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateTurnEvent(TurnEvent& turnEvent, const float deltaTimeMs)
    {
        if (auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
        {
            if (!makeProgress(moveBlock, deltaTimeMs * 0.0075f))
                return false;

            Block& grabbedBlock   = m_world.getBlockById(moveBlock->objectId);
            grabbedBlock.position = moveBlock->newPosition;

            return true;
        }

        if (auto* fallBlock = turnEvent.get_if<TEFallBlock>())
        {
            if (!makeProgress(fallBlock, deltaTimeMs * 0.0015f))
                return false;

            Block& grabbedBlock   = m_world.getBlockById(fallBlock->objectId);
            grabbedBlock.position = fallBlock->newPosition;

            if (fallBlock->squish)
                m_turnEvents.pushBack(TESquishBlock{.objectId = fallBlock->objectId, .dir = fallBlock->dir});

            return true;
        }

        if (auto* squishBlock = turnEvent.get_if<TESquishBlock>())
            return makeProgress(squishBlock, deltaTimeMs * 0.0020f);

        if (auto* kill = turnEvent.get_if<TEKill>())
        {
            if (!makeProgress(kill, deltaTimeMs * 0.0015f))
                return false;

            m_world.killObject(kill->objectId);
            m_grabbedObjectId.reset();

            return true;
        }

        if (auto* rotateGravityDir = turnEvent.get_if<TERotateGravityDir>())
        {
            if (!makeProgress(rotateGravityDir, deltaTimeMs * 0.0020f))
                return false;

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

        if (auto* unlock = turnEvent.get_if<TEUnlock>())
        {
            if (!makeProgress(unlock, deltaTimeMs * 0.0020f))
                return false;

            m_world.getBlockById(unlock->objectId).locked.reset();
            return true;
        }

        return false;
    }

    /////////////////////////////////////////////////////////////
    void updateTurnEvents(const float deltaTimeMs)
    {
        forTurnEventsToProcess([&](TurnEvent& turnEvent) { return updateTurnEvent(turnEvent, deltaTimeMs); });
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        resetLevel();

        while (true)
        {
            const bool lmbPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            const auto mousePosition = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_window.getView()).toVec2f() *
                                       2.f;
            const auto adjustedMousePosition = mousePosition + sf::Vec2f{64.f, 64.f};
            const auto worldMousePosition    = (mousePosition / 128.f).toVec2i();

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
                    if (eKeyPressed->code == sf::Keyboard::Key::R)
                    {
                        resetLevel();
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::D)
                    {
                        {
                            auto optObjectId = m_world.getBlockByPosition(worldMousePosition);
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        {
                            auto optObjectId = m_world.getTileByPosition(worldMousePosition);
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }


                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::E)
                    {
                        {
                            auto optObjectId = m_world.getBlockByPosition(worldMousePosition);
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        {
                            auto optObjectId = m_world.getTileByPosition(worldMousePosition);
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        const auto clampedGravity = m_editorGravity.componentWiseClamp({-1, -1}, {1, 1});
                        const auto colorToUse     = m_editorSelectedColorIdx == 0
                                                        ? colorRed
                                                        : (m_editorSelectedColorIdx == 1 ? colorGreen : colorBlue);

                        const auto lockColorToUse = m_editorSelectedLockedColorIdx == 0
                                                        ? colorRed
                                                        : (m_editorSelectedLockedColorIdx == 1 ? colorGreen : colorBlue);


                        if (m_editorSelectedObjectIdx == 0)
                        {
                            m_world.addWall(worldMousePosition);
                        }
                        else if (m_editorSelectedObjectIdx == 1)
                        {
                            auto& coloredBlock = m_world.addColored(worldMousePosition, colorToUse, clampedGravity);
                            coloredBlock.as<Block>().fixed = m_editorObjectFixed;

                            if (m_editorObjectLocked)
                                coloredBlock.as<Block>().locked.emplace(lockColorToUse);
                        }
                        else if (m_editorSelectedObjectIdx == 2)
                        {
                            auto& key             = m_world.addKey(worldMousePosition, colorToUse, clampedGravity);
                            key.as<Block>().fixed = m_editorObjectFixed;

                            if (m_editorObjectLocked)
                                key.as<Block>().locked.emplace(lockColorToUse);
                        }
                        else if (m_editorSelectedObjectIdx == 3)
                        {
                            m_world.addGravityRotator(worldMousePosition, m_editorObjectClockwise);
                        }
                    }
                }
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

            if (!lmbPressed)
                m_grabbedObjectId.reset();

            if (m_turnEvents.empty())
            {
                checkForKill();
                checkForUnlock();
                checkForFall();
            }

            m_world.forBlocks([&](const ObjectId objectId, const Block& block)
            {
                const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 128.f, block.position.y * 128.f} +
                                               sf::Vec2f{64.f, 64.f};

                const bool hoveredByMouse = adjustedMousePosition.x > drawPosition.x &&
                                            adjustedMousePosition.x < drawPosition.x + 128.f &&
                                            adjustedMousePosition.y > drawPosition.y &&
                                            adjustedMousePosition.y < drawPosition.y + 128.f;

                if (!block.fixed && !block.locked && !block.type.is<BWall>() && hoveredByMouse && lmbPressed &&
                    !m_grabbedObjectId.hasValue())
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

                    const bool movingDiagonally = dir.x != 0 && dir.y != 0;
                    if (!movingDiagonally)
                    {
                        if (!m_world.isBlocked(newPos))
                        {
                            if (!movingAgainstGravity)
                                m_turnEvents.pushBack(TEMoveBlock{.objectId    = *m_grabbedObjectId,
                                                                  .newPosition = grabbedBlock.position + dir});
                        }
                        else if (diff != sf::Vec2i{0, 0})
                            m_turnEvents.pushBack(TESquishBlock{.objectId = *m_grabbedObjectId, .dir = dir});
                    }
                }
            }
            else
            {
                updateTurnEvents(deltaTimeMs);
            }

            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // ImGui step
            ////////////////////////////////////////////////////////////
            // ---
            m_imGuiContext.update(m_window, deltaTime);

            ImGui::Begin("What", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            constexpr const char* objectNames[]{
                "Wall",
                "Block",
                "Key",
                "Gravity Rotator",
            };

            constexpr const char* colorNames[]{
                "Red",
                "Green",
                "Blue",
            };

            ImGui::SetNextItemWidth(160.f);
            ImGui::Combo("Object", &m_editorSelectedObjectIdx, objectNames, sf::base::getArraySize(objectNames));

            ImGui::SetNextItemWidth(160.f);
            ImGui::Combo("Color", &m_editorSelectedColorIdx, colorNames, sf::base::getArraySize(colorNames));

            ImGui::SetNextItemWidth(160.f);
            ImGui::Combo("Lock Color", &m_editorSelectedLockedColorIdx, colorNames, sf::base::getArraySize(colorNames));

            ImGui::InputInt("Gravity X", &m_editorGravity.x, 1);
            ImGui::InputInt("Gravity Y", &m_editorGravity.y, 1);

            ImGui::Checkbox("Fixed", &m_editorObjectFixed);
            ImGui::Checkbox("Locked", &m_editorObjectLocked);
            ImGui::Checkbox("Clockwise", &m_editorObjectClockwise);

            ImGui::End();
            // ---
            ////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////
            // Draw step
            ////////////////////////////////////////////////////////////
            // ---
            m_window.clear();

            m_dbBackground.clear();
            m_dbTile.clear();
            m_dbWall.clear();
            m_dbObjectBg.clear();
            m_dbObject.clear();

            m_world.forTiles([&](const ObjectId objectId, const Tile& tile)
            {
                const sf::Vec2f drawPosition = sf::Vec2f{tile.position.x * 128.f, tile.position.y * 128.f} +
                                               sf::Vec2f{64.f, 64.f};

                tile.type.linear_match([&](const TGravityRotator& gravityRotator)
                {
                    m_dbTile.add(sf::CurvedArrowShapeData{
                        .position = drawPosition,
                        .scale    = {gravityRotator.clockwise ? 0.6f : -0.6f, 0.6f},
                        .origin   = {32.f, 32.f},
                        .rotation = sf::radians(gravityRotator.clockwise ? m_time * 0.002f : -m_time * 0.002f).wrapUnsigned(),
                        .textureRect      = m_txrWhiteDot,
                        .fillColor        = sf::Color::LightYellow,
                        .outlineColor     = sf::Color::White,
                        .outlineThickness = gravityRotator.clockwise ? 2.f : -2.f,
                        .outerRadius      = 32.f,
                        .innerRadius      = 24.f,
                        .startAngle       = sf::degrees(0.f),
                        .sweepAngle       = sf::degrees(270.f),
                        .headLength       = 16.f,
                        .headWidth        = 24.f,
                    });
                });

                return ControlFlow::Continue;
            });

            const sf::Cursor* cursorToUse = &m_cursorArrow;

            m_world.forBlocks([&](const ObjectId objectId, const Block& block)
            {
                sf::Vec2f drawPositionOffset   = {0.f, 0.f};
                sf::Vec2f scaleMultiplier      = {1.f, 1.f};
                float     rotationRadians      = 0.f;
                float     arrowRotationRadians = 0.f;
                float     lockRotationRadians  = 0.f;

                forTurnEventsToProcess([&](TurnEvent& turnEvent)
                {
                    if (const auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
                    {
                        if (moveBlock->objectId != objectId)
                            return false;

                        const auto worldOffset = moveBlock->newPosition.toVec2f() - block.position.toVec2f();
                        drawPositionOffset     = easeInOutBack(moveBlock->progress) * worldOffset * 128.f;

                        const auto dir = (moveBlock->newPosition - block.position).toVec2f().abs();
                        scaleMultiplier += easeInOutSine(bounce(moveBlock->progress)) * dir * 0.25f;
                        scaleMultiplier -= easeInOutSine(bounce(moveBlock->progress)) * dir.perpendicular().abs() * 0.25f;

                        return false;
                    }

                    if (const auto* fallBlock = turnEvent.get_if<TEFallBlock>())
                    {
                        if (fallBlock->objectId != objectId)
                            return false;

                        const auto worldOffset = fallBlock->newPosition.toVec2f() - block.position.toVec2f();
                        drawPositionOffset     = easeInBack(fallBlock->progress) * worldOffset * 128.f;

                        const auto dir = (fallBlock->newPosition - block.position).toVec2f().normalized().abs();
                        scaleMultiplier += easeInOutSine(fallBlock->progress) * dir * 0.25f;
                        scaleMultiplier -= easeInOutSine(fallBlock->progress) * dir.perpendicular().abs() * 0.25f;

                        return false;
                    }

                    if (const auto* squishBlock = turnEvent.get_if<TESquishBlock>())
                    {
                        if (squishBlock->objectId != objectId)
                            return false;

                        const auto absDir = sf::Vec2f{sf::base::fabs(static_cast<float>(squishBlock->dir.x)),
                                                      sf::base::fabs(static_cast<float>(squishBlock->dir.y))};

                        const float progress = easeInOutBack(bounce(squishBlock->progress));

                        scaleMultiplier += (progress * absDir * -0.35f);
                        scaleMultiplier += (progress * sf::Vec2f{absDir.y, absDir.x} * 0.35f);

                        drawPositionOffset = progress * squishBlock->dir.toVec2f() * 32.f;

                        return false;
                    }

                    if (const auto* kill = turnEvent.get_if<TEKill>())
                    {
                        if (kill->objectId != objectId)
                            return false;

                        scaleMultiplier *= 1.f - easeInOutBack(kill->progress);
                        rotationRadians = easeInOutSine(kill->progress) * sf::base::tau;

                        return false;
                    }

                    if (const auto* rotateGravityDir = turnEvent.get_if<TERotateGravityDir>())
                    {
                        if (rotateGravityDir->objectId != objectId)
                            return false;

                        arrowRotationRadians = easeInOutSine(rotateGravityDir->progress) * sf::base::halfPi;

                        scaleMultiplier.x += 0.35f * easeInOutSine(bounce(rotateGravityDir->progress));
                        scaleMultiplier.y += 0.35f * easeInOutSine(bounce(rotateGravityDir->progress));

                        return false;
                    }

                    if (const auto* unlock = turnEvent.get_if<TEUnlock>())
                    {
                        if (unlock->objectId != objectId)
                            return false;

                        lockRotationRadians = easeInOutSine(unlock->progress) * sf::base::tau;

                        scaleMultiplier.x += 0.35f * easeInOutSine(bounce(unlock->progress));
                        scaleMultiplier.y += 0.35f * easeInOutSine(bounce(unlock->progress));

                        return false;
                    }

                    return false;
                });

                const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 128.f, block.position.y * 128.f} +
                                               drawPositionOffset + sf::Vec2f{64.f, 64.f};

                const bool hoveredByMouse = adjustedMousePosition.x > drawPosition.x &&
                                            adjustedMousePosition.x < drawPosition.x + 128.f &&
                                            adjustedMousePosition.y > drawPosition.y &&
                                            adjustedMousePosition.y < drawPosition.y + 128.f;

                const bool canInteractWithHoveredBlock = !block.fixed && !block.locked && hoveredByMouse;

                if (cursorToUse == &m_cursorArrow && canInteractWithHoveredBlock)
                    cursorToUse = &m_cursorHand;
                else if (cursorToUse == &m_cursorArrow && hoveredByMouse && !canInteractWithHoveredBlock)
                    cursorToUse = &m_cursorNotAllowed;

                const auto drawAttributes = [&](const auto& blockImpl)
                {
                    const float offset = 32.f - 4.f;

                    if (block.fixed)
                        m_dbObject.add(sf::Sprite{
                            .position    = drawPosition + sf::Vec2f{offset, -offset},
                            .scale       = scaleMultiplier * 0.4f,
                            .origin      = {64.f, 64.f},
                            .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                            .textureRect = m_txrFixed,
                            .color       = hueColor(blockImpl.color.toHSL().hue, 255u),
                        });

                    if (block.locked)
                        m_dbObject.add(sf::Sprite{
                            .position    = drawPosition + sf::Vec2f{offset, offset},
                            .scale       = scaleMultiplier * 0.4f,
                            .origin      = {64.f, 64.f},
                            .rotation    = sf::radians(rotationRadians + lockRotationRadians).wrapUnsigned(),
                            .textureRect = m_txrLocked,
                            .color       = hueColor(block.locked.value().toHSL().hue, 255u),
                        });

                    if (block.gravityDir != sf::Vec2i{0, 0})
                        m_dbObject.add(sf::Sprite{
                            .position = drawPosition + sf::Vec2f{-offset, -offset},
                            .scale    = scaleMultiplier.rotatedBy(block.gravityDir.toVec2f().abs().angle()) * 0.4f,
                            .origin   = {64.f, 64.f},
                            .rotation = sf::radians(
                                            block.gravityDir.toVec2f().componentWiseMul({-1.f, 1.f}).angle().asRadians() +
                                            rotationRadians + arrowRotationRadians)
                                            .wrapUnsigned(),
                            .textureRect = m_txrGravArrow,
                            .color       = hueColor(blockImpl.color.toHSL().hue, 255u),
                        });

                    /*
                    m_dbObject.add(sf::ArrowShapeData{
                        .position = drawPosition + sf::Vec2f{-16.f, -16.f},
                        .scale    = scaleMultiplier.rotatedBy(sf::degrees(90.f)) * 0.55f,
                        .origin   = {12.f * sf::base::fabs(scaleMultiplier.x), 0.f},
                        .rotation = sf::radians(block.gravityDir.toVec2f().angle().asRadians() + rotationRadians +
                                                arrowRotationRadians + sf::base::pi)
                                        .wrapUnsigned(),
                        .textureRect        = m_txrWhiteDot,
                        .outlineTextureRect = m_txrWhiteDot,
                        .fillColor          = blockImpl.color.withLightness(0.35f),
                        .outlineColor       = sf::Color::White,
                        .outlineThickness   = 1.f * sf::base::fabs(scaleMultiplier.x),
                        .shaftLength        = 12.f * sf::base::fabs(scaleMultiplier.x),
                        .shaftWidth         = 10.f * sf::base::fabs(scaleMultiplier.x),
                        .headLength         = 12.f * sf::base::fabs(scaleMultiplier.x),
                        .headWidth          = 20.f * sf::base::fabs(scaleMultiplier.x),
                    });
                    */
                };

                block.type.linear_match(
                    [&](const BWall&)
                {
                    unsigned int neighbors = 0u;

                    if (m_world.isWall(block.position + sf::Vec2i{0, -1}))
                        neighbors |= 1u << 0;

                    if (m_world.isWall(block.position + sf::Vec2i{-1, 0}))
                        neighbors |= 1u << 1;

                    if (m_world.isWall(block.position + sf::Vec2i{1, 0}))
                        neighbors |= 1u << 2;

                    if (m_world.isWall(block.position + sf::Vec2i{0, 1}))
                        neighbors |= 1u << 3;

                    unsigned int tileX = neighbors % 4u;
                    unsigned int tileY = neighbors / 4u;

                    const sf::FloatRect txr{
                        .position = m_txrWallSet.position + sf::Vec2f{tileX * 128.f, tileY * 128.f},
                        .size     = {128.f, 128.f},
                    };

                    m_dbWall.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = txr,
                        .color       = sf::Color::White,
                    });

                    const auto patchCorner = [&](const float index)
                    {
                        m_dbWall.add(sf::Sprite{
                            .position    = drawPosition,
                            .scale       = scaleMultiplier,
                            .origin      = {64.f, 64.f},
                            .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                            .textureRect = {m_txrWallBits.position + sf::Vec2f{128.f * index, 0.f}, {128.f, 128.f}},
                            .color       = sf::Color::White,
                        });
                    };

                    if (!m_world.isWall(block.position + sf::Vec2i{-1, -1}) && (neighbors & 1u << 1) &&
                        (neighbors & 1u << 0))
                        patchCorner(0.f);

                    if (!m_world.isWall(block.position + sf::Vec2i{1, -1}) && (neighbors & 1u << 2) &&
                        (neighbors & 1u << 0))
                        patchCorner(1.f);

                    if (!m_world.isWall(block.position + sf::Vec2i{1, 1}) && (neighbors & 1u << 2) && (neighbors & 1u << 3))
                        patchCorner(2.f);

                    if (!m_world.isWall(block.position + sf::Vec2i{-1, 1}) && (neighbors & 1u << 1) &&
                        (neighbors & 1u << 3))
                        patchCorner(3.f);
                },
                    [&](const BColored& bColored)
                {
                    m_dbObjectBg.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = m_txrBlockBg,
                    });

                    const sf::FloatRect* txr = bColored.color == colorRed
                                                   ? &m_txrBlock0
                                                   : (bColored.color == colorGreen ? &m_txrBlock1 : &m_txrBlock2);

                    m_dbObject.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = *txr,
                        .color       = hueColor(bColored.color.toHSL().hue, 255u),
                    });

                    drawAttributes(bColored);
                },
                    [&](const BKey& bKey)
                {
                    m_dbObjectBg.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = m_txrKeyBg,
                    });

                    const sf::FloatRect* txr = bKey.color == colorRed
                                                   ? &m_txrKey0
                                                   : (bKey.color == colorGreen ? &m_txrKey1 : &m_txrKey2);

                    m_dbObject.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = *txr,
                        .color       = hueColor(bKey.color.toHSL().hue, 255u),
                    });

                    drawAttributes(bKey);
                });

                return ControlFlow::Continue;
            });

            m_window.setMouseCursor(*cursorToUse);

            const sf::RenderStates states{
                .texture = &m_textureAtlas.getTexture(),
                .shader  = &m_shader,
            };

            for (int iX = 0; iX < 8; ++iX)
                for (int iY = 0; iY < 8; ++iY)
                    m_dbBackground.add(sf::Sprite{
                        .position    = {iX * 256.f, iY * 256.f},
                        .textureRect = m_txrGrid,
                        .color       = sf::Color::White,
                    });

            m_rtGame.clear();
            m_rtGame.draw(m_dbBackground, states);
            m_rtGame.draw(m_dbWall, states);
            m_rtGame.draw(m_dbTile, states);
            m_rtGame.draw(m_dbObjectBg, states);
            m_rtGame.draw(m_dbObject, states);


            m_rtGame.draw(sf::RectangleShapeData{
                .position  = mousePosition,
                .fillColor = sf::Color::Black,
                .size      = {20.f, 20.f},
            });

            m_rtGame.display();

            m_window.clear();
            m_window.draw(m_rtGame.getTexture(),
                          {
                              .scale = {0.5f, 0.5f},
                              .color = hueColor(sf::base::fmod(m_time * 0.01f, 360.f), 255u),
                          },
                          {.shader = &m_shader});
            m_imGuiContext.render(m_window);
            m_window.display();
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
