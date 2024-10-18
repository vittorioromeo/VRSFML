#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/iOS/EaglContext.hpp"
#include "SFML/Window/iOS/SFView.hpp"
#include "SFML/Window/iOS/WindowImplUIKit.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include <OpenGLES/EAGL.h>
#include <OpenGLES/EAGLDrawable.h>
#include <QuartzCore/CAEAGLLayer.h>
#include <dlfcn.h>


#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


namespace
{
PFNGLBINDFRAMEBUFFEROESPROC            glBindFramebufferOESFunc            = nullptr;
PFNGLBINDRENDERBUFFEROESPROC           glBindRenderbufferOESFunc           = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSOESPROC     glCheckFramebufferStatusOESFunc     = nullptr;
PFNGLDELETEFRAMEBUFFERSOESPROC         glDeleteFramebuffersOESFunc         = nullptr;
PFNGLDELETERENDERBUFFERSOESPROC        glDeleteRenderbuffersOESFunc        = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFEROESPROC    glFramebufferRenderbufferOESFunc    = nullptr;
PFNGLGENFRAMEBUFFERSOESPROC            glGenFramebuffersOESFunc            = nullptr;
PFNGLGENRENDERBUFFERSOESPROC           glGenRenderbuffersOESFunc           = nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVOESPROC glGetRenderbufferParameterivOESFunc = nullptr;
PFNGLRENDERBUFFERSTORAGEOESPROC        glRenderbufferStorageOESFunc        = nullptr;


void ensureInit()
{
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;

        glBindFramebufferOESFunc = reinterpret_cast<PFNGLBINDFRAMEBUFFEROESPROC>(
            sf::priv::EaglContext::getFunction("glBindFramebufferOES"));
        glBindRenderbufferOESFunc = reinterpret_cast<PFNGLBINDRENDERBUFFEROESPROC>(
            sf::priv::EaglContext::getFunction("glBindRenderbufferOES"));
        glCheckFramebufferStatusOESFunc = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSOESPROC>(
            sf::priv::EaglContext::getFunction("glCheckFramebufferStatusOES"));
        glDeleteFramebuffersOESFunc = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSOESPROC>(
            sf::priv::EaglContext::getFunction("glDeleteFramebuffersOES"));
        glDeleteRenderbuffersOESFunc = reinterpret_cast<PFNGLDELETERENDERBUFFERSOESPROC>(
            sf::priv::EaglContext::getFunction("glDeleteRenderbuffersOES"));
        glFramebufferRenderbufferOESFunc = reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFEROESPROC>(
            sf::priv::EaglContext::getFunction("glFramebufferRenderbufferOES"));
        glGenFramebuffersOESFunc = reinterpret_cast<PFNGLGENFRAMEBUFFERSOESPROC>(
            sf::priv::EaglContext::getFunction("glGenFramebuffersOES"));
        glGenRenderbuffersOESFunc = reinterpret_cast<PFNGLGENRENDERBUFFERSOESPROC>(
            sf::priv::EaglContext::getFunction("glGenRenderbuffersOES"));
        glGetRenderbufferParameterivOESFunc = reinterpret_cast<PFNGLGETRENDERBUFFERPARAMETERIVOESPROC>(
            sf::priv::EaglContext::getFunction("glGetRenderbufferParameterivOES"));
        glRenderbufferStorageOESFunc = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEOESPROC>(
            sf::priv::EaglContext::getFunction("glRenderbufferStorageOES"));
    }
}
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
EaglContext::EaglContext(EaglContext* shared) : m_context(nil)
{
    ensureInit();

    // Create the context
    if (shared)
        m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1
                                          sharegroup:[shared->m_context sharegroup]];
    else
        m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
}


////////////////////////////////////////////////////////////
EaglContext::EaglContext(EaglContext*           shared,
                         const ContextSettings& contextSettings,
                         const WindowImpl&      owner,
                         unsigned int           bitsPerPixel) :
m_context(nil)
{
    ensureInit();

    const auto& window = static_cast<const WindowImplUIKit&>(owner);

    createContext(shared, window, bitsPerPixel, contextSettings);
}


////////////////////////////////////////////////////////////
EaglContext::~EaglContext()
{
    // Notify unshared OpenGL resources of context destruction
    WindowContext::ensureInstalled().cleanupUnsharedFrameBuffers(*this);

    if (m_context)
    {
        // Activate the context, so that we can destroy the buffers
        EAGLContext* const previousContext = [EAGLContext currentContext];
        [EAGLContext setCurrentContext:m_context];

        // Destroy the buffers
        if (m_framebuffer)
            glDeleteFramebuffersOESFunc(1, &m_framebuffer);
        if (m_colorbuffer)
            glDeleteRenderbuffersOESFunc(1, &m_colorbuffer);
        if (m_depthbuffer)
            glDeleteRenderbuffersOESFunc(1, &m_depthbuffer);

        // Restore the previous context
        [EAGLContext setCurrentContext:previousContext];

        if (m_context == [EAGLContext currentContext])
            [EAGLContext setCurrentContext:nil];
    }
}


