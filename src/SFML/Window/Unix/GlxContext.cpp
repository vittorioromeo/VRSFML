#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Unix/Display.hpp"
#include "SFML/Window/Unix/GlxContext.hpp"
#include "SFML/Window/Unix/Utils.hpp"
#include "SFML/Window/Unix/WindowImplX11.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include <array>
#include <mutex>
#include <vector>

// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef GLAD_GLX_IMPLEMENTATION_INCLUDED
#define GLAD_GLX_IMPLEMENTATION_INCLUDED
#define GLAD_GLX_IMPLEMENTATION
#include <glad/glx.h>
#endif

#if !defined(GLX_DEBUGGING) && defined(SFML_DEBUG)
// Enable this to print messages to err() every time GLX produces errors
//#define GLX_DEBUGGING
#endif

namespace
{
std::recursive_mutex glxErrorMutex;
bool                 glxErrorOccurred = false;


////////////////////////////////////////////////////////////
void ensureExtensionsInit(::Display* display, int screen)
{
    static bool initialized = false;
    if (initialized)
        return;

    initialized = true;

    // We don't check the return value since the extension
    // flags are cleared even if loading fails
    gladLoaderLoadGLX(display, screen);

    auto f = [](const char* name) { return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name)); };
    gladLoadGLX(display, screen, f);
    // gladLoadGLX(display, screen, sf::priv::GlxContext::getFunction); // TODO P0:
}


int handleXError(::Display*, XErrorEvent*)
{
    glxErrorOccurred = true;
    return 0;
}


class GlxErrorHandler
{
public:
    explicit GlxErrorHandler(::Display* display) : m_display(display)
    {
        glxErrorOccurred  = false;
        m_previousHandler = XSetErrorHandler(handleXError);
    }

    ~GlxErrorHandler()
    {
        XSync(m_display, False);
        XSetErrorHandler(m_previousHandler);
    }

private:
    std::lock_guard<std::recursive_mutex> m_lock{glxErrorMutex};
    ::Display*                            m_display;
    int (*m_previousHandler)(::Display*, XErrorEvent*);
};
} // namespace


namespace sf::priv
{

////////////////////////////////////////////////////////////
template <typename... TSurfaceArgs>
GlxContext::GlxContext(unsigned int id, GlxContext* shared, const ContextSettings& contextSettings, TSurfaceArgs&&... surfaceArgs) :
GlContext(id, {}),
m_display(openDisplay())
{
    // Save the creation settings
    GlContext::m_settings = contextSettings;

    // Make sure that extensions are initialized
    ensureExtensionsInit(m_display.get(), DefaultScreen(m_display.get()));

    // Create the rendering surface from the owner window
    createSurface(SFML_BASE_FORWARD(surfaceArgs)...);

    // Create the context
    createContext(shared);
}


////////////////////////////////////////////////////////////
GlxContext::GlxContext(unsigned int id, GlxContext* shared) :
GlxContext(id,
           shared,
           ContextSettings{},
           // surface args:
           shared,
           Vector2u{1u, 1u},
           VideoModeUtils::getDesktopMode().bitsPerPixel)
{
}


////////////////////////////////////////////////////////////
GlxContext::GlxContext(unsigned int           id,
                       GlxContext*            shared,
                       const ContextSettings& contextSettings,
                       const WindowImpl&      owner,
                       unsigned int /*bitsPerPixel*/) :
GlxContext(id,
           shared,
           contextSettings,
           // surface args:
           owner.getNativeHandle())
{
}


////////////////////////////////////////////////////////////
GlxContext::~GlxContext()
{
    // Notify unshared OpenGL resources of context destruction
    cleanupUnsharedFrameBuffers();

    // Destroy the context
    if (m_context)
    {
#if defined(GLX_DEBUGGING)
        GlxErrorHandler handler(m_display.get());
#endif

        if (glXGetCurrentContext() == m_context)
            glXMakeCurrent(m_display.get(), None, nullptr);
        glXDestroyContext(m_display.get(), m_context);

#if defined(GLX_DEBUGGING)
        if (glxErrorOccurred)
            priv::err() << "GLX error in GlxContext::~GlxContext()";
#endif
    }

    if (m_pbuffer)
    {
        glXDestroyPbuffer(m_display.get(), m_pbuffer);
    }

    // Destroy the window if we own it
    if (m_window && m_ownsWindow)
    {
        XDestroyWindow(m_display.get(), m_window);
        XFlush(m_display.get());
    }
}


////////////////////////////////////////////////////////////
GlFunctionPointer GlxContext::getFunction(const char* name) const
{
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name));
}


