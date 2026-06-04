// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/FontFace.hpp"
#include "Zancle/Graphics/FontInfo.hpp"
#include "Zancle/Graphics/Glyph.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/TextureAtlas.hpp"
#include "Zancle/System/Err.hpp"
#include "Zancle/System/InputStream.hpp"
#include "Zancle/System/Path.hpp"
#include "ZancleBase/Abort.hpp"
#include "ZancleBase/AnkerlUnorderedDense.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Builtin/Memcpy.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/UniquePtr.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline zb::I32 quantizeOutlineThickness(const float outlineThickness)
{
    return static_cast<zb::I32>(outlineThickness * float{1 << 6});
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline zb::U32 bitCastU32(const zb::I32 value)
{
    zb::U32 result; // NOLINT(cppcoreguidelines-init-variables)
    ZB_MEMCPY(&result, &value, sizeof(value));
    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline zb::U64 combineGlyphTableKey(
    const float    outlineThickness,
    const bool     bold,
    const char32_t index)
{
    return (zb::U64{bitCastU32(quantizeOutlineThickness(outlineThickness))} << 32) | (zb::U64{bold} << 31) | index;
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
struct Font::Impl
{
    ////////////////////////////////////////////////////////////
    template <typename K, typename V>
    using MapType = ankerl::unordered_dense::map<K, V>;


    ////////////////////////////////////////////////////////////
    using GlyphTable = MapType</* character size */ unsigned int, MapType</* combined key */ zb::U64, Glyph>>;


    ////////////////////////////////////////////////////////////
    explicit Impl(FontFace&& theFontFace, TextureAtlas* theTextureAtlasPtr) :
        fontFace(ZB_MOVE(theFontFace)),
        textureAtlasPtr{theTextureAtlasPtr},
        fallbackTextureAtlas{
            theTextureAtlasPtr == nullptr
                ? zb::makeOptional<TextureAtlas>(Texture::create({1024u, 1024u}, {.smooth = true}).value())
                : zb::nullOpt}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] TextureAtlas& getTextureAtlas() const
    {
        return textureAtlasPtr == nullptr ? *fallbackTextureAtlas : *textureAtlasPtr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] auto loadGlyphImpl(auto&              glyphsByCharacterSize,
                                     const zb::U64      key,
                                     const char32_t     codePoint,
                                     const unsigned int characterSize,
                                     const bool         bold,
                                     const float        outlineThickness) const
    {
        const auto optGlyph = fontFace.rasterizeAndPackGlyph(getTextureAtlas(), codePoint, characterSize, bold, outlineThickness);

        if (!optGlyph.hasValue())
        {
            priv::errMsg("Failed to load glyph for code point {}, character size {}, bold {}, outline thickness {}",
                         static_cast<unsigned int>(codePoint),
                         characterSize,
                         bold,
                         outlineThickness);

            zb::abort();
        }

        return glyphsByCharacterSize.try_emplace(key, *optGlyph);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const Glyph& getGlyphImpl(
        auto&              glyphsByCharacterSize,
        const zb::U64      key,
        const char32_t     codePoint,
        const unsigned int characterSize,
        const bool         bold,
        const float        outlineThickness) const
    {
        if (const auto* it = glyphsByCharacterSize.find(key); it != glyphsByCharacterSize.end()) [[likely]]
            return it->second;

        return loadGlyphImpl(glyphsByCharacterSize, key, codePoint, characterSize, bold, outlineThickness).first->second;
    }


    ////////////////////////////////////////////////////////////
    mutable FontFace                   fontFace;
    TextureAtlas*                      textureAtlasPtr;
    mutable zb::Optional<TextureAtlas> fallbackTextureAtlas;
    mutable GlyphTable                 glyphs;
};


////////////////////////////////////////////////////////////
Font::Font(zb::PassKey<Font>&&, FontFace&& fontFace, TextureAtlas* textureAtlas) :
    m_impl{zb::makeUnique<Impl>(ZB_MOVE(fontFace), textureAtlas)}
{
    // m_impl is set by the factory methods after construction
}


////////////////////////////////////////////////////////////
Font::~Font()                          = default;
Font::Font(Font&&) noexcept            = default;
Font& Font::operator=(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
zb::Optional<Font> Font::openFromFile(const Path& filename, TextureAtlas* textureAtlas)
{
    auto optFontFace = FontFace::openFromFile(filename);
    if (!optFontFace.hasValue())
        return zb::nullOpt;

    return zb::makeOptional<Font>(zb::PassKey<Font>{}, ZB_MOVE(*optFontFace), textureAtlas);
}


////////////////////////////////////////////////////////////
zb::Optional<Font> Font::openFromMemory(const void* data, zb::SizeT sizeInBytes, TextureAtlas* textureAtlas)
{
    auto optFontFace = FontFace::openFromMemory(data, sizeInBytes);
    if (!optFontFace.hasValue())
        return zb::nullOpt;

    return zb::makeOptional<Font>(zb::PassKey<Font>{}, ZB_MOVE(*optFontFace), textureAtlas);
}


////////////////////////////////////////////////////////////
zb::Optional<Font> Font::openFromStreamImpl(InputStream& stream, TextureAtlas* textureAtlas, const char* /* type */)
{
    auto optFontFace = FontFace::openFromStream(stream);
    if (!optFontFace.hasValue())
        return zb::nullOpt;

    return zb::makeOptional<Font>(zb::PassKey<Font>{}, ZB_MOVE(*optFontFace), textureAtlas);
}


////////////////////////////////////////////////////////////
zb::Optional<Font> Font::openFromStream(InputStream& stream, TextureAtlas* textureAtlas)
{
    if (!stream.seek(0).hasValue())
    {
        priv::errMsg("Failed to seek font stream");
        return zb::nullOpt;
    }

    return openFromStreamImpl(stream, textureAtlas, "stream");
}


////////////////////////////////////////////////////////////
const FontInfo& Font::getInfo() const
{
    return m_impl->fontFace.getInfo();
}


////////////////////////////////////////////////////////////
const Glyph& Font::getGlyph(const char32_t     codePoint,
                            const unsigned int characterSize,
                            const bool         bold,
                            const float        outlineThickness) const
{
    ZB_ASSERT(m_impl->fontFace.hasGlyph(codePoint));

    return m_impl->getGlyphImpl(m_impl->glyphs[characterSize],
                                combineGlyphTableKey(outlineThickness, bold, codePoint),
                                codePoint,
                                characterSize,
                                bold,
                                outlineThickness);
}


////////////////////////////////////////////////////////////
Font::GlyphPair Font::getFillAndOutlineGlyph(const char32_t     codePoint,
                                             const unsigned int characterSize,
                                             const bool         bold,
                                             const float        outlineThickness) const
{
    ZB_ASSERT(outlineThickness != 0.f);
    ZB_ASSERT(m_impl->fontFace.hasGlyph(codePoint));

    auto& glyphsByCharacterSize = m_impl->glyphs[characterSize];

    const auto fillGlyphKey    = combineGlyphTableKey(0.f, bold, codePoint);
    const auto outlineGlyphKey = combineGlyphTableKey(outlineThickness, bold, codePoint);

    const auto* fillGlyphIt = glyphsByCharacterSize.find(fillGlyphKey);
    if (fillGlyphIt == glyphsByCharacterSize.end()) [[unlikely]]
    {
        fillGlyphIt = m_impl->loadGlyphImpl(glyphsByCharacterSize, fillGlyphKey, codePoint, characterSize, bold, 0.f).first;
    }

    const auto* outlineGlyphIt = glyphsByCharacterSize.find(outlineGlyphKey);
    if (outlineGlyphIt == glyphsByCharacterSize.end()) [[unlikely]]
    {
        outlineGlyphIt = m_impl
                             ->loadGlyphImpl(glyphsByCharacterSize, outlineGlyphKey, codePoint, characterSize, bold, outlineThickness)
                             .first;

        fillGlyphIt = glyphsByCharacterSize.find(fillGlyphKey);
        ZB_ASSERT(fillGlyphIt != glyphsByCharacterSize.end());
    }

    return {.fillGlyph = fillGlyphIt->second, .outlineGlyph = outlineGlyphIt->second};
}


////////////////////////////////////////////////////////////
bool Font::hasGlyph(const char32_t codePoint) const
{
    return m_impl->fontFace.hasGlyph(codePoint);
}


////////////////////////////////////////////////////////////
float Font::getKerning(const char32_t first, const char32_t second, const unsigned int characterSize, const bool bold) const
{
    return m_impl->fontFace.getKerning(first, second, characterSize, bold);
}


////////////////////////////////////////////////////////////
float Font::getAscent(unsigned int characterSize) const
{
    return m_impl->fontFace.getAscent(characterSize);
}


////////////////////////////////////////////////////////////
float Font::getDescent(unsigned int characterSize) const
{
    return m_impl->fontFace.getDescent(characterSize);
}


////////////////////////////////////////////////////////////
float Font::getLineSpacing(const unsigned int characterSize) const
{
    return m_impl->fontFace.getLineSpacing(characterSize);
}


////////////////////////////////////////////////////////////
float Font::getUnderlinePosition(const unsigned int characterSize) const
{
    return m_impl->fontFace.getUnderlinePosition(characterSize);
}


////////////////////////////////////////////////////////////
float Font::getUnderlineThickness(const unsigned int characterSize) const
{
    return m_impl->fontFace.getUnderlineThickness(characterSize);
}


////////////////////////////////////////////////////////////
Texture& Font::getTexture()
{
    return m_impl->getTextureAtlas().getTexture();
}


////////////////////////////////////////////////////////////
const Texture& Font::getTexture() const
{
    return m_impl->getTextureAtlas().getTexture();
}

} // namespace za
