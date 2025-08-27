#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Image.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#ifdef SFML_SYSTEM_ANDROID
    #include "SFML/System/Android/Activity.hpp"
    #include "SFML/System/Android/ResourceStream.hpp"
#endif

#define QOI_IMPLEMENTATION
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
// stb_image callbacks that operate on a sf::InputStream
int read(void* const user, char* const data, int size)
{
    auto&                    stream = *static_cast<sf::InputStream*>(user);
    const sf::base::Optional count  = stream.read(data, static_cast<sf::base::SizeT>(size));
    return count.hasValue() ? static_cast<int>(*count) : -1;
}


////////////////////////////////////////////////////////////
void skip(void* const user, int size)
{
    auto& stream = *static_cast<sf::InputStream*>(user);
    if (!stream.seek(stream.tell().value() + static_cast<sf::base::SizeT>(size)).hasValue())
        sf::priv::err() << "Failed to seek image loader input stream";
}


////////////////////////////////////////////////////////////
int eof(void* const user)
{
    auto& stream = *static_cast<sf::InputStream*>(user);
    return stream.tell().value() >= stream.getSize().value();
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
        priv::err() << "Failed to create image, invalid size (zero) provided";
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
        priv::err() << "Failed to create image, invalid size (zero) provided";
        return base::nullOpt;
    }

    if (pixels == nullptr)
    {
        priv::err() << "Failed to create image, null pixels pointer provided";
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
#ifdef SFML_SYSTEM_ANDROID

    if (priv::getActivityStatesPtr() != nullptr)
    {
        priv::ResourceStream stream;
        if (!stream.open(filename))
            return false;
        return loadFromStream(stream);
    }

#endif

    // Set up the stb_image callbacks for the input stream
    const auto readStdIfStream = [](void* user, char* data, int size)
    {
        auto& file = *static_cast<InFileStream*>(user);
        file.read(data, size);
        return static_cast<int>(file.gcount());
    };

    const auto skipStdIfStream = [](void* user, int size)
    {
        auto& file = *static_cast<InFileStream*>(user);
        if (!file.seekg(size, SeekDir::cur))
            priv::err() << "Failed to seek image loader InFileStream";
    };

    const auto eofStdIfStream = [](void* user)
    {
        auto& file = *static_cast<InFileStream*>(user);
        return static_cast<int>(file.isEOF());
    };

    const stbi_io_callbacks callbacks{readStdIfStream, skipStdIfStream, eofStdIfStream};

    // Open file
    InFileStream file(filename.c_str(), FileOpenMode::bin);
    if (!file.isOpen())
    {
        // Error, failed to open the file
        priv::err() << "Failed to load image\n"
                    << priv::PathDebugFormatter{filename} << "\nReason: Failed to open the file";
        return base::nullOpt;
    }

    // Read a (possible) QOI magic number
    char qoiMagicNumber[4]{};
    file.read(qoiMagicNumber, 4);

    // Seek back to the start of the file for reading
    file.seekg(0, SeekDir::beg);

    // Read the QOI file if it's valid
    if (isQoiMagicNumber(qoiMagicNumber, static_cast<base::SizeT>(file.gcount())))
    {
        // Get the size of the file
        file.seekg(0, SeekDir::end);
        const auto streamSize = file.tellg();
        file.seekg(0, SeekDir::beg);

        // Read in the file contents since QOI doesn't support streams
        base::Vector<base::U8> buffer(static_cast<base::SizeT>(streamSize));
        file.read(reinterpret_cast<char*>(buffer.data()), streamSize);
        return loadQOIImpl(base::PassKey<Image>{}, buffer.data(), static_cast<base::SizeT>(streamSize));
    }

    // Load the image and get a pointer to the pixels in memory
    Vec2i imageSize;
    int   channels = 0;

    if (const auto ptr = StbPtr(
            stbi_load_from_callbacks(&callbacks, &file, &imageSize.x, &imageSize.y, &channels, STBI_rgb_alpha)))
    {
        SFML_BASE_ASSERT(imageSize.x > 0 && "Loaded image from file with width == 0");
        SFML_BASE_ASSERT(imageSize.y > 0 && "Loaded image from file with height == 0");

        return base::makeOptional<Image>(base::PassKey<Image>{},
                                         Vec2i{imageSize.x, imageSize.y}.toVec2u(),
                                         ptr.get(),
                                         ptr.get() + imageSize.x * imageSize.y * 4);
    }

    // Error, failed to load the image
    priv::err() << "Failed to load image\n"
                << priv::PathDebugFormatter{filename} << "\nReason: " << stbi_failure_reason();

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromMemory(const void* data, base::SizeT size)
{
    // Check input parameters
    if (data == nullptr || size == 0)
    {
        priv::err() << "Failed to load image from memory, no data provided";
        return base::nullOpt;
    }

    // Load the image and get a pointer to the pixels in memory
    int width    = 0;
    int height   = 0;
    int channels = 0;

    const auto*  buffer = static_cast<const unsigned char*>(data);
    const StbPtr ptr(stbi_load_from_memory(buffer, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha));

    if (ptr == nullptr)
    {
        priv::err() << "Failed to load image from memory. Reason: " << stbi_failure_reason();
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
    // Make sure that the stream's reading position is at the beginning
    if (!stream.seek(0).hasValue())
    {
        priv::err() << "Failed to seek image stream";
        return base::nullOpt;
    }

    // Read a (possible) QOI magic number
    char       qoiMagicNumber[4]{};
    const auto qoiMagicCount = stream.read(qoiMagicNumber, 4);

    if (const auto seekToStartResult = stream.seek(0); !seekToStartResult.hasValue())
    {
        priv::err() << "Failed to seek back the image stream";
        return base::nullOpt;
    }

    // Read the QOI file if it's valid
    if (qoiMagicCount.hasValue() && isQoiMagicNumber(qoiMagicNumber, *qoiMagicCount))
    {
        if (const auto streamSize = stream.getSize(); streamSize.hasValue())
        {
            // Read in the file contents since QOI doesn't support streams
            base::Vector<base::U8> buffer(*streamSize);
            const auto             readDataSize = stream.read(buffer.data(), *streamSize);
            return loadQOIImpl(base::PassKey<Image>{}, buffer.data(), static_cast<base::SizeT>(*readDataSize));
        }
    }

    // Setup the stb_image callbacks
    stbi_io_callbacks callbacks;
    callbacks.read = read;
    callbacks.skip = skip;
    callbacks.eof  = eof;

    // Load the image and get a pointer to the pixels in memory
    int width    = 0;
    int height   = 0;
    int channels = 0;

    const StbPtr ptr(stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &channels, STBI_rgb_alpha));

    if (ptr == nullptr)
    {
        priv::err() << "Failed to load image from stream. Reason: " << stbi_failure_reason();
        return base::nullOpt;
    }

    SFML_BASE_ASSERT(width > 0 && "Loaded image from stream with width == 0");
    SFML_BASE_ASSERT(height > 0 && "Loaded image from stream with height == 0");

    return base::makeOptional<Image>(base::PassKey<Image>{},
                                     Vec2i{width, height}.toVec2u(),
                                     ptr.get(),
                                     ptr.get() + width * height * 4);
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
bool Image::copy(const Image& source, Vec2u dest, const IntRect& sourceRect, bool applyAlpha)
{
    // Make sure that both images are valid
    SFML_BASE_ASSERT(source.m_size.x > 0 && source.m_size.y > 0 && m_size.x > 0 && m_size.y > 0);

    // Make sure the sourceRect components are non-negative before casting them to unsigned values
    if (sourceRect.position.x < 0 || sourceRect.position.y < 0 || sourceRect.size.x < 0 || sourceRect.size.y < 0)
        return false;

    auto srcRect = sourceRect.to<Rect<unsigned int>>();

    // Use the whole source image as srcRect if the provided source rectangle is empty
    if (srcRect.size.x == 0 || srcRect.size.y == 0)
    {
        srcRect = Rect<unsigned int>({0, 0}, source.m_size);
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
    const Path extension     = filename.extension();
    const auto convertedSize = m_size.toVec2i();

    // Callback to write to output stream
    auto writeStdOfstream = [](void* context, void* data, int size)
    {
        auto& file = *static_cast<OutFileStream*>(context);
        if (file)
            file.write(static_cast<const char*>(data), static_cast<base::PtrDiffT>(size));
    };

    // Deduce the image type from its extension
    if (extension == ".bmp")
    {
        // BMP format
        OutFileStream file(filename.c_str(), FileOpenMode::bin);
        if (stbi_write_bmp_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, m_pixels.data()) && file)
            return true;
    }
    else if (extension == ".tga")
    {
        // TGA format
        OutFileStream file(filename.c_str(), FileOpenMode::bin);
        if (stbi_write_tga_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, m_pixels.data()) && file)
            return true;
    }
    else if (extension == ".png")
    {
        // PNG format
        OutFileStream file(filename.c_str(), FileOpenMode::bin);
        if (stbi_write_png_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 0) && file)
            return true;
    }
    else if (extension == ".jpg" || extension == ".jpeg")
    {
        // JPG format
        OutFileStream file(filename.c_str(), FileOpenMode::bin);
        if (stbi_write_jpg_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 90) && file)
            return true;
    }
    else if (extension == ".qoi")
    {
        if (const auto [data, size] = saveQOIImpl(m_pixels.data(), m_size); data)
        {
            OutFileStream file(filename.c_str(), FileOpenMode::bin);
            file.write(reinterpret_cast<const char*>(data.get()), static_cast<base::PtrDiffT>(size));
            return true;
        }
    }
    else
    {
        priv::err() << "Image file extension " << extension << " not supported\n";
    }

    priv::err() << "Failed to save image\n" << priv::PathDebugFormatter{filename};
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
            buffer.resize(size);
            SFML_BASE_MEMCPY(buffer.data(), data.get(), size);
            return buffer;
        }
    }

    SFML_BASE_ASSERT(false);
    return buffer;
}

} // namespace sf
