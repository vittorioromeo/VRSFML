#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Vector2.hpp>


namespace sf
{

struct ContextSettings;

namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for render-texture implementations
///
////////////////////////////////////////////////////////////
class RenderTextureImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] RenderTextureImpl() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~RenderTextureImpl() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTextureImpl(const RenderTextureImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderTextureImpl& operator=(const RenderTextureImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Create the render texture implementation
    ///
    /// \param size       Width and height of the texture to render to
    /// \param textureId  OpenGL identifier of the target texture
    /// \param settings   Context settings to create render-texture with
    ///
    /// \return True if creation has been successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool create(Vector2u size, unsigned int textureId, const ContextSettings& settings) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the render texture for rendering
    ///
    /// \param active True to activate, false to deactivate
    ///
    /// \return True on success, false on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool activate(bool active) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the render-texture will use sRGB encoding when drawing on it
    ///
    /// You can request sRGB encoding for a render-texture
    /// by having the sRgbCapable flag set for the context parameter of create() method
    ///
    /// \return True if the render-texture use sRGB encoding, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool isSrgb() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Update the pixels of the target texture
    ///
    /// \param textureId OpenGL identifier of the target texture
    ///
    ////////////////////////////////////////////////////////////
    virtual void updateTexture(unsigned int textureId) = 0;
};

} // namespace priv

} // namespace sf
