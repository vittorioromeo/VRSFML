#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/FontInfo.hpp"
#include "SFML/Graphics/Glyph.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/TextureAtlas.hpp"

#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"
#ifdef SFML_SYSTEM_ANDROID
#include "SFML/System/Android/ResourceStream.hpp"
#endif
#include "SFML/System/Err.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Math/Floor.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Memcpy.hpp"

#include <memory>
#include <unordered_map>
#include <vector>


namespace
{
////////////////////////////////////////////////////////////
// FreeType callbacks that operate on a sf::InputStream
[[nodiscard]] unsigned long read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    auto* stream = static_cast<sf::InputStream*>(rec->descriptor.pointer);
    if (sf::base::Optional seekResult = stream->seek(offset); seekResult.hasValue() && *seekResult == offset)
    {
        if (count > 0)
            return static_cast<unsigned long>(stream->read(reinterpret_cast<char*>(buffer), count).value());

        return 0;
    }

    return count > 0 ? 0 : 1; // error code is 0 if we're reading, or nonzero if we're seeking
}


////////////////////////////////////////////////////////////
void close(FT_Stream)
{
}


////////////////////////////////////////////////////////////
// Helper to interpret memory as a specific type
template <typename T, typename U>
[[nodiscard, gnu::always_inline]] inline T reinterpret(const U& input)
{
    T output;
    SFML_BASE_MEMCPY(&output, &input, sizeof(U));
    return output;
}


////////////////////////////////////////////////////////////
// Combine outline thickness, boldness and font glyph index into a single 64-bit key
[[nodiscard, gnu::always_inline]] std::uint64_t combine(float outlineThickness, bool bold, std::uint32_t index)
{
    return (std::uint64_t{reinterpret<std::uint32_t>(outlineThickness)} << 32) | (std::uint64_t{bold} << 31) | index;
}


////////////////////////////////////////////////////////////
bool setFaceCurrentSize(FT_Face face, unsigned int characterSize)
{
    // FT_Set_Pixel_Sizes is an expensive function, so we must call it
    // only when necessary to avoid killing performances

    const FT_UShort currentSize = face->size->metrics.x_ppem;

    if (currentSize == characterSize)
        return true;

    const FT_Error result = FT_Set_Pixel_Sizes(face, 0, characterSize);

    if (result != FT_Err_Invalid_Pixel_Size)
        return result == FT_Err_Ok;

    if (FT_IS_SCALABLE(face))
    {
        sf::priv::err() << "Failed to set font size to " << characterSize;
        return false;
    }

    // In the case of bitmap fonts, resizing can fail if the requested size is not available

    sf::priv::err(true /* multiLine */) << "Failed to set bitmap font size to " << characterSize << '\n'
                                        << "Available sizes are: ";

    for (int i = 0; i < face->num_fixed_sizes; ++i)
    {
        const long size = (face->available_sizes[i].y_ppem + 32) >> 6;
        sf::priv::err(true /* multiLine */) << size << " ";
    }

    sf::priv::err() << '\n';
    return false;
}


////////////////////////////////////////////////////////////
struct FontHandles
{
    FontHandles() = default;

    ~FontHandles()
    {
        // All the function below are safe to call with null pointer arguments.
        // The documentation of FreeType isn't clear on the matter, but the
        // implementation does explicitly check for null.

        FT_Stroker_Done(stroker);
        FT_Done_Face(face);
        // `streamRec` doesn't need to be explicitly freed.
        FT_Done_FreeType(library);
    }

    // clang-format off
    FontHandles(const FontHandles&)            = delete;
    FontHandles& operator=(const FontHandles&) = delete;

    FontHandles(FontHandles&&)            = delete;
    FontHandles& operator=(FontHandles&&) = delete;
    // clang-format on

    FT_Library   library{};   //< Pointer to the internal library interface
    FT_StreamRec streamRec{}; //< Stream rec object describing an input stream
    FT_Face      face{};      //< Pointer to the internal font face
    FT_Stroker   stroker{};   //< Pointer to the stroker
};