////////////////////////////////////////////////////////////
GlFunctionPointer EaglContext::getFunction(const char* name)
{
    static void* module = nullptr;

    constexpr const char* libs[] = {"libGLESv1_CM.dylib",
                                    "/System/Library/Frameworks/OpenGLES.framework/OpenGLES",
                                    "OpenGLES.framework/OpenGLES"};

    for (const auto& lib : libs)
    {
        if (!module)
            module = dlopen(lib, RTLD_LAZY | RTLD_LOCAL);
    }

    if (module)
        return reinterpret_cast<GlFunctionPointer>(reinterpret_cast<uintptr_t>(dlsym(module, name)));

    return nullptr;
}


////////////////////////////////////////////////////////////
void EaglContext::recreateRenderBuffers(SFView* glView)
{
    // Activate the context
    EAGLContext* const previousContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:m_context];

    // Bind the frame buffer
    glBindFramebufferOESFunc(GL_FRAMEBUFFER_OES, m_framebuffer);

    // Destroy previous render-buffers
    if (m_colorbuffer)
        glDeleteRenderbuffersOESFunc(1, &m_colorbuffer);
    if (m_depthbuffer)
        glDeleteRenderbuffersOESFunc(1, &m_depthbuffer);

    // Create the color buffer
    glGenRenderbuffersOESFunc(1, &m_colorbuffer);
    glBindRenderbufferOESFunc(GL_RENDERBUFFER_OES, m_colorbuffer);
    if (glView)
        [m_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(static_cast<CAEAGLLayer*>(glView.layer))];
    glFramebufferRenderbufferOESFunc(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_colorbuffer);

    // Create a depth buffer if requested
    if (m_settings.depthBits > 0)
    {
        // Find the best internal format
        const GLenum format = m_settings.depthBits > 16
                                  ? (m_settings.stencilBits == 0 ? GL_DEPTH_COMPONENT24_OES : GL_DEPTH24_STENCIL8_OES)
                                  : GL_DEPTH_COMPONENT16_OES;

        // Get the size of the color-buffer (which fits the current size of the GL view)
        GLint width  = 0;
        GLint height = 0;
        glGetRenderbufferParameterivOESFunc(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
        glGetRenderbufferParameterivOESFunc(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);

        // Create the depth buffer
        glGenRenderbuffersOESFunc(1, &m_depthbuffer);
        glBindRenderbufferOESFunc(GL_RENDERBUFFER_OES, m_depthbuffer);
        glRenderbufferStorageOESFunc(GL_RENDERBUFFER_OES, format, width, height);
        glFramebufferRenderbufferOESFunc(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_depthbuffer);
        if (m_settings.stencilBits > 0)
            glFramebufferRenderbufferOESFunc(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_depthbuffer);
    }

    // Make sure that everything's ok
    const GLenum status = glCheckFramebufferStatusOESFunc(GL_FRAMEBUFFER_OES);
    if (status != GL_FRAMEBUFFER_COMPLETE_OES)
        priv::err() << "Failed to create a valid frame buffer (error code: " << status << ")" priv::errEndl;

    // Restore the previous context
    [EAGLContext setCurrentContext:previousContext];
}


////////////////////////////////////////////////////////////
bool EaglContext::makeCurrent(bool current)
{
    if (current)
        return [EAGLContext setCurrentContext:m_context];

    return [EAGLContext setCurrentContext:nil];
}


////////////////////////////////////////////////////////////
void EaglContext::display()
{
    glBindRenderbufferOESFunc(GL_RENDERBUFFER_OES, m_colorbuffer);
    [m_context presentRenderbuffer:GL_RENDERBUFFER_OES];

    // The proper way of doing v-sync on iOS would be to use CADisplayLink
    // notifications, but it is not compatible with the way SFML is designed;
    // therefore we fake it with a manual framerate limit
    if (m_vsyncEnabled)
    {
        constexpr Time frameDuration = seconds(1.f / 60.f);
        sleep(frameDuration - m_clock.getElapsedTime());
        m_clock.restart();
    }
}


////////////////////////////////////////////////////////////
void EaglContext::setVerticalSyncEnabled(bool enabled)
{
    m_vsyncEnabled = enabled;
}


////////////////////////////////////////////////////////////
void EaglContext::createContext(EaglContext*           shared,
                                const WindowImplUIKit& window,
                                unsigned int /* bitsPerPixel */,
                                const ContextSettings& contextSettings)
{
    // Save the settings
    m_settings = contextSettings;

    // Adjust the depth buffer format to those available
    if (m_settings.depthBits > 16)
        m_settings.depthBits = 24;
    else if (m_settings.depthBits > 0)
        m_settings.depthBits = 16;

    // Create the context
    if (shared)
    {
        [EAGLContext setCurrentContext:nil];

        m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1
                                          sharegroup:[shared->m_context sharegroup]];
    }
    else
    {
        m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    }

    // Activate it
    makeCurrent(true);

    // Create the framebuffer (this is the only allowed drawable on iOS)
    glGenFramebuffersOESFunc(1, &m_framebuffer);

    // Create the render buffers
    recreateRenderBuffers(window.getGlView());

    // Attach the context to the GL view for future updates
    window.getGlView().context = this;

    // Deactivate it
    makeCurrent(false);
}

} // namespace sf::priv
