#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderTextureImplFBO.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"

#include <unordered_map>


namespace
{
////////////////////////////////////////////////////////////
void deleteFrameBuffer(unsigned int id)
{
    glCheck(glDeleteFramebuffers(1, &id));
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct RenderTextureImplFBO::Impl
{
    explicit Impl(GraphicsContext& theGraphicsContext) : graphicsContext(&theGraphicsContext)
    {
    }

    using FrameBufferIdMap = std::unordered_map<unsigned int, unsigned int>;

    GraphicsContext* graphicsContext; //!< The window context

    FrameBufferIdMap frameBuffers;            //!< OpenGL frame buffer objects per context
    FrameBufferIdMap multisampleFrameBuffers; //!< Optional per-context OpenGL frame buffer objects with multisample attachments

    unsigned int depthStencilBuffer{}; //!< Optional depth/stencil buffer attached to the frame buffer
    unsigned int colorBuffer{};        //!< Optional multisample color buffer attached to the frame buffer
    Vector2u     size;                 //!< Width and height of the attachments
    unsigned int textureId{};          //!< The ID of the texture to attach to the FBO
    bool         multisample{};        //!< Whether we have to create a multisample frame buffer as well
    bool         depth{};              //!< Whether we have depth attachment
    bool         stencil{};            //!< Whether we have stencil attachment
    bool         sRgb{};               //!< Whether we need to encode drawn pixels into sRGB color space
};


////////////////////////////////////////////////////////////
RenderTextureImplFBO::RenderTextureImplFBO(GraphicsContext& graphicsContext) : m_impl(graphicsContext)
{
}


////////////////////////////////////////////////////////////
RenderTextureImplFBO::~RenderTextureImplFBO()
{
    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Destroy the color buffer
    if (m_impl->colorBuffer)
    {
        const GLuint colorBuffer = m_impl->colorBuffer;
        glCheck(glDeleteRenderbuffers(1, &colorBuffer));
    }

    // Destroy the depth/stencil buffer
    if (m_impl->depthStencilBuffer)
    {
        const GLuint depthStencilBuffer = m_impl->depthStencilBuffer;
        glCheck(glDeleteRenderbuffers(1, &depthStencilBuffer));
    }

    // Unregister FBOs with the contexts if they haven't already been destroyed
    for (const auto& [glContextId, frameBufferId] : m_impl->frameBuffers)
        m_impl->graphicsContext->unregisterUnsharedFrameBuffer(glContextId, frameBufferId);

    for (const auto& [glContextId, multisampleFrameBufferId] : m_impl->multisampleFrameBuffers)
        m_impl->graphicsContext->unregisterUnsharedFrameBuffer(glContextId, multisampleFrameBufferId);
}


////////////////////////////////////////////////////////////
RenderTextureImplFBO::RenderTextureImplFBO(RenderTextureImplFBO&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTextureImplFBO& RenderTextureImplFBO::operator=(RenderTextureImplFBO&&) noexcept = default;


////////////////////////////////////////////////////////////
unsigned int RenderTextureImplFBO::getMaximumAntiAliasingLevel([[maybe_unused]] GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
    return static_cast<unsigned int>(getGLInteger(GL_MAX_SAMPLES));
}


////////////////////////////////////////////////////////////
void RenderTextureImplFBO::unbind()
{
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::create(Vector2u size, unsigned int textureId, const ContextSettings& contextSettings)
{
    // Store the dimensions
    m_impl->size = size;

    {
        SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

        m_impl->sRgb = contextSettings.sRgbCapable && GL_EXT_texture_sRGB;

        // Check if the requested anti-aliasing level is supported
        if (contextSettings.antiAliasingLevel)
        {
            const auto samples = getGLInteger(GL_MAX_SAMPLES);

            if (contextSettings.antiAliasingLevel > static_cast<unsigned int>(samples))
            {
                err() << "Impossible to create render texture (unsupported anti-aliasing level)"
                      << " Requested: " << contextSettings.antiAliasingLevel << " Maximum supported: " << samples;

                return false;
            }
        }

        if (!contextSettings.antiAliasingLevel)
        {
            // Create the depth/stencil buffer if requested
            if (contextSettings.stencilBits && contextSettings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached depth/stencil "
                             "buffer)";
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(glRenderbufferStorage(GL_RENDERBUFFER,
                                              GL_DEPTH24_STENCIL8,
                                              static_cast<GLsizei>(size.x),
                                              static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = true;
            }
            else if (contextSettings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached depth buffer)";
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(glRenderbufferStorage(GL_RENDERBUFFER,
                                              GL_DEPTH_COMPONENT16,
                                              static_cast<GLsizei>(size.x),
                                              static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = false;
            }
            else if (contextSettings.stencilBits)
            {
                GLuint depthStencil = 0;
                glCheck(glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached stencil buffer)";
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(glRenderbufferStorage(GL_RENDERBUFFER,
                                              GL_STENCIL_INDEX8,
                                              static_cast<GLsizei>(size.x),
                                              static_cast<GLsizei>(size.y)));

                m_impl->depth   = false;
                m_impl->stencil = true;
            }
        }
        else
        {
            // Create the multisample color buffer
            GLuint color = 0;
            glCheck(glGenRenderbuffers(1, &color));

            m_impl->colorBuffer = color;

            if (!m_impl->colorBuffer)
            {
                err() << "Impossible to create render texture (failed to create the attached multisample color buffer)";
                return false;
            }
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->colorBuffer));
            glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                     static_cast<GLsizei>(contextSettings.antiAliasingLevel),
                                                     m_impl->sRgb ? GL_SRGB8_ALPHA8_EXT : GL_RGBA,
                                                     static_cast<GLsizei>(size.x),
                                                     static_cast<GLsizei>(size.y)));

            // Create the multisample depth/stencil buffer if requested
            if (contextSettings.stencilBits && contextSettings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached multisample "
                             "depth/stencil buffer)";
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                         static_cast<GLsizei>(contextSettings.antiAliasingLevel),
                                                         GL_DEPTH24_STENCIL8,
                                                         static_cast<GLsizei>(size.x),
                                                         static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = true;
            }
            else if (contextSettings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached multisample "
                             "depth "
                             "buffer)";
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                         static_cast<GLsizei>(contextSettings.antiAliasingLevel),
                                                         GL_DEPTH_COMPONENT,
                                                         static_cast<GLsizei>(size.x),
                                                         static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = false;
            }
            else if (contextSettings.stencilBits)
            {
                GLuint depthStencil = 0;
                glCheck(glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached multisample "
                             "stencil buffer)";
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                         static_cast<GLsizei>(contextSettings.antiAliasingLevel),
                                                         GL_STENCIL_INDEX8,
                                                         static_cast<GLsizei>(size.x),
                                                         static_cast<GLsizei>(size.y)));

                m_impl->depth   = false;
                m_impl->stencil = true;
            }

            m_impl->multisample = true;
        }
    }

    // Save our texture ID in order to be able to attach it to an FBO at any time
    m_impl->textureId = textureId;

    // We can't create an FBO now if there is no active context
    if (!m_impl->graphicsContext->hasActiveThreadLocalGlContext())
        return true;

    // Save the current bindings so we can restore them after we are done
    const auto readFramebuffer = getGLInteger(GL_READ_FRAMEBUFFER_BINDING);
    const auto drawFramebuffer = getGLInteger(GL_DRAW_FRAMEBUFFER_BINDING);

    if (createFrameBuffer())
    {
        // Restore previously bound framebuffers
        glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFramebuffer)));
        glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFramebuffer)));

        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::createFrameBuffer()
{
    // Create the framebuffer object
    GLuint frameBufferId{};
    glCheck(glGenFramebuffers(1, &frameBufferId));

    if (!frameBufferId)
    {
        err() << "Impossible to create render texture (failed to create the frame buffer object)";
        return false;
    }

    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId));

    // Link the depth/stencil renderbuffer to the frame buffer
    if (!m_impl->multisample && m_impl->depthStencilBuffer)
    {
        if (m_impl->depth)
        {
            glCheck(
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_impl->depthStencilBuffer));
        }

        if (m_impl->stencil)
        {
            glCheck(
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_impl->depthStencilBuffer));
        }
    }

    // Link the texture to the frame buffer
    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_impl->textureId, 0));

    // A final check, just to be sure...
    if (glCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        err() << "Impossible to create render texture (failed to link the target texture to the frame buffer)";
        return false;
    }

    // Get current GL context id
    const unsigned int glContextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

    // Insert the FBO into our map
    m_impl->frameBuffers.try_emplace(glContextId, frameBufferId);

    // Register the object with the current context so it is automatically destroyed
    m_impl->graphicsContext->registerUnsharedFrameBuffer(glContextId, frameBufferId, &deleteFrameBuffer);

    if (m_impl->multisample)
    {
        // Create the multisample framebuffer object
        GLuint multisampleFrameBufferId{};
        glCheck(glGenFramebuffers(1, &multisampleFrameBufferId));

        if (!multisampleFrameBufferId)
        {
            err() << "Impossible to create render texture (failed to create the multisample frame buffer object)";
            return false;
        }
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, multisampleFrameBufferId));

        // Link the multisample color buffer to the frame buffer
        glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_impl->colorBuffer));
        glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_impl->colorBuffer));

        // Link the depth/stencil renderbuffer to the frame buffer
        if (m_impl->depthStencilBuffer)
        {
            if (m_impl->depth)
            {
                glCheck(
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_impl->depthStencilBuffer));
            }

            if (m_impl->stencil)
            {
                glCheck(
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_impl->depthStencilBuffer));
            }
        }

        // A final check, just to be sure...
        if (glCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
        {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            err() << "Impossible to create render texture (failed to link the render buffers to the multisample "
                     "frame "
                     "buffer)";
            return false;
        }

        // Insert the FBO into our map
        m_impl->multisampleFrameBuffers.try_emplace(glContextId, multisampleFrameBufferId);

        // Register the object with the current context so it is automatically destroyed
        m_impl->graphicsContext->registerUnsharedFrameBuffer(glContextId, multisampleFrameBufferId, &deleteFrameBuffer);
    }

    return true;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::activate(bool active)
{
    // Unbind the FBO if requested
    if (!active)
    {
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        return true;
    }

    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());
    const unsigned int glContextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

    // Lookup the FBO corresponding to the currently active context
    // If none is found, there is no FBO corresponding to the
    // currently active context so we will have to create a new FBO
    if (m_impl->multisample)
    {
        const auto it = m_impl->multisampleFrameBuffers.find(glContextId);

        if (it != m_impl->multisampleFrameBuffers.end())
        {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, it->second));
            return true;
        }
    }
    else
    {
        const auto it = m_impl->frameBuffers.find(glContextId);

        if (it != m_impl->frameBuffers.end())
        {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, it->second));
            return true;
        }
    }

    return createFrameBuffer();
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::isSrgb() const
{
    return m_impl->sRgb;
}


