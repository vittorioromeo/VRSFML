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

#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/MemoryInputStream.hpp"
#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

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

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"


namespace
{
////////////////////////////////////////////////////////////
// FreeType callbacks that operate on a sf::InputStream
[[nodiscard]] unsigned long read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    auto* stream = static_cast<sf::InputStream*>(rec->descriptor.pointer);
    if (sf::base::Optional seekResult = stream->seek(offset); seekResult.hasValue() && *seekResult == offset)
    {
        return count == 0ul ? 0ul
                            : static_cast<unsigned long>(stream->read(reinterpret_cast<char*>(buffer), count).value());
    }

    return count == 0ul ? 1ul : 0ul; // error code is 0 if we're reading, or nonzero if we're seeking
}


////////////////////////////////////////////////////////////
void close(FT_Stream)
{
}


////////////////////////////////////////////////////////////
// Helper to interpret memory as a specific type
template <typename T, typename U>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline T reinterpret(const U input)
{
    T output;
    SFML_BASE_MEMCPY(&output, &input, sizeof(U));
    return output;
}


////////////////////////////////////////////////////////////
// Combine outline thickness, boldness and font glyph index into a single 64-bit key
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline sf::base::U64 combineGlyphTableKey(
    const float    outlineThickness,
    const bool     bold,
    const char32_t index)
{
    return (sf::base::U64{reinterpret<sf::base::U32>(outlineThickness)} << 32) | (sf::base::U64{bold} << 31) | index;
}


////////////////////////////////////////////////////////////
// Helper function to combine two `char32_t` into a single 64-bit key
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline sf::base::U64 combineKerningCharPair(
    const char32_t first,
    const char32_t second)
{
    return (static_cast<sf::base::U64>(first) << 32) | static_cast<sf::base::U64>(second);
}


