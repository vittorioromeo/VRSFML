#include "SystemUtil.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/FontInfo.hpp"
#include "Zancle/Graphics/Glyph.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/TextureWrapMode.hpp"

// Other 1st party headers
#include "GraphicsUtil.hpp"
#include "LoadIntoMemoryUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"
#include "Zancle/System/FileInputStream.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsDefaultConstructible.hpp"
#include "ZancleBase/Trait/IsMoveAssignable.hpp"
#include "ZancleBase/Trait/IsMoveConstructible.hpp"


TEST_CASE("[Graphics] za::Font" * tst::skip(skipDisplayTests))
{
    auto graphicsContext = za::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_DEFAULT_CONSTRUCTIBLE(za::Font));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::Font));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::Font));
        STATIC_CHECK(ZB_IS_MOVE_CONSTRUCTIBLE(za::Font));
        STATIC_CHECK(ZB_IS_MOVE_ASSIGNABLE(za::Font));
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid filename")
        {
            CHECK(!za::Font::openFromFile("does/not/exist.ttf").hasValue());
        }

        SECTION("Valid file")
        {
            const za::Path filenameSuffixes[] = {U"", U"-ń", U"-🐌"};
            for (const auto& filenameSuffix : filenameSuffixes)
            {
                const za::Path filename = U"tuffy" + filenameSuffix + U".ttf";
                INFO("Filename: " << filename.to<zb::String>().cStr());

                const auto font = za::Font::openFromFile(filename).value();

                CHECK(font.getInfo().family == "Tuffy");
                const auto& glyph = font.getGlyph(0x45, 16, false, /* outlineThickness */ 0.f);
                CHECK(glyph.advance == 9);
                CHECK(glyph.lsbDelta == 9);
                CHECK(glyph.rsbDelta == 16);
                CHECK(glyph.bounds == za::Rect2f({0, -12}, {8, 12}));
                CHECK(glyph.textureRect == za::Rect2f({2, 2}, {8, 12}));
                CHECK(font.hasGlyph(0x41));
                CHECK(font.hasGlyph(0xC0));
                CHECK(font.getKerning(0x41, 0x42, 12, false) == -1);
                CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
                CHECK(font.getLineSpacing(24) == 30);
                CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
                CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
                const auto& texture = font.getTexture();
                CHECK(texture.getSize() == za::Vec2u{1024u, 1024u});
                CHECK(texture.isSmooth());
                CHECK(!texture.isSrgb());
                CHECK(texture.getWrapMode() == za::TextureWrapMode::Clamp);
                CHECK(texture.getNativeHandle() != 0);
            }
        }
    }

    SECTION("openFromMemory()")
    {
        SECTION("Invalid data and size")
        {
            CHECK(!za::Font::openFromMemory(nullptr, 1).hasValue());
            const unsigned char testByte{0xCD};
            CHECK(!za::Font::openFromMemory(&testByte, 0).hasValue());
        }

        SECTION("Valid data")
        {
            const auto memory = loadIntoMemory("tuffy.ttf");
            const auto font   = za::Font::openFromMemory(memory.data(), memory.size()).value();
            CHECK(font.getInfo().family == "Tuffy");
            const auto& glyph = font.getGlyph(0x45, 16, false, /* outlineThickness */ 0.f);
            CHECK(glyph.advance == 9);
            CHECK(glyph.lsbDelta == 9);
            CHECK(glyph.rsbDelta == 16);
            CHECK(glyph.bounds == za::Rect2f({0, -12}, {8, 12}));
            CHECK(glyph.textureRect == za::Rect2f({2, 2}, {8, 12}));
            CHECK(font.hasGlyph(0x41));
            CHECK(font.hasGlyph(0xC0));
            CHECK(font.getKerning(0x41, 0x42, 12, false) == -1);
            CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
            CHECK(font.getLineSpacing(24) == 30);
            CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
            CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
            const auto& texture = font.getTexture();
            CHECK(texture.getSize() == za::Vec2u{1024u, 1024u});
            CHECK(texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(texture.getWrapMode() == za::TextureWrapMode::Clamp);
            CHECK(texture.getNativeHandle() != 0);
        }
    }

    SECTION("openFromStream()")
    {
        auto       stream = za::FileInputStream::open("tuffy.ttf").value();
        const auto font   = za::Font::openFromStream(stream).value();
        CHECK(font.getInfo().family == "Tuffy");
        const auto& glyph = font.getGlyph(0x45, 16, false, /* outlineThickness */ 0.f);
        CHECK(glyph.advance == 9);
        CHECK(glyph.lsbDelta == 9);
        CHECK(glyph.rsbDelta == 16);
        CHECK(glyph.bounds == za::Rect2f({0, -12}, {8, 12}));
        CHECK(glyph.textureRect == za::Rect2f({2, 2}, {8, 12}));
        CHECK(font.hasGlyph(0x41));
        CHECK(font.hasGlyph(0xC0));
        CHECK(font.getKerning(0x41, 0x42, 12, false) == -1);
        CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
        CHECK(font.getLineSpacing(24) == 30);
        CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
        CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
        const auto& texture = font.getTexture();
        CHECK(texture.getSize() == za::Vec2u{1024u, 1024u});
        CHECK(texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(texture.getWrapMode() == za::TextureWrapMode::Clamp);
        CHECK(texture.getNativeHandle() != 0);
    }
}
