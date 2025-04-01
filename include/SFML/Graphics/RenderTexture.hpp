#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/TextureWrapMode.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Texture;
struct ContextSettings;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Target for off-screen 2D rendering into a texture
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API RenderTexture : public RenderTarget
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~RenderTexture() override;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTexture(const RenderTexture&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderTexture& operator=(const RenderTexture&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTexture(RenderTexture&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment operator
    ///
    ////////////////////////////////////////////////////////////
    RenderTexture& operator=(RenderTexture&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Create the render-texture
    ///
    /// The last parameter, `settings`, is useful if you want to enable
    /// multi-sampling or use the render-texture for OpenGL rendering that
    /// requires a depth or stencil buffer. Otherwise it is unnecessary, and
    /// you should leave this parameter at its default value.
    ///
    /// After creation, the contents of the render-texture are undefined.
    /// Call `RenderTexture::clear` first to ensure a single color fill.
    ///
    /// \param size     Width and height of the render-texture
    /// \param settings Additional settings for the underlying OpenGL texture and context
    ///
    /// \return Render texture on success, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<RenderTexture> create(Vector2u size);
    [[nodiscard]] static base::Optional<RenderTexture> create(Vector2u size, const ContextSettings& contextSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Get the maximum anti-aliasing level supported by the system
    ///
    /// \return The maximum anti-aliasing level supported by the system
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getMaximumAntiAliasingLevel();

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable texture smoothing
    ///
    /// This function is similar to `Texture::setSmooth`.
    /// This parameter is disabled by default.
    ///
    /// \param smooth `true` to enable smoothing, `false` to disable it
    ///
    /// \see `isSmooth`
    ///
    ////////////////////////////////////////////////////////////
    void setSmooth(bool smooth);

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether the smooth filtering is enabled or not
    ///
    /// \return `true` if texture smoothing is enabled
    ///
    /// \see `setSmooth`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSmooth() const;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable texture repeating
    ///
    /// This function is similar to `Texture::setWrapMode`.
    /// This parameter is disabled by default.
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
    /// This function is similar to `Texture::generateMipmap` and operates
    /// on the texture used as the target for drawing.
    /// Be aware that any draw operation may modify the base level image data.
    /// For this reason, calling this function only makes sense after all
    /// drawing is completed and display has been called. Not calling display
    /// after subsequent drawing will lead to undefined behavior if a mipmap
    /// had been previously generated.
    ///
    /// \return `true` if mipmap generation was successful, `false` if unsuccessful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool generateMipmap();

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the render-texture for rendering
    ///
    /// This function makes the render-texture's context current for
    /// future OpenGL rendering operations (so you shouldn't care
    /// about it if you're not doing direct OpenGL stuff).
    /// Only one context can be current in a thread, so if you
    /// want to draw OpenGL geometry to another render target
    /// (like a RenderWindow) don't forget to activate it again.
    ///
    /// \param active `true` to activate, `false` to deactivate
    ///
    /// \return `true` if operation was successful, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setActive(bool active = true) override;

    ////////////////////////////////////////////////////////////
    /// \brief Update the contents of the target texture
    ///
    /// This function updates the target texture with what
    /// has been drawn so far. Like for windows, calling this
    /// function is mandatory at the end of rendering. Not calling
    /// it may leave the texture in an undefined state.
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget::DrawStatistics display();

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the rendering region of the texture
    ///
    /// The returned value is the size that you passed to
    /// the create function.
    ///
    /// \return Size in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector2u getSize() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the render-texture will use sRGB encoding when drawing on it
    ///
    /// You can request sRGB encoding for a render-texture
    /// by having the sRgbCapable flag set for the context parameter of `create()` method
    ///
    /// \return `true` if the render-texture use sRGB encoding, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSrgb() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Get a read-only reference to the target texture
    ///
    /// After drawing to the render-texture and calling Display,
    /// you can retrieve the updated texture using this function,
    /// and draw it using a sprite (for example).
    /// The internal `sf::Texture` of a render-texture is always the
    /// same instance, so that it is possible to call this function
    /// once and keep a reference to the texture even after it is
    /// modified.
    ///
    /// \return Const reference to the texture
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Texture& getTexture() const;

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct from texture
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderTexture(base::PassKey<RenderTexture>&&, Texture&& texture);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 384> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderTexture
/// \ingroup graphics
///
/// `sf::RenderTexture` is the little brother of `sf::RenderWindow`.
/// It implements the same 2D drawing and OpenGL-related functions
/// (see their base class `sf::RenderTarget` for more details),
/// the difference is that the result is stored in an off-screen
/// texture rather than being show in a window.
///
/// Rendering to a texture can be useful in a variety of situations:
/// \li precomputing a complex static texture (like a level's background from multiple tiles)
/// \li applying post-effects to the whole scene with shaders
/// \li creating a sprite from a 3D object rendered with OpenGL
/// \li etc.
///
/// Usage example:
///
/// \code
/// // Create a new render-window
/// sf::RenderWindow window({.size{800u, 600u}, .title = "SFML Window"});
///
/// // Create a new render-texture
/// auto renderTexture = sf::RenderTexture::create({500, 500}).value();
///
/// // The main loop
/// while (true)
/// {
///    // Event processing
///    // ...
///
///    // Clear the whole texture with red color
///    renderTexture.clear(sf::Color::Red);
///
///    // Draw stuff to the texture
///    renderTexture.draw(shape);   // shape is a sf::Shape
///    renderTexture.draw(text);    // text is a sf::Text
///
///    // We're done drawing to the texture
///    renderTexture.display();
///
///    // Now we start rendering to the window, clear it first
///    window.clear();
///
///    // Draw the texture
///    window.draw(renderTexture.getTexture());
///
///    // End the current frame and display its contents on screen
///    window.display();
/// }
/// \endcode
///
/// Like `sf::RenderWindow`, `sf::RenderTexture` is still able to render direct
/// OpenGL stuff. It is even possible to mix together OpenGL calls
/// and regular SFML drawing commands. If you need a depth buffer for
/// 3D rendering, don't forget to request it when calling `RenderTexture::create`.
///
/// \see `sf::RenderTarget`, `sf::RenderWindow`, `sf::View`, `sf::Texture`
///
////////////////////////////////////////////////////////////