////////////////////////////////////////////////////////////
bool GlxContext::makeCurrent(bool current)
{
    if (!m_context)
        return false;

#if defined(GLX_DEBUGGING)
    GlxErrorHandler handler(m_display.get());
#endif

    bool result = false;

    if (current)
    {
        if (m_pbuffer)
        {
            result = glXMakeContextCurrent(m_display.get(), m_pbuffer, m_pbuffer, m_context);
        }
        else if (m_window)
        {
            result = glXMakeCurrent(m_display.get(), m_window, m_context);
        }
    }
    else
    {
        result = glXMakeCurrent(m_display.get(), None, nullptr);
    }

#if defined(GLX_DEBUGGING)
    if (glxErrorOccurred)
        priv::err() << "GLX error in GlxContext::makeCurrent()";
#endif

    return result;
}


////////////////////////////////////////////////////////////
void GlxContext::display()
{
#if defined(GLX_DEBUGGING)
    GlxErrorHandler handler(m_display.get());
#endif

    if (m_pbuffer)
        glXSwapBuffers(m_display.get(), m_pbuffer);
    else if (m_window)
        glXSwapBuffers(m_display.get(), m_window);

#if defined(GLX_DEBUGGING)
    if (glxErrorOccurred)
        priv::err() << "GLX error in GlxContext::display()";
#endif
}


////////////////////////////////////////////////////////////
void GlxContext::setVerticalSyncEnabled(bool enabled)
{
    int result = 0;

    // Prioritize the EXT variant and fall back to MESA or SGI if needed
    // We use the direct pointer to the MESA entry point instead of the alias
    // because glx.h declares the entry point as an external function
    // which would require us to link in an additional library
    if (GLAD_GLX_EXT_swap_control)
    {
        glXSwapIntervalEXT(m_display.get(), m_pbuffer ? m_pbuffer : m_window, enabled);
    }
    else if (GLAD_GLX_MESA_swap_control)
    {
        result = glXSwapIntervalMESA(enabled);
    }
    else if (GLAD_GLX_SGI_swap_control)
    {
        result = glXSwapIntervalSGI(enabled);
    }
    else
    {
        static bool warned = false;

        if (!warned)
        {
            priv::err() << "Setting vertical sync not supported";
            warned = true;
        }
    }

    if (result != 0)
        priv::err() << "Setting vertical sync failed";
}


