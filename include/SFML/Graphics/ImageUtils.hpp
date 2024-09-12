#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include <vector>

#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Path;
class Image;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Class for loading, manipulating and saving images
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API ImageUtils
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Supported image saving formats
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] SaveFormat
    {
        BMP,
        TGA,
        PNG,
        JPG
    };

    ////////////////////////////////////////////////////////////
    /// \brief Save an image to a file on disk
    ///
    /// The format of the image is automatically deduced from
    /// the extension. The supported image formats are bmp, png,
    /// tga and jpg. The destination file is overwritten
    /// if it already exists. This function fails if the image is empty.
    ///
    /// \param image Image to save
    /// \param filename Path of the file to save
    ///
    /// \return True if saving was successful
    ///
    /// \see create, loadFromFile, loadFromMemory
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool saveToFile(const Image& image, const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Save an image to a buffer in memory
    ///
    /// The format of the image must be specified.
    /// The supported image formats are bmp, png, tga and jpg.
    /// This function fails if the image is empty, or if
    /// the format was invalid.
    ///
    /// \param image Image to save
    /// \param format Encoding format to use
    ///
    /// \return Buffer with encoded data if saving was successful, `base::nullOpt` otherwise
    ///
    /// \see create, loadFromFile, loadFromMemory, saveToFile
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::vector<std::uint8_t> saveToMemory(const Image& image, SaveFormat format);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::ImageUtils
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see sf::Image
///
////////////////////////////////////////////////////////////
