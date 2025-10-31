#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/TextureWrapMode.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Image;
class InputStream;
class Path;
class TextureAtlas;
class Window;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] TextureCreateSettings
{
    bool            sRgb     = false;
    bool            smooth   = false;
    TextureWrapMode wrapMode = TextureWrapMode::Clamp;
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] TextureLoadSettings
{
    bool            sRgb     = false;
    Rect2i          area     = {};
    bool            smooth   = false;
    TextureWrapMode wrapMode = TextureWrapMode::Clamp;
};

////////////////////////////////////////////////////////////
/// \brief Image living on the graphics card that can be used for drawing
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Texture
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Texture();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Texture(const Texture& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment operator
    ///
    ////////////////////////////////////////////////////////////
    Texture& operator=(const Texture&);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Texture(Texture&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment operator
    ///
    ////////////////////////////////////////////////////////////
    Texture& operator=(Texture&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Create the texture
    ///
    /// If this function fails, the texture is left unchanged.
    ///
    /// \param size Width and height of the texture
    /// \param sRgb `true` to enable sRGB conversion, `false` to disable it
    ///
    /// \return Texture on success, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Texture> create(Vec2u size, const TextureCreateSettings& settings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Load the texture from a file on disk
    ///
    /// The `area` argument can be used to load only a sub-rectangle
    /// of the whole image. If you want the entire image then leave
    /// the default value (which is an empty `Rect2i`).
    /// If the `area` rectangle crosses the bounds of the image, it
    /// is adjusted to fit the image size.
    ///
    /// The maximum size for a texture depends on the graphics
    /// driver and can be retrieved with the `getMaximumSize` function.
    ///
    /// If this function fails, the texture is left unchanged.
    ///
    /// \param filename Path of the image file to load
    /// \param sRgb     `true` to enable sRGB conversion, `false` to disable it
    /// \param area     Area of the image to load
    ///
    /// \return Texture on success, `base::nullOpt` otherwise
    ///
    /// \see `loadFromMemory`, `loadFromStream`, `loadFromImage`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Texture> loadFromFile(const Path& filename, const TextureLoadSettings& settings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Load the texture from a file in memory
    ///
    /// The `area` argument can be used to load only a sub-rectangle
    /// of the whole image. If you want the entire image then leave
    /// the default value (which is an empty `Rect2i`).
    /// If the `area` rectangle crosses the bounds of the image, it
    /// is adjusted to fit the image size.
    ///
    /// The maximum size for a texture depends on the graphics
    /// driver and can be retrieved with the `getMaximumSize` function.
    ///
    /// If this function fails, the texture is left unchanged.
    ///
    /// \param data Pointer to the file data in memory
    /// \param size Size of the data to load, in bytes
    /// \param sRgb `true` to enable sRGB conversion, `false` to disable it
    /// \param area Area of the image to load
    ///
    /// \return Texture on success, `base::nullOpt` otherwise
    ///
    /// \see `loadFromFile`, `loadFromStream`, `loadFromImage`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Texture> loadFromMemory(const void*                data,
                                                                base::SizeT                size,
                                                                const TextureLoadSettings& settings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Load the texture from a custom stream
    ///
    /// The `area` argument can be used to load only a sub-rectangle
    /// of the whole image. If you want the entire image then leave
    /// the default value (which is an empty `Rect2i`).
    /// If the `area` rectangle crosses the bounds of the image, it
    /// is adjusted to fit the image size.
    ///
    /// The maximum size for a texture depends on the graphics
    /// driver and can be retrieved with the `getMaximumSize` function.
    ///
    /// If this function fails, the texture is left unchanged.
    ///
    /// \param stream Source stream to read from
    /// \param sRgb   `true` to enable sRGB conversion, `false` to disable it
    /// \param area   Area of the image to load
    ///
    /// \return Texture on success, `base::nullOpt` otherwise
    ///
    /// \see `loadFromFile`, `loadFromMemory`, `loadFromImage`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Texture> loadFromStream(InputStream&               stream,
                                                                const TextureLoadSettings& settings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Load the texture from an image
    ///
    /// The `area` argument can be used to load only a sub-rectangle
    /// of the whole image. If you want the entire image then leave
    /// the default value (which is an empty `Rect2i`).
    /// If the `area` rectangle crosses the bounds of the image, it
    /// is adjusted to fit the image size.
    ///
    /// The maximum size for a texture depends on the graphics
    /// driver and can be retrieved with the `getMaximumSize` function.
    ///
    /// If this function fails, the texture is left unchanged.
    ///
    /// \param image Image to load into the texture
    /// \param sRgb   True to enable sRGB conversion, false to disable it
    /// \param area  Area of the image to load
    ///
    /// \return Texture on success, `base::nullOpt` otherwise
    ///
    /// \see `loadFromFile`, `loadFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Texture> loadFromImage(const Image& image, const TextureLoadSettings& settings = {});

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the texture
    ///
    /// \return Size in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2u getSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Copy the texture pixels to an image
    ///
    /// This function performs a slow operation that downloads
    /// the texture's pixels from the graphics card and copies
    /// them to a new image, potentially applying transformations
    /// to pixels if necessary (texture may be padded).
    ///
    /// \return Image containing the texture's pixels
    ///
    /// \see `loadFromImage`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Image copyToImage() const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the whole texture from an array of pixels
    ///
    /// The pixel array is assumed to have the same size as
    /// the `area` rectangle, and to contain 32-bits RGBA pixels.
    ///
    /// No additional check is performed on the size of the pixel
    /// array. Passing invalid arguments will lead to an undefined
    /// behavior.
    ///
    /// This function does nothing if `pixels` is `nullptr`
    /// or if the texture was not previously created.
    ///
    /// \param pixels Array of pixels to copy to the texture
    ///
    ////////////////////////////////////////////////////////////
    void update(const base::U8* pixels);

    ////////////////////////////////////////////////////////////
    /// \brief Update a part of the texture from an array of pixels
    ///
    /// The size of the pixel array must match the `size` argument,
    /// and it must contain 32-bits RGBA pixels.
    ///
    /// No additional check is performed on the size of the pixel
    /// array or the bounds of the area to update. Passing invalid
    /// arguments will lead to an undefined behavior.
    ///
    /// This function does nothing if `pixels` is null or if the
    /// texture was not previously created.
    ///
    /// \param pixels Array of pixels to copy to the texture
    /// \param size   Width and height of the pixel region contained in `pixels`
    /// \param dest   Coordinates of the destination position
    ///
    ////////////////////////////////////////////////////////////
    void update(const base::U8* pixels, Vec2u size, Vec2u dest);

    ////////////////////////////////////////////////////////////
    /// \brief Update a part of this texture from another texture
    ///
    /// No additional check is performed on the size of the texture.
    /// Passing an invalid combination of texture size and destination
    /// will lead to an undefined behavior.
    ///
    /// This function does nothing if either texture was not
    /// previously created.
    ///
    /// \param texture Source texture to copy to this texture
    /// \param dest    Coordinates of the destination position
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool update(const Texture& texture, Vec2u dest = {0u, 0u});

    ////////////////////////////////////////////////////////////
    /// \brief Update a part of the texture from an image
    ///
    /// No additional check is performed on the size of the image.
    /// Passing an invalid combination of image size and destination
    /// will lead to an undefined behavior.
    ///
    /// This function does nothing if the texture was not
    /// previously created.
    ///
    /// \param image Image to copy to the texture
    /// \param dest  Coordinates of the destination position
    ///
    ////////////////////////////////////////////////////////////
    void update(const Image& image, Vec2u dest = {0u, 0u});

    ////////////////////////////////////////////////////////////
    /// \brief Update a part of the texture from the contents of a window
    ///
    /// No additional check is performed on the size of the window.
    /// Passing an invalid combination of window size and destination
    /// will lead to an undefined behavior.
    ///
    /// Must be called before `window.display()`.
    ///
    /// This function does nothing if either the texture or the window
    /// was not previously created.
    ///
    /// \param window Window to copy to the texture
    /// \param dest   Coordinates of the destination position
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool update(const Window& window, Vec2u dest = {0u, 0u});

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable the smooth filter
    ///
    /// When the filter is activated, the texture appears smoother
    /// so that pixels are less noticeable. However if you want
    /// the texture to look exactly the same as its source file,
    /// you should leave it disabled.
    /// The smooth filter is disabled by default.
    ///
    /// \param smooth `true` to enable smoothing, `false` to disable it
    ///
    /// \see `isSmooth`
    ///
    ////////////////////////////////////////////////////////////
    void setSmooth(bool smooth);

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether the smooth filter is enabled or not
    ///
    /// \return `true` if smoothing is enabled, `false` if it is disabled
    ///
    /// \see `setSmooth`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSmooth() const;

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether the texture source is converted from sRGB or not
    ///
    /// \return `true` if the texture source is converted from sRGB, `false` if not
    ///
    /// \see `setSrgb`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSrgb() const;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable repeating
    ///
    /// Repeating is involved when using texture coordinates
    /// outside the texture rectangle [0, 0, width, height].
    /// In this case, if repeat mode is enabled, the whole texture
    /// will be repeated as many times as needed to reach the
    /// coordinate (for example, if the X texture coordinate is
    /// 3 * width, the texture will be repeated 3 times).
    /// If repeat mode is disabled, the "extra space" will instead
    /// be filled with border pixels.
    ///
    /// Repeating is disabled by default.
    ///
    /// \param wrapMode TODO P1: docs
    ///
    /// \see `getWrapMode`
    ///
    ////////////////////////////////////////////////////////////
    void setWrapMode(TextureWrapMode wrapMode);

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether the texture is repeated or not
    ///
    /// \return TODO P1: docs
    ///
    /// \see `setWrapMode`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TextureWrapMode getWrapMode() const;

    ////////////////////////////////////////////////////////////
    /// \brief Generate a mipmap using the current texture data
    ///
    /// Mipmaps are pre-computed chains of optimized textures. Each
    /// level of texture in a mipmap is generated by halving each of
    /// the previous level's dimensions. This is done until the final
    /// level has the size of 1x1. The textures generated in this process may
    /// make use of more advanced filters which might improve the visual quality
    /// of textures when they are applied to objects much smaller than they are.
    /// This is known as minification. Because fewer texels (texture elements)
    /// have to be sampled from when heavily minified, usage of mipmaps
    /// can also improve rendering performance in certain scenarios.
    ///
    /// Mipmap generation relies on the necessary OpenGL extension being
    /// available. If it is unavailable or generation fails due to another
    /// reason, this function will return `false`. Mipmap data is only valid from
    /// the time it is generated until the next time the base level image is
    /// modified, at which point this function will have to be called again to
    /// regenerate it.
    ///
    /// \return `true` if mipmap generation was successful, `false` if unsuccessful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool generateMipmap();

    ////////////////////////////////////////////////////////////
    /// \brief Swap the contents of this texture with those of another
    ///
    /// \param right Instance to swap with
    ///
    ////////////////////////////////////////////////////////////
    void swap(Texture& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the underlying OpenGL handle of the texture.
    ///
    /// You shouldn't need to use this function, unless you have
    /// very specific stuff to implement that SFML doesn't support,
    /// or implement a temporary workaround until a bug is fixed.
    ///
    /// \return OpenGL handle of the texture or 0 if not yet created
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getNativeHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get a rectangle covering the entire texture
    ///
    /// This function is useful to conveniently initialize `sf::Sprite`
    /// objects that are intended to be used with this texture.
    ///
    /// \return Rectangle covering the entire texture, from {0, 0} to {width, height}
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Rect2f getRect() const;

    ////////////////////////////////////////////////////////////
    /// \brief Bind a texture for rendering
    ///
    /// This function is not part of the graphics API, it mustn't be
    /// used when drawing SFML entities. It must be used only if you
    /// mix `sf::Texture` with OpenGL code.
    ///
    /// \code
    /// sf::Texture t1, t2;
    /// ...
    /// t1.bind();
    /// // draw OpenGL stuff that use t1...
    /// t2.bind();
    /// // draw OpenGL stuff that use t2...
    /// sf::Texture::unbind();
    /// // draw OpenGL stuff that use no texture...
    /// \endcode
    ///
    ////////////////////////////////////////////////////////////
    void bind() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    static void unbind();

    ////////////////////////////////////////////////////////////
    /// \brief Get the maximum texture size allowed
    ///
    /// This maximum size is defined by the graphics driver.
    /// You can expect a value of 512 pixels for low-end graphics
    /// card, and up to 8192 pixels or more for newer hardware.
    ///
    /// \return Maximum size allowed for textures, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getMaximumSize();

private:
    friend class Text;
    friend class RenderTexture;
    friend class RenderTarget;
    friend struct StatesCache;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    /// Creates an empty texture.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Texture(base::PassKey<Texture>&&, Vec2u size, unsigned int texture, bool sRgb);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Invalidate the mipmap if one exists
    ///
    /// This also resets the texture's minifying function.
    /// This function is mainly for internal use by RenderTexture.
    ///
    ////////////////////////////////////////////////////////////
    void invalidateMipmap();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vec2u           m_size;            //!< Public texture size
    unsigned int    m_texture{};       //!< Internal texture identifier
    bool            m_isSmooth{};      //!< Status of the smooth filter
    bool            m_sRgb{};          //!< Should the texture source be converted from sRGB?
    TextureWrapMode m_wrapMode{};      //!< Is the texture in repeat mode?
    bool            m_fboAttachment{}; //!< Is this texture owned by a framebuffer object?
    bool            m_hasMipmap{};     //!< Has the mipmap been generated?
    unsigned int    m_cacheId;         //!< Unique number that identifies the texture to the render target's cache
};

////////////////////////////////////////////////////////////
/// \brief Swap the contents of one texture with those of another
///
/// \param lhs First instance to swap
/// \param rhs Second instance to swap
///
////////////////////////////////////////////////////////////
SFML_GRAPHICS_API void swap(Texture& lhs, Texture& rhs) noexcept;

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Texture
/// \ingroup graphics
///
/// `sf::Texture` stores pixels that can be drawn, with a sprite
/// for example. A texture lives in the graphics card memory,
/// therefore it is very fast to draw a texture to a render target,
/// or copy a render target to a texture (the graphics card can
/// access both directly).
///
/// Being stored in the graphics card memory has some drawbacks.
/// A texture cannot be manipulated as freely as a `sf::Image`,
/// you need to prepare the pixels first and then upload them
/// to the texture in a single operation (see `Texture::update`).
///
/// `sf::Texture` makes it easy to convert from/to `sf::Image`, but
/// keep in mind that these calls require transfers between
/// the graphics card and the central memory, therefore they are
/// slow operations.
///
/// A texture can be loaded from an image, but also directly
/// from a file/memory/stream. The necessary shortcuts are defined
/// so that you don't need an image first for the most common cases.
/// However, if you want to perform some modifications on the pixels
/// before creating the final texture, you can load your file to a
/// `sf::Image`, do whatever you need with the pixels, and then call
/// `Texture::loadFromImage`.
///
/// Since they live in the graphics card memory, the pixels of a texture
/// cannot be accessed without a slow copy first. And they cannot be
/// accessed individually. Therefore, if you need to read the texture's
/// pixels (like for pixel-perfect collisions), it is recommended to
/// store the collision information separately, for example in an array
/// of booleans.
///
/// Like `sf::Image`, `sf::Texture` can handle a unique internal
/// representation of pixels, which is RGBA 32 bits. This means
/// that a pixel must be composed of 8 bit red, green, blue and
/// alpha channels -- just like a `sf::Color`.
///
/// When providing texture data from an image file or memory, it can
/// either be stored in a linear color space or an sRGB color space.
/// Most digital images account for gamma correction already, so they
/// would need to be "uncorrected" back to linear color space before
/// being processed by the hardware. The hardware can automatically
/// convert it from the sRGB color space to a linear color space when
/// it gets sampled. When the rendered image gets output to the final
/// framebuffer, it gets converted back to sRGB.
///
/// This option is only useful in conjunction with an sRGB capable
/// framebuffer. This can be requested during window creation.
///
/// Usage example:
/// \code
/// // This example shows the most common use of `sf::Texture`:
/// // drawing a sprite
///
/// // Load a texture from a file
/// const auto texture = sf::Texture::loadFromFile("texture.png").value();
///
/// // Draw the texture
/// window.draw(texture);
/// \endcode
///
/// \code
/// // This example shows another common use of sf::Texture:
/// // streaming real-time data, like video frames
///
/// // Create an empty texture
/// auto texture = sf::Texture::create({640, 480}).value();
///
/// while (...) // the main loop
/// {
///     ...
///
///     // update the texture
///     base::U8* pixels = ...; // get a fresh chunk of pixels (the next frame of a movie, for example)
///     texture.update(pixels);
///
///     // draw the texture
///     window.draw(texture);
///
///     ...
/// }
///
/// \endcode
///
/// Like `sf::Shader` that can be used as a raw OpenGL shader,
/// `sf::Texture` can also be used directly as a raw texture for
/// custom OpenGL geometry.
/// \code
/// texture.bind();
/// ... render OpenGL geometry ...
/// sf::Texture::unbind();
/// \endcode
///
/// \see `sf::Sprite`, `sf::Image`, `sf::RenderTexture`
///
////////////////////////////////////////////////////////////
