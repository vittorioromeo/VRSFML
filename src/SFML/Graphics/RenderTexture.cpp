#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderTextureImplFBO.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Macros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct RenderTexture::Impl
{
    priv::RenderTextureImplFBO renderTextureImpl; //!< Platform/hardware specific implementation
    Texture                    texture;           //!< Target texture to draw on

    explicit Impl(GraphicsContext& graphicsContext, Texture&& theTexture) :
    renderTextureImpl(graphicsContext),
    texture(SFML_BASE_MOVE(theTexture))
    {
    }
};


////////////////////////////////////////////////////////////
RenderTexture::~RenderTexture() = default;


////////////////////////////////////////////////////////////
RenderTexture::RenderTexture(RenderTexture&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTexture& RenderTexture::operator=(RenderTexture&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<RenderTexture> RenderTexture::create(GraphicsContext&       graphicsContext,
                                                    Vector2u               size,
                                                    const ContextSettings& contextSettings)
{
    base::Optional<RenderTexture> result; // Use a single local variable for NRVO

    // Create the texture
    auto texture = sf::Texture::create(graphicsContext, size, contextSettings.sRgbCapable);
    if (!texture.hasValue())
    {
        priv::err() << "Impossible to create render texture (failed to create the target texture)";
        return result; // Empty optional
    }

    // Use frame-buffer object (FBO)
    result.emplace(base::PassKey<RenderTexture>{}, graphicsContext, SFML_BASE_MOVE(*texture));

    // Mark the texture as being a framebuffer object attachment
    result->m_impl->texture.m_fboAttachment = true;

    // We disable smoothing by default for render textures
    result->setSmooth(false);

    // Initialize the render texture
    // We pass the actual size of our texture since OpenGL ES requires that all attachments have identical sizes
    if (!result->m_impl->renderTextureImpl
             .create(result->m_impl->texture.m_actualSize, result->m_impl->texture.m_texture, contextSettings))
    {
        priv::err() << "Impossible to create render texture (failed to create render texture renderTextureImpl)";

        result.reset();
        return result; // Empty optional
    }

    // We can now initialize the render target part
    result->initialize();

    return result;
}


////////////////////////////////////////////////////////////
unsigned int RenderTexture::getMaximumAntiAliasingLevel(GraphicsContext& graphicsContext)
{
    return priv::RenderTextureImplFBO::getMaximumAntiAliasingLevel(graphicsContext);
}


////////////////////////////////////////////////////////////
void RenderTexture::setSmooth(bool smooth)
{
    m_impl->texture.setSmooth(smooth);
}


////////////////////////////////////////////////////////////
bool RenderTexture::isSmooth() const
{
    return m_impl->texture.isSmooth();
}


////////////////////////////////////////////////////////////
void RenderTexture::setRepeated(bool repeated)
{
    m_impl->texture.setRepeated(repeated);
}


////////////////////////////////////////////////////////////
bool RenderTexture::isRepeated() const
{
    return m_impl->texture.isRepeated();
}


////////////////////////////////////////////////////////////
bool RenderTexture::generateMipmap()
{
    return m_impl->texture.generateMipmap();
}


////////////////////////////////////////////////////////////
bool RenderTexture::setActive(bool active)
{
    // Update RenderTarget tracking
    if (m_impl->renderTextureImpl.activate(active))
        return RenderTarget::setActive(active);

    return false;
}


////////////////////////////////////////////////////////////
void RenderTexture::display()
{
    // Perform a RenderTarget-only activation if we are using FBOs
    if (!RenderTarget::setActive())
        return;

    // Update the target texture
    m_impl->renderTextureImpl.updateTexture(m_impl->texture.m_texture);
    m_impl->texture.m_pixelsFlipped = true;
    m_impl->texture.invalidateMipmap();
}


////////////////////////////////////////////////////////////
Vector2u RenderTexture::getSize() const
{
    return m_impl->texture.getSize();
}


////////////////////////////////////////////////////////////
bool RenderTexture::isSrgb() const
{
    return m_impl->renderTextureImpl.isSrgb();
}


////////////////////////////////////////////////////////////
const Texture& RenderTexture::getTexture() const
{
    return m_impl->texture;
}


////////////////////////////////////////////////////////////
RenderTexture::RenderTexture(base::PassKey<RenderTexture>&&, GraphicsContext& graphicsContext, Texture&& texture) :
RenderTarget(graphicsContext),
m_impl(graphicsContext, SFML_BASE_MOVE(texture))
{
}


} // namespace sf
