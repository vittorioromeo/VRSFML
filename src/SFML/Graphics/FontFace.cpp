// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/FontFace.hpp"

#include "SFML/Graphics/FontInfo.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/GlyphMapping.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/MemoryInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

//
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

#include <freetype/config/ftheader.h>
#include <freetype/config/integer-types.h>
#include <freetype/ftimage.h>
#include <freetype/ftsystem.h>
#include <freetype/fttypes.h>

#ifdef SFML_SYSTEM_ANDROID
    #include "SFML/System/Android/ResourceStream.hpp"
#endif


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] unsigned long read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    auto* stream = static_cast<sf::InputStream*>(rec->descriptor.pointer);

    if (sf::base::Optional seekResult = stream->seek(offset); seekResult.hasValue() && *seekResult == offset)
        return count == 0ul ? 0ul
                            : static_cast<unsigned long>(stream->read(reinterpret_cast<char*>(buffer), count).value());

    return count == 0ul ? 1ul : 0ul;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline sf::base::I32 quantizeOutlineThickness(const float outlineThickness)
{
    return static_cast<sf::base::I32>(outlineThickness * float{1 << 6});
}


////////////////////////////////////////////////////////////
struct KerningKey
{
    char32_t     first;
    char32_t     second;
    unsigned int characterSize;
    bool         bold;

    [[nodiscard]] bool operator==(const KerningKey&) const = default;
};