////////////////////////////////////////////////////////////
sf::Glyph loadGlyph(const FontHandles&         fontHandles,
                    sf::TextureAtlas&          textureAtlas,
                    std::vector<std::uint8_t>& pixelBuffer,
                    std::uint32_t              codePoint,
                    unsigned int               characterSize,
                    bool                       bold,
                    float                      outlineThickness)
{
    // Get our FT_Face
    FT_Face face = fontHandles.face;
    if (!face)
        return {};

    // Set the character size
    if (!setFaceCurrentSize(face, characterSize))
        return {};

    // Load the glyph corresponding to the code point
    FT_Int32 flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT;
    if (outlineThickness != 0)
        flags |= FT_LOAD_NO_BITMAP;

    if (FT_Load_Char(face, codePoint, flags) != 0)
        return {};

    // Retrieve the glyph
    FT_Glyph glyphDesc = nullptr;
    if (FT_Get_Glyph(face->glyph, &glyphDesc) != 0)
        return {};

    // Apply bold and outline (there is no fallback for outline) if necessary -- first technique using outline (highest quality)
    const FT_Pos weight  = 1 << 6;
    const bool   outline = (glyphDesc->format == FT_GLYPH_FORMAT_OUTLINE);
    if (outline)
    {
        if (bold)
        {
            auto* outlineGlyph = reinterpret_cast<FT_OutlineGlyph>(glyphDesc);
            FT_Outline_Embolden(&outlineGlyph->outline, weight);
        }

        if (outlineThickness != 0)
        {
            FT_Stroker stroker = fontHandles.stroker;

            FT_Stroker_Set(stroker,
                           static_cast<FT_Fixed>(outlineThickness * float{1 << 6}),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);

            FT_Glyph_Stroke(&glyphDesc, stroker, true);
        }
    }

    // Convert the glyph to a bitmap (i.e. rasterize it)
    // Warning! After this line, do not read any data from glyphDesc directly, use
    // bitmapGlyph.root to access the FT_Glyph data.
    FT_Glyph_To_Bitmap(&glyphDesc, FT_RENDER_MODE_NORMAL, nullptr, 1);
    auto*      bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyphDesc);
    FT_Bitmap& bitmap      = bitmapGlyph->bitmap;

    // Apply bold if necessary -- fallback technique using bitmap (lower quality)
    if (!outline)
    {
        if (bold)
            FT_Bitmap_Embolden(fontHandles.library, &bitmap, weight, weight);

        if (outlineThickness != 0)
            sf::priv::err() << "Failed to outline glyph (no fallback available)";
    }

    sf::Glyph glyph;

    // Compute the glyph's advance offset
    glyph.advance = static_cast<float>(bitmapGlyph->root.advance.x >> 16);
    if (bold)
        glyph.advance += static_cast<float>(weight) / float{1 << 6};

    glyph.lsbDelta = static_cast<int>(face->glyph->lsb_delta);
    glyph.rsbDelta = static_cast<int>(face->glyph->rsb_delta);

    sf::Vector2u size(bitmap.width, bitmap.rows);

    if ((size.x > 0) && (size.y > 0))
    {
        // Leave a small padding around characters, so that filtering doesn't
        // pollute them with pixels from neighbors
        const unsigned int padding = 2;

        size += 2u * sf::Vector2u{padding, padding};

        // Find a good position for the new glyph into the texture
        {
            const sf::Vector2u pos = textureAtlas.getRectPacker().pack(size).value(); // TODO P0: what if there is no room?

            glyph.textureRect = {.position{static_cast<int>(pos.x), static_cast<int>(pos.y)},
                                 .size{static_cast<int>(size.x), static_cast<int>(size.y)}};
        }

        // Make sure the texture data is positioned in the center
        // of the allocated texture rectangle
        glyph.textureRect.position += sf::Vector2i{padding, padding};
        glyph.textureRect.size -= 2 * sf::Vector2i{padding, padding};

        // Compute the glyph's bounding box
        glyph.bounds.position = sf::Vector2i(bitmapGlyph->left, -bitmapGlyph->top).to<sf::Vector2f>();
        glyph.bounds.size     = sf::Vector2u(bitmap.width, bitmap.rows).to<sf::Vector2f>();

        // Resize the pixel buffer to the new size and fill it with transparent white pixels
        pixelBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * 4);

        std::uint8_t* current = pixelBuffer.data();
        std::uint8_t* end     = current + size.x * size.y * 4;

        while (current != end)
        {
            (*current++) = 255;
            (*current++) = 255;
            (*current++) = 255;
            (*current++) = 0;
        }

        // Extract the glyph's pixels from the bitmap
        const std::uint8_t* pixels = bitmap.buffer;
        if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            // Pixels are 1 bit monochrome values
            for (unsigned int y = padding; y < size.y - padding; ++y)
            {
                for (unsigned int x = padding; x < size.x - padding; ++x)
                {
                    // The color channels remain white, just fill the alpha channel
                    const std::size_t index = x + y * size.x;
                    pixelBuffer[index * 4 + 3] = ((pixels[(x - padding) / 8]) & (1 << (7 - ((x - padding) % 8)))) ? 255 : 0;
                }

                pixels += bitmap.pitch;
            }
        }
        else
        {
            // Pixels are 8 bit gray levels
            for (unsigned int y = padding; y < size.y - padding; ++y)
            {
                for (unsigned int x = padding; x < size.x - padding; ++x)
                {
                    // The color channels remain white, just fill the alpha channel
                    const std::size_t index    = x + y * size.x;
                    pixelBuffer[index * 4 + 3] = pixels[x - padding];
                }

                pixels += bitmap.pitch;
            }
        }

        // Write the pixels to the texture
        const auto dest       = glyph.textureRect.position.to<sf::Vector2u>() - sf::Vector2u{padding, padding};
        const auto updateSize = glyph.textureRect.size.to<sf::Vector2u>() + 2u * sf::Vector2u{padding, padding};
        textureAtlas.getTexture().update(pixelBuffer.data(), updateSize, dest);
    }

    // Delete the FT glyph
    FT_Done_Glyph(glyphDesc);

    // Done :)
    return glyph;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Font::Impl
{
    using GlyphTable = std::unordered_map</* character size */ unsigned int,
                                          std::unordered_map</* combined key */ std::uint64_t, Glyph>>; //!< Table mapping a codepoint to its glyph

    [[nodiscard]] static Vector2u getMaxTextureSizeVec(GraphicsContext& graphicsContext)
    {
        const unsigned int size = Texture::getMaximumSize(graphicsContext);
        return {size, size};
    }

    [[nodiscard]] static base::Optional<TextureAtlas> initFallbackTextureAtlas(GraphicsContext& theGraphicsContext,
                                                                               TextureAtlas*    ptr)
    {
        if (ptr != nullptr)
            return base::nullOpt;

        return base::makeOptional<TextureAtlas>(Texture::create(theGraphicsContext, {1024u, 1024u}).value());
    }

    explicit Impl(GraphicsContext&               theGraphicsContext,
                  TextureAtlas*                  theTextureAtlasPtr,
                  std::shared_ptr<FontHandles>&& theFontHandles,
                  const char*                    theFamilyName) :
    graphicsContext(&theGraphicsContext),
    textureAtlasPtr(theTextureAtlasPtr),
    fallbackTextureAtlas(initFallbackTextureAtlas(theGraphicsContext, theTextureAtlasPtr)),
    fontHandles(SFML_BASE_MOVE(theFontHandles)),
    info{theFamilyName}
    {
    }

    GraphicsContext* graphicsContext; //!< The window context

    TextureAtlas*                        textureAtlasPtr;      //!< Texture atlas containing the pixels of the glyphs
    mutable base::Optional<TextureAtlas> fallbackTextureAtlas; //!< TODO P1: docs

    std::shared_ptr<FontHandles> fontHandles;    //!< Shared information about the internal font instance
    bool                         isSmooth{true}; //!< Status of the smooth filter
    FontInfo                     info;           //!< Information about the font

    mutable GlyphTable glyphs; //!< Table mapping code points to their corresponding glyph

    mutable std::vector<std::uint8_t> pixelBuffer; //!< Pixel buffer holding a glyph's pixels before being written to the texture

#ifdef SFML_SYSTEM_ANDROID
    base::UniquePtr<priv::ResourceStream> m_stream; //!< Asset file streamer (if loaded from file)
#endif

    TextureAtlas& getTextureAtlas() const
    {
        return textureAtlasPtr == nullptr ? *fallbackTextureAtlas : *textureAtlasPtr;
    }
};