////////////////////////////////////////////////////////////
// Helper function to combine character size and bold flag into a 64-bit key
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline sf::base::U64 combineKerningSizeBold(
    const unsigned int characterSize,
    const bool         bold)
{
    return (static_cast<sf::base::U64>(characterSize) << 1) | static_cast<sf::base::U64>(bold);
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool setFaceCurrentSize(const FT_Face face, const unsigned int characterSize)
{
    // FT_Set_Pixel_Sizes is an expensive function, so we must call it
    // only when necessary to avoid killing performance

    const FT_UShort currentSize = face->size->metrics.x_ppem;

    if (currentSize == characterSize)
        return true;

    const FT_Error result = FT_Set_Pixel_Sizes(face, 0u, characterSize);

    if (result != FT_Err_Invalid_Pixel_Size)
        return result == FT_Err_Ok;

    if (FT_IS_SCALABLE(face))
    {
        sf::priv::err() << "Failed to set font size to " << characterSize;
        return false;
    }

    // In the case of bitmap fonts, resizing can fail if the requested size is not available

    auto& multilineErr = sf::priv::err(true /* multiLine */);
    multilineErr << "Failed to set bitmap font size to " << characterSize << '\n' << "Available sizes are: ";

    for (int i = 0; i < face->num_fixed_sizes; ++i)
    {
        const long size = (face->available_sizes[i].y_ppem + 32) >> 6;
        multilineErr << size << " ";
    }

    multilineErr << '\n';
    return false;
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::Glyph loadGlyph(
    const FT_Library&               library,
    const FT_Face&                  face,
    const FT_Stroker&               stroker,
    sf::TextureAtlas&               textureAtlas,
    sf::base::Vector<sf::base::U8>& pixelBuffer,
    const char32_t                  codePoint,
    const unsigned int              characterSize,
    const bool                      bold,
    const float                     outlineThickness)
{
    sf::Glyph glyph; // Use a single local variable for NRVO

    // Get our FT_Face
    if (!face)
        return glyph; // Empty glyph

    // Set the character size
    if (!setFaceCurrentSize(face, characterSize))
        return glyph; // Empty glyph

    // Load the glyph corresponding to the code point
    const FT_Int32 flags = outlineThickness == 0.f ? FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT
                                                   : FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_NO_BITMAP;

    if (FT_Load_Char(face, codePoint, flags) != 0)
        return glyph; // Empty glyph

    // Retrieve the glyph
    FT_Glyph glyphDesc = nullptr;
    if (FT_Get_Glyph(face->glyph, &glyphDesc) != 0)
        return glyph; // Empty glyph

    // Apply bold and outline (there is no fallback for outline) if necessary -- first technique using outline (highest quality)
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
            FT_Stroker_Set(stroker,
                           static_cast<FT_Fixed>(outlineThickness * float{1 << 6}),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);

            FT_Glyph_Stroke(&glyphDesc, stroker, true);
        }
    }

    // Convert the glyph to a bitmap (i.e. rasterize it)
    // Warning! After this line, do not read any data from `glyphDesc` directly, use
    // `bitmapGlyph.root` to access the `FT_Glyph` data.
    FT_Glyph_To_Bitmap(&glyphDesc, FT_RENDER_MODE_NORMAL, nullptr, 1);
    auto*      bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyphDesc);
    FT_Bitmap& bitmap      = bitmapGlyph->bitmap;

    // Apply bold if necessary -- fallback technique using bitmap (lower quality)
    if (!supportsOutline)
    {
        if (bold)
            FT_Bitmap_Embolden(library, &bitmap, weight, weight);

        if (outlineThickness != 0.f)
            sf::priv::err() << "Failed to outline glyph (no fallback available)";
    }

    // Compute the glyph's advance offset
    glyph.advance = static_cast<float>(bitmapGlyph->root.advance.x >> 16) +
                    (bold ? static_cast<float>(weight) / float{1 << 6} : 0.f);

    glyph.lsbDelta = static_cast<int>(face->glyph->lsb_delta);
    glyph.rsbDelta = static_cast<int>(face->glyph->rsb_delta);

    if (bitmap.width == 0u || bitmap.rows == 0u)
    {
        FT_Done_Glyph(glyphDesc);
        return glyph;
    }

    // Leave a small padding around characters, so that filtering doesn't
    // pollute them with pixels from neighbors
    constexpr unsigned int padding = 2u;

    const sf::Vector2u size{bitmap.width + 2u * padding, bitmap.rows + 2u * padding};

    // Find a good position for the new glyph into the texture
    {
        const auto pos = textureAtlas.getRectPacker().pack(size).value().toVector2f(); // TODO P0: what if there is no room?
        glyph.textureRect = {pos, size.toVector2f()};
    }

    // Make sure the texture data is positioned in the center
    // of the allocated texture rectangle
    glyph.textureRect.position += sf::Vector2f{padding, padding};
    glyph.textureRect.size -= 2.f * sf::Vector2f{padding, padding};

    // Compute the glyph's bounding box
    glyph.bounds.position = sf::Vector2i(bitmapGlyph->left, -bitmapGlyph->top).toVector2f();
    glyph.bounds.size     = sf::Vector2u(bitmap.width, bitmap.rows).toVector2f();

    // Resize the pixel buffer to the new size and fill it with transparent white pixels
    pixelBuffer.resize(static_cast<sf::base::SizeT>(size.x) * static_cast<sf::base::SizeT>(size.y) * 4u);

    sf::base::U8* current = pixelBuffer.data();
    sf::base::U8* end     = current + size.x * size.y * 4u;

    while (current != end)
    {
        (*current++) = 255u;
        (*current++) = 255u;
        (*current++) = 255u;
        (*current++) = 0u;
    }

    // Extract the glyph's pixels from the bitmap
    const sf::base::U8* pixels = bitmap.buffer;
    if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
    {
        // Pixels are 1 bit monochrome values
        for (unsigned int y = padding; y < size.y - padding; ++y)
        {
            for (unsigned int x = padding; x < size.x - padding; ++x)
            {
                // The color channels remain white, just fill the alpha channel
                const sf::base::SizeT index = x + y * size.x;
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
                const sf::base::SizeT index = x + y * size.x;
                pixelBuffer[index * 4 + 3]  = pixels[x - padding];
            }

            pixels += bitmap.pitch;
        }
    }

    // Write the pixels to the texture
    const auto dest       = glyph.textureRect.position.toVector2u() - sf::Vector2u{padding, padding};
    const auto updateSize = glyph.textureRect.size.toVector2u() + 2u * sf::Vector2u{padding, padding};
    textureAtlas.getTexture().update(pixelBuffer.data(), updateSize, dest);

    // Delete the FT glyph
    FT_Done_Glyph(glyphDesc);
    return glyph;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Font::Impl
{
    template <typename K, typename V>
    using MapType = ankerl::unordered_dense::map<K, V>;

    using GlyphTable = MapType</* character size */ unsigned int,
                               MapType</* combined key */ base::U64, Glyph>>; //!< Table mapping a codepoint to its glyph

    explicit Impl(TextureAtlas* theTextureAtlasPtr) :
    textureAtlasPtr{theTextureAtlasPtr},
    fallbackTextureAtlas{theTextureAtlasPtr == nullptr
                             ? base::makeOptional<TextureAtlas>(Texture::create({1024u, 1024u}, {.smooth = true}).value())
                             : base::nullOpt}
    {
    }

    ~Impl()
    {
        // All the function below are safe to call with null pointer arguments.
        // The documentation of FreeType isn't clear on the matter, but the
        // implementation does explicitly check for null.

        FT_Stroker_Done(ftStroker);
        FT_Done_Face(ftFace);
        // `streamRec` doesn't need to be explicitly freed.
        FT_Done_FreeType(ftLibrary);
    }

    Impl(const Impl&) = delete;
    Impl(Impl&&)      = delete;

    FT_Library   ftLibrary{};   //< Pointer to the internal library interface
    FT_StreamRec ftStreamRec{}; //< Stream rec object describing an input stream
    FT_Face      ftFace{};      //< Pointer to the internal font face
    FT_Stroker   ftStroker{};   //< Pointer to the stroker

    TextureAtlas* textureAtlasPtr; //!< User-provided referenced texture atlas to store glyphs
    mutable base::Optional<TextureAtlas> fallbackTextureAtlas; //!< Owned texture atlas used if the user didn't provide one

    bool     isSmooth{true}; //!< Status of the smooth filter
    FontInfo info;           //!< Information about the font

    mutable GlyphTable glyphs; //!< Table mapping code points to their corresponding glyph

    mutable base::Vector<base::U8> pixelBuffer; //!< Pixel buffer holding a glyph's pixels before being written to the texture

    // Key for the outer map: combines character size and bold flag
    // Key for the inner map: combines the two char32_t code points
    mutable MapType<base::U64, MapType<base::U64, float>> kerningCache;   //!< Cache for kerning values
    mutable MapType<char32_t, unsigned int>               charIndexCache; //!< Cache for character indices

    base::UniquePtr<InputStream> stream; //!< Stream for `openFromFile` and `openFromMemory`

    [[nodiscard]] TextureAtlas& getTextureAtlas() const
    {
        return textureAtlasPtr == nullptr ? *fallbackTextureAtlas : *textureAtlasPtr;
    }
};


