#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class InputStream;
class Path;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Class for loading, manipulating and saving images
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Image
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Supported image saving formats
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] SaveFormat : unsigned char
    {
        BMP,
        TGA,
        PNG,
        JPG,
        QOI
    };

    ////////////////////////////////////////////////////////////
    /// \brief Construct the image and fill it with a unique color
    ///
    /// If \a size is zero, the behavior is undefined.
    ///
    /// \param size  Width and height of the image
    /// \param color Fill color
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Image> create(Vec2u size, Color color = Color::Black);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the image from an array of pixels
    ///
    /// The pixel array is assumed to contain 32-bits RGBA pixels,
    /// and have the given `size`. If not, this is an undefined behavior.
    /// If `pixels` is `nullptr`, an empty image is created.
    ///
    /// \param size   Width and height of the image
    /// \param pixels Array of pixels to copy to the image
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Image> create(Vec2u size, const base::U8* pixels);

    ////////////////////////////////////////////////////////////
    /// \brief Load the image from a file on disk
    ///
    /// The supported image formats are bmp, png, tga, jpg, gif,
    /// psd, hdr, pic and pnm. Some format options are not supported,
    /// like jpeg with arithmetic coding or ASCII pnm.
    /// If this function fails, the image is left unchanged.
    ///
    /// \param filename Path of the image file to load
    ///
    /// \return Image if loading was successful, `base::nullOpt` otherwise
    ///
    /// \see `loadFromMemory`, `loadFromStream`, `saveToFile`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Image> loadFromFile(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Load the image from a file in memory
    ///
    /// The supported image formats are bmp, png, tga, jpg, gif,
    /// psd, hdr, pic and pnm. Some format options are not supported,
    /// like jpeg with arithmetic coding or ASCII pnm.
    /// If this function fails, the image is left unchanged.
    ///
    /// \param data Pointer to the file data in memory
    /// \param size Size of the data to load, in bytes
    ///
    /// \return Image if loading was successful, `base::nullOpt` otherwise
    ///
    /// \see loadFromFile, loadFromStream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Image> loadFromMemory(const void* data, base::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief Load the image from a custom stream
    ///
    /// The supported image formats are bmp, png, tga, jpg, gif,
    /// psd, hdr, pic and pnm. Some format options are not supported,
    /// like jpeg with arithmetic coding or ASCII pnm.
    /// If this function fails, the image is left unchanged.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Image if loading was successful, `base::nullOpt` otherwise
    ///
    /// \see `loadFromFile`, `loadFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Image> loadFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Return the size (width and height) of the image
    ///
    /// \return Size of the image, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2u getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Create a transparency mask from a specified color-key
    ///
    /// This function sets the alpha value of every pixel matching
    /// the given color to `alpha` (0 by default), so that they
    /// become transparent.
    ///
    /// \param color Color to make transparent
    /// \param alpha Alpha value to assign to transparent pixels
    ///
    ////////////////////////////////////////////////////////////
    void createMaskFromColor(Color color, base::U8 alpha = 0);

    ////////////////////////////////////////////////////////////
    /// \brief Copy pixels from another image onto this one
    ///
    /// This function does a slow pixel copy and should not be
    /// used intensively. It can be used to prepare a complex
    /// static image from several others, but if you need this
    /// kind of feature in real-time you'd better use `sf::RenderTexture`.
    ///
    /// If `sourceRect` is empty, the whole image is copied.
    /// If `applyAlpha` is set to `true`, alpha blending is
    /// applied from the source pixels to the destination pixels
    /// using the \b over operator. If it is `false`, the source
    /// pixels are copied unchanged with their alpha value.
    ///
    /// See https://en.wikipedia.org/wiki/Alpha_compositing for
    /// details on the \b over operator.
    ///
    /// Note that this function can fail if either image is invalid
    /// (i.e. zero-sized width or height), or if `sourceRect` is
    /// not within the boundaries of the `source` parameter, or
    /// if the destination area is out of the boundaries of this image.
    ///
    /// On failure, the destination image is left unchanged.
    ///
    /// \param source     Source image to copy
    /// \param dest       Coordinates of the destination position
    /// \param sourceRect Sub-rectangle of the source image to copy
    /// \param applyAlpha Should the copy take into account the source transparency?
    ///
    /// \return `true` if the operation was successful, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool copy(const Image& source, Vec2u dest, const Rect2i& sourceRect = {}, bool applyAlpha = false);

    ////////////////////////////////////////////////////////////
    /// \brief Change the color of a pixel
    ///
    /// This function doesn't check the validity of the pixel
    /// coordinates, using out-of-range values will result in
    /// an undefined behavior.
    ///
    /// \param coords Coordinates of pixel to change
    /// \param color  New color of the pixel
    ///
    /// \see `getPixel`
    ///
    ////////////////////////////////////////////////////////////
    void setPixel(Vec2u coords, Color color);

    ////////////////////////////////////////////////////////////
    /// \brief Get the color of a pixel
    ///
    /// This function doesn't check the validity of the pixel
    /// coordinates, using out-of-range values will result in
    /// an undefined behavior.
    ///
    /// \param coords Coordinates of pixel to change
    ///
    /// \return Color of the pixel at given coordinates
    ///
    /// \see `setPixel`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Color getPixel(Vec2u coords) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a read-only pointer to the array of pixels
    ///
    /// The returned value points to an array of RGBA pixels made of
    /// 8 bit integer components. The size of the array is
    /// `width * height * 4 (getSize().x * getSize().y * 4)`.
    /// Warning: the returned pointer may become invalid if you
    /// modify the image, so you should never store it for too long.
    /// If the image is empty, a null pointer is returned.
    ///
    /// \return Read-only pointer to the array of pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const base::U8* getPixelsPtr() const;

    ////////////////////////////////////////////////////////////
    /// \brief Flip the image horizontally (left <-> right)
    ///
    ////////////////////////////////////////////////////////////
    void flipHorizontally();

    ////////////////////////////////////////////////////////////
    /// \brief Flip the image vertically (top <-> bottom)
    ///
    ////////////////////////////////////////////////////////////
    void flipVertically();

    ////////////////////////////////////////////////////////////
    /// \brief Apply a custom transformation to each pixel of the image
    ///
    /// This function iterates over all the pixels of the image and
    /// applies the provided callable `f` to each one.
    ///
    /// The callable `f` must take three arguments: the x-coordinate
    /// of the pixel (`unsigned int`), the y-coordinate of the pixel
    /// (`unsigned int`), and the current `sf::Color` of the pixel.
    ///
    /// It must return an `sf::Color` which will be the new color of that pixel.
    ///
    /// \param f A callable (e.g., lambda function) that takes
    ///          `(unsigned int x, unsigned int y, sf::Color color)`
    ///          and returns `sf::Color`.
    ///
    ////////////////////////////////////////////////////////////
    template <typename Func>
    void applyTransformation(Func&& f)
    {
        SFML_BASE_ASSERT(!m_pixels.empty());

        base::U8* ptr = m_pixels.data();

        for (unsigned int y = 0u; y < m_size.y; ++y)
            for (unsigned int x = 0u; x < m_size.x; ++x)
            {
                const Color currentColor{ptr[0], ptr[1], ptr[2], ptr[3]};
                const auto [newR, newG, newB, newA] = f(x, y, currentColor);

                *ptr++ = newR;
                *ptr++ = newG;
                *ptr++ = newB;
                *ptr++ = newA;
            }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Rotate the hue of the image
    ///
    /// This function applies a hue rotation to each pixel of the image.
    ///
    /// The rotation is done in degrees, and the value must be
    /// between 0 and 360. A value of 0 means no rotation, while
    /// a value of 360 means a full rotation (which is equivalent to 0).
    /// The rotation is done in the HSL color space.
    ///
    /// The function uses the `Color::withRotatedHue` method to
    /// perform the rotation.
    ///
    /// \param degrees The angle in degrees to rotate the hue.
    ///
    ////////////////////////////////////////////////////////////
    void rotateHue(float degrees);

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
    /// \return `true` if saving was successful
    ///
    /// \see create, loadFromFile, loadFromMemory
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool saveToFile(const Path& filename) const;

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
    [[nodiscard]] base::Vector<base::U8> saveToMemory(SaveFormat format) const;

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Directly initialize data members
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Image(base::PassKey<Image>&&, Vec2u size, base::SizeT pixelCount);
    [[nodiscard]] explicit Image(base::PassKey<Image>&&, Vec2u size, const base::U8* itBegin, const base::U8* itEnd);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vec2u                  m_size;   //!< Image size
    base::Vector<base::U8> m_pixels; //!< Pixels of the image
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Image
/// \ingroup graphics
///
/// `sf::Image` is an abstraction to manipulate images
/// as bi-dimensional arrays of pixels. The class provides
/// functions to load, read, write and save pixels, as well
/// as many other useful functions.
///
/// `sf::Image` can handle a unique internal representation of
/// pixels, which is RGBA 32 bits. This means that a pixel
/// must be composed of 8 bit red, green, blue and alpha
/// channels -- just like a `sf::Color`.
/// All the functions that return an array of pixels follow
/// this rule, and all parameters that you pass to `sf::Image`
/// functions (such as `loadFromMemory`) must use this
/// representation as well.
///
/// A `sf::Image` can be copied, but it is a heavy resource and
/// if possible you should always use [const] references to
/// pass or return them to avoid useless copies.
///
/// Usage example:
/// \code
/// // Load an image file from a file
/// const auto background = sf::Image::loadFromFile("background.jpg").value();
///
/// // Create a 20x20 image filled with black color
/// sf::Image image({20, 20}, sf::Color::Black);
///
/// // Copy background on image at position (10, 10)
/// if (!image.copy(background, {10, 10}))
///     return -1;
///
/// // Make the top-left pixel transparent
/// sf::Color color = image.getPixel({0, 0});
/// color.a = 0;
/// image.setPixel({0, 0}, color);
///
/// // Save the image to a file
/// if (!image.saveToFile("result.png"))
///     return -1;
/// \endcode
///
/// \see `sf::Texture`
///
////////////////////////////////////////////////////////////
