#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/ContextSettings.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLRenderBufferObject.hpp"
#include "SFML/GLUtils/GLUniqueResource.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"


namespace
{
////////////////////////////////////////////////////////////
void deleteFramebuffer(unsigned int id)
{
    glCheck(glDeleteFramebuffers(1, &id));
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr GLenum getGLInternalFormat(const bool stencil, const bool depth)
{
    if (stencil && depth)
        return GL_DEPTH24_STENCIL8;

    if (stencil)
        return GL_STENCIL_INDEX8;

    if (depth)
        return GL_DEPTH_COMPONENT16;

    SFML_BASE_ASSERT(false);
    return {};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr const char* getBufferTypeStr(const bool multisample,
                                                                                const bool stencil,
                                                                                const bool depth)
{
    if (stencil && depth)
        return multisample ? "multisample depth/stencil buffer" : "depth/stencil buffer";

    if (stencil)
        return multisample ? "multisample stencil buffer" : "stencil buffer";

    if (depth)
        return multisample ? "multisample depth buffer" : "depth buffer";

    SFML_BASE_ASSERT(false);
    return {};
}


////////////////////////////////////////////////////////////
void linkStencilDepthBuffer(const sf::base::Optional<sf::GLRenderBufferObject>& stencilDepthBuffer,
                            const bool                                          stencil,
                            const bool                                          depth)
{
    if (!stencilDepthBuffer.hasValue())
        return;

    if (stencil)
        glCheck(
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilDepthBuffer->getId()));

    if (depth)
        glCheck(
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, stencilDepthBuffer->getId()));
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline bool isBoundFramebufferComplete()
{
    return glCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct RenderTexture::Impl
{
    using FramebufferIdMap = ankerl::unordered_dense::map<unsigned int, unsigned int>;

    Texture texture;    //!< Target texture to draw on
    Texture tmpTexture; //!< Temporary texture used for Y-axis flipping

    FramebufferIdMap framebuffers;    //!< Per-context OpenGL FBOs
    FramebufferIdMap auxFramebuffers; //!< Per-context auxiliary OpenGL FBOs (either multisample or temp for Y-flipping)

    base::Optional<GLRenderBufferObject> stencilDepthBuffer; //!< Optional depth/stencil buffer attached to the framebuffer
    base::Optional<GLRenderBufferObject> colorBuffer; //!< Optional multisample color buffer attached to the framebuffer

    bool multisample{}; //!< Must create a multisample framebuffer as well
    bool stencil{};     //!< Has stencil attachment
    bool depth{};       //!< Has depth attachment
    bool sRgb{};        //!< Must encode drawn pixels into sRGB color space

    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Impl(Texture&& theTexture) :
    texture(SFML_BASE_MOVE(theTexture)),
    tmpTexture(Texture::create(texture.getSize(), {.sRgb = texture.isSrgb(), .smooth = texture.isSmooth()}).value())
    {
    }

    Impl(const Impl&)            = delete;
    Impl& operator=(const Impl&) = delete;

    Impl(Impl&&) noexcept            = default;
    Impl& operator=(Impl&&) noexcept = default;

    ////////////////////////////////////////////////////////////
    void cleanup()
    {
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());

        stencilDepthBuffer.reset();
        colorBuffer.reset();

        // Unregister FBOs with the contexts if they haven't already been destroyed
        {
            for (const auto& [glContextId, framebufferId] : framebuffers)
                GraphicsContext::unregisterUnsharedFrameBuffer(glContextId, framebufferId);

            framebuffers.clear();
        }

        {
            for (const auto& [glContextId, auxFramebufferId] : auxFramebuffers)
                GraphicsContext::unregisterUnsharedFrameBuffer(glContextId, auxFramebufferId);

            auxFramebuffers.clear();
        }
    }


private:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createFail(const char* what)
    {
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0u));
        priv::err() << "Impossible to create render texture (" << what << ")";
        return false;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool completeAuxFramebufferCreation(GLuint auxFramebufferId)
    {
        linkStencilDepthBuffer(stencilDepthBuffer, stencil, depth);

        if (!isBoundFramebufferComplete())
            return createFail("failed to link the render buffers to the auxiliary framebuffer");

        // Register the FBO in our map and with the current context so it is automatically destroyed
        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();
        auxFramebuffers.try_emplace(glContextId, auxFramebufferId);
        GraphicsContext::registerUnsharedFrameBuffer(glContextId, auxFramebufferId, &deleteFramebuffer);

        return true;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createAuxMultisampleFramebuffer()
    {
        SFML_BASE_ASSERT(multisample);

        // Create the multisample framebuffer object
        const GLuint multisampleFramebufferId = priv::generateAndBindFramebuffer();
        if (!multisampleFramebufferId)
            return createFail("failed to create the multisample framebuffer object");

        // Link the multisample color buffer to the framebuffer
        colorBuffer->bind();
        glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer->getId()));

        return completeAuxFramebufferCreation(multisampleFramebufferId);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createAuxTempFramebuffer()
    {
        SFML_BASE_ASSERT(!multisample);

        // Create the framebuffer object
        const GLuint tempFramebufferId = priv::generateAndBindFramebuffer();
        if (!tempFramebufferId)
            return createFail("failed to create the temp framebuffer object");

        // Link the texture to the framebuffer
        glCheck(
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmpTexture.getNativeHandle(), 0));

        return completeAuxFramebufferCreation(tempFramebufferId);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createFramebuffer()
    {
        // Create the framebuffer object
        const GLuint framebufferId = priv::generateAndBindFramebuffer();
        if (!framebufferId)
            return createFail("failed to create the framebuffer object");

        // Link the texture to the framebuffer
        glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getNativeHandle(), 0));

