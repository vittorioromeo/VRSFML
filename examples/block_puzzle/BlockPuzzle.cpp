#include "../bubble_idle/ControlFlow.hpp" // TODO P1: avoid the relative path...?
#include "../bubble_idle/Easing.hpp"      // TODO P1: avoid the relative path...?
#include "../bubble_idle/HueColor.hpp"    // TODO P1: avoid the relative path...?
#include "../bubble_idle/MathUtils.hpp"   // TODO P1: avoid the relative path...?
#include "../bubble_idle/RNGFast.hpp"     // TODO P1: avoid the relative path...?

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
#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Constants.hpp"
#include "SFML/Base/Math/Fmod.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include "ExampleUtils.hpp"

#include <algorithm>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <initializer_list>

// TODO P0:
// - keys should have different colors compared to blocks
// - kill blocks in lava
// - portals
// - 4x1 surround kill

namespace
{
////////////////////////////////////////////////////////////
constexpr sf::Vec2f resolution{1024.f, 768.f};


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr float bounce(const float value) noexcept
{
    // return 4.f * value * (1.f - value);
    return 1.f - sf::base::fabs(value - 0.5f) * 2.f;
}


////////////////////////////////////////////////////////////
enum class BlockKind : sf::base::U8
{
    A,
    B,
    C,
};


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] constexpr sf::Color kindToColor(const BlockKind kind) noexcept
{
    if (kind == BlockKind::A)
        return sf::Color::Red;

    if (kind == BlockKind::B)
        return sf::Color::Green;

    SFML_BASE_ASSERT(kind == BlockKind::C);
    return sf::Color::Blue;
}


////////////////////////////////////////////////////////////
struct BWall
{
};


////////////////////////////////////////////////////////////
struct BColored
{
    BlockKind kind;
};


////////////////////////////////////////////////////////////
struct BKey
{
    BlockKind kind;
};


////////////////////////////////////////////////////////////
struct BPadlock
{
};


////////////////////////////////////////////////////////////
using BlockType = sfvr::tinyvariant<BWall, BColored, BKey, BPadlock>;


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
    sf::base::Optional<BlockKind> locked     = {};
};


////////////////////////////////////////////////////////////
using ObjectId = sf::base::SizeT;


////////////////////////////////////////////////////////////
struct TGravityRotator
{
    bool clockwise{false};
};


////////////////////////////////////////////////////////////
struct TLava
{
};


