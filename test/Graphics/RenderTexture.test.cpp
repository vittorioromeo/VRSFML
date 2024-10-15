#include "SFML/Graphics/RenderTexture.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] sf::RenderTexture" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::RenderTexture));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::RenderTexture));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::RenderTexture));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::RenderTexture));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::RenderTexture));
    }

    SECTION("create()")
    {
        CHECK(!sf::RenderTexture::create({1'000'000, 1'000'000}).hasValue());

        CHECK(sf::RenderTexture::create({100, 100}, sf::ContextSettings{.depthBits = 8, .stencilBits = 0}).hasValue());

        CHECK(sf::RenderTexture::create({100, 100}, sf::ContextSettings{.depthBits = 0, .stencilBits = 8}).hasValue());

        const auto renderTexture = sf::RenderTexture::create({360, 480}).value();
        CHECK(renderTexture.getSize() == sf::Vector2u{360, 480});
        CHECK(!renderTexture.isSmooth());
        CHECK(!renderTexture.isRepeated());
        CHECK(!renderTexture.isSrgb());

        const auto& texture = renderTexture.getTexture();
        CHECK(texture.getSize() == sf::Vector2u{360, 480});
        CHECK(!texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(!texture.isRepeated());
        CHECK(texture.getNativeHandle() != 0);
    }

    SECTION("getMaximumAntiAliasingLevel()")
    {
        CHECK(sf::RenderTexture::getMaximumAntiAliasingLevel() <= 64);
    }

    SECTION("Set/get smooth")
    {
        auto renderTexture = sf::RenderTexture::create({64, 64}).value();
        renderTexture.setSmooth(true);
        CHECK(renderTexture.isSmooth());
    }

    SECTION("Set/get repeated")
    {
        auto renderTexture = sf::RenderTexture::create({64, 64}).value();
        renderTexture.setRepeated(true);
        CHECK(renderTexture.isRepeated());
    }

    SECTION("generateMipmap()")
    {
        auto renderTexture = sf::RenderTexture::create({64, 64}).value();
        CHECK(renderTexture.generateMipmap());
    }

    SECTION("setActive()")
    {
        auto renderTexture = sf::RenderTexture::create({64, 64}).value();
        CHECK(renderTexture.setActive());
        CHECK(renderTexture.setActive(false));
        CHECK(renderTexture.setActive(true));
    }

    SECTION("getTexture()")
    {
        const auto renderTexture = sf::RenderTexture::create({64, 64}).value();
        CHECK(renderTexture.getTexture().getSize() == sf::Vector2u{64, 64});
    }

    SECTION("Sanity check 1")
    {
        unsigned int testAALevel{};
        bool         testSRGBCapable{};

        // clang-format off
        SUBCASE("no AA, no SRGB") { testAALevel = 0u; testSRGBCapable = false; }
        SUBCASE("AA, no SRGB")    { testAALevel = 4u; testSRGBCapable = false; }
        SUBCASE("no AA, SRGB")    { testAALevel = 0u; testSRGBCapable = true; }
        SUBCASE("AA, SRGB")       { testAALevel = 4u; testSRGBCapable = true; }
        // clang-format on

        auto renderTexture = sf::RenderTexture::create({64, 64},
                                                       {.antiAliasingLevel = testAALevel, .sRgbCapable = testSRGBCapable})
                                 .value();

        renderTexture.clear(sf::Color::Green);
        renderTexture.display();

        auto image = renderTexture.getTexture().copyToImage();
        CHECK((image.getPixel({0u, 0u}) == sf::Color::Green));
    }

    SECTION("Sanity check 2")
    {
        unsigned int testAALevel{};
        bool         testSRGBCapable{};

        // clang-format off
        SUBCASE("no AA, no SRGB") { testAALevel = 0u; testSRGBCapable = false; }
        SUBCASE("AA, no SRGB")    { testAALevel = 4u; testSRGBCapable = false; }
        SUBCASE("no AA, SRGB")    { testAALevel = 0u; testSRGBCapable = true; }
        SUBCASE("AA, SRGB")       { testAALevel = 4u; testSRGBCapable = true; }
        // clang-format on

        const float width     = 128.f;
        const float height    = 64.f;
        const float halfWidth = width / 2.f;

        const sf::Vector2u size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)};

        auto image   = sf::Image::create(size, sf::Color::White).value();
        auto texture = sf::Texture::loadFromImage(image).value();

        auto baseRenderTexture = sf::RenderTexture::create(size, {.antiAliasingLevel = testAALevel, .sRgbCapable = testSRGBCapable})
                                     .value();

        auto leftInnerRT = sf::RenderTexture::create(size, {.antiAliasingLevel = testAALevel, .sRgbCapable = testSRGBCapable})
                               .value();
        auto rightInnerRT = sf::RenderTexture::create(size, {.antiAliasingLevel = testAALevel, .sRgbCapable = testSRGBCapable})
                                .value();

        const sf::Vertex leftVertexArray[6]{{{0.f, 0.f}, sf::Color::White, {0.f, 0.f}},
                                            {{halfWidth, 0.f}, sf::Color::White, {halfWidth, 0.f}},
                                            {{0.f, height}, sf::Color::White, {0.f, height}},
                                            {{0.f, height}, sf::Color::White, {0.f, height}},
                                            {{halfWidth, 0.f}, sf::Color::White, {halfWidth, 0.f}},
                                            {{halfWidth, height}, sf::Color::White, {halfWidth, height}}};

        const sf::Vertex rightVertexArray[6]{{{halfWidth, 0.f}, sf::Color::White, {halfWidth, 0.f}},
                                             {{width, 0.f}, sf::Color::White, {width, 0.f}},
                                             {{halfWidth, height}, sf::Color::White, {halfWidth, height}},
                                             {{halfWidth, height}, sf::Color::White, {halfWidth, height}},
                                             {{width, 0.f}, sf::Color::White, {width, 0.f}},
                                             {{width, height}, sf::Color::White, {width, height}}};

        leftInnerRT.clear();
        rightInnerRT.clear();

        leftInnerRT.draw(texture);
        rightInnerRT.draw(texture, {.color = sf::Color::Green});

        baseRenderTexture.clear();

        leftInnerRT.display();
        baseRenderTexture.draw(leftVertexArray, sf::PrimitiveType::Triangles, {.texture = &leftInnerRT.getTexture()});

        rightInnerRT.display();
        baseRenderTexture.draw(rightVertexArray, sf::PrimitiveType::Triangles, {.texture = &rightInnerRT.getTexture()});

        baseRenderTexture.display();

        auto finalImage = baseRenderTexture.getTexture().copyToImage();

        CHECK(finalImage.getSize() == size);
        CHECK((finalImage.getPixel({0u, 0u}) == sf::Color::White));
        CHECK((finalImage.getPixel({static_cast<unsigned int>(width / 2.f) + 1u, 0u}) == sf::Color::Green));
    }

    SECTION("Sanity check -- flipping")
    {
        unsigned int testAALevel{};
        bool         testSRGBCapable{};

        // clang-format off
        SUBCASE("no AA, no SRGB") { testAALevel = 0u; testSRGBCapable = false; }
        SUBCASE("AA, no SRGB")    { testAALevel = 4u; testSRGBCapable = false; }
        SUBCASE("no AA, SRGB")    { testAALevel = 0u; testSRGBCapable = true; }
        SUBCASE("AA, SRGB")       { testAALevel = 4u; testSRGBCapable = true; }
        // clang-format on

        const float width     = 128.f;
        const float height    = 64.f;
        const float halfWidth = width / 2.f;

        const sf::Vector2u size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)};

        auto image   = sf::Image::create(size, sf::Color::White).value();
        auto texture = sf::Texture::loadFromImage(image).value();

        auto baseRenderTexture = sf::RenderTexture::create(size, {.antiAliasingLevel = testAALevel, .sRgbCapable = testSRGBCapable})
                                     .value();

        auto leftInnerRT = sf::RenderTexture::create(size, {.antiAliasingLevel = testAALevel, .sRgbCapable = testSRGBCapable})
                               .value();

        const sf::Vertex leftVertexArray[6]{{{0.f, 0.f}, sf::Color::Red, {0.f, 0.f}},
                                            {{halfWidth, 0.f}, sf::Color::Red, {halfWidth, 0.f}},
                                            {{0.f, height}, sf::Color::Red, {0.f, height}},
                                            {{0.f, height}, sf::Color::Green, {0.f, height}},
                                            {{halfWidth, 0.f}, sf::Color::Green, {halfWidth, 0.f}},
                                            {{halfWidth, height}, sf::Color::Green, {halfWidth, height}}};

        leftInnerRT.clear();
        leftInnerRT.draw(texture);

        baseRenderTexture.clear();

        leftInnerRT.display();
        baseRenderTexture.draw(leftVertexArray, sf::PrimitiveType::Triangles, {.texture = &leftInnerRT.getTexture()});

        baseRenderTexture.display();

        auto finalImage = baseRenderTexture.getTexture().copyToImage();

        CHECK(finalImage.getSize() == size);
        CHECK(finalImage.getPixel({0u, 0u}) == sf::Color::Red);

        CHECK(finalImage.getPixel({0u, 31u}) == sf::Color::Red);
        CHECK(finalImage.getPixel({31u, 0u}) == sf::Color::Red);
        CHECK(finalImage.getPixel({31u, 31u}) == sf::Color::Red);

        CHECK(finalImage.getPixel({0u, 60u}) == sf::Color::Red);
        CHECK(finalImage.getPixel({60u, 0u}) == sf::Color::Red);

        CHECK(finalImage.getPixel({6u, 58u}) == sf::Color::Green);
        CHECK(finalImage.getPixel({58u, 6u}) == sf::Color::Green);
        CHECK(finalImage.getPixel({61u, 61u}) == sf::Color::Green);
    }
}