        // Link the depth/stencil renderbuffer to the framebuffer
        if (!multisample)
            linkStencilDepthBuffer(stencilDepthBuffer, stencil, depth);

        if (!isBoundFramebufferComplete())
            return createFail("failed to link the target texture to the framebuffer");

        // Register the FBO in our map and with the current context so it is automatically destroyed
        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();
        framebuffers.try_emplace(glContextId, framebufferId);
        GraphicsContext::registerUnsharedFrameBuffer(glContextId, framebufferId, &deleteFramebuffer);

        return multisample ? createAuxMultisampleFramebuffer() : createAuxTempFramebuffer();
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool create(const ContextSettings& contextSettings)
    {
        // OpenGL ES requires that all attachments have identical sizes

        const auto fail = [&](const auto&... what)
        {
            priv::err(/* multiline */ true) << "Impossible to create render texture (";
            (priv::err(/* multiline */ true) << ... << what);
            priv::err(/* multiline */ true) << ")\n";

            return false;
        };

        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());

        const auto size = texture.getSize();

        sRgb = contextSettings.sRgbCapable;

        // Check if the requested anti-aliasing level is supported
        if (const auto samples = getMaximumAntiAliasingLevel(); contextSettings.antiAliasingLevel > samples)
            return fail("unsupported anti-aliasing level ", contextSettings.antiAliasingLevel, ", maximum supported is ", samples);

        const auto bindRenderbufferAndSetFormat =
            [&size](GLRenderBufferObject& rbo, const unsigned int antiAliasingLevel, const GLenum internalFormat)
        {
            rbo.bind();

            glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                     static_cast<GLsizei>(antiAliasingLevel),
                                                     internalFormat,
                                                     static_cast<GLsizei>(size.x),
                                                     static_cast<GLsizei>(size.y)));
        };

        depth       = contextSettings.depthBits != 0u;
        stencil     = contextSettings.stencilBits != 0u;
        multisample = contextSettings.antiAliasingLevel != 0u;

        // Create the (possibly multisample) depth/stencil buffer if requested
        if (stencil || depth)
        {
            stencilDepthBuffer = tryCreateGLUniqueResource<GLRenderBufferObject>();
            if (!stencilDepthBuffer.hasValue())
                return fail("failed to create the attached ", getBufferTypeStr(multisample, stencil, depth));

            bindRenderbufferAndSetFormat(*stencilDepthBuffer,
                                         contextSettings.antiAliasingLevel,
                                         getGLInternalFormat(stencil, depth));
        }

        // Create the multisample color buffer if needed
        if (multisample)
        {
            colorBuffer = tryCreateGLUniqueResource<GLRenderBufferObject>();
            if (!colorBuffer.hasValue())
                return fail("failed to create the attached multisample color buffer");

            bindRenderbufferAndSetFormat(*colorBuffer, contextSettings.antiAliasingLevel, sRgb ? GL_SRGB8_ALPHA8 : GL_RGBA8);
        }

        // We can't create an FBO now if there is no active context
        if (!GraphicsContext::hasActiveThreadLocalGlContext())
            return true;

        // Save the current bindings so we can restore them after we are done
        const auto readFramebuffer = priv::getGLInteger(GL_READ_FRAMEBUFFER_BINDING);
        const auto drawFramebuffer = priv::getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING);

        if (!createFramebuffer())
            return false;

        // Restore previously bound framebuffers
        glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFramebuffer)));
        glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFramebuffer)));

        return true;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool activate(const bool active)
    {
        // Unbind the FBO if requested
        if (!active)
        {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0u));
            return true;
        }

        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();

        // Lookup the FBO corresponding to the currently active context
        // If none is found, there is no FBO corresponding to the
        // currently active context so we will have to create a new FBO

        if (const auto* it = auxFramebuffers.find(glContextId); it != auxFramebuffers.end())
        {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, it->second));
            return true;
        }

        return createFramebuffer();
    }

    ////////////////////////////////////////////////////////////
    void updateTexture()
    {
        // If multisampling is enabled, we need to resolve by blitting from our FBO with multisample
        // renderbuffer attachments to our FBO to which our target texture is attached

        // In case of multisampling, make sure both FBOs are already available within the current context

        const auto size = texture.getSize();

        if (size.x == 0u || size.y == 0u || !activate(true))
            return;

        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();

        const auto* framebufferIt = framebuffers.find(glContextId);
        if (framebufferIt == framebuffers.end())
            return;

        const auto* auxFramebufferIt = auxFramebuffers.find(glContextId);
        if (auxFramebufferIt == auxFramebuffers.end())
            return;

        // Since we don't want scissor testing to interfere with blits, so we temporarily disable it if needed
        const priv::ScissorDisableGuard scissorDisableGuard;

        // Blit from the auxiliary (multisample or temp) FBO to the main FBO, flipping Y axis
        if (!priv::copyFlippedFramebuffer(tmpTexture.getNativeHandle(), size, auxFramebufferIt->second, framebufferIt->second))
            priv::err() << "Error flipping render texture during FBO copy";
    }
};