////////////////////////////////////////////////////////////
Font::Font(base::PassKey<Font>&&,
           GraphicsContext& graphicsContext,
           TextureAtlas*    textureAtlas,
           void*            fontHandlesSharedPtr,
           const char*      familyName) :
m_impl(graphicsContext, textureAtlas, SFML_BASE_MOVE(*static_cast<std::shared_ptr<FontHandles>*>(fontHandlesSharedPtr)), familyName)
{
}


////////////////////////////////////////////////////////////
Font::~Font() = default;


////////////////////////////////////////////////////////////
Font::Font(const Font&) = default;


////////////////////////////////////////////////////////////
Font::Font(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
Font& Font::operator=(const Font&) = default;


////////////////////////////////////////////////////////////
Font& Font::operator=(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromFile(GraphicsContext& graphicsContext, const Path& filename, TextureAtlas* textureAtlas)
{
    [[maybe_unused]] const auto fail = [&](const char* what)
    {
        priv::err() << "Failed to load font (" << priv::PathDebugFormatter{filename} << "): " << what;
        return base::nullOpt;
    };

#ifndef SFML_SYSTEM_ANDROID

    auto fontHandles = std::make_shared<FontHandles>();

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&fontHandles->library) != 0)
        return fail("failed to initialize FreeType");

    // Load the new font face from the specified file
    FT_Face face = nullptr;
    if (FT_New_Face(fontHandles->library, filename.to<std::string>().c_str(), 0, &face) != 0)
        return fail("failed to create the font face");

    fontHandles->face = face;

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(fontHandles->library, &fontHandles->stroker) != 0)
        return fail("failed to create the stroker");

    // Select the unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
        return fail("failed to set the Unicode character set");

    return base::makeOptional<Font>(base::PassKey<Font>{}, graphicsContext, textureAtlas, &fontHandles, face->family_name);

