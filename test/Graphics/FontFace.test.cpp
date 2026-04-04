#include "SystemUtil.hpp"

#include "SFML/Graphics/FontFace.hpp"

#include "SFML/Graphics/FontInfo.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"

// Other 1st party headers
#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <WindowUtil.hpp>


TEST_CASE("[Graphics] sf::FontFace" * doctest::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::FontFace));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::FontFace));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::FontFace));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::FontFace));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::FontFace));
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid filename")
        {
            CHECK(!sf::FontFace::openFromFile("does/not/exist.ttf").hasValue());
        }

        SECTION("Valid file")
        {
            const auto fontFace = sf::FontFace::openFromFile("tuffy.ttf").value();
            CHECK(fontFace.getInfo().family == "Tuffy");
            CHECK(fontFace.hasGlyph(0x41));
            CHECK(fontFace.hasGlyph(0xC0));
            CHECK(fontFace.getKerning(0x41, 0x42, 12, false) == -1);
            CHECK(fontFace.getKerning(0x43, 0x44, 24, true) == 0);
            CHECK(fontFace.getLineSpacing(24) == 30);
            CHECK(fontFace.getUnderlinePosition(36) == Approx(2.20312f));
            CHECK(fontFace.getUnderlineThickness(48) == Approx(1.17188f));
        }
    }

    SECTION("openFromMemory()")
    {
        SECTION("Invalid data and size")
        {
            CHECK(!sf::FontFace::openFromMemory(nullptr, 1).hasValue());
            const unsigned char testByte{0xCD};
            CHECK(!sf::FontFace::openFromMemory(&testByte, 0).hasValue());
        }

        SECTION("Valid data")
        {
            const auto memory   = loadIntoMemory("tuffy.ttf");
            const auto fontFace = sf::FontFace::openFromMemory(memory.data(), memory.size()).value();
            CHECK(fontFace.getInfo().family == "Tuffy");
            CHECK(fontFace.hasGlyph(0x41));
            CHECK(fontFace.getLineSpacing(24) == 30);
        }
    }

    SECTION("openFromStream()")
    {
        auto       stream   = sf::FileInputStream::open("tuffy.ttf").value();
        const auto fontFace = sf::FontFace::openFromStream(stream).value();
        CHECK(fontFace.getInfo().family == "Tuffy");
        CHECK(fontFace.hasGlyph(0x41));
        CHECK(fontFace.getKerning(0x41, 0x42, 12, false) == -1);
    }
}
