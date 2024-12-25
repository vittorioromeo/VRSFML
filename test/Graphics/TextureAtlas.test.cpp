#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Texture.hpp"

// Other 1st party headers
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"

#include "SFML/System/Vector2.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] sf::TextureAtlas" * doctest::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    const auto makeColoredTexture = [&](sf::Color color)
    { return sf::Texture::loadFromImage(sf::Image::create({64u, 64u}, color).value()).value(); };

    constexpr sf::Vector2u atlasSize{512u, 512u};

    SECTION("Add -- failure case")
    {
        auto textureAtlas = sf::TextureAtlas(sf::Texture::create({32u, 32u}).value());

        const auto p0 = textureAtlas.add(makeColoredTexture(sf::Color::Red));
        CHECK(!p0.hasValue());
    }

    SECTION("Add -- one texture")
    {
        auto textureAtlas = sf::TextureAtlas(sf::Texture::create(atlasSize).value());

        const auto p0 = textureAtlas.add(makeColoredTexture(sf::Color::Red));
        CHECK(p0.hasValue());
        CHECK(p0->position.x == 0u);
        CHECK(p0->position.y == 0u);
        CHECK(p0->size.x == 64u);
        CHECK(p0->size.y == 64u);

        const auto atlasImage = textureAtlas.getTexture().copyToImage();
        CHECK(atlasImage.getPixel({0u, 0u}) == sf::Color::Red);
        CHECK(atlasImage.getPixel({64u, 64u}) != sf::Color::Red);
    }

    SECTION("Add -- two textures")
    {
        auto textureAtlas = sf::TextureAtlas(sf::Texture::create(atlasSize).value());

        const auto p0 = textureAtlas.add(makeColoredTexture(sf::Color::Red));
        CHECK(p0.hasValue());
        CHECK(p0->position.x == 0u);
        CHECK(p0->position.y == 0u);
        CHECK(p0->size.x == 64u);
        CHECK(p0->size.y == 64u);

        const auto p1 = textureAtlas.add(makeColoredTexture(sf::Color::Blue));
        CHECK(p1.hasValue());
        CHECK(p1->position.x == 64u);
        CHECK(p1->position.y == 0u);
        CHECK(p1->size.x == 64u);
        CHECK(p1->size.y == 64u);

        const auto atlasImage = textureAtlas.getTexture().copyToImage();
        CHECK(atlasImage.getPixel({0u, 0u}) == sf::Color::Red);
        CHECK(atlasImage.getPixel({64u, 0u}) == sf::Color::Blue);

        CHECK(atlasImage.getPixel({128u, 0u}) != sf::Color::Red);
        CHECK(atlasImage.getPixel({128u, 0u}) != sf::Color::Blue);
    }
}