////////////////////////////////////////////////////////////
void RenderTextureImplFBO::updateTexture(unsigned int)
{
    // If multisampling is enabled, we need to resolve by blitting
    // from our FBO with multisample renderbuffer attachments
    // to our FBO to which our target texture is attached

    // In case of multisampling, make sure both FBOs
    // are already available within the current context
    if (m_impl->multisample && m_impl->size.x && m_impl->size.y && activate(true))
    {
        const unsigned int glContextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

        const auto frameBufferIt = m_impl->frameBuffers.find(glContextId);
        const auto multisampleIt = m_impl->multisampleFrameBuffers.find(glContextId);

        if ((frameBufferIt != m_impl->frameBuffers.end()) && (multisampleIt != m_impl->multisampleFrameBuffers.end()))
        {
            // Scissor testing affects framebuffer blits as well
            // Since we don't want scissor testing to interfere with our copying, we temporarily disable it for the blit if it is enabled
            GLboolean scissorEnabled = GL_FALSE;
            glCheck(glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled));

            if (scissorEnabled == GL_TRUE)
                glCheck(glDisable(GL_SCISSOR_TEST));

            // Set up the blit target (draw framebuffer) and blit (from the read framebuffer, our multisample FBO)
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferIt->second));
            glCheck(glBlitFramebuffer(0,
                                      0,
                                      static_cast<GLint>(m_impl->size.x),
                                      static_cast<GLint>(m_impl->size.y),
                                      0,
                                      0,
                                      static_cast<GLint>(m_impl->size.x),
                                      static_cast<GLint>(m_impl->size.y),
                                      GL_COLOR_BUFFER_BIT,
                                      GL_NEAREST));
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleIt->second));

            // Re-enable scissor testing if it was previously enabled
            if (scissorEnabled == GL_TRUE)
                glCheck(glEnable(GL_SCISSOR_TEST));
        }
    }
}

} // namespace sf::priv