////////////////////////////////////////////////////////////
XVisualInfo GlxContext::selectBestVisual(::Display* display, unsigned int bitsPerPixel, const ContextSettings& contextSettings)
{
    // Make sure that extensions are initialized
    ensureExtensionsInit(display, DefaultScreen(display));

    const int screen = DefaultScreen(display);

    // Retrieve all the visuals
    int count = 0;
    if (const auto visuals = X11Ptr<XVisualInfo[]>(XGetVisualInfo(display, 0, nullptr, &count)))
    {
        // Evaluate all the returned visuals, and pick the best one
        int         bestScore  = 0x7FFFFFFF;
        XVisualInfo bestVisual = XVisualInfo();
        for (base::SizeT i = 0; i < static_cast<base::SizeT>(count); ++i)
        {
            auto& visual = visuals.get()[i];

            // Filter by screen
            if (visual.screen != screen)
                continue;

            // Check mandatory attributes
            int doubleBuffer = 0;
            glXGetConfig(display, &visual, GLX_DOUBLEBUFFER, &doubleBuffer);
            if (!doubleBuffer)
                continue;

            // Extract the components of the current visual
            int red           = 0;
            int green         = 0;
            int blue          = 0;
            int alpha         = 0;
            int depth         = 0;
            int stencil       = 0;
            int multiSampling = 0;
            int samples       = 0;
            int sRgb          = 0;
            glXGetConfig(display, &visual, GLX_RED_SIZE, &red);
            glXGetConfig(display, &visual, GLX_GREEN_SIZE, &green);
            glXGetConfig(display, &visual, GLX_BLUE_SIZE, &blue);
            glXGetConfig(display, &visual, GLX_ALPHA_SIZE, &alpha);
            glXGetConfig(display, &visual, GLX_DEPTH_SIZE, &depth);
            glXGetConfig(display, &visual, GLX_STENCIL_SIZE, &stencil);

            if (GLAD_GLX_ARB_multisample)
            {
                glXGetConfig(display, &visual, GLX_SAMPLE_BUFFERS_ARB, &multiSampling);
                glXGetConfig(display, &visual, GLX_SAMPLES_ARB, &samples);
            }
            else
            {
                multiSampling = 0;
                samples       = 0;
            }

            if (GLAD_GLX_EXT_framebuffer_sRGB || GLAD_GLX_ARB_framebuffer_sRGB)
            {
                glXGetConfig(display, &visual, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, &sRgb);
            }
            else
            {
                sRgb = 0;
            }

            // TODO P2: Replace this with proper acceleration detection
            const bool accelerated = true;

            // Evaluate the visual
            const int color = red + green + blue + alpha;
            const int score = GlContext::evaluateFormat(bitsPerPixel,
                                                        contextSettings,
                                                        color,
                                                        depth,
                                                        stencil,
                                                        multiSampling ? samples : 0,
                                                        accelerated,
                                                        sRgb == True);

            // If it's better than the current best, make it the new best
            if (score < bestScore)
            {
                bestScore  = score;
                bestVisual = visual;
            }
        }

        return bestVisual;
    }

    // Should never happen...
    priv::err() << "No GLX visual found. You should check your graphics driver";
    return {};
}


////////////////////////////////////////////////////////////
void GlxContext::updateSettingsFromVisualInfo(XVisualInfo* visualInfo)
{
    // Update the creation contextSettings from the chosen format
    int depth         = 0;
    int stencil       = 0;
    int multiSampling = 0;
    int samples       = 0;
    int sRgb          = 0;
    glXGetConfig(m_display.get(), visualInfo, GLX_DEPTH_SIZE, &depth);
    glXGetConfig(m_display.get(), visualInfo, GLX_STENCIL_SIZE, &stencil);

    if (GLAD_GLX_ARB_multisample)
    {
        glXGetConfig(m_display.get(), visualInfo, GLX_SAMPLE_BUFFERS_ARB, &multiSampling);
        glXGetConfig(m_display.get(), visualInfo, GLX_SAMPLES_ARB, &samples);
    }
    else
    {
        multiSampling = 0;
        samples       = 0;
    }

    if (GLAD_GLX_EXT_framebuffer_sRGB || GLAD_GLX_ARB_framebuffer_sRGB)
    {
        glXGetConfig(m_display.get(), visualInfo, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, &sRgb);
    }
    else
    {
        sRgb = 0;
    }

    m_settings.depthBits         = static_cast<unsigned int>(depth);
    m_settings.stencilBits       = static_cast<unsigned int>(stencil);
    m_settings.antiAliasingLevel = multiSampling ? static_cast<unsigned int>(samples) : 0;
    m_settings.sRgbCapable       = (sRgb == True);
}


