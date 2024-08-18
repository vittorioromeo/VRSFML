#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderTextureImplDefault.hpp"
#include "SFML/Graphics/RenderTextureImplFBO.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Variant.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct RenderTexture::Impl
{
    sfvr::tinyvariant<priv::RenderTextureImplDefault, priv::RenderTextureImplFBO> renderTextureImpl; //!< Platform/hardware specific implementation
    Texture texture; //!< Target texture to draw on

    template <typename TRenderTextureImplTag>
    explicit Impl(TRenderTextureImplTag theRenderTextureImplTag, GraphicsContext& graphicsContext, Texture&& theTexture) :
    renderTextureImpl(theRenderTextureImplTag, graphicsContext),
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

    if (priv::RenderTextureImplFBO::isAvailable(graphicsContext))
    {
        // Use frame-buffer object (FBO)
        result.emplace(base::PassKey<RenderTexture>{},
                       graphicsContext,
                       sfvr::inplace_type<priv::RenderTextureImplFBO>,
                       SFML_BASE_MOVE(*texture));

        // Mark the texture as being a framebuffer object attachment
        result->m_impl->texture.m_fboAttachment = true;
    }
    else
    {
        // Use default implementation
        result.emplace(base::PassKey<RenderTexture>{},
                       graphicsContext,
                       sfvr::inplace_type<priv::RenderTextureImplDefault>,
                       SFML_BASE_MOVE(*texture));
    }

    // We disable smoothing by default for render textures
    result->setSmooth(false);

    // Initialize the render texture
    // We pass the actual size of our texture since OpenGL ES requires that all attachments have identical sizes
    if (!result->m_impl->renderTextureImpl.linear_visit(
            [&](auto&& impl) {
                return impl.create(result->m_impl->texture.m_actualSize, result->m_impl->texture.m_texture, contextSettings);
            }))
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
unsigned int RenderTexture::getMaximumAntialiasingLevel(GraphicsContext& graphicsContext)
{
    return priv::RenderTextureImplFBO::isAvailable(graphicsContext)
               ? priv::RenderTextureImplFBO::getMaximumAntialiasingLevel(graphicsContext)
               : priv::RenderTextureImplDefault::getMaximumAntialiasingLevel();
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
    if (m_impl->renderTextureImpl.linear_visit([&](auto&& impl) { return impl.activate(active); }))
        return RenderTarget::setActive(active);

    return false;
}


////////////////////////////////////////////////////////////
void RenderTexture::display()
{
    if (priv::RenderTextureImplFBO::isAvailable(getGraphicsContext()))
    {
        // Perform a RenderTarget-only activation if we are using FBOs
        if (!RenderTarget::setActive())
            return;
    }
    else
    {
        // Perform a full activation if we are not using FBOs
        if (!setActive())
            return;
    }

    // Update the target texture
    m_impl->renderTextureImpl.linear_visit([&](auto&& impl) { impl.updateTexture(m_impl->texture.m_texture); });
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
    return m_impl->renderTextureImpl.linear_visit([&](auto&& impl) { return impl.isSrgb(); });
}


////////////////////////////////////////////////////////////
const Texture& RenderTexture::getTexture() const
{
    return m_impl->texture;
}


////////////////////////////////////////////////////////////
template <typename TRenderTextureImplTag>
RenderTexture::RenderTexture(base::PassKey<RenderTexture>&&,
                             GraphicsContext&      graphicsContext,
                             TRenderTextureImplTag renderTextureImplTag,
                             Texture&&             texture) :
RenderTarget(graphicsContext),
m_impl(renderTextureImplTag, graphicsContext, SFML_BASE_MOVE(texture))
{
}


} // namespace sf
