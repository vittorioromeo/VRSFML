#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/ImageUtils.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <fstream>


namespace
{
// stb_image callback for constructing a buffer
void bufferFromCallback(void* context, void* data, int size)
{
    const auto* source = static_cast<sf::base::U8*>(data);
    auto*       dest   = static_cast<std::vector<sf::base::U8>*>(context);

    dest->insert(dest->end(), source, source + size);
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
bool ImageUtils::saveToFile(const Image& image, const Path& filename)
{
    // Extract the extension
    const Path extension     = filename.extension();
    const auto convertedSize = image.getSize().toVector2i();

    // Callback to write to std::ofstream
    auto writeStdOfstream = [](void* context, void* data, int size)
    {
        auto& file = *static_cast<std::ofstream*>(context);
        if (file)
            file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    };

    // Deduce the image type from its extension
    if (extension == ".bmp")
    {
        // BMP format
        std::ofstream file(filename.c_str(), std::ios::binary);
        if (stbi_write_bmp_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr()) &&
            file)
            return true;
    }
    else if (extension == ".tga")
    {
        // TGA format
        std::ofstream file(filename.c_str(), std::ios::binary);
        if (stbi_write_tga_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr()) &&
            file)
            return true;
    }
    else if (extension == ".png")
    {
        // PNG format
        std::ofstream file(filename.c_str(), std::ios::binary);
        if (stbi_write_png_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr(), 0) &&
            file)
            return true;
    }
    else if (extension == ".jpg" || extension == ".jpeg")
    {
        // JPG format
        std::ofstream file(filename.c_str(), std::ios::binary);
        if (stbi_write_jpg_to_func(writeStdOfstream, &file, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr(), 90) &&
            file)
            return true;
    }
    else
    {
        priv::err() << "Image file extension " << extension << " not supported\n";
    }

    priv::err() << "Failed to save image\n" << priv::PathDebugFormatter{filename};
    return false;
}


////////////////////////////////////////////////////////////
std::vector<base::U8> ImageUtils::saveToMemory(const Image& image, SaveFormat format)
{
    // Choose function based on format
    const auto convertedSize = image.getSize().toVector2i();

    std::vector<base::U8> buffer; // Use a single local variable for NRVO

    if (format == SaveFormat::BMP)
    {
        if (stbi_write_bmp_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr()))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::TGA)
    {
        if (stbi_write_tga_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr()))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::PNG)
    {
        if (stbi_write_png_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr(), 0))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::JPG)
    {
        if (stbi_write_jpg_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, image.getPixelsPtr(), 90))
            return buffer; // Non-empty
    }

    SFML_BASE_ASSERT(false);
    return buffer;
}

} // namespace sf
