#pragma once

#include "ShowcaseExample.hpp"

#include "ExampleUtils/RNGFast.hpp"

#include "SFML/Graphics/Image.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

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
    GameDependencies  m_deps;
    sf::TextureAtlas& m_textureAtlas;

    ////////////////////////////////////////////////////////////
    const sf::Rect2f m_bunnyTextureRects[8];

    ////////////////////////////////////////////////////////////
    float m_time = 0.f;

    ////////////////////////////////////////////////////////////
    sf::base::Vector<Bunny> m_bunnies;
    sf::base::SizeT         m_bunnyTargetCount = 100'000u;

    ////////////////////////////////////////////////////////////
    RNGFast m_rng{/* seed */ 1234};

    ////////////////////////////////////////////////////////////
    [[nodiscard]] sf::Rect2f              addImgToAtlasWithRotatedHue(const sf::Path& path, float hueDegrees);
    [[nodiscard]] static sf::base::String toDigitSeparatedString(sf::base::SizeT value);

public:
    ////////////////////////////////////////////////////////////
    explicit ExampleBunnyMark(const GameDependencies& deps, sf::TextureAtlas& textureAtlas);

    ////////////////////////////////////////////////////////////
    void update(float deltaTimeMs) override;

    ////////////////////////////////////////////////////////////
    void draw() override;
};
