// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Font.hpp"

#include "SFML/Graphics/FontFace.hpp"
#include "SFML/Graphics/FontInfo.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/GlyphMapping.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline sf::base::I32 quantizeOutlineThickness(const float outlineThickness)
{
    return static_cast<sf::base::I32>(outlineThickness * float{1 << 6});
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline sf::base::U32 bitCastU32(const sf::base::I32 value)
{
    sf::base::U32 result; // NOLINT(cppcoreguidelines-init-variables)
    SFML_BASE_MEMCPY(&result, &value, sizeof(value));
    return result;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline sf::base::U64 combineGlyphTableKey(
    const float    outlineThickness,
    const bool     bold,
    const char32_t index)
{
    return (sf::base::U64{bitCastU32(quantizeOutlineThickness(outlineThickness))} << 32) | (sf::base::U64{bold} << 31) |
           index;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Font::Impl
{
    ////////////////////////////////////////////////////////////
    template <typename K, typename V>
    using MapType = ankerl::unordered_dense::map<K, V>;


    ////////////////////////////////////////////////////////////
    using GlyphTable = MapType</* character size */ unsigned int, MapType</* combined key */ base::U64, Glyph>>;


    ////////////////////////////////////////////////////////////
    explicit Impl(FontFace&& theFontFace, TextureAtlas* theTextureAtlasPtr) :
        fontFace(SFML_BASE_MOVE(theFontFace)),
        textureAtlasPtr{theTextureAtlasPtr},
        fallbackTextureAtlas{
            theTextureAtlasPtr == nullptr
                ? base::makeOptional<TextureAtlas>(Texture::create({1024u, 1024u}, {.smooth = true}).value())
                : base::nullOpt}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] TextureAtlas& getTextureAtlas() const
    {
        return textureAtlasPtr == nullptr ? *fallbackTextureAtlas : *textureAtlasPtr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] auto loadGlyphImpl(auto&              glyphsByCharacterSize,
                                     const base::U64    key,
                                     const char32_t     codePoint,
                                     const unsigned int characterSize,
                                     const bool         bold,
                                     const float        outlineThickness) const
    {
        const auto optGlyph = fontFace.rasterizeAndPackGlyph(getTextureAtlas(), codePoint, characterSize, bold, outlineThickness);

        if (!optGlyph.hasValue())
        {
            priv::err() << "Failed to load glyph for code point " << static_cast<unsigned int>(codePoint)
                        << ", character size " << characterSize << ", bold " << bold << ", outline thickness "
                        << outlineThickness;

            base::abort();
        }

        return glyphsByCharacterSize.try_emplace(key, *optGlyph);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const Glyph& getGlyphImpl(
        auto&              glyphsByCharacterSize,
        const base::U64    key,
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
    mutable FontFace                     fontFace;
    TextureAtlas*                        textureAtlasPtr;
    mutable base::Optional<TextureAtlas> fallbackTextureAtlas;
    mutable GlyphTable                   glyphs;
};


////////////////////////////////////////////////////////////
Font::Font(base::PassKey<Font>&&, TextureAtlas* /* textureAtlas */)
{
    // m_impl is set by the factory methods after construction
}


////////////////////////////////////////////////////////////
Font::~Font()                          = default;
Font::Font(Font&&) noexcept            = default;
Font& Font::operator=(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromFile(const Path& filename, TextureAtlas* textureAtlas)
{
    auto optFontFace = FontFace::openFromFile(filename);
    if (!optFontFace.hasValue())
        return base::nullOpt;

    base::Optional<Font> result;
    result.emplace(base::PassKey<Font>{}, textureAtlas);
    result->m_impl = base::makeUnique<Impl>(SFML_BASE_MOVE(*optFontFace), textureAtlas);
    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromMemory(const void* data, base::SizeT sizeInBytes, TextureAtlas* textureAtlas)
{
    auto optFontFace = FontFace::openFromMemory(data, sizeInBytes);
    if (!optFontFace.hasValue())
        return base::nullOpt;

    base::Optional<Font> result;
    result.emplace(base::PassKey<Font>{}, textureAtlas);
    result->m_impl = base::makeUnique<Impl>(SFML_BASE_MOVE(*optFontFace), textureAtlas);
    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromStreamImpl(InputStream& stream, TextureAtlas* textureAtlas, const char* /* type */)
{
    auto optFontFace = FontFace::openFromStream(stream);
    if (!optFontFace.hasValue())
        return base::nullOpt;

    base::Optional<Font> result;
    result.emplace(base::PassKey<Font>{}, textureAtlas);
    result->m_impl = base::makeUnique<Impl>(SFML_BASE_MOVE(*optFontFace), textureAtlas);
    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromStream(InputStream& stream, TextureAtlas* textureAtlas)
{
    if (!stream.seek(0).hasValue())
    {
        priv::err() << "Failed to seek font stream";
        return base::nullOpt;
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
    return m_impl
        ->getGlyphImpl(m_impl->glyphs[characterSize],
                       combineGlyphTableKey(outlineThickness,
                                            bold,
                                            static_cast<char32_t>(m_impl->fontFace.hasGlyph(codePoint) ? codePoint : 0)),
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
    SFML_BASE_ASSERT(outlineThickness != 0.f);

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
        SFML_BASE_ASSERT(fillGlyphIt != glyphsByCharacterSize.end());
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

} // namespace sf