////////////////////////////////////////////////////////////
Font::Font(base::PassKey<Font>&&, TextureAtlas* textureAtlas) : m_impl(base::makeUnique<Impl>(textureAtlas))
{
}


////////////////////////////////////////////////////////////
Font::~Font() = default;


////////////////////////////////////////////////////////////
Font::Font(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
Font& Font::operator=(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromFile(const Path& filename, TextureAtlas* textureAtlas)
{
    base::Optional<Font> result; // Use a single local variable for NRVO

#ifndef SFML_SYSTEM_ANDROID

    // Create the input stream and open the file
    auto optStream = FileInputStream::open(filename);
    if (!optStream.hasValue())
    {
        priv::err() << "Failed to load font (" << priv::PathDebugFormatter{filename} << "): failed to open file";
        return result; // Empty optional
    }

    auto                  stream = base::makeUnique<FileInputStream>(SFML_BASE_MOVE(*optStream));
    constexpr const char* type   = "file";

#else

    // Create the input stream and open the file
    auto optStream = ResourceStream::open(filename);
    if (!optStream.hasValue())
    {
        priv::err() << "Failed to load font (" << priv::PathDebugFormatter{filename} << "): failed to open file";
        return result; // Empty optional
    }

    auto                  stream = base::makeUnique<priv::ResourceStream>(SFML_BASE_MOVE(*optStream));
    constexpr const char* type   = "Android resource stream";

#endif

    result = openFromStreamImpl(*stream, textureAtlas, type);

    // Open the font, and if successful save the stream to keep it alive
    if (result.hasValue())
        result->m_impl->stream = SFML_BASE_MOVE(stream);
    else
    {
        // If loading failed, print filename (after the error message already printed in openFromStreamImpl)
        priv::err() << priv::PathDebugFormatter{filename};
    }

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromMemory(const void* data, base::SizeT sizeInBytes, TextureAtlas* textureAtlas)
{
    base::Optional<Font> result; // Use a single local variable for NRVO

    if (!data)
    {
        priv::err() << "Failed to load font from memory: provided data pointer is null";
        return result; // Empty optional
    }

    // Create memory stream - the memory is owned by the user
    auto memoryStream = base::makeUnique<MemoryInputStream>(data, sizeInBytes);

    result = openFromStreamImpl(*memoryStream, textureAtlas, "memory");

    // Open the font, and if successful save the stream to keep it alive
    if (result.hasValue())
        result->m_impl->stream = SFML_BASE_MOVE(memoryStream);

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromStreamImpl(InputStream& stream, TextureAtlas* const textureAtlas, const char* const type)
{
    const auto fail = [&](const char* what)
    {
        priv::err() << "Failed to load font from stream (type:" << type << "): " << what;
        return base::nullOpt;
    };

    auto  result = base::makeOptional<Font>(base::PassKey<Font>{}, textureAtlas);
    auto& impl   = *result->m_impl;

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&impl.ftLibrary) != 0)
        return fail("failed to initialize FreeType");

    // Prepare a wrapper for our stream, that we'll pass to FreeType callbacks
    impl.ftStreamRec.base               = nullptr;
    impl.ftStreamRec.size               = static_cast<unsigned long>(stream.getSize().value());
    impl.ftStreamRec.pos                = 0;
    impl.ftStreamRec.descriptor.pointer = &stream;
    impl.ftStreamRec.read               = &read;
    impl.ftStreamRec.close              = &close;

    // Setup the FreeType callbacks that will read our stream
    FT_Open_Args args;
    args.flags  = FT_OPEN_STREAM;
    args.stream = &impl.ftStreamRec;
    args.driver = nullptr;

    // Load the new font face from the specified stream
    if (FT_Open_Face(impl.ftLibrary, &args, 0, &impl.ftFace) != 0)
        return fail("failed to create the font face");

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(impl.ftLibrary, &impl.ftStroker) != 0)
        return fail("failed to create the stroker");

    // Select the Unicode character map
    if (FT_Select_Charmap(impl.ftFace, FT_ENCODING_UNICODE) != 0)
        return fail("failed to set the Unicode character set");

    // Set family name
    result->m_impl->info.family = impl.ftFace->family_name;

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromStream(InputStream& stream, TextureAtlas* const textureAtlas)
{
    // Make sure that the stream's reading position is at the beginning
    if (!stream.seek(0).hasValue())
    {
        priv::err() << "Failed to seek font stream";
        return base::nullOpt;
    }

    // Open the font, do not save the stream in m_stream, its owned by the caller
    return openFromStreamImpl(stream, textureAtlas, "stream");
}


////////////////////////////////////////////////////////////
const FontInfo& Font::getInfo() const
{
    return m_impl->info;
}


////////////////////////////////////////////////////////////
unsigned int Font::getCharIndex(const char32_t codePoint) const
{
    if (const auto* it = m_impl->charIndexCache.find(codePoint); it != m_impl->charIndexCache.end())
        return it->second;

    const auto result                 = FT_Get_Char_Index(m_impl->ftFace, codePoint);
    m_impl->charIndexCache[codePoint] = result;
    return result;
}


////////////////////////////////////////////////////////////
const Glyph& Font::getGlyph(const char32_t     codePoint,
                            const unsigned int characterSize,
                            const bool         bold,
                            const float        outlineThickness) const
{
    // Get the page corresponding to the character size
    auto& glyphs = m_impl->glyphs[characterSize];

    // Build the key by combining the glyph index (based on code point), bold flag, and outline thickness
    const base::U64 key = combineGlyphTableKey(outlineThickness, bold, getCharIndex(codePoint));

    // Glyph cached: just return it
    if (const auto* it = glyphs.find(key); it != glyphs.end())
        return it->second;

    // Glyph not cached: we have to load it
    const Glyph loadedGlyph = loadGlyph(m_impl->ftLibrary,
                                        m_impl->ftFace,
                                        m_impl->ftStroker,
                                        m_impl->getTextureAtlas(),
                                        m_impl->pixelBuffer,
                                        codePoint,
                                        characterSize,
                                        bold,
                                        outlineThickness);

    return glyphs.try_emplace(key, loadedGlyph).first->second;
}


////////////////////////////////////////////////////////////
bool Font::hasGlyph(const char32_t codePoint) const
{
    return getCharIndex(codePoint) != 0u;
}


////////////////////////////////////////////////////////////
float Font::getKerning(const char32_t first, const char32_t second, const unsigned int characterSize, const bool bold) const
{
    SFML_BASE_ASSERT(m_impl->ftLibrary != nullptr);

    // Special case where first or second is 0 (null character)
    if (first == 0 || second == 0)
        return 0.f;

    // Cache lookup
    const base::U64 sizeBoldKey = combineKerningSizeBold(characterSize, bold);
    const base::U64 pairKey     = combineKerningCharPair(first, second);

    // Find the map for the given size and bold state
    if (const auto* sizeBoldIt = m_impl->kerningCache.find(sizeBoldKey); sizeBoldIt != m_impl->kerningCache.end())
    {
        // Find the kerning value for the specific character pair
        const auto& pairMap = sizeBoldIt->second;
        if (const auto* pairIt = pairMap.find(pairKey); pairIt != pairMap.end())
            return pairIt->second;
    }

    FT_Face face = m_impl->ftFace;

    if (!face || !setCurrentSize(characterSize))
    {
        // Invalid font
        m_impl->kerningCache[sizeBoldKey][pairKey] = 0.f;
        return 0.f;
    }

    // Convert the characters to indices
    const FT_UInt index1 = FT_Get_Char_Index(face, first);
    const FT_UInt index2 = FT_Get_Char_Index(face, second);

    // Retrieve position compensation deltas generated by FT_LOAD_FORCE_AUTOHINT flag
    const auto firstRsbDelta  = static_cast<float>(getGlyph(first, characterSize, bold).rsbDelta);
    const auto secondLsbDelta = static_cast<float>(getGlyph(second, characterSize, bold).lsbDelta);

    float calculatedKerning = 0.f;

    // Get the kerning vector if present
    FT_Vector kerning{0, 0};
    if (FT_HAS_KERNING(face))
        FT_Get_Kerning(face, index1, index2, FT_KERNING_UNFITTED, &kerning);

    // X advance is already in pixels for bitmap fonts
    if (!FT_IS_SCALABLE(face))
    {
        calculatedKerning = static_cast<float>(kerning.x);
    }
    else
    {
        // Combine kerning with compensation deltas and return the X advance
        // Flooring is required as we use FT_KERNING_UNFITTED flag which is not quantized in 64 based grid
        calculatedKerning = SFML_BASE_MATH_FLOORF(
            (secondLsbDelta - firstRsbDelta + static_cast<float>(kerning.x) + 32) / float{1 << 6});
    }

    m_impl->kerningCache[sizeBoldKey][pairKey] = calculatedKerning;
    return calculatedKerning;
}


////////////////////////////////////////////////////////////
float Font::getLineSpacing(const unsigned int characterSize) const
{
    FT_Face face = m_impl->ftFace;

    if (setCurrentSize(characterSize))
        return static_cast<float>(face->size->metrics.height) / float{1 << 6};

    return 0.f;
}


////////////////////////////////////////////////////////////
float Font::getUnderlinePosition(const unsigned int characterSize) const
{
    FT_Face face = m_impl->ftFace;

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
float Font::getUnderlineThickness(const unsigned int characterSize) const
{
    FT_Face face = m_impl->ftFace;

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
const Texture& Font::getTexture() const
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
    return setFaceCurrentSize(m_impl->ftFace, characterSize);
}

} // namespace sf