#else

    auto stream = base::makeUnique<priv::ResourceStream>(filename);
    auto font   = openFromStream(*stream);

    if (font)
        font->m_stream = SFML_BASE_MOVE(stream);

    return font;

#endif
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromMemory(GraphicsContext& graphicsContext,
                                          const void*      data,
                                          std::size_t      sizeInBytes,
                                          TextureAtlas*    textureAtlas)
{
    const auto fail = [&](const char* what)
    {
        priv::err() << "Failed to load font from memory: " << what;
        return base::nullOpt;
    };

    auto fontHandles = std::make_shared<FontHandles>();

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&fontHandles->library) != 0)
        return fail("failed to initialize FreeType");

    // Load the new font face from the specified file
    FT_Face face = nullptr;
    if (FT_New_Memory_Face(fontHandles->library,
                           reinterpret_cast<const FT_Byte*>(data),
                           static_cast<FT_Long>(sizeInBytes),
                           0,
                           &face) != 0)
        return fail("failed to create the font face");

    fontHandles->face = face;

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(fontHandles->library, &fontHandles->stroker) != 0)
        return fail("failed to create the stroker");

    // Select the Unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
        return fail("failed to set the Unicode character set");

    return base::makeOptional<Font>(base::PassKey<Font>{}, graphicsContext, textureAtlas, &fontHandles, face->family_name);
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromStream(GraphicsContext& graphicsContext, InputStream& stream, TextureAtlas* textureAtlas)
{
    const auto fail = [&](const char* what)
    {
        priv::err() << "Failed to load font from stream: " << what;
        return base::nullOpt;
    };

    auto fontHandles = std::make_shared<FontHandles>();

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&fontHandles->library) != 0)
        return fail("failed to initialize FreeType");

    // Make sure that the stream's reading position is at the beginning
    if (!stream.seek(0).hasValue())
        return fail("failed to seek font stream");

    // Prepare a wrapper for our stream, that we'll pass to FreeType callbacks
    fontHandles->streamRec.base               = nullptr;
    fontHandles->streamRec.size               = static_cast<unsigned long>(stream.getSize().value());
    fontHandles->streamRec.pos                = 0;
    fontHandles->streamRec.descriptor.pointer = &stream;
    fontHandles->streamRec.read               = &read;
    fontHandles->streamRec.close              = &close;

    // Setup the FreeType callbacks that will read our stream
    FT_Open_Args args;
    args.flags  = FT_OPEN_STREAM;
    args.stream = &fontHandles->streamRec;
    args.driver = nullptr;

    // Load the new font face from the specified stream
    FT_Face face = nullptr;
    if (FT_Open_Face(fontHandles->library, &args, 0, &face) != 0)
        return fail("failed to create the font face");

    fontHandles->face = face;

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(fontHandles->library, &fontHandles->stroker) != 0)
        return fail("failed to create the stroker");

    // Select the Unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
        return fail("failed to set the Unicode character set");

    return base::makeOptional<Font>(base::PassKey<Font>{}, graphicsContext, textureAtlas, &fontHandles, face->family_name);
}


////////////////////////////////////////////////////////////
const FontInfo& Font::getInfo() const
{
    return m_impl->info;
}


////////////////////////////////////////////////////////////
unsigned int Font::getCharIndex(std::uint32_t codePoint) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);
    return FT_Get_Char_Index(m_impl->fontHandles->face, codePoint);
}


