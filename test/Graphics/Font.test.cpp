#include "SFML/Graphics/Font.hpp"

#include "SFML/Graphics/FontInfo.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Texture.hpp"

// Other 1st party headers
#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <WindowUtil.hpp>

TEST_CASE("[Graphics] sf::Font" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::Font));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Font));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Font));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::Font));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::Font));
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid filename")
        {
            CHECK(!sf::Font::openFromFile(graphicsContext, "does/not/exist.ttf").hasValue());
        }

        SECTION("Valid file")
        {
            const auto font = sf::Font::openFromFile(graphicsContext, "Graphics/tuffy.ttf").value();
            CHECK(font.getInfo().family == "Tuffy");
            const auto& glyph = font.getGlyph(0x45, 16, false);
            CHECK(glyph.advance == 9);
            CHECK(glyph.lsbDelta == 9);
            CHECK(glyph.rsbDelta == 16);
            CHECK(glyph.bounds == sf::FloatRect({0, -12}, {8, 12}));
            CHECK(glyph.textureRect == sf::FloatRect({2, 2}, {8, 12}));
            CHECK(font.hasGlyph(0x41));
            CHECK(font.hasGlyph(0xC0));
            CHECK(font.getKerning(0x41, 0x42, 12) == -1);
            CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
            CHECK(font.getLineSpacing(24) == 30);
            CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
            CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
            const auto& texture = font.getTexture(10);
            CHECK(texture.getSize() == sf::Vector2u{1024u, 1024u});
            CHECK(texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(!texture.isRepeated());
            CHECK(texture.getNativeHandle() != 0);
            CHECK(font.isSmooth());
        }
    }

    SECTION("openFromMemory()")
    {
        SECTION("Invalid data and size")
        {
            CHECK(!sf::Font::openFromMemory(graphicsContext, nullptr, 1).hasValue());
            const unsigned char testByte{0xCD};
            CHECK(!sf::Font::openFromMemory(graphicsContext, &testByte, 0).hasValue());
        }

        SECTION("Valid data")
        {
            const auto memory = loadIntoMemory("Graphics/tuffy.ttf");
            const auto font   = sf::Font::openFromMemory(graphicsContext, memory.data(), memory.size()).value();
            CHECK(font.getInfo().family == "Tuffy");
            const auto& glyph = font.getGlyph(0x45, 16, false);
            CHECK(glyph.advance == 9);
            CHECK(glyph.lsbDelta == 9);
            CHECK(glyph.rsbDelta == 16);
            CHECK(glyph.bounds == sf::FloatRect({0, -12}, {8, 12}));
            CHECK(glyph.textureRect == sf::FloatRect({2, 2}, {8, 12}));
            CHECK(font.hasGlyph(0x41));
            CHECK(font.hasGlyph(0xC0));
            CHECK(font.getKerning(0x41, 0x42, 12) == -1);
            CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
            CHECK(font.getLineSpacing(24) == 30);
            CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
            CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
            const auto& texture = font.getTexture(10);
            CHECK(texture.getSize() == sf::Vector2u{1024u, 1024u});
            CHECK(texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(!texture.isRepeated());
            CHECK(texture.getNativeHandle() != 0);
            CHECK(font.isSmooth());
        }
    }

    SECTION("openFromStream()")
    {
        auto       stream = sf::FileInputStream::open("Graphics/tuffy.ttf").value();
        const auto font   = sf::Font::openFromStream(graphicsContext, stream).value();
        CHECK(font.getInfo().family == "Tuffy");
        const auto& glyph = font.getGlyph(0x45, 16, false);
        CHECK(glyph.advance == 9);
        CHECK(glyph.lsbDelta == 9);
        CHECK(glyph.rsbDelta == 16);
        CHECK(glyph.bounds == sf::FloatRect({0, -12}, {8, 12}));
        CHECK(glyph.textureRect == sf::FloatRect({2, 2}, {8, 12}));
        CHECK(font.hasGlyph(0x41));
        CHECK(font.hasGlyph(0xC0));
        CHECK(font.getKerning(0x41, 0x42, 12) == -1);
        CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
        CHECK(font.getLineSpacing(24) == 30);
        CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
        CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
        const auto& texture = font.getTexture(10);
        CHECK(texture.getSize() == sf::Vector2u{1024u, 1024u});
        CHECK(texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(!texture.isRepeated());
        CHECK(texture.getNativeHandle() != 0);
        CHECK(font.isSmooth());
    }

    SECTION("Set/get smooth")
    {
        auto font = sf::Font::openFromFile(graphicsContext, "Graphics/tuffy.ttf").value();
        font.setSmooth(false);
        CHECK(!font.isSmooth());
    }
}
