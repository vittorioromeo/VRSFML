#include "Zancle/Graphics/TextureAtlas.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/Texture.hpp"

// Other 1st party headers
#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/Image.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"


TEST_CASE("[Graphics] za::TextureAtlas" * tst::skip(skipDisplayTests))
{
    auto graphicsContext = za::GraphicsContext::create().value();

    const auto makeColoredTexture = [&](za::Color color)
    { return za::Texture::loadFromImage(za::Image::create({64u, 64u}, color).value()).value(); };

    constexpr za::Vec2u atlasSize{512u, 512u};

    SECTION("Add -- failure case")
    {
        auto textureAtlas = za::TextureAtlas(za::Texture::create({32u, 32u}).value());

        const auto p0 = textureAtlas.add(makeColoredTexture(za::Color::Red));
        CHECK(!p0.hasValue());
    }

    SECTION("Add -- one texture")
    {
        auto textureAtlas = za::TextureAtlas(za::Texture::create(atlasSize).value());

        const auto p0 = textureAtlas.add(makeColoredTexture(za::Color::Red));
        CHECK(p0.hasValue());
        CHECK(p0->position.x == 0u);
        CHECK(p0->position.y == 0u);
        CHECK(p0->size.x == 64u);
        CHECK(p0->size.y == 64u);

        const auto atlasImage = textureAtlas.getTexture().copyToImage();
        CHECK(atlasImage.getPixel({0u, 0u}) == za::Color::Red);
        CHECK(atlasImage.getPixel({64u, 64u}) != za::Color::Red);
    }

    SECTION("Add -- two textures")
    {
        auto textureAtlas = za::TextureAtlas(za::Texture::create(atlasSize).value());

        const auto p0 = textureAtlas.add(makeColoredTexture(za::Color::Red));
        CHECK(p0.hasValue());
        CHECK(p0->position.x == 0u);
        CHECK(p0->position.y == 0u);
        CHECK(p0->size.x == 64u);
        CHECK(p0->size.y == 64u);

        const auto p1 = textureAtlas.add(makeColoredTexture(za::Color::Blue));
        CHECK(p1.hasValue());
        CHECK(p1->position.x == 64u);
        CHECK(p1->position.y == 0u);
        CHECK(p1->size.x == 64u);
        CHECK(p1->size.y == 64u);

        const auto atlasImage = textureAtlas.getTexture().copyToImage();
        CHECK(atlasImage.getPixel({0u, 0u}) == za::Color::Red);
        CHECK(atlasImage.getPixel({64u, 0u}) == za::Color::Blue);

        CHECK(atlasImage.getPixel({128u, 0u}) != za::Color::Red);
        CHECK(atlasImage.getPixel({128u, 0u}) != za::Color::Blue);
    }
}