////////////////////////////////////////////////////////////
struct KerningKeyHash
{
    [[nodiscard, gnu::always_inline]] sf::base::U64 operator()(const KerningKey& k) const noexcept
    {
        // FNV-1a style mixing of the fields
        auto h = sf::base::U64{14'695'981'039'346'656'037ull};

        const auto mix = [&](sf::base::U64 v)
        {
            h ^= v;
            h *= sf::base::U64{1'099'511'628'211ull};
        };

        mix(k.first);
        mix(k.second);
        mix(k.characterSize);
        mix(k.bold);

        return h;
    }
};


} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct FontFace::Impl
{
    ////////////////////////////////////////////////////////////
    Impl() = default;


    ////////////////////////////////////////////////////////////
    ~Impl()
    {
        if (m_ftStroker)
            FT_Stroker_Done(m_ftStroker);

        if (m_ftFace)
            FT_Done_Face(m_ftFace);

        if (m_ftLibrary)
            FT_Done_FreeType(m_ftLibrary);
    }


    ////////////////////////////////////////////////////////////
    Impl(const Impl&) = delete;
    Impl(Impl&&)      = delete;


    ////////////////////////////////////////////////////////////
    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&)      = delete;


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool openFromStream(InputStream& streamToOpen, const char* const type)
    {
        const auto fail = [&](const char* what)
        {
            priv::err() << "Failed to load font from stream (type:" << type << "): " << what;
            return false;
        };

        if (FT_Init_FreeType(&m_ftLibrary) != 0)
            return fail("failed to initialize FreeType");

        const auto streamSize = streamToOpen.getSize();
        if (!streamSize.hasValue())
            return fail("failed to query stream size");

        m_ftStreamRec.base               = nullptr;
        m_ftStreamRec.size               = static_cast<unsigned long>(*streamSize);
        m_ftStreamRec.pos                = 0;
        m_ftStreamRec.descriptor.pointer = &streamToOpen;
        m_ftStreamRec.read               = &read;
        m_ftStreamRec.close              = [](FT_Stream) {};

        FT_Open_Args args{};
        args.flags  = FT_OPEN_STREAM;
        args.stream = &m_ftStreamRec;
        args.driver = nullptr;

        if (FT_Open_Face(m_ftLibrary, &args, 0, &m_ftFace) != 0)
            return fail("failed to create the font face");

        if (FT_Stroker_New(m_ftLibrary, &m_ftStroker) != 0)
            return fail("failed to create the stroker");

        if (FT_Select_Charmap(m_ftFace, FT_ENCODING_UNICODE) != 0)
            return fail("failed to set the Unicode character set");

        m_info.family = m_ftFace->family_name;
        return true;
    }


    ////////////////////////////////////////////////////////////
    void setOwnedStream(base::UniquePtr<InputStream>&& ownedStream)
    {
        m_stream = SFML_BASE_MOVE(ownedStream);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FontInfo& getInfo() const
    {
        return m_info;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getCharIndex(const char32_t codePoint) const
    {
        if (const auto* it = m_charIndexCache.find(codePoint); it != m_charIndexCache.end())
            return it->second;

        const auto result           = FT_Get_Char_Index(m_ftFace, codePoint);
        m_charIndexCache[codePoint] = result;
        return result;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool hasGlyph(const char32_t codePoint) const
    {
        return getCharIndex(codePoint) != 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getKerning(const char32_t first, const char32_t second, const unsigned int characterSize, const bool bold) const
    {
        SFML_BASE_ASSERT(m_ftLibrary != nullptr);

        if (first == 0 || second == 0)
            return 0.f;

        const KerningKey key{first, second, characterSize, bold};

        if (const auto* it = m_kerningCache.find(key); it != m_kerningCache.end())
            return it->second;

        if (!m_ftFace || !setCurrentSize(characterSize))
        {
            m_kerningCache[key] = 0.f;
            return 0.f;
        }

        const FT_UInt index1 = getCharIndex(first);
        const FT_UInt index2 = getCharIndex(second);

        // Load glyphs to get lsb/rsb deltas without full rasterization
        constexpr FT_Int32 loadFlags = FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT;

        float firstRsbDelta  = 0.f;
        float secondLsbDelta = 0.f;

        if (FT_Load_Char(m_ftFace, first, loadFlags) == 0)
            firstRsbDelta = static_cast<float>(m_ftFace->glyph->rsb_delta);

        if (FT_Load_Char(m_ftFace, second, loadFlags) == 0)
            secondLsbDelta = static_cast<float>(m_ftFace->glyph->lsb_delta);

        FT_Vector kerning{0, 0};

        if (FT_HAS_KERNING(m_ftFace))
            FT_Get_Kerning(m_ftFace, index1, index2, FT_KERNING_UNFITTED, &kerning);

        const float calculatedKerning = //
            (FT_IS_SCALABLE(m_ftFace))
                ? SFML_BASE_MATH_FLOORF((secondLsbDelta - firstRsbDelta + static_cast<float>(kerning.x) + 32) / float{1 << 6})
                : static_cast<float>(kerning.x);

        m_kerningCache[key] = calculatedKerning;
        return calculatedKerning;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getAscent(unsigned int characterSize) const
    {
        if (!setCurrentSize(characterSize))
            return 0.f;

        if (!FT_IS_SCALABLE(m_ftFace))
            return static_cast<float>(m_ftFace->size->metrics.ascender) / float{1 << 6};

        return static_cast<float>(FT_MulFix(m_ftFace->ascender, m_ftFace->size->metrics.y_scale)) / float{1 << 6};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDescent(unsigned int characterSize) const
    {
        if (!setCurrentSize(characterSize))
            return 0.f;

        if (!FT_IS_SCALABLE(m_ftFace))
            return static_cast<float>(m_ftFace->size->metrics.descender) / float{1 << 6};

        return static_cast<float>(FT_MulFix(m_ftFace->descender, m_ftFace->size->metrics.y_scale)) / float{1 << 6};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getLineSpacing(const unsigned int characterSize) const
    {
        if (!setCurrentSize(characterSize))
            return 0.f;

        return static_cast<float>(m_ftFace->size->metrics.height) / float{1 << 6};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getUnderlinePosition(const unsigned int characterSize) const
    {
        if (!setCurrentSize(characterSize))
            return 0.f;

        if (!FT_IS_SCALABLE(m_ftFace))
            return static_cast<float>(characterSize) / 10.f;

        return -static_cast<float>(FT_MulFix(m_ftFace->underline_position, m_ftFace->size->metrics.y_scale)) /
               float{1 << 6};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getUnderlineThickness(const unsigned int characterSize) const
    {
        if (!setCurrentSize(characterSize))
            return 0.f;

        if (!FT_IS_SCALABLE(m_ftFace))
            return static_cast<float>(characterSize) / 14.f;

        return static_cast<float>(FT_MulFix(m_ftFace->underline_thickness, m_ftFace->size->metrics.y_scale)) /
               float{1 << 6};
    }


    ////////////////////////////////////////////////////////////
    struct RasterizedGlyph
    {
        Glyph glyph;
        Vec2u bufferSize{};
    };


    ////////////////////////////////////////////////////////////
    [[nodiscard]] RasterizedGlyph rasterizeGlyph(const char32_t     codePoint,
                                                 const unsigned int characterSize,
                                                 const bool         bold,
                                                 const float        outlineThickness) const
    {
        RasterizedGlyph result;

        if (!m_ftFace)
            return result;

        if (!setCurrentSize(characterSize))
            return result;

        const FT_Int32 flags = outlineThickness == 0.f
                                   ? FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT
                                   : FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_NO_BITMAP;

        if (FT_Load_Char(m_ftFace, codePoint, flags) != 0)
            return result;

        FT_Glyph glyphDesc = nullptr;

        SFML_BASE_SCOPE_GUARD({
            if (glyphDesc)
                FT_Done_Glyph(glyphDesc);
        });

        if (FT_Get_Glyph(m_ftFace->glyph, &glyphDesc) != 0)
            return result;

        const FT_Pos weight          = 1 << 6;
        const bool   supportsOutline = (glyphDesc->format == FT_GLYPH_FORMAT_OUTLINE);
        if (supportsOutline)
        {
            if (bold)
            {
                auto* outlineGlyph = reinterpret_cast<FT_OutlineGlyph>(glyphDesc);
                FT_Outline_Embolden(&outlineGlyph->outline, weight);
            }

            if (outlineThickness != 0.f)
            {
                FT_Stroker_Set(m_ftStroker,
                               static_cast<FT_Fixed>(quantizeOutlineThickness(outlineThickness)),
                               FT_STROKER_LINECAP_ROUND,
                               FT_STROKER_LINEJOIN_ROUND,
                               0);

                if (FT_Glyph_Stroke(&glyphDesc, m_ftStroker, true) != FT_Err_Ok)
                {
                    sf::priv::err() << "Failed to outline glyph";
                    return result;
                }
            }
        }

        if (FT_Glyph_To_Bitmap(&glyphDesc, FT_RENDER_MODE_NORMAL, nullptr, 1) != FT_Err_Ok)
        {
            sf::priv::err() << "Failed to render glyph to bitmap";
            return result;
        }

        auto*      bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyphDesc);
        FT_Bitmap& bitmap      = bitmapGlyph->bitmap;

        if (!supportsOutline)
        {
            if (bold)
                FT_Bitmap_Embolden(m_ftLibrary, &bitmap, weight, weight);

            if (outlineThickness != 0.f)
                sf::priv::err() << "Failed to outline glyph (no fallback available)";
        }

        result.glyph.advance = static_cast<float>(bitmapGlyph->root.advance.x >> 16) +
                               (bold ? static_cast<float>(weight) / float{1 << 6} : 0.f);

        result.glyph.lsbDelta = static_cast<sf::base::I16>(m_ftFace->glyph->lsb_delta);
        result.glyph.rsbDelta = static_cast<sf::base::I16>(m_ftFace->glyph->rsb_delta);

        if (bitmap.width == 0u || bitmap.rows == 0u)
            return result;

        constexpr unsigned int padding = 2u;

        const sf::Vec2u size{bitmap.width + 2u * padding, bitmap.rows + 2u * padding};

        result.glyph.textureRect.position = sf::Vec2f{static_cast<float>(padding), static_cast<float>(padding)};
        result.glyph.textureRect.size     = sf::Vec2u(bitmap.width, bitmap.rows).toVec2f();

        result.glyph.bounds.position = sf::Vec2i(bitmapGlyph->left, -bitmapGlyph->top).toVec2f();
        result.glyph.bounds.size     = sf::Vec2u(bitmap.width, bitmap.rows).toVec2f();

        result.bufferSize = size;

        m_pixelBuffer.resize(static_cast<sf::base::SizeT>(size.x * size.y * 4u));

        sf::base::U8* current = m_pixelBuffer.data();
        sf::base::U8* end     = current + size.x * size.y * 4u;

        while (current != end)
        {
            (*current++) = 255u;
            (*current++) = 255u;
            (*current++) = 255u;
            (*current++) = 0u;
        }

        const sf::base::U8* pixels = bitmap.buffer;

        if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            for (unsigned int y = padding; y < size.y - padding; ++y)
            {
                for (unsigned int x = padding; x < size.x - padding; ++x)
                {
                    const sf::base::SizeT index = x + y * size.x;
                    m_pixelBuffer[index * 4 + 3] = ((pixels[(x - padding) / 8]) & (1 << (7 - ((x - padding) % 8)))) ? 255 : 0;
                }

                pixels += bitmap.pitch;
            }
        }
        else
        {
            for (unsigned int y = padding; y < size.y - padding; ++y)
            {
                for (unsigned int x = padding; x < size.x - padding; ++x)
                {
                    const sf::base::SizeT index  = x + y * size.x;
                    m_pixelBuffer[index * 4 + 3] = pixels[x - padding];
                }

                pixels += bitmap.pitch;
            }
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::U8* getPixelBufferData()
    {
        return m_pixelBuffer.data();
    }

private:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setCurrentSize(const unsigned int characterSize) const
    {
        const FT_UShort currentSize = m_ftFace->size->metrics.x_ppem;

        if (currentSize == characterSize)
            return true;

        const FT_Error result = FT_Set_Pixel_Sizes(m_ftFace, 0u, characterSize);

        if (result != FT_Err_Invalid_Pixel_Size)
            return result == FT_Err_Ok;

        if (FT_IS_SCALABLE(m_ftFace))
        {
            priv::err() << "Failed to set font size to " << characterSize;
            return false;
        }

        auto& multilineErr = priv::err(true /* multiLine */);
        multilineErr << "Failed to set bitmap font size to " << characterSize << '\n' << "Available sizes are: ";

        for (int i = 0; i < m_ftFace->num_fixed_sizes; ++i)
        {
            const long size = (m_ftFace->available_sizes[i].y_ppem + 32) >> 6;
            multilineErr << size << " ";
        }

        multilineErr << '\n';
        return false;
    }


    ////////////////////////////////////////////////////////////
    FT_Library   m_ftLibrary{};
    FT_StreamRec m_ftStreamRec{};
    FT_Face      m_ftFace{};
    FT_Stroker   m_ftStroker{};

    FontInfo m_info;

    mutable ankerl::unordered_dense::map<KerningKey, float, KerningKeyHash> m_kerningCache;
    mutable ankerl::unordered_dense::map<char32_t, unsigned int>            m_charIndexCache;

    base::UniquePtr<InputStream> m_stream;

    mutable base::Vector<base::U8> m_pixelBuffer;
};


////////////////////////////////////////////////////////////
FontFace::FontFace(base::PassKey<FontFace>&&) : m_impl(base::makeUnique<Impl>())
{
}


////////////////////////////////////////////////////////////
FontFace::~FontFace()                              = default;
FontFace::FontFace(FontFace&&) noexcept            = default;
FontFace& FontFace::operator=(FontFace&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<FontFace> FontFace::openFromFile(const Path& filename)
{
    base::Optional<FontFace> result;

#ifndef SFML_SYSTEM_ANDROID
    auto optStream = FileInputStream::open(filename);
    if (!optStream.hasValue())
    {
        priv::err() << "Failed to load font face (" << priv::PathDebugFormatter{filename} << "): failed to open file";
        return result;
    }

    auto                  stream = base::makeUnique<FileInputStream>(SFML_BASE_MOVE(*optStream));
    constexpr const char* type   = "file";
#else
    auto optStream = ResourceStream::open(filename);
    if (!optStream.hasValue())
    {
        priv::err() << "Failed to load font face (" << priv::PathDebugFormatter{filename} << "): failed to open file";
        return result;
    }

    auto                  stream = base::makeUnique<priv::ResourceStream>(SFML_BASE_MOVE(*optStream));
    constexpr const char* type   = "Android resource stream";
#endif

    result = openFromStreamImpl(*stream, type);
    if (result.hasValue())
        result->m_impl->setOwnedStream(SFML_BASE_MOVE(stream));
    else
        priv::err() << priv::PathDebugFormatter{filename};

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<FontFace> FontFace::openFromMemory(const void* data, base::SizeT sizeInBytes)
{
    base::Optional<FontFace> result;

    if (!data)
    {
        priv::err() << "Failed to load font face from memory: provided data pointer is null";
        return result;
    }

    auto memoryStream = base::makeUnique<MemoryInputStream>(data, sizeInBytes);
    result            = openFromStreamImpl(*memoryStream, "memory");

    if (result.hasValue())
        result->m_impl->setOwnedStream(SFML_BASE_MOVE(memoryStream));

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<FontFace> FontFace::openFromStreamImpl(InputStream& stream, const char* const type)
{
    base::Optional<FontFace> result;
    result.emplace(base::PassKey<FontFace>{});

    if (!result->m_impl->openFromStream(stream, type))
        result.reset();

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<FontFace> FontFace::openFromStream(InputStream& stream)
{
    if (!stream.seek(0).hasValue())
    {
        priv::err() << "Failed to seek font face stream";
        return base::nullOpt;
    }

    return openFromStreamImpl(stream, "stream");
}


////////////////////////////////////////////////////////////
const FontInfo& FontFace::getInfo() const
{
    return m_impl->getInfo();
}


////////////////////////////////////////////////////////////
bool FontFace::hasGlyph(const char32_t codePoint) const
{
    return m_impl->hasGlyph(codePoint);
}


////////////////////////////////////////////////////////////
float FontFace::getKerning(const char32_t first, const char32_t second, const unsigned int characterSize, const bool bold) const
{
    return m_impl->getKerning(first, second, characterSize, bold);
}


////////////////////////////////////////////////////////////
float FontFace::getAscent(const unsigned int characterSize) const
{
    return m_impl->getAscent(characterSize);
}


////////////////////////////////////////////////////////////
float FontFace::getDescent(const unsigned int characterSize) const
{
    return m_impl->getDescent(characterSize);
}


////////////////////////////////////////////////////////////
float FontFace::getLineSpacing(const unsigned int characterSize) const
{
    return m_impl->getLineSpacing(characterSize);
}


////////////////////////////////////////////////////////////
float FontFace::getUnderlinePosition(const unsigned int characterSize) const
{
    return m_impl->getUnderlinePosition(characterSize);
}


////////////////////////////////////////////////////////////
float FontFace::getUnderlineThickness(const unsigned int characterSize) const
{
    return m_impl->getUnderlineThickness(characterSize);
}


////////////////////////////////////////////////////////////
base::Optional<Glyph> FontFace::rasterizeAndPackGlyph(
    TextureAtlas&      atlas,
    const char32_t     codePoint,
    const unsigned int characterSize,
    const bool         bold,
    const float        outlineThickness) const
{
    auto result = m_impl->rasterizeGlyph(codePoint, characterSize, bold, outlineThickness);

    if (result.bufferSize.x > 0u && result.bufferSize.y > 0u)
    {
        auto optRect = atlas.add(m_impl->getPixelBufferData(), result.bufferSize);

        if (!optRect.hasValue())
        {
            priv::err() << "Failed to add glyph to the atlas (code point: " << static_cast<unsigned int>(codePoint) << ")";
            return base::nullOpt;
        }

        result.glyph.textureRect.position += optRect->position;
    }

    return base::makeOptional<Glyph>(result.glyph);
}


////////////////////////////////////////////////////////////
base::Optional<GlyphMapping> FontFace::loadGlyphs(TextureAtlas& atlas, const GlyphLoadSettings& settings) const
{
    GlyphMapping result{
        .fillGlyphs{},
        .outlineGlyphs{},

        .fontFace         = this,
        .characterSize    = settings.characterSize,
        .bold             = settings.bold,
        .outlineThickness = settings.outlineThickness,

        .cachedLineSpacing        = m_impl->getLineSpacing(settings.characterSize),
        .cachedAscent             = m_impl->getAscent(settings.characterSize),
        .cachedDescent            = m_impl->getDescent(settings.characterSize),
        .cachedUnderlinePosition  = m_impl->getUnderlinePosition(settings.characterSize),
        .cachedUnderlineThickness = m_impl->getUnderlineThickness(settings.characterSize),
    };

    for (base::SizeT i = 0u; i < settings.codePointCount; ++i)
    {
        const char32_t codePoint = settings.codePoints[i];

        const auto optFillGlyph = rasterizeAndPackGlyph(atlas, codePoint, settings.characterSize, settings.bold, 0.f);

        if (!optFillGlyph.hasValue())
            return base::nullOpt; // Error already logged in `rasterizeAndPackGlyph`

        result.fillGlyphs[codePoint] = *optFillGlyph;

        if (settings.outlineThickness == 0.f)
            continue;

        const auto optOutlineGlyph = rasterizeAndPackGlyph(atlas,
                                                           codePoint,
                                                           settings.characterSize,
                                                           settings.bold,
                                                           settings.outlineThickness);

        if (!optOutlineGlyph.hasValue())
            return base::nullOpt; // Error already logged in `rasterizeAndPackGlyph`

        result.outlineGlyphs[codePoint] = *optOutlineGlyph;
    }

    return base::makeOptional(SFML_BASE_MOVE(result));
}

} // namespace sf
