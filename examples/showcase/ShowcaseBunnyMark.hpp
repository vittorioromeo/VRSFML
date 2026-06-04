#pragma once

#include "ShowcaseExample.hpp"

#include "ExampleUtils/RNGFast.hpp"

#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/Shader.hpp"
#include "Zancle/Graphics/Sprite.hpp"
#include "Zancle/Graphics/VAOHandle.hpp"
#include "Zancle/Graphics/VBOHandle.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class TextureAtlas;
} // namespace za


////////////////////////////////////////////////////////////
class ExampleBunnyMark : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    struct Bunny
    {
        za::Vec2f position;
        za::Vec2f velocity;
        za::Angle rotation;
        float     scale{};
    };

    ////////////////////////////////////////////////////////////
    // Per-instance data for instanced rendering
    ////////////////////////////////////////////////////////////
    struct BunnyInstanceData // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        za::Vec2f position;
        float     scale;
        float     rotation;
        za::Vec2f texRectPos;
        za::Vec2f texRectSize;
    };

    ////////////////////////////////////////////////////////////
    enum class DrawMode
    {
        Normal,
        Instanced,
    };

    ////////////////////////////////////////////////////////////
    GameDependencies  m_deps;
    za::TextureAtlas& m_textureAtlas;

    ////////////////////////////////////////////////////////////
    const za::Rect2f m_bunnyTextureRects[8];

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    ////////////////////////////////////////////////////////////
    zb::Vector<Bunny>             m_bunnies;
    zb::Vector<za::Sprite>        m_sprites;
    zb::Vector<BunnyInstanceData> m_instanceData;
    zb::SizeT                     m_bunnyTargetCount = 100'000u;

    ////////////////////////////////////////////////////////////
    RNGFast  m_rng{/* seed */ 1234};
    DrawMode m_drawMode{DrawMode::Normal};
    float    m_scaleMultiplier{1.f};

    ////////////////////////////////////////////////////////////
    // Instanced rendering resources
    ////////////////////////////////////////////////////////////
    zb::Optional<za::Shader>                  m_instancedShader;
    zb::Optional<za::Shader::UniformLocation> m_ulInvTexSize;
    za::VAOHandle                             m_vaoHandle;
    za::VBOHandle                             m_instanceVBO;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] za::Rect2f        addImgToAtlasWithRotatedHue(const za::Path& path, float hueDegrees);
    [[nodiscard]] static zb::String toDigitSeparatedString(zb::SizeT value);

    ////////////////////////////////////////////////////////////
    void drawInstanced();

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleBunnyMark(const GameDependencies& deps, za::TextureAtlas& textureAtlas);

    ////////////////////////////////////////////////////////////
    void update(float deltaTimeMs) override;

    ////////////////////////////////////////////////////////////
    void imgui() override;

    ////////////////////////////////////////////////////////////
    void draw() override;
};