////////////////////////////////////////////////////////////
void GlxContext::updateSettingsFromWindow()
{
    // Retrieve the attributes of the target window
    XWindowAttributes windowAttributes;
    if (XGetWindowAttributes(m_display.get(), m_window, &windowAttributes) == 0)
    {
        priv::err() << "Failed to get the window attributes";
        return;
    }

    // Get its visuals
    XVisualInfo tpl;
    tpl.screen            = DefaultScreen(m_display.get());
    tpl.visualid          = XVisualIDFromVisual(windowAttributes.visual);
    int        nbVisuals  = 0;
    const auto visualInfo = X11Ptr<XVisualInfo>(
        XGetVisualInfo(m_display.get(), VisualIDMask | VisualScreenMask, &tpl, &nbVisuals));

    if (!visualInfo)
        return;

    updateSettingsFromVisualInfo(visualInfo.get());
}


////////////////////////////////////////////////////////////
void GlxContext::createSurface(GlxContext* shared, Vector2u size, unsigned int bitsPerPixel)
{
    // Choose the visual according to the context settings
    XVisualInfo visualInfo = selectBestVisual(m_display.get(), bitsPerPixel, m_settings);

    // Check if the shared context already exists and pbuffers are supported
    if (shared && GLAD_GLX_SGIX_pbuffer)
    {
        // There are no GLX versions prior to 1.0
        int major = 0;
        int minor = 0;

        glXQueryVersion(m_display.get(), &major, &minor);

        // Check if glXCreatePbuffer is available (requires GLX 1.3 or greater)
        const bool hasCreatePbuffer = ((major > 1) || (minor >= 3));

        if (hasCreatePbuffer)
        {
            // Get a GLXFBConfig that matches the visual
            GLXFBConfig* config = nullptr;

            // We don't supply attributes to match against, since
            // the visual we are matching against was already
            // deemed suitable in selectBestVisual()
            int        nbConfigs = 0;
            const auto configs   = X11Ptr<GLXFBConfig[]>(
                glXChooseFBConfig(m_display.get(), DefaultScreen(m_display.get()), nullptr, &nbConfigs));

            for (base::SizeT i = 0; configs && (i < static_cast<base::SizeT>(nbConfigs)); ++i)
            {
                const auto visual = X11Ptr<XVisualInfo>(glXGetVisualFromFBConfig(m_display.get(), configs.get()[i]));

                if (!visual)
                    continue;

                if (visual->visualid == visualInfo.visualid)
                {
                    config = &configs.get()[i];
                    break;
                }
            }

            if (config)
            {
                std::array attributes =
                    {GLX_PBUFFER_WIDTH, static_cast<int>(size.x), GLX_PBUFFER_HEIGHT, static_cast<int>(size.y), 0, 0};

                m_pbuffer = glXCreatePbuffer(m_display.get(), *config, attributes.data());

                updateSettingsFromVisualInfo(&visualInfo);

                return;
            }
        }
    }

    // If pbuffers are not available we use a hidden window as the off-screen surface to draw to
    const int screen = DefaultScreen(m_display.get());

    // Define the window attributes
    XSetWindowAttributes attributes;
    attributes.colormap = XCreateColormap(m_display.get(), RootWindow(m_display.get(), screen), visualInfo.visual, AllocNone);

    // Note: bitsPerPixel is explicitly ignored. Instead, DefaultDepth() is used in order to avoid window creation failure due to
    // a depth not supported by the X window system. On Unix/Linux, the window's pixel format is not directly associated with the
    // rendering surface (unlike on Windows, for example).
    m_window = XCreateWindow(m_display.get(),
                             RootWindow(m_display.get(), screen),
                             0,
                             0,
                             size.x,
                             size.y,
                             0,
                             DefaultDepth(m_display.get(), screen),
                             InputOutput,
                             visualInfo.visual,
                             CWColormap,
                             &attributes);

    m_ownsWindow = true;

    updateSettingsFromWindow();
}