////////////////////////////////////////////////////////////
RenderTexture::~RenderTexture()
{
    m_impl->cleanup();
}


////////////////////////////////////////////////////////////
RenderTexture::RenderTexture(RenderTexture&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTexture& RenderTexture::operator=(RenderTexture&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    m_impl->cleanup();
    m_impl = SFML_BASE_MOVE(rhs.m_impl);

    return *this;
}


////////////////////////////////////////////////////////////
base::Optional<RenderTexture> RenderTexture::create(const Vector2u size)
{
    return create(size, {});
}


////////////////////////////////////////////////////////////
base::Optional<RenderTexture> RenderTexture::create(const Vector2u size, const ContextSettings& contextSettings)
{
    base::Optional<RenderTexture> result; // Use a single local variable for NRVO

    // Create the texture
    auto texture = sf::Texture::create(size, {.sRgb = contextSettings.sRgbCapable});
    if (!texture.hasValue())
    {
        priv::err() << "Impossible to create render texture (failed to create the target texture)";
        return result; // Empty optional
    }

    // Use frame-buffer object (FBO)
    result.emplace(base::PassKey<RenderTexture>{}, SFML_BASE_MOVE(*texture));

    // Mark the texture as being a framebuffer object attachment
    result->m_impl->texture.m_fboAttachment = true;

    // We disable smoothing by default for render textures
    result->setSmooth(false);

    // Initialize the render texture
    if (!result->m_impl->create(contextSettings))
    {
        priv::err() << "Impossible to create render texture (failed to create render texture renderTextureImpl)";

        result.reset();
        return result; // Empty optional
    }

    return result;
}


////////////////////////////////////////////////////////////
unsigned int RenderTexture::getMaximumAntiAliasingLevel()
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    return static_cast<unsigned int>(priv::getGLInteger(GL_MAX_SAMPLES));
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
void RenderTexture::setWrapMode(TextureWrapMode wrapMode)
{
    m_impl->texture.setWrapMode(wrapMode);
}


////////////////////////////////////////////////////////////
TextureWrapMode RenderTexture::getWrapMode() const
{
    return m_impl->texture.getWrapMode();
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
    if (m_impl->activate(active))
        return RenderTarget::setActive(active);

    return false;
}


////////////////////////////////////////////////////////////
RenderTarget::DrawStatistics RenderTexture::display()
{
    // Perform a RenderTarget-only activation if we are using FBOs
    if (!RenderTarget::setActive())
        return {};

    const auto result = RenderTarget::flush();
    RenderTarget::syncGPUEndFrame();

    // Update the target texture
    m_impl->updateTexture();
    m_impl->texture.invalidateMipmap();

    return result;
}


////////////////////////////////////////////////////////////
Vector2u RenderTexture::getSize() const
{
    return m_impl->texture.getSize();
}


////////////////////////////////////////////////////////////
bool RenderTexture::isSrgb() const
{
    return m_impl->sRgb;
}


////////////////////////////////////////////////////////////
const Texture& RenderTexture::getTexture() const
{
    return m_impl->texture;
}


////////////////////////////////////////////////////////////
RenderTexture::RenderTexture(base::PassKey<RenderTexture>&&, Texture&& texture) :
RenderTarget(View::fromRect({{0.f, 0.f}, texture.getSize().toVector2f()})),
m_impl(SFML_BASE_MOVE(texture))
{
}

} // namespace sf
