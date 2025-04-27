#include "SFML/Graphics/Texture.hpp"

// Other 1st party headers
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"

#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Traits/IsNothrowSwappable.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] sf::Texture" * doctest::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::Texture));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Texture));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Texture));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Texture));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Texture));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_SWAPPABLE(sf::Texture));
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            sf::Texture       movedTexture = sf::Texture::create({64, 64}).value();
            const sf::Texture texture      = SFML_BASE_MOVE(movedTexture);
            CHECK(texture.getSize() == sf::Vector2u{64, 64});
            CHECK(!texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
            CHECK(texture.getNativeHandle() != 0);
        }

        SECTION("Assignment")
        {
            sf::Texture movedTexture = sf::Texture::create({64, 64}).value();
            sf::Texture texture      = sf::Texture::create({128, 128}).value();
            texture                  = SFML_BASE_MOVE(movedTexture);
            CHECK(texture.getSize() == sf::Vector2u{64, 64});
            CHECK(!texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
            CHECK(texture.getNativeHandle() != 0);
        }

        SECTION("Move assignment")
        {
            auto rt0 = sf::Texture::create({100, 100});
            CHECK(rt0.hasValue());
            rt0->setSmooth(true);

            auto rt1 = sf::Texture::create({100, 100});
            CHECK(rt1.hasValue());
            rt1->setSmooth(true);

            *rt0 = SFML_BASE_MOVE(*rt1);
            rt0->setSmooth(true);
        }
    }

    SECTION("create()")
    {
        SECTION("At least one zero dimension")
        {
            CHECK(!sf::Texture::create({}).hasValue());
            CHECK(!sf::Texture::create({0, 1}).hasValue());
            CHECK(!sf::Texture::create({1, 0}).hasValue());
        }

        SECTION("Valid size")
        {
            const auto texture = sf::Texture::create({100, 100}).value();
            CHECK(texture.getSize() == sf::Vector2u{100, 100});
            CHECK(texture.getNativeHandle() != 0);
        }

        SECTION("Too large")
        {
            CHECK(!sf::Texture::create({100'000, 100'000}).hasValue());
            CHECK(!sf::Texture::create({1'000'000, 1'000'000}).hasValue());
        }
    }

    SECTION("loadFromFile()")
    {
        const auto texture = sf::Texture::loadFromFile("Graphics/sfml-logo-big.png").value();
        CHECK(texture.getSize() == sf::Vector2u{1001, 304});
        CHECK(!texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
        CHECK(texture.getNativeHandle() != 0);
    }

    SECTION("loadFromMemory()")
    {
        const auto memory  = loadIntoMemory("Graphics/sfml-logo-big.png");
        const auto texture = sf::Texture::loadFromMemory(memory.data(), memory.size()).value();
        CHECK(texture.getSize() == sf::Vector2u{1001, 304});
        CHECK(!texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
        CHECK(texture.getNativeHandle() != 0);
    }

    SECTION("loadFromStream()")
    {
        auto       stream  = sf::FileInputStream::open("Graphics/sfml-logo-big.png").value();
        const auto texture = sf::Texture::loadFromStream(stream).value();
        CHECK(texture.getSize() == sf::Vector2u{1001, 304});
        CHECK(!texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
        CHECK(texture.getNativeHandle() != 0);
    }

    SECTION("loadFromImage()")
    {
        SECTION("Subarea of image")
        {
            const auto image = sf::Image::create(sf::Vector2u{10, 15}).value();

            SECTION("Non-truncated area")
            {
                const auto texture = sf::Texture::loadFromImage(image, {.sRgb = false, .area = {{0, 0}, {5, 10}}}).value();
                CHECK(texture.getSize() == sf::Vector2u{5, 10});
                CHECK(texture.getNativeHandle() != 0);
            }

            SECTION("Truncated area (negative position)")
            {
                const auto texture = sf::Texture::loadFromImage(image, {.sRgb = false, .area = {{-5, -5}, {4, 8}}}).value();
                CHECK(texture.getSize() == sf::Vector2u{4, 8});
                CHECK(texture.getNativeHandle() != 0);
            }

            SECTION("Truncated area (width/height too big)")
            {
                const auto texture = sf::Texture::loadFromImage(image, {.sRgb = false, .area = {{5, 5}, {12, 18}}}).value();
                CHECK(texture.getSize() == sf::Vector2u{5, 10});
                CHECK(texture.getNativeHandle() != 0);
            }
        }
    }

    SECTION("Copy semantics")
    {
        constexpr sf::base::U8 red[]{0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};

        auto texture = sf::Texture::create({1, 2}).value();
        texture.update(red);

        SECTION("Construction")
        {
            const sf::Texture textureCopy(texture); // NOLINT(performance-unnecessary-copy-initialization)
            REQUIRE(textureCopy.getSize() == sf::Vector2u{1, 2});
            CHECK(textureCopy.copyToImage().getPixel(sf::Vector2u{0, 1}) == sf::Color::Red);
        }

        SECTION("Assignment")
        {
            sf::Texture textureCopy = sf::Texture::create({64, 64}).value();
            textureCopy             = texture;
            REQUIRE(textureCopy.getSize() == sf::Vector2u{1, 2});
            CHECK(textureCopy.copyToImage().getPixel(sf::Vector2u{0, 1}) == sf::Color::Red);
        }
    }

    SECTION("update()")
    {
        constexpr sf::base::U8 yellow[]{0xFF, 0xFF, 0x00, 0xFF};
        constexpr sf::base::U8 cyan[]{0x00, 0xFF, 0xFF, 0xFF};

        SECTION("Pixels")
        {
            auto texture = sf::Texture::create(sf::Vector2u{1, 1}).value();
            texture.update(yellow);
            CHECK(texture.copyToImage().getPixel(sf::Vector2u{0, 0}) == sf::Color::Yellow);
        }

        SECTION("Pixels, size and destination")
        {
            auto texture = sf::Texture::create(sf::Vector2u{2, 1}).value();
            texture.update(yellow, sf::Vector2u{1, 1}, sf::Vector2u{0, 0});
            texture.update(cyan, sf::Vector2u{1, 1}, sf::Vector2u{1, 0});

            const auto textureAsImage = texture.copyToImage();
            CHECK(textureAsImage.getPixel(sf::Vector2u{0, 0}) == sf::Color::Yellow);
            CHECK(textureAsImage.getPixel(sf::Vector2u{1, 0}) == sf::Color::Cyan);
        }

        SECTION("Another texture")
        {
            auto otherTexture = sf::Texture::create(sf::Vector2u{1, 1}).value();
            otherTexture.update(cyan);
            auto texture = sf::Texture::create(sf::Vector2u{1, 1}).value();
            CHECK(texture.update(otherTexture));
            CHECK(texture.copyToImage().getPixel(sf::Vector2u{0, 0}) == sf::Color::Cyan);
        }

        SECTION("Another texture and destination")
        {
            auto texture       = sf::Texture::create(sf::Vector2u{2, 1}).value();
            auto otherTexture1 = sf::Texture::create(sf::Vector2u{1, 1}).value();
            otherTexture1.update(cyan);
            auto otherTexture2 = sf::Texture::create(sf::Vector2u{1, 1}).value();
            otherTexture2.update(yellow);
            CHECK(texture.update(otherTexture1, sf::Vector2u{0, 0}));
            CHECK(texture.update(otherTexture2, sf::Vector2u{1, 0}));

            const auto textureAsImage = texture.copyToImage();
            CHECK(textureAsImage.getPixel(sf::Vector2u{0, 0}) == sf::Color::Cyan);
            CHECK(textureAsImage.getPixel(sf::Vector2u{1, 0}) == sf::Color::Yellow);
        }

        SECTION("Image")
        {
            auto       texture = sf::Texture::create(sf::Vector2u{16, 32}).value();
            const auto image   = sf::Image::create(sf::Vector2u{16, 32}, sf::Color::Red).value();
            texture.update(image);
            CHECK(texture.copyToImage().getPixel(sf::Vector2u{7, 15}) == sf::Color::Red);
        }

        SECTION("Image and destination")
        {
            auto       texture = sf::Texture::create(sf::Vector2u{16, 32}).value();
            const auto image1  = sf::Image::create(sf::Vector2u{16, 16}, sf::Color::Red).value();
            texture.update(image1);
            const auto image2 = sf::Image::create(sf::Vector2u{16, 16}, sf::Color::Green).value();
            texture.update(image1, sf::Vector2u{0, 0});
            texture.update(image2, sf::Vector2u{0, 16});

            const auto textureAsImage = texture.copyToImage();
            CHECK(textureAsImage.getPixel(sf::Vector2u{7, 7}) == sf::Color::Red);
            CHECK(textureAsImage.getPixel(sf::Vector2u{7, 22}) == sf::Color::Green);
        }
    }

    SECTION("Set/get smooth")
    {
        auto texture = sf::Texture::create({64, 64}).value();
        CHECK(!texture.isSmooth());
        texture.setSmooth(true);
        CHECK(texture.isSmooth());
        texture.setSmooth(false);
        CHECK(!texture.isSmooth());
    }

    SECTION("Set/get repeated")
    {
        auto texture = sf::Texture::create({64, 64}).value();
        CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
        texture.setWrapMode(sf::TextureWrapMode::Repeat);
        CHECK(texture.getWrapMode() == sf::TextureWrapMode::Repeat);
        texture.setWrapMode(sf::TextureWrapMode::Clamp);
        CHECK(texture.getWrapMode() == sf::TextureWrapMode::Clamp);
    }

    SECTION("generateMipmap()")
    {
        auto texture = sf::Texture::create({100, 100}).value();
        CHECK(texture.generateMipmap());
    }

    SECTION("swap()")
    {
        constexpr sf::base::U8 blue[]{0x00, 0x00, 0xFF, 0xFF};
        constexpr sf::base::U8 green[]{0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};

        auto texture1 = sf::Texture::create(sf::Vector2u{1, 1}, {.sRgb = true}).value();
        texture1.update(blue);
        texture1.setSmooth(false);
        texture1.setWrapMode(sf::TextureWrapMode::Repeat);

        auto texture2 = sf::Texture::create(sf::Vector2u{2, 1}, {.sRgb = false}).value();
        texture2.update(green);
        texture2.setSmooth(true);
        texture2.setWrapMode(sf::TextureWrapMode::Clamp);

        sf::swap(texture1, texture2);
        CHECK_FALSE(texture1.isSrgb());
        CHECK(texture1.isSmooth());
        CHECK_FALSE(texture1.getWrapMode() == sf::TextureWrapMode::Repeat);
        // Cannot check texture2.isSrgb() because Srgb is sometimes disabled when using OpenGL ES
        CHECK_FALSE(texture2.isSmooth());
        CHECK(texture2.getWrapMode() == sf::TextureWrapMode::Repeat);

        const auto image1 = texture1.copyToImage();
        const auto image2 = texture2.copyToImage();
        REQUIRE(image1.getSize() == sf::Vector2u{2, 1});
        REQUIRE(image2.getSize() == sf::Vector2u{1, 1});
        CHECK(image1.getPixel(sf::Vector2u{1, 0}) == sf::Color::Green);
        CHECK(image2.getPixel(sf::Vector2u{0, 0}) == sf::Color::Blue);
    }

    SECTION("Get Maximum Size")
    {
        CHECK(sf::Texture::getMaximumSize() > 0);
    }
}