////////////////////////////////////////////////////////////
void GlxContext::createSurface(::Window window)
{
    // A window already exists, so just use it
    m_window = window;

    updateSettingsFromWindow();
}


////////////////////////////////////////////////////////////
void GlxContext::createContext(GlxContext* shared)
{
    // Get a working copy of the context settings
    const ContextSettings contextSettings = m_settings;

    X11Ptr<XVisualInfo> visualInfo;

    if (m_pbuffer)
    {
        unsigned int fbConfigId = 0;

        glXQueryDrawable(m_display.get(), m_pbuffer, GLX_FBCONFIG_ID, &fbConfigId);

        std::array attributes = {GLX_FBCONFIG_ID, static_cast<int>(fbConfigId), 0, 0};

        int        count    = 0;
        const auto fbconfig = X11Ptr<GLXFBConfig>(
            glXChooseFBConfig(m_display.get(), DefaultScreen(m_display.get()), attributes.data(), &count));

        if (count == 1)
            visualInfo = X11Ptr<XVisualInfo>(glXGetVisualFromFBConfig(m_display.get(), *fbconfig));
    }
    else
    {
        // Retrieve the attributes of the target window
        XWindowAttributes windowAttributes;
        if (XGetWindowAttributes(m_display.get(), m_window, &windowAttributes) == 0)
        {
            priv::err() << "Failed to get the window attributes";
            return;
        }

        // Get its visuals
        XVisualInfo tpl;
        tpl.screen    = DefaultScreen(m_display.get());
        tpl.visualid  = XVisualIDFromVisual(windowAttributes.visual);
        int nbVisuals = 0;
        visualInfo = X11Ptr<XVisualInfo>(XGetVisualInfo(m_display.get(), VisualIDMask | VisualScreenMask, &tpl, &nbVisuals));
    }

    if (!visualInfo)
    {
        priv::err() << "Failed to get visual info";
        return;
    }

    // Get the context to share display lists with
    GLXContext toShare = shared ? shared->m_context : nullptr;

    // There are no GLX versions prior to 1.0
    int major = 0;
    int minor = 0;

    if (!glXQueryVersion(m_display.get(), &major, &minor))
        priv::err() << "Failed to query GLX version, limited to legacy context creation";

    // Check if glXCreateContextAttribsARB is available (requires GLX 1.3 or greater)
    const bool hasCreateContextArb = GLAD_GLX_ARB_create_context && ((major > 1) || (minor >= 3));

    // Create the OpenGL context -- first try using glXCreateContextAttribsARB
    if (hasCreateContextArb)
    {
        // Get a GLXFBConfig that matches the window's visual, for glXCreateContextAttribsARB
        GLXFBConfig* config = nullptr;

        // We don't supply attributes to match against, since
        // the visual we are matching against was already
        // deemed suitable in selectBestVisual()
        int        nbConfigs = 0;
        const auto configs   = X11Ptr<GLXFBConfig[]>(
            glXChooseFBConfig(m_display.get(), DefaultScreen(m_display.get()), nullptr, &nbConfigs));

        for (base::SizeT i = 0; configs && (i < static_cast<base::SizeT>(nbConfigs)); ++i)
        {
            const auto visual = X11Ptr<XVisualInfo>(glXGetVisualFromFBConfig(m_display.get(), configs.get()[i]));

            if (!visual)
                continue;

            if (visual->visualid == visualInfo->visualid)
            {
                config = &configs.get()[i];
                break;
            }
        }

        if (!config)
            priv::err() << "Failed to get GLXFBConfig which corresponds to the window's visual";

        while (config && !m_context && m_settings.majorVersion)
        {
            std::vector<int> attributes;

            // Check if the user requested a specific context version (anything > 1.1)
            if ((m_settings.majorVersion > 1) || ((m_settings.majorVersion == 1) && (m_settings.minorVersion > 1)))
            {
                attributes.push_back(GLX_CONTEXT_MAJOR_VERSION_ARB);
                attributes.push_back(static_cast<int>(m_settings.majorVersion));
                attributes.push_back(GLX_CONTEXT_MINOR_VERSION_ARB);
                attributes.push_back(static_cast<int>(m_settings.minorVersion));
            }

            // Check if setting the profile is supported
            if (GLAD_GLX_ARB_create_context_profile)
            {
                const int profile = !!(m_settings.attributeFlags & ContextSettings::Attribute::Core)
                                        ? GLX_CONTEXT_CORE_PROFILE_BIT_ARB
                                        : GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                const int debug   = !!(m_settings.attributeFlags & ContextSettings::Attribute::Debug)
                                        ? GLX_CONTEXT_DEBUG_BIT_ARB
                                        : 0;

                attributes.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
                attributes.push_back(profile);
                attributes.push_back(GLX_CONTEXT_FLAGS_ARB);
                attributes.push_back(debug);
            }
            else
            {
                if (!!(m_settings.attributeFlags & ContextSettings::Attribute::Core) ||
                    !!(m_settings.attributeFlags & ContextSettings::Attribute::Debug))
                    priv::err() << "Selecting a profile during context creation is not supported,"
                                << "disabling compatibility and debug";

                m_settings.attributeFlags = ContextSettings::Attribute::Default;
            }

            // Append the terminating 0
            attributes.push_back(0);
            attributes.push_back(0);

            // RAII GLX error handler (we simply ignore errors here)
            // On an error, glXCreateContextAttribsARB will return 0 anyway
            const GlxErrorHandler handler(m_display.get());

            if (toShare)
            {
                if (!glXMakeCurrent(m_display.get(), None, nullptr))
                {
                    priv::err() << "Failed to deactivate shared context before sharing";
                    return;
                }
            }

            // Create the context
            m_context = glXCreateContextAttribsARB(m_display.get(), *config, toShare, true, attributes.data());

            if (!m_context)
            {
                // If we couldn't create the context, first try disabling flags,
                // then lower the version number and try again -- stop at 0.0
                // Invalid version numbers will be generated by this algorithm (like 3.9), but we really don't care
                if (m_settings.attributeFlags != ContextSettings::Attribute::Default)
                {
                    m_settings.attributeFlags = ContextSettings::Attribute::Default;
                }
                else if (m_settings.minorVersion > 0)
                {
                    // If the minor version is not 0, we decrease it and try again
                    --m_settings.minorVersion;

                    m_settings.attributeFlags = contextSettings.attributeFlags;
                }
                else
                {
                    // If the minor version is 0, we decrease the major version
                    --m_settings.majorVersion;
                    m_settings.minorVersion = 9;

                    m_settings.attributeFlags = contextSettings.attributeFlags;
                }
            }
        }
    }

    // If glXCreateContextAttribsARB failed, use glXCreateContext
    if (!m_context)
    {
        // set the context version to 2.1 (arbitrary) and disable flags
        m_settings.majorVersion   = 2;
        m_settings.minorVersion   = 1;
        m_settings.attributeFlags = ContextSettings::Attribute::Default;

#if defined(GLX_DEBUGGING)
        GlxErrorHandler handler(m_display.get());
#endif

        if (toShare)
        {
            if (!glXMakeCurrent(m_display.get(), None, nullptr))
            {
                priv::err() << "Failed to deactivate shared context before sharing";
                return;
            }
        }

        // Create the context, using the target window's visual
        m_context = glXCreateContext(m_display.get(), visualInfo.get(), toShare, true);

#if defined(GLX_DEBUGGING)
        if (glxErrorOccurred)
            priv::err() << "GLX error in GlxContext::createContext()";
#endif
    }

    if (!m_context)
        priv::err() << "Failed to create an OpenGL context for this window";
}

} // namespace sf::priv