////////////////////////////////////////////////////////////
const Glyph& Font::getGlyph(std::uint32_t codePoint, unsigned int characterSize, bool bold, float outlineThickness) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    // Get the page corresponding to the character size
    auto& glyphs = m_impl->glyphs[characterSize];

    // Build the key by combining the glyph index (based on code point), bold flag, and outline thickness
    const std::uint64_t key = combine(outlineThickness, bold, getCharIndex(codePoint));

    // Glyph cached: just return it
    if (const auto it = glyphs.find(key); it != glyphs.end())
        return it->second;

    // Glyph not cached: we have to load it
    const Glyph loadedGlyph = loadGlyph(*m_impl->fontHandles,
                                        m_impl->getTextureAtlas(),
                                        m_impl->pixelBuffer,
                                        codePoint,
                                        characterSize,
                                        bold,
                                        outlineThickness);

    return glyphs.emplace(key, loadedGlyph).first->second;
}


////////////////////////////////////////////////////////////
bool Font::hasGlyph(std::uint32_t codePoint) const
{
    return getCharIndex(codePoint) != 0;
}


////////////////////////////////////////////////////////////
float Font::getKerning(std::uint32_t first, std::uint32_t second, unsigned int characterSize, bool bold) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    // Special case where first or second is 0 (null character)
    if (first == 0 || second == 0)
        return 0.f;

    FT_Face face = m_impl->fontHandles->face;

    if (!face || !setCurrentSize(characterSize))
    {
        // Invalid font
        return 0.f;
    }

    // Convert the characters to indices
    const FT_UInt index1 = FT_Get_Char_Index(face, first);
    const FT_UInt index2 = FT_Get_Char_Index(face, second);

    // Retrieve position compensation deltas generated by FT_LOAD_FORCE_AUTOHINT flag
    const auto firstRsbDelta  = static_cast<float>(getGlyph(first, characterSize, bold).rsbDelta);
    const auto secondLsbDelta = static_cast<float>(getGlyph(second, characterSize, bold).lsbDelta);

    // Get the kerning vector if present
    FT_Vector kerning{0, 0};
    if (FT_HAS_KERNING(face))
        FT_Get_Kerning(face, index1, index2, FT_KERNING_UNFITTED, &kerning);

    // X advance is already in pixels for bitmap fonts
    if (!FT_IS_SCALABLE(face))
        return static_cast<float>(kerning.x);

    // Combine kerning with compensation deltas and return the X advance
    // Flooring is required as we use FT_KERNING_UNFITTED flag which is not quantized in 64 based grid
    return base::floor((secondLsbDelta - firstRsbDelta + static_cast<float>(kerning.x) + 32) / float{1 << 6});
}


////////////////////////////////////////////////////////////
float Font::getLineSpacing(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    FT_Face face = m_impl->fontHandles->face;

    if (setCurrentSize(characterSize))
        return static_cast<float>(face->size->metrics.height) / float{1 << 6};

    return 0.f;
}


////////////////////////////////////////////////////////////
float Font::getUnderlinePosition(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    FT_Face face = m_impl->fontHandles->face;

    if (setCurrentSize(characterSize))
    {
        // Return a fixed position if font is a bitmap font
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(characterSize) / 10.f;

        return -static_cast<float>(FT_MulFix(face->underline_position, face->size->metrics.y_scale)) / float{1 << 6};
    }

    return 0.f;
}


////////////////////////////////////////////////////////////
float Font::getUnderlineThickness(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    FT_Face face = m_impl->fontHandles->face;

    if (face && setCurrentSize(characterSize))
    {
        // Return a fixed thickness if font is a bitmap font
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(characterSize) / 14.f;

        return static_cast<float>(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale)) / float{1 << 6};
    }

    return 0.f;
}


////////////////////////////////////////////////////////////
const Texture& Font::getTexture(unsigned int /* characterSize */) const
{
    return m_impl->getTextureAtlas().getTexture();
}


////////////////////////////////////////////////////////////
void Font::setSmooth(bool smooth)
{
    if (smooth == m_impl->isSmooth)
        return;

    m_impl->isSmooth = smooth;
    m_impl->getTextureAtlas().getTexture().setSmooth(m_impl->isSmooth);
}


////////////////////////////////////////////////////////////
bool Font::isSmooth() const
{
    return m_impl->isSmooth;
}


////////////////////////////////////////////////////////////
bool Font::setCurrentSize(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);
    SFML_BASE_ASSERT(m_impl->fontHandles->face);

    return setFaceCurrentSize(m_impl->fontHandles->face, characterSize);
}

} // namespace sf
