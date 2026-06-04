// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Image.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#ifdef SFML_SYSTEM_ANDROID
    #include "SFML/System/Android/Activity.hpp"
    #include "SFML/System/Android/ResourceStream.hpp"
#endif

#define QOI_IMPLEMENTATION

// Avoid conflicts when static linking:
// See https://github.com/SFML/SFML/commit/548cdb9c394da1b08e563a1d7f6fa6225a6f3ece
#define qoi_decode sf_qoi_decode
#define qoi_encode sf_qoi_encode
#define qoi_read   sf_qoi_read
#define qoi_write  sf_qoi_write

#include <qoi.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Warray-bounds"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#pragma GCC diagnostic pop

#include <cstdlib>


namespace
{
////////////////////////////////////////////////////////////
// stb_image callback for constructing a buffer
void bufferFromCallback(void* const context, void* const data, int size)
{
    const auto* const source = static_cast<sf::base::U8*>(data);
    auto* const       dest   = static_cast<sf::base::Vector<sf::base::U8>*>(context);

    dest->emplaceRange(source, static_cast<sf::base::SizeT>(size));
}

////////////////////////////////////////////////////////////
// Deleter for STB pointers
struct StbDeleter
{
    void operator()(stbi_uc* const image) const
    {
        stbi_image_free(image);
    }
};


////////////////////////////////////////////////////////////
using StbPtr = sf::base::UniquePtr<stbi_uc, StbDeleter>;


////////////////////////////////////////////////////////////
// RAII wrapper for malloc-ed pointers
// (this is used with qoi.h)
struct MallocPointerDeleter
{
    void operator()(sf::base::U8* ptr) const
    {
        free(ptr); // NOLINT(*-no-malloc)
    }
};


////////////////////////////////////////////////////////////
using MallocPtr = sf::base::UniquePtr<sf::base::U8, MallocPointerDeleter>;


////////////////////////////////////////////////////////////
// A helper to check if the given buffer is a valid QOI file magic number
[[nodiscard]] bool isQoiMagicNumber(const char* const buffer, const sf::base::SizeT size)
{
    return size >= 4 && buffer[0] == 'q' && buffer[1] == 'o' && buffer[2] == 'i' && buffer[3] == 'f';
}


////////////////////////////////////////////////////////////
struct QOISaveData
{
    MallocPtr       ptr;
    sf::base::SizeT size;
};


////////////////////////////////////////////////////////////
[[nodiscard]] QOISaveData saveQOIImpl(const sf::base::U8* pixels, const sf::Vec2u size)
{
    const qoi_desc desc = {
        .width      = size.x,
        .height     = size.y,
        .channels   = 4,
        .colorspace = QOI_LINEAR,
    };

    int dataSize = 0;
    if (auto ptr = MallocPtr(static_cast<sf::base::U8*>(qoi_encode(pixels, &desc, &dataSize))))
        return {SFML_BASE_MOVE(ptr), static_cast<sf::base::SizeT>(dataSize)};

    return {nullptr, 0u};
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Optional<sf::Image> loadQOIImpl(sf::base::PassKey<sf::Image>&& passKey,
                                                        const sf::base::U8* const      data,
                                                        const sf::base::SizeT          size)
{
    qoi_desc   formatDesc{};
    const auto ptr = MallocPtr(static_cast<sf::base::U8*>(qoi_decode(data, static_cast<int>(size), &formatDesc, 4)));

    if (!ptr)
        return sf::base::nullOpt;

    const sf::Vec2u imageSize{formatDesc.width, formatDesc.height};
    return sf::base::makeOptional<sf::Image>(SFML_BASE_MOVE(passKey),
                                             imageSize,
                                             ptr.get(),
                                             ptr.get() + imageSize.x * imageSize.y * 4);
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
base::Optional<Image> Image::create(Vec2u size, Color color)
{
    base::Optional<Image> result; // Use a single local variable for NRVO

    if (size.x == 0 || size.y == 0)
    {
        priv::errMsg("Failed to create image, invalid size (zero) provided");
        return result; // Empty optional
    }

    result.emplace(base::PassKey<Image>{}, size, base::SizeT{size.x} * base::SizeT{size.y} * 4);

    // Fill it with the specified color
    base::U8*       ptr = result->m_pixels.data();
    base::U8* const end = ptr + result->m_pixels.size();

    while (ptr != end)
    {
        *ptr++ = color.r;
        *ptr++ = color.g;
        *ptr++ = color.b;
        *ptr++ = color.a;
    }

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::create(Vec2u size, const base::U8* pixels)
{
    if (size.x == 0 || size.y == 0)
    {
        priv::errMsg("Failed to create image, invalid size (zero) provided");
        return base::nullOpt;
    }

    if (pixels == nullptr)
    {
        priv::errMsg("Failed to create image, null pixels pointer provided");
        return base::nullOpt;
    }

    return base::makeOptional<Image>(base::PassKey<Image>{}, size, pixels, pixels + size.x * size.y * 4);
}


////////////////////////////////////////////////////////////
Image::Image(base::PassKey<Image>&&, Vec2u size, base::SizeT pixelCount) : m_size(size), m_pixels(pixelCount)
{
    SFML_BASE_ASSERT(size.x > 0 && "Attempted to create an image with size.x == 0");
    SFML_BASE_ASSERT(size.y > 0 && "Attempted to create an image with size.y == 0");
}


////////////////////////////////////////////////////////////
Image::Image(base::PassKey<Image>&&, Vec2u size, const base::U8* itBegin, const base::U8* itEnd) :
    m_size(size),
    m_pixels(itBegin, itEnd)
{
    SFML_BASE_ASSERT(size.x > 0 && "Attempted to create an image with size.x == 0");
    SFML_BASE_ASSERT(size.y > 0 && "Attempted to create an image with size.y == 0");
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromFile(const Path& filename)
{
    base::Optional<Image> result; // Use a single local variable for NRVO

#ifdef SFML_SYSTEM_ANDROID

    if (priv::getActivityStatesPtr() != nullptr)
    {
        priv::ResourceStream stream;

        if (!stream.open(filename))
            return result; // Empty optional

        return loadFromStream(stream);
    }

#endif

    // Read the entire file into the thread-local scratch buffer in a single
    // native I/O call, then delegate to `loadFromMemory` which is the single
    // decode entry point (handles QOI + stbi formats).
    base::Vector<char>& scratch = getThreadLocalScratchCharBuffer();

    if (!readFromFile(filename, scratch))
    {
        priv::errMsg("Failed to load image\n{}\nReason: Failed to open the file", priv::PathDebugFormatter{filename});

        return result; // Empty optional
    }

    // Hoist `result` to function scope so NRVO can apply (declaring it inside
    // an `if (...)` initializer would defeat the optimisation).
    result = loadFromMemory(scratch.data(), scratch.size());
    if (!result.hasValue())
    {
        // `loadFromMemory` already wrote a decoder-specific error; add path context.
        priv::errMsg("Failed to load image\n{}", priv::PathDebugFormatter{filename});
    }

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromMemory(const void* data, base::SizeT size)
{
    if (data == nullptr || size == 0)
    {
        priv::errMsg("Failed to load image from memory, no data provided");
        return base::nullOpt;
    }

    // QOI fast path: detect by magic and decode in-place. This is the single
    // QOI entry point -- `loadFromFile` and `loadFromStream` both delegate here.
    if (isQoiMagicNumber(static_cast<const char*>(data), size))
        return loadQOIImpl(base::PassKey<Image>{}, static_cast<const base::U8*>(data), size);

    // stb_image path for everything else (PNG/JPG/BMP/...)
    int width    = 0;
    int height   = 0;
    int channels = 0;

    const auto*  buffer = static_cast<const unsigned char*>(data);
    const StbPtr ptr(stbi_load_from_memory(buffer, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha));

    if (ptr == nullptr)
    {
        priv::errMsg("Failed to load image from memory. Reason: {}", stbi_failure_reason());
        return base::nullOpt;
    }

    SFML_BASE_ASSERT(width > 0 && "Loaded image from memory with width == 0");
    SFML_BASE_ASSERT(height > 0 && "Loaded image from memory with height == 0");

    return base::makeOptional<Image>(base::PassKey<Image>{},
                                     Vec2i{width, height}.toVec2u(),
                                     ptr.get(),
                                     ptr.get() + width * height * 4);
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromStream(InputStream& stream)
{
    // stb_image and QOI both need the entire encoded blob in memory anyway, so
    // skip the streaming-callbacks dance: slurp the stream into the scratch
    // buffer and delegate to `loadFromMemory`.
    if (!stream.seek(0).hasValue())
    {
        priv::errMsg("Failed to seek image stream");
        return base::nullOpt;
    }

    const base::Optional streamSize = stream.getSize();
    if (!streamSize.hasValue())
    {
        priv::errMsg("Failed to determine image stream size");
        return base::nullOpt;
    }

    base::Vector<char>& scratch = getThreadLocalScratchCharBuffer();
    scratch.reserve(*streamSize);
    scratch.unsafeSetSize(*streamSize);

    const base::Optional readSize = stream.read(scratch.data(), *streamSize);
    if (!readSize.hasValue() || *readSize != *streamSize)
    {
        priv::errMsg("Failed to read full image stream contents");
        return base::nullOpt;
    }

    return loadFromMemory(scratch.data(), scratch.size());
}


////////////////////////////////////////////////////////////
Vec2u Image::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void Image::createMaskFromColor(Color color, base::U8 alpha)
{
    // Make sure that the image is not empty
    SFML_BASE_ASSERT(!m_pixels.empty());

    // Replace the alpha of the pixels that match the transparent color
    base::U8* ptr = m_pixels.data();
    base::U8* end = ptr + m_pixels.size();

    while (ptr != end)
    {
        if ((ptr[0] == color.r) && (ptr[1] == color.g) && (ptr[2] == color.b) && (ptr[3] == color.a))
            ptr[3] = alpha;
        ptr += 4;
    }
}


////////////////////////////////////////////////////////////
bool Image::copy(const Image& source, Vec2u dest, const Rect2i& sourceRect, bool applyAlpha)
{
    // Make sure that both images are valid
    SFML_BASE_ASSERT(source.m_size.x > 0 && source.m_size.y > 0 && m_size.x > 0 && m_size.y > 0);

    // Make sure the sourceRect components are non-negative before casting them to unsigned values
    if (sourceRect.position.x < 0 || sourceRect.position.y < 0 || sourceRect.size.x < 0 || sourceRect.size.y < 0)
        return false;

    auto srcRect = sourceRect.toRect2u();

    // Use the whole source image as srcRect if the provided source rectangle is empty
    if (srcRect.size.x == 0 || srcRect.size.y == 0)
    {
        srcRect = Rect2u({0, 0}, source.m_size);
    }
    // Otherwise make sure the provided source rectangle fits into the source image
    else
    {
        // Checking the bottom right corner is enough because
        // left and top are non-negative and width and height are positive.
        if (source.m_size.x < srcRect.position.x + srcRect.size.x || source.m_size.y < srcRect.position.y + srcRect.size.y)
            return false;
    }

    // Make sure the destination position is within this image bounds
    if (m_size.x <= dest.x || m_size.y <= dest.y)
        return false;

    // Then find the valid size of the destination rectangle
    const Vec2u dstSize(base::min(m_size.x - dest.x, srcRect.size.x), base::min(m_size.y - dest.y, srcRect.size.y));

    // Precompute as much as possible
    const base::SizeT  pitch     = static_cast<base::SizeT>(dstSize.x) * 4;
    const unsigned int srcStride = source.m_size.x * 4;
    const unsigned int dstStride = m_size.x * 4;

    const base::U8* srcPixels = source.m_pixels.data() + (srcRect.position.x + srcRect.position.y * source.m_size.x) * 4;
    base::U8* dstPixels = m_pixels.data() + (dest.x + dest.y * m_size.x) * 4;

    // Copy the pixels
    if (applyAlpha)
    {
        // Interpolation using alpha values, pixel by pixel (slower)
        for (unsigned int i = 0; i < dstSize.y; ++i)
        {
            for (unsigned int j = 0; j < dstSize.x; ++j)
            {
                // Get a direct pointer to the components of the current pixel
                const base::U8* src = srcPixels + j * 4;
                base::U8*       dst = dstPixels + j * 4;

                // Interpolate RGBA components using the alpha values of the destination and source pixels
                const base::U8 srcAlpha = src[3];
                const base::U8 dstAlpha = dst[3];
                const auto     outAlpha = static_cast<base::U8>(srcAlpha + dstAlpha - srcAlpha * dstAlpha / 255);

                dst[3] = outAlpha;

                if (outAlpha)
                    for (int k = 0; k < 3; k++)
                        dst[k] = static_cast<base::U8>((src[k] * srcAlpha + dst[k] * (outAlpha - srcAlpha)) / outAlpha);
                else
                    for (int k = 0; k < 3; k++)
                        dst[k] = src[k];
            }

            srcPixels += srcStride;
            dstPixels += dstStride;
        }
    }
    else
    {
        // Optimized copy ignoring alpha values, row by row (faster)
        for (unsigned int i = 0u; i < dstSize.y; ++i)
        {
            SFML_BASE_MEMCPY(dstPixels, srcPixels, pitch);
            srcPixels += srcStride;
            dstPixels += dstStride;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
void Image::setPixel(Vec2u coords, Color color)
{
    SFML_BASE_ASSERT(coords.x < m_size.x && "Image::setPixel() x coordinate is out of bounds");
    SFML_BASE_ASSERT(coords.y < m_size.y && "Image::setPixel() y coordinate is out of bounds");

    const auto index = (coords.x + coords.y * m_size.x) * 4;
    base::U8*  pixel = &m_pixels[index];

    *pixel++ = color.r;
    *pixel++ = color.g;
    *pixel++ = color.b;
    *pixel++ = color.a;
}


////////////////////////////////////////////////////////////
Color Image::getPixel(Vec2u coords) const
{
    SFML_BASE_ASSERT(coords.x < m_size.x && "Image::getPixel() x coordinate is out of bounds");
    SFML_BASE_ASSERT(coords.y < m_size.y && "Image::getPixel() y coordinate is out of bounds");

    const auto      index = (coords.x + coords.y * m_size.x) * 4;
    const base::U8* pixel = &m_pixels[index];

    return {pixel[0], pixel[1], pixel[2], pixel[3]};
}


////////////////////////////////////////////////////////////
const base::U8* Image::getPixelsPtr() const
{
    SFML_BASE_ASSERT(!m_pixels.empty());
    return m_pixels.data();
}


////////////////////////////////////////////////////////////
void Image::flipHorizontally()
{
    SFML_BASE_ASSERT(!m_pixels.empty());

    const base::SizeT rowSize = m_size.x * 4;

    for (base::SizeT y = 0; y < m_size.y; ++y)
    {
        auto* left  = m_pixels.begin() + static_cast<base::PtrDiffT>(y * rowSize);
        auto* right = m_pixels.begin() + static_cast<base::PtrDiffT>((y + 1) * rowSize - 4);

        for (base::SizeT x = 0; x < m_size.x / 2; ++x)
        {
            base::swapRanges(left, left + 4, right);

            left += 4;
            right -= 4;
        }
    }
}


////////////////////////////////////////////////////////////
void Image::flipVertically()
{
    SFML_BASE_ASSERT(!m_pixels.empty());

    const auto rowSize = static_cast<base::PtrDiffT>(m_size.x * 4);

    auto* top    = m_pixels.begin();
    auto* bottom = m_pixels.end() - rowSize;

    for (base::SizeT y = 0; y < m_size.y / 2; ++y)
    {
        base::swapRanges(top, top + rowSize, bottom);

        top += rowSize;
        bottom -= rowSize;
    }
}


////////////////////////////////////////////////////////////
void Image::rotateHue(const float degrees)
{
    applyTransformation([degrees](const unsigned int, const unsigned int, const Color color)
    { return color.withRotatedHue(degrees); });
}


////////////////////////////////////////////////////////////
bool Image::saveToFile(const Path& filename) const
{
    // Extract the extension
    const Path extension     = filename.getExtension();
    const auto convertedSize = m_size.toVec2i();

    // Context for the stbi write callback. The callback's signature is fixed
    // (void return), so failures from `OutFile::write` are accumulated into
    // `error` and checked once `stbi_*_to_func` returns.
    struct StbiOutCtx
    {
        OutFile& file;
        bool     error = false;
    };

    auto writeStdOfstream = [](void* context, void* data, int size)
    {
        auto& ctx = *static_cast<StbiOutCtx*>(context);
        if (ctx.error)
            return;

        if (!ctx.file.write(static_cast<const char*>(data), static_cast<base::SizeT>(size)))
            ctx.error = true;
    };

    const auto runStbiWriter = [&](auto writerFn)
    {
        auto optFile = OutFile::open(filename.c_str(), FileOpenMode::bin);
        if (!optFile.hasValue())
            return false;

        StbiOutCtx ctx{*optFile};
        return writerFn(&ctx) != 0 && !ctx.error;
        // Destructor closes the file when `optFile` goes out of scope.
    };

    // Deduce the image type from its extension
    if (extension == ".bmp")
    {
        if (runStbiWriter([&](void* ctx)
        { return stbi_write_bmp_to_func(writeStdOfstream, ctx, convertedSize.x, convertedSize.y, 4, m_pixels.data()); }))
            return true;
    }
    else if (extension == ".tga")
    {
        if (runStbiWriter([&](void* ctx)
        { return stbi_write_tga_to_func(writeStdOfstream, ctx, convertedSize.x, convertedSize.y, 4, m_pixels.data()); }))
            return true;
    }
    else if (extension == ".png")
    {
        if (runStbiWriter([&](void* ctx) {
            return stbi_write_png_to_func(writeStdOfstream, ctx, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 0);
        }))
            return true;
    }
    else if (extension == ".jpg" || extension == ".jpeg")
    {
        if (runStbiWriter([&](void* ctx) {
            return stbi_write_jpg_to_func(writeStdOfstream, ctx, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 90);
        }))
            return true;
    }
    else if (extension == ".qoi")
    {
        if (const auto [data, size] = saveQOIImpl(m_pixels.data(), m_size); data)
        {
            auto optFile = OutFile::open(filename.c_str(), FileOpenMode::bin);
            if (optFile.hasValue() && optFile->write(reinterpret_cast<const char*>(data.get()), size))
                return true;
        }
    }
    else
    {
        priv::errMsg("Image file extension {} not supported\n", extension);
    }

    priv::errMsg("Failed to save image\n{}", priv::PathDebugFormatter{filename});
    return false;
}


////////////////////////////////////////////////////////////
base::Vector<base::U8> Image::saveToMemory(SaveFormat format) const
{
    // Choose function based on format
    const auto convertedSize = m_size.toVec2i();

    base::Vector<base::U8> buffer; // Use a single local variable for NRVO

    if (format == SaveFormat::BMP)
    {
        if (stbi_write_bmp_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data()))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::TGA)
    {
        if (stbi_write_tga_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data()))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::PNG)
    {
        if (stbi_write_png_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 0))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::JPG)
    {
        if (stbi_write_jpg_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 90))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::QOI)
    {
        if (const auto [data, size] = saveQOIImpl(m_pixels.data(), m_size); data)
        {
            // Skip the zero-init pass that `resize` would do; we immediately overwrite via memcpy.
            buffer.reserve(size);
            buffer.unsafeSetSize(size);
            SFML_BASE_MEMCPY(buffer.data(), data.get(), size);
            return buffer;
        }
    }

    SFML_BASE_ASSERT(false);
    return buffer;
}

} // namespace sf
