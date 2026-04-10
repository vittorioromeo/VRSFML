#pragma once

#include "ShowcaseExample.hpp"

#include "ExampleUtils/RNGFast.hpp"

#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/VAOHandle.hpp"
#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class TextureAtlas;
} // namespace sf


////////////////////////////////////////////////////////////
class ExampleBunnyMark : public ShowcaseExample
{
private:
    ////////////////////////////////////////////////////////////
    struct Bunny
    {
        sf::Vec2f position;
        sf::Vec2f velocity;
        sf::Angle rotation;
        float     scale{};
    };

    ////////////////////////////////////////////////////////////
    // Per-instance data for instanced rendering
    ////////////////////////////////////////////////////////////
    struct BunnyInstanceData // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        sf::Vec2f position;
        float     scale;
        float     rotation;
        sf::Vec2f texRectPos;
        sf::Vec2f texRectSize;
    };

    ////////////////////////////////////////////////////////////
    enum class DrawMode
    {
        Normal,
        Instanced,
    };

    ////////////////////////////////////////////////////////////
    GameDependencies  m_deps;
    sf::TextureAtlas& m_textureAtlas;

    ////////////////////////////////////////////////////////////
    const sf::Rect2f m_bunnyTextureRects[8];

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<Bunny>             m_bunnies;
    sf::base::Vector<sf::Sprite>        m_sprites;
    sf::base::Vector<BunnyInstanceData> m_instanceData;
    sf::base::SizeT                     m_bunnyTargetCount = 100'000u;

    ////////////////////////////////////////////////////////////
    RNGFast  m_rng{/* seed */ 1234};
    DrawMode m_drawMode{DrawMode::Normal};

    ////////////////////////////////////////////////////////////
    // Instanced rendering resources
    ////////////////////////////////////////////////////////////
    sf::base::Optional<sf::Shader>                  m_instancedShader;
    sf::base::Optional<sf::Shader::UniformLocation> m_ulInvTexSize;
    sf::VAOHandle                                   m_vaoHandle;
    sf::VBOHandle                                   m_instanceVBO;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f              addImgToAtlasWithRotatedHue(const sf::Path& path, float hueDegrees);
    [[nodiscard]] static sf::base::String toDigitSeparatedString(sf::base::SizeT value);

    ////////////////////////////////////////////////////////////
    void drawInstanced();

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleBunnyMark(const GameDependencies& deps, sf::TextureAtlas& textureAtlas);

    ////////////////////////////////////////////////////////////
    void update(float deltaTimeMs) override;

    ////////////////////////////////////////////////////////////
    void imgui() override;

    ////////////////////////////////////////////////////////////
    void draw() override;
};
