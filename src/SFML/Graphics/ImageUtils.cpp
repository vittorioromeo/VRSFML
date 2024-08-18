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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <string>

#include <cstring>


namespace
{
// stb_image callback for constructing a buffer
void bufferFromCallback(void* context, void* data, int size)
{
    const auto* source = static_cast<std::uint8_t*>(data);
    auto*       dest   = static_cast<std::vector<std::uint8_t>*>(context);

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
    const auto convertedSize = image.getSize().to<Vector2i>();

    // Deduce the image type from its extension
    if (extension == ".bmp")
    {
        // BMP format
        if (stbi_write_bmp(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, image.getPixelsPtr()))
            return true;
    }
    else if (extension == ".tga")
    {
        // TGA format
        if (stbi_write_tga(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, image.getPixelsPtr()))
            return true;
    }
    else if (extension == ".png")
    {
        // PNG format
        if (stbi_write_png(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, image.getPixelsPtr(), 0))
            return true;
    }
    else if (extension == ".jpg" || extension == ".jpeg")
    {
        // JPG format
        if (stbi_write_jpg(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, image.getPixelsPtr(), 90))
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
std::vector<std::uint8_t> ImageUtils::saveToMemory(const Image& image, SaveFormat format)
{
    // Choose function based on format
    const auto convertedSize = image.getSize().to<Vector2i>();

    std::vector<std::uint8_t> buffer; // Use a single local variable for NRVO

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