////////////////////////////////////////////////////////////
using TileType = sfvr::tinyvariant<TGravityRotator, TLava>;


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
    Object& addColored(const sf::Vec2i position, const BlockKind kind, const sf::Vec2i gravityDir = {0, 0})
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Block{
                                          .position   = position,
                                          .type       = BlockType{BColored{.kind = kind}},
                                          .gravityDir = gravityDir,
                                      });
    }

    ////////////////////////////////////////////////////////////
    Object& addKey(const sf::Vec2i position, const BlockKind kind, const sf::Vec2i gravityDir = {0, 0})
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Block{
                                          .position   = position,
                                          .type       = BlockType{BKey{.kind = kind}},
                                          .gravityDir = gravityDir,
                                      });
    }

    ////////////////////////////////////////////////////////////
    Object& addPadlock(const sf::Vec2i position, const sf::Vec2i gravityDir = {0, 0})
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Block{
                                          .position   = position,
                                          .type       = BlockType{BPadlock{}},
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
    Object& addLava(const sf::Vec2i position)
    {
        return *m_objects.emplaceBack(sf::base::inPlace,
                                      Tile{
                                          .position = position,
                                          .type     = TileType{TLava{}},
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
    [[nodiscard]] bool isOOB(const sf::Vec2i position) const
    {
        return position.x < 0 || position.x >= 12 || position.y < 0 || position.y >= 12;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isLava(const sf::Vec2i position) const
    {
        for (const sf::base::Optional<Object>& object : m_objects)
        {
            if (!object.hasValue() || !object->is<Tile>() || !object->as<Tile>().type.is<TLava>())
                continue;

            if (object->as<Tile>().position == position)
                return true;
        }

        return false;
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
        if (isOOB(position))
            return true;

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

    sf::Shader::UniformLocation m_ulTime        = m_shader.getUniformLocation("u_time").value();
    sf::Shader::UniformLocation m_ulWaveEnabled = m_shader.getUniformLocation("u_waveEnabled").value();

    ////////////////////////////////////////////////////////////
    sf::Shader m_shaderSpriteAlpha{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/spritealpha.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    ////////////////////////////////////////////////////////////
    sf::Shader m_shaderBlurQuad{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/blurquad.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    sf::Shader::UniformLocation m_ulBlurQuadBlurDirection = m_shaderBlurQuad.getUniformLocation("u_blurDirection").value();
    sf::Shader::UniformLocation m_ulBlurQuadRadiusPixels = m_shaderBlurQuad.getUniformLocation("u_blurRadiusPixels").value();

    ////////////////////////////////////////////////////////////
    sf::Shader m_shaderShadow{[]
    {
        auto result = sf::Shader::loadFromFile({.fragmentPath = "resources/shadow.frag"}).value();
        result.setUniform(result.getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);
        return result;
    }()};

    sf::Shader::UniformLocation m_ulShadowTexture = m_shaderShadow.getUniformLocation("sf_u_texture").value();
    sf::Shader::UniformLocation m_ulShadowColor   = m_shaderShadow.getUniformLocation("u_shadowColor").value();

    ////////////////////////////////////////////////////////////
    const sf::Font m_font = sf::Font::openFromFile("resources/Born2bSportyFS.ttf").value();

    ////////////////////////////////////////////////////////////
    sf::ImGuiContext m_imGuiContext;

    ////////////////////////////////////////////////////////////
    sf::Clock m_fpsClock;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<World>             m_prevWorlds;
    World                               m_world;
    sf::base::Optional<sf::base::SizeT> m_grabbedObjectId;
    sf::base::Vector<TurnEvent>         m_turnEvents;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<ParticleData> m_lavaParticles;

    ////////////////////////////////////////////////////////////
    RNGFast m_rngFast; // very fast, low-quality, but good enough for VFXs

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
    const sf::FloatRect m_txrWhiteDot     = {{0.f, 0.f}, {1.f, 1.f}};
    const sf::FloatRect m_txrGrid         = addImgResourceToAtlas("grid.png");
    const sf::FloatRect m_txrGrid0        = addImgResourceToAtlas("grid0.png");
    const sf::FloatRect m_txrGrid1        = addImgResourceToAtlas("grid1.png");
    const sf::FloatRect m_txrFixed        = addImgResourceToAtlas("fixed.png");
    const sf::FloatRect m_txrLocked       = addImgResourceToAtlas("locked.png");
    const sf::FloatRect m_txrGravArrow    = addImgResourceToAtlas("gravarrow.png");
    const sf::FloatRect m_txrKey0         = addImgResourceToAtlas("key0.png");
    const sf::FloatRect m_txrKey1         = addImgResourceToAtlas("key1.png");
    const sf::FloatRect m_txrKey2         = addImgResourceToAtlas("key2.png");
    const sf::FloatRect m_txrKeyBg        = addImgResourceToAtlas("keybg.png");
    const sf::FloatRect m_txrBlock0       = addImgResourceToAtlas("block0.png");
    const sf::FloatRect m_txrBlock1       = addImgResourceToAtlas("block1.png");
    const sf::FloatRect m_txrBlock2       = addImgResourceToAtlas("block2.png");
    const sf::FloatRect m_txrBlockBg      = addImgResourceToAtlas("blockbg.png");
    const sf::FloatRect m_txrWall         = addImgResourceToAtlas("wall.png");
    const sf::FloatRect m_txrWallCorner   = addImgResourceToAtlas("wallcorner.png");
    const sf::FloatRect m_txrWallH        = addImgResourceToAtlas("wallh.png");
    const sf::FloatRect m_txrWallV        = addImgResourceToAtlas("wallv.png");
    const sf::FloatRect m_txrWallBg       = addImgResourceToAtlas("wallbg.png");
    const sf::FloatRect m_txrWallSet      = addImgResourceToAtlas("wallset.png");
    const sf::FloatRect m_txrWallBits     = addImgResourceToAtlas("wallbits.png");
    const sf::FloatRect m_txrLavaParticle = addImgResourceToAtlas("lavaparticle.png");
    const sf::FloatRect m_txrLock0        = addImgResourceToAtlas("lock0.png");
    const sf::FloatRect m_txrPinned       = addImgResourceToAtlas("pinned.png");

    //////////////////////////////////////////////////////////////
    sf::Texture m_txLava = sf::Texture::loadFromFile("resources/lava.png", {.smooth = true}).value();

    //////////////////////////////////////////////////////////////
    sf::CPUDrawableBatch m_dbBackground;
    sf::CPUDrawableBatch m_dbLavaParticles;
    sf::CPUDrawableBatch m_dbTile;
    sf::CPUDrawableBatch m_dbWall;
    sf::CPUDrawableBatch m_dbObjectBg;
    sf::CPUDrawableBatch m_dbObject;
    sf::CPUDrawableBatch m_dbObjectAttributes;

    //////////////////////////////////////////////////////////////
    sf::RenderTexture m_rtSpriteBg{
        sf::RenderTexture::create(m_window.getSize() * 2.f, {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};

    sf::RenderTexture m_rtSpriteBgTemp{
        sf::RenderTexture::create(m_window.getSize() * 2.f, {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};

    sf::RenderTexture m_rtGame{
        sf::RenderTexture::create(m_window.getSize() * 2.f, {.antiAliasingLevel = 0u, .sRgbCapable = false}).value()};

    //////////////////////////////////////////////////////////////
    sf::Texture m_txUndo        = sf::Texture::create(m_rtGame.getSize(), {.smooth = true}).value();
    float       m_undoCountdown = 0.f;

    ////////////////////////////////////////////////////////////
    int m_moves = 0;

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
            if (blockColored == nullptr || isLocked(block))
                return ControlFlow::Continue;

            m_world.forOrthogonalNeighbors(objectId,
                                           [&](const ObjectId neighborObjectId)
            {
                if (neighborObjectId >= objectId)
                    return ControlFlow::Continue;

                const Block& neighborBlock        = m_world.getBlockById(neighborObjectId);
                const auto*  neighborBlockColored = neighborBlock.type.get_if<BColored>();

                if (neighborBlockColored == nullptr || isLocked(neighborBlock))
                    return ControlFlow::Continue;

                if (blockColored->kind == neighborBlockColored->kind)
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
                if (!isLocked(neighborBlock))
                    return ControlFlow::Continue;

                if (neighborBlock.locked.value() != blockKey->kind)
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
            if (block.gravityDir == sf::Vec2i{0, 0} || isLocked(block))
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

        for (int iX = 0; iX < 12; ++iX)
            for (int iY = 0; iY < 12; ++iY)
                if (iX == 0 || iX == 11 || iY == 0 || iY == 11)
                    m_world.addWall({iX, iY});

        m_world.addColored({4, 4}, BlockKind::A);

        m_world.addColored({7, 4}, BlockKind::A, {0, 1});
        m_world.addColored({6, 6}, BlockKind::A, {0, -1});
        m_world.addColored({7, 6}, BlockKind::A);
        m_world.addColored({3, 3}, BlockKind::A);
        m_world.addColored({8, 8}, BlockKind::B);
        m_world.addColored({9, 9}, BlockKind::B);

        m_world.addColored({3, 8}, BlockKind::B).as<Block>().fixed = true;

        m_world.addColored({4, 10}, BlockKind::B).as<Block>().locked.emplace(BlockKind::A);
        m_world.addKey({6, 10}, BlockKind::B);

        // m_world.addPadlock({8, 8});
        m_world.addPadlock({8, 10});

        // m_world.addGravityRotator({9, 10}, /* clockwise */ true);
        // m_world.addGravityRotator({2, 10}, /* clockwise */ true);
        // m_world.addGravityRotator({7, 1}, /* clockwise */ false);

        m_world.addLava({2, 9});
        m_world.addLava({2, 10});
        m_world.addLava({3, 9});

        m_world.addColored({10, 10}, BlockKind::C);
        m_world.addColored({10, 6}, BlockKind::C, {-1, 0});

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
    void finishTurn()
    {
        m_prevWorlds.emplaceBack(m_world);
        ++m_moves;
    }

    /////////////////////////////////////////////////////////////
    void undoTurn()
    {
        if (m_prevWorlds.empty())
            return;

        if (!m_txUndo.update(m_rtGame.getTexture()))
            sf::base::abort();

        m_undoCountdown = 1.f;

        m_world = SFML_BASE_MOVE(m_prevWorlds.back());
        m_prevWorlds.popBack();

        m_turnEvents.clear();
        m_grabbedObjectId.reset();

        --m_moves;
    }

    /////////////////////////////////////////////////////////////
    [[nodiscard]] bool updateTurnEvent(TurnEvent& turnEvent, const float deltaTimeMs)
    {
        if (auto* moveBlock = turnEvent.get_if<TEMoveBlock>())
        {
            if (!makeProgress(moveBlock, deltaTimeMs * 0.0075f))
                return false;

            finishTurn();

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

            const int mult = rotateGravityDir->clockwise ? 1 : -1;

            m_world.getBlockById(rotateGravityDir->objectId)
                .gravityDir = sf::Vec2i{m_world.getBlockById(rotateGravityDir->objectId).gravityDir.y * -mult,
                                        m_world.getBlockById(rotateGravityDir->objectId).gravityDir.x * mult};

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

    ////////////////////////////////////////////////////////////
    void updateTurn(const float deltaTimeMs)
    {
        if (!m_turnEvents.empty())
        {
            forTurnEventsToProcess([&](TurnEvent& turnEvent) { return updateTurnEvent(turnEvent, deltaTimeMs); });
            return;
        }

        checkForKill();

        if (!m_turnEvents.empty())
            return;

        checkForUnlock();

        if (!m_turnEvents.empty())
            return;

        checkForFall();

        if (!m_turnEvents.empty())
            return;

        m_world.forBlocks([&](const ObjectId objectId, const Block& block)
        {
            const sf::Vec2f drawPosition = sf::Vec2f{block.position.x * 128.f, block.position.y * 128.f} +
                                           sf::Vec2f{64.f, 64.f};

            const bool hoveredByMouse = getAdjustedMousePos().x > drawPosition.x &&
                                        getAdjustedMousePos().x < drawPosition.x + 128.f &&
                                        getAdjustedMousePos().y > drawPosition.y &&
                                        getAdjustedMousePos().y < drawPosition.y + 128.f;

            if (!block.fixed && !block.locked && !block.type.is<BWall>() && hoveredByMouse && isLMBPressed() &&
                !m_grabbedObjectId.hasValue())
            {
                m_grabbedObjectId.emplace(objectId);
                return ControlFlow::Break;
            }

            return ControlFlow::Continue;
        });

        if (m_grabbedObjectId.hasValue())
        {
            Block& grabbedBlock = m_world.getBlockById(*m_grabbedObjectId);

            auto            diff = getWorldMousePos() - grabbedBlock.position;
            const sf::Vec2i dir  = {diff.x < 0 ? -1 : (diff.x > 0 ? 1 : 0), diff.y < 0 ? -1 : (diff.y > 0 ? 1 : 0)};

            const auto newPos = grabbedBlock.position + dir;

            const bool movingAgainstGravity = grabbedBlock.gravityDir != sf::Vec2i{0, 0} && grabbedBlock.gravityDir == -dir;

            const bool movingDiagonally = dir.x != 0 && dir.y != 0;
            if (!movingDiagonally)
            {
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
    [[nodiscard, gnu::always_inline]] sf::Vec2f getAdjustedMousePos() const noexcept
    {
        return getMousePos() + sf::Vec2f{64.f, 64.f};
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] sf::Vec2i getWorldMousePos() const noexcept
    {
        return (getMousePos() / 128.f).toVec2i();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool isLocked(const Block& block) const noexcept
    {
        return block.locked.hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] sf::Color getHueColor(const float hue, const sf::base::U8 alpha = 255u) const noexcept
    {
        return hueColor(sf::base::fmod(m_time * 0.06f + hue, 360.f), alpha);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] sf::Color getLavaColor() const noexcept
    {
        return hueColor(-5.f, 215u);
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool run()
    {
        resetLevel();

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
                    if (eKeyPressed->code == sf::Keyboard::Key::Backspace && m_turnEvents.empty() && m_undoCountdown == 0.f)
                    {
                        undoTurn();
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::R)
                    {
                        resetLevel();
                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::D)
                    {
                        {
                            auto optObjectId = m_world.getBlockByPosition(getWorldMousePos());
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        {
                            auto optObjectId = m_world.getTileByPosition(getWorldMousePos());
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        continue;
                    }

                    if (eKeyPressed->code == sf::Keyboard::Key::E)
                    {
                        {
                            auto optObjectId = m_world.getBlockByPosition(getWorldMousePos());
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        {
                            auto optObjectId = m_world.getTileByPosition(getWorldMousePos());
                            if (optObjectId.hasValue())
                                m_world.killObject(*optObjectId);
                        }

                        const auto clampedGravity = m_editorGravity.componentWiseClamp({-1, -1}, {1, 1});
                        const auto kindToUse      = m_editorSelectedColorIdx == 0
                                                        ? BlockKind::A
                                                        : (m_editorSelectedColorIdx == 1 ? BlockKind::B : BlockKind::C);

                        const auto lockKindToUse = m_editorSelectedLockedColorIdx == 0
                                                       ? BlockKind::A
                                                       : (m_editorSelectedLockedColorIdx == 1 ? BlockKind::B : BlockKind::C);


                        if (m_editorSelectedObjectIdx == 0)
                        {
                            m_world.addWall(getWorldMousePos());
                        }
                        else if (m_editorSelectedObjectIdx == 1)
                        {
                            auto& coloredBlock = m_world.addColored(getWorldMousePos(), kindToUse, clampedGravity);
                            coloredBlock.as<Block>().fixed = m_editorObjectFixed;

                            if (m_editorObjectLocked)
                                coloredBlock.as<Block>().locked.emplace(lockKindToUse);
                        }
                        else if (m_editorSelectedObjectIdx == 2)
                        {
                            auto& key             = m_world.addKey(getWorldMousePos(), kindToUse, clampedGravity);
                            key.as<Block>().fixed = m_editorObjectFixed;

                            if (m_editorObjectLocked)
                                key.as<Block>().locked.emplace(lockKindToUse);
                        }
                        else if (m_editorSelectedObjectIdx == 3)
                        {
                            m_world.addGravityRotator(getWorldMousePos(), m_editorObjectClockwise);
                        }
                        else if (m_editorSelectedObjectIdx == 4)
                        {
                            m_world.addLava(getWorldMousePos());
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
            m_shader.setUniform(m_ulTime, m_time);

            if (!isLMBPressed())
                m_grabbedObjectId.reset();

            updateTurn(deltaTimeMs);

            m_world.forTiles([&](const ObjectId objectId, const Tile& tile)
            {
                float particleBudget = deltaTimeMs;

                tile.type.linear_match([&](const TGravityRotator&) {},
                                       [&](const TLava&)
                {
                    const auto makeLavaParticle = [&](const sf::Vec2f offset)
                    {
                        m_lavaParticles.emplaceBack(
                            ParticleData{.position   = tile.position.toVec2f() * 128.f + sf::Vec2f{64.f, 64.f} + offset,
                                         .velocity   = m_rngFast.getVec2f({-0.75f, -0.75f}, {0.75f, 0.75f}) * 0.05f,
                                         .scale      = m_rngFast.getF(0.08f, 0.27f) * 1.25f,
                                         .scaleDecay = -0.0015f,
                                         .accelerationY = 0.f,
                                         .opacity       = 0.35f,
                                         .opacityDecay  = m_rngFast.getF(0.001f, 0.002f) * 0.47f,
                                         .rotation      = m_rngFast.getF(0.f, sf::base::tau),
                                         .torque        = m_rngFast.getF(-0.001f, 0.001f)});
                    };

                    const auto makeLavaParticlePerDirection = [&](const sf::Vec2i dir)
                    {
                        if (m_rngFast.getI(0, 100) > 30)
                            return;

                        if (m_world.isOOB(tile.position + dir) || m_world.isLava(tile.position + dir) ||
                            m_world.isWall(tile.position + dir))
                            return;

                        if (particleBudget <= 0.f)
                            return;

                        const auto rndOffset = m_rngFast.getF(-64.f, 64.f);
                        const auto dirOffset = m_rngFast.getF(-2.f, 6.f);

                        if (dir.x == 0)
                            makeLavaParticle(sf::Vec2f{rndOffset, dir.y * 64.f + -dir.y * dirOffset});
                        else if (dir.y == 0)
                            makeLavaParticle(sf::Vec2f{dir.x * 64.f + -dir.x * dirOffset, rndOffset});

                        particleBudget -= deltaTimeMs * 0.35f;
                    };

                    makeLavaParticlePerDirection({1, 0});
                    makeLavaParticlePerDirection({-1, 0});
                    makeLavaParticlePerDirection({0, 1});
                    makeLavaParticlePerDirection({0, -1});

                    while (particleBudget > 0.f)
                    {
                        if (m_rngFast.getI(0, 100) > 98)
                            makeLavaParticle(sf::Vec2f{m_rngFast.getF(-64.f, 64.f), m_rngFast.getF(-64.f, 64.f)});

                        particleBudget -= deltaTimeMs * 0.35f;
                    }
                });

                return ControlFlow::Continue;
            });

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

                sf::base::vectorEraseIf(particleLikeVec,
                                        [](const auto& particleLike) { return particleLike.opacity <= 0.f; });
            };

            updateParticleLike(m_lavaParticles);

            m_undoCountdown = sf::base::max(m_undoCountdown - deltaTimeMs * 0.0065f, 0.f);
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
                "Lava",
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
            m_dbLavaParticles.clear();
            m_dbTile.clear();
            m_dbWall.clear();
            m_dbObjectBg.clear();
            m_dbObject.clear();
            m_dbObjectAttributes.clear();

            m_world.forTiles([&](const ObjectId objectId, const Tile& tile)
            {
                const sf::Vec2f drawPosition = sf::Vec2f{tile.position.x * 128.f, tile.position.y * 128.f} +
                                               sf::Vec2f{64.f, 64.f};

                tile.type.linear_match(
                    [&](const TGravityRotator& gravityRotator)
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
                },
                    [&](const TLava&) {});

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

                        const float mult     = rotateGravityDir->clockwise ? 1.f : -1.f;
                        arrowRotationRadians = easeInOutSine(rotateGravityDir->progress) * sf::base::halfPi * mult;

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

                const bool hoveredByMouse = getAdjustedMousePos().x > drawPosition.x &&
                                            getAdjustedMousePos().x < drawPosition.x + 128.f &&
                                            getAdjustedMousePos().y > drawPosition.y &&
                                            getAdjustedMousePos().y < drawPosition.y + 128.f;

                const bool canInteractWithHoveredBlock = !block.fixed && !isLocked(block) && hoveredByMouse;

                if (cursorToUse == &m_cursorArrow && canInteractWithHoveredBlock)
                    cursorToUse = &m_cursorHand;
                else if (cursorToUse == &m_cursorArrow && hoveredByMouse && !canInteractWithHoveredBlock)
                    cursorToUse = &m_cursorNotAllowed;

                const auto drawAttributes = [&](const auto& blockImpl)
                {
                    const float offset = 32.f - 6.f;

                    if (block.fixed)
                        m_dbObjectAttributes.add(sf::Sprite{
                            .position    = drawPosition,
                            .scale       = scaleMultiplier,
                            .origin      = {64.f, 64.f},
                            .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                            .textureRect = m_txrPinned,
                            .color       = getHueColor(kindToColor(blockImpl.kind).toHSL().hue),
                        });

                    if (block.gravityDir != sf::Vec2i{0, 0})
                        m_dbObjectAttributes.add(sf::Sprite{
                            .position = drawPosition,
                            .scale    = scaleMultiplier.rotatedBy(block.gravityDir.toVec2f().abs().angle()) * 0.6f,
                            .origin   = {64.f, 64.f},
                            .rotation = sf::radians(
                                            block.gravityDir.toVec2f().componentWiseMul({-1.f, 1.f}).angle().asRadians() +
                                            rotationRadians + arrowRotationRadians)
                                            .wrapUnsigned(),
                            .textureRect = m_txrGravArrow,
                            .color       = getHueColor(kindToColor(blockImpl.kind).toHSL().hue),
                        });

                    if (isLocked(block))
                        m_dbObjectAttributes.add(sf::Sprite{
                            .position    = drawPosition + sf::Vec2f{40.f, 30.f},
                            .scale       = scaleMultiplier * 0.5f,
                            .origin      = {64.f, 64.f},
                            .rotation    = sf::radians(rotationRadians + lockRotationRadians + 0.2f).wrapUnsigned(),
                            .textureRect = m_txrLock0,
                            .color       = getHueColor(kindToColor(block.locked.value()).toHSL().hue),
                        });
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
                        .color       = getHueColor(0.f),
                    });

                    const auto patchCorner = [&](const float index)
                    {
                        m_dbWall.add(sf::Sprite{
                            .position    = drawPosition,
                            .scale       = scaleMultiplier,
                            .origin      = {64.f, 64.f},
                            .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                            .textureRect = {m_txrWallBits.position + sf::Vec2f{128.f * index, 0.f}, {128.f, 128.f}},
                            .color       = getHueColor(0.f),
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
                    const sf::FloatRect* txr = bColored.kind == BlockKind::A
                                                   ? &m_txrBlock0
                                                   : (bColored.kind == BlockKind::B ? &m_txrBlock1 : &m_txrBlock2);

                    m_dbObject.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = *txr,
                        .color       = getHueColor(kindToColor(bColored.kind).toHSL().hue),
                    });

                    drawAttributes(bColored);
                },
                    [&](const BKey& bKey)
                {
                    const sf::FloatRect* txr = bKey.kind == BlockKind::A
                                                   ? &m_txrKey0
                                                   : (bKey.kind == BlockKind::B ? &m_txrKey1 : &m_txrKey2);

                    m_dbObject.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = *txr,
                        .color       = getHueColor(kindToColor(bKey.kind).toHSL().hue),
                    });

                    drawAttributes(bKey);
                },
                    [&](const BPadlock& bPadlock)
                {
                    const sf::FloatRect* txr = &m_txrLock0;

                    m_dbObject.add(sf::Sprite{
                        .position    = drawPosition,
                        .scale       = scaleMultiplier,
                        .origin      = {64.f, 64.f},
                        .rotation    = sf::radians(rotationRadians).wrapUnsigned(),
                        .textureRect = *txr,
                        .color       = getHueColor(kindToColor(BlockKind::A).toHSL().hue),
                    });

                    // drawAttributes(bPadlock);
                });

                return ControlFlow::Continue;
            });

            m_window.setMouseCursor(*cursorToUse);

            const sf::RenderStates states{
                .texture = &m_textureAtlas.getTexture(),
                .shader  = &m_shader,
            };

            for (int iX = 0; iX < 16; ++iX)
                for (int iY = 0; iY < 16; ++iY)
                {
                    if (m_world.isLava({iX, iY}))
                        continue;

                    const sf::FloatRect* txr = iX % 2 == 0 ^ iY % 2 == 0 ? &m_txrGrid1 : &m_txrGrid0;

                    m_dbBackground.add(sf::Sprite{
                        .position    = {iX * 128.f, iY * 128.f},
                        .textureRect = *txr,
                        .color       = getHueColor(0.f),
                    });
                }

            for (sf::base::SizeT i = 0; i < m_lavaParticles.size(); ++i)
                m_dbLavaParticles.add(
                    particleToSprite(m_lavaParticles[m_lavaParticles.size() - i - 1], m_txrLavaParticle, getLavaColor()));

            const auto updateShadowTexture = [&](const float blurRadius, const sf::base::U8 alpha, auto&&... toDraw)
            {
                m_rtSpriteBg.clear(sf::Color::Transparent);
                (m_rtSpriteBg.draw(toDraw, {.texture = &m_textureAtlas.getTexture(), .shader = &m_shaderSpriteAlpha}), ...);
                m_rtSpriteBg.display();

                m_shaderBlurQuad.setUniform(m_ulBlurQuadBlurDirection, sf::Vec2f{1.f, 0.f});
                m_shaderBlurQuad.setUniform(m_ulBlurQuadRadiusPixels, blurRadius);

                m_rtSpriteBgTemp.clear(sf::Color::Transparent);
                m_rtSpriteBgTemp.draw(m_rtSpriteBg.getTexture(), {.shader = &m_shaderBlurQuad});
                m_rtSpriteBgTemp.display();

                m_shaderBlurQuad.setUniform(m_ulBlurQuadBlurDirection, sf::Vec2f{0.f, 1.f});

                m_rtSpriteBg.clear(sf::Color::Transparent);
                m_rtSpriteBg.draw(m_rtSpriteBgTemp.getTexture(), {.shader = &m_shaderBlurQuad});
                m_rtSpriteBg.display();

                m_shaderShadow.setUniform(m_ulShadowColor, sf::Color::blackMask(alpha).toVec4<sf::Glsl::Vec4>());
            };


            m_rtGame.clear();
            m_shader.setUniform(m_ulWaveEnabled, true);
            m_rtGame.draw(m_txLava,
                          {.position = {0.f, 0.f}, .scale = {2.f, 2.f}, .color = getLavaColor()},
                          {.shader = &m_shader});
            m_rtGame.flush();
            m_shader.setUniform(m_ulWaveEnabled, false);
            m_rtGame.draw(m_dbBackground, states);
            m_rtGame.draw(m_dbLavaParticles,
                          {
                              .blendMode = sf::BlendAdd,
                              .texture   = &m_textureAtlas.getTexture(),
                              .shader    = &m_shader,
                          });
            m_rtGame.draw(m_dbTile, states);

            updateShadowTexture(/* blurRadius */ 10.f, /* alpha */ 128u, m_dbWall, m_dbObject);
            m_rtGame.draw(m_rtSpriteBg.getTexture(), {.position = {8.f, 8.f}}, {.shader = &m_shaderShadow});

            m_rtGame.draw(m_dbWall, states);
            m_rtGame.draw(m_dbObject, states);

            updateShadowTexture(/* blurRadius */ 5.f, /* alpha */ 196u, m_dbObjectAttributes);
            m_rtGame.draw(m_rtSpriteBg.getTexture(), {.position = {4.f, 4.f}}, {.shader = &m_shaderShadow});

            m_rtGame.draw(m_dbObjectAttributes, states);

            if (false)
                m_rtGame.draw(sf::RectangleShapeData{
                    .position  = getMousePos(),
                    .fillColor = sf::Color::Black,
                    .size      = {20.f, 20.f},
                });

            m_rtGame.display();


            m_window.clear();

            m_window.draw(m_rtGame.getTexture(),
                          {
                              .scale = {0.5f, 0.5f},
                          },
                          {.shader = &m_shader});

            if (m_undoCountdown > 0.f)
                m_window.draw(m_txUndo,
                              {
                                  .scale = {0.5f, 0.5f},
                                  .color = sf::Color::whiteMask(static_cast<sf::base::U8>(
                                      remap(easeInOutSine(m_undoCountdown), 0.f, 1.f, 0.f, 255.f))),
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
