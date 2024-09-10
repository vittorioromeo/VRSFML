#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/EGLCheck.hpp"
#include "SFML/Window/EglContext.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowImpl.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"

#include <memory>
#ifdef SFML_SYSTEM_ANDROID
#include "SFML/System/Android/Activity.hpp"

#include <mutex>
#endif
#if defined(SFML_SYSTEM_LINUX) && !defined(SFML_USE_DRM)
#include "SFML/Window/Unix/Utils.hpp"

#include <X11/Xlib.h>
#endif

// I guess Emscripten provides its own EGL definitions and adding our own causes issues.
#ifndef SFML_SYSTEM_EMSCRIPTEN

// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef GLAD_EGL_IMPLEMENTATION_INCLUDED
#define GLAD_EGL_IMPLEMENTATION_INCLUDED
#define GLAD_EGL_IMPLEMENTATION
#include <glad/egl.h>
#endif

#endif


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace EglContextImpl
{
[[nodiscard]] EGLDisplay getInitializedDisplay()
{
#if defined(SFML_SYSTEM_ANDROID)

    // On Android, its native activity handles this for us
    sf::priv::ActivityStates& states = sf::priv::getActivity();
    const std::lock_guard     lock(states.mutex);

    return states.display;

#endif

    static EGLDisplay display = EGL_NO_DISPLAY;

    if (display == EGL_NO_DISPLAY)
    {
        eglCheck(display = eglGetDisplay(EGL_DEFAULT_DISPLAY));
        eglCheck(eglInitialize(display, nullptr, nullptr));
    }

    SFML_BASE_ASSERT(display != EGL_NO_DISPLAY);
    return display;
}


////////////////////////////////////////////////////////////
bool ensureInit()
{
#ifndef SFML_SYSTEM_EMSCRIPTEN

    static bool result = []
    {
        if (!gladLoaderLoadEGL(EGL_NO_DISPLAY))
        {
            // At this point, the failure is unrecoverable
            // Dump a message to the console and let the application terminate
            sf::priv::err() << "Failed to load EGL entry points";

            SFML_BASE_ASSERT(false);

            return false;
        }

        // Continue loading with a display
        gladLoaderLoadEGL(getInitializedDisplay());

        return true;
    }();

    SFML_BASE_ASSERT(result);
    return result;

#else

    return true;

#endif
}

} // namespace EglContextImpl
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
EglContext::EglContext(WindowContext& windowContext, std::uint64_t id, EglContext* shared) :
GlContext(windowContext, id, {})
{
    EglContextImpl::ensureInit();

    // Get the initialized EGL display
    m_display = EglContextImpl::getInitializedDisplay();

    // Get the best EGL config matching the default video contextSettings
    m_config = getBestConfig(m_display, VideoModeUtils::getDesktopMode().bitsPerPixel, ContextSettings{});
    updateSettings();

#ifndef SFML_SYSTEM_EMSCRIPTEN
    // Note: The EGL specs say that attribList can be a null pointer when passed to eglCreatePbufferSurface,
    // but this is resulting in a segfault. Bug in Android?
    EGLint attribList[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};

    eglCheck(m_surface = eglCreatePbufferSurface(m_display, m_config, attribList));
    SFML_BASE_ASSERT(m_surface != EGL_NO_SURFACE);
#else
    EGLNativeWindowType dummyWindow{};
    createSurface(dummyWindow);
#endif

    // Create EGL context
    createContext(shared);
}


////////////////////////////////////////////////////////////
EglContext::EglContext(WindowContext&                     windowContext,
                       std::uint64_t                      id,
                       EglContext*                        shared,
                       const ContextSettings&             contextSettings,
                       [[maybe_unused]] const WindowImpl& owner,
                       unsigned int                       bitsPerPixel) :
GlContext(windowContext, id, contextSettings)
{
    EglContextImpl::ensureInit();

#ifdef SFML_SYSTEM_ANDROID

    // On Android, we must save the created context
    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    states.context = this;

#endif

    // Get the initialized EGL display
    m_display = EglContextImpl::getInitializedDisplay();

    // Get the best EGL config matching the requested video contextSettings
    m_config = getBestConfig(m_display, bitsPerPixel, contextSettings);
    updateSettings();

    // Create EGL context
    createContext(shared);

#if !defined(SFML_SYSTEM_ANDROID)
    // Create EGL surface (except on Android because the window is created
    // asynchronously, its activity manager will call it for us)
    createSurface(owner.getNativeHandle());

#endif
}


////////////////////////////////////////////////////////////
EglContext::EglContext(WindowContext& windowContext,
                       std::uint64_t  id,
                       EglContext* /* shared */,
                       const ContextSettings& /* contextSettings */,
                       Vector2u /* size */) :
GlContext(windowContext, id, {})
{
    EglContextImpl::ensureInit();

    // TODO P0: this gets called from `RenderTextureImplDefault`
    sf::priv::err() << "Warning: context has not been initialized. The constructor EglContext(shared, "
                       "contextSettings, size) is currently not implemented.";
}


////////////////////////////////////////////////////////////
EglContext::~EglContext()
{
    // Notify unshared OpenGL resources of context destruction
    m_windowContext.cleanupUnsharedFrameBuffers(*this);

    // Deactivate the current context
    EGLContext currentContext = EGL_NO_CONTEXT;
    eglCheck(currentContext = eglGetCurrentContext());

    if (currentContext == m_context)
        eglCheck(eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

// Destroying the context on Emscripten seem to make every subsequent GL call fail,
// even if switching to another context...
#ifndef SFML_SYSTEM_EMSCRIPTEN
    if (m_context != EGL_NO_CONTEXT)
        eglCheck(eglDestroyContext(m_display, m_context));
#endif

    if (m_surface != EGL_NO_SURFACE)
        eglCheck(eglDestroySurface(m_display, m_surface));
}


////////////////////////////////////////////////////////////
GlFunctionPointer EglContext::getFunction(const char* name) const
{
    EglContextImpl::ensureInit();

    return eglGetProcAddress(name);
}


////////////////////////////////////////////////////////////
bool EglContext::makeCurrent(bool current)
{
    if (m_surface == EGL_NO_SURFACE)
        return false;

    EGLBoolean result = EGL_FALSE;

    if (current)
    {
        eglCheck(result = eglMakeCurrent(m_display, m_surface, m_surface, m_context));
    }
    else
    {
        eglCheck(result = eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    }

    return result != EGL_FALSE;
}


////////////////////////////////////////////////////////////
void EglContext::display()
{
    if (m_surface != EGL_NO_SURFACE)
        eglCheck(eglSwapBuffers(m_display, m_surface));
}


////////////////////////////////////////////////////////////
void EglContext::setVerticalSyncEnabled([[maybe_unused]] bool enabled)
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    eglCheck(eglSwapInterval(m_display, enabled ? 1 : 0));
#endif
}


////////////////////////////////////////////////////////////
void EglContext::createContext(EglContext* shared)
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    constexpr EGLint contextVersion[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE};
#else
    constexpr EGLint contextVersion[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
#endif

    EGLContext toShared = shared != nullptr ? shared->m_context : EGL_NO_CONTEXT;

    if (toShared != EGL_NO_CONTEXT)
        eglCheck(eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    // Create EGL context
    eglCheck(m_context = eglCreateContext(m_display, m_config, toShared, contextVersion));
    SFML_BASE_ASSERT(m_context != EGL_NO_CONTEXT);
}


////////////////////////////////////////////////////////////
void EglContext::createSurface(EGLNativeWindowType window)
{
    eglCheck(m_surface = eglCreateWindowSurface(m_display, m_config, window, nullptr));
    SFML_BASE_ASSERT(m_surface != EGL_NO_SURFACE);
}


////////////////////////////////////////////////////////////
void EglContext::destroySurface()
{
    // Seems to only be called by `WindowImplAndroid`

    if (!m_windowContext.setActiveThreadLocalGlContext(*this, false))
        err() << "Failure to disable EGL context in `EglContext::destroySurface`";

    eglCheck(eglDestroySurface(m_display, m_surface));
    m_surface = EGL_NO_SURFACE;
}


////////////////////////////////////////////////////////////
EGLConfig EglContext::getBestConfig(EGLDisplay display, unsigned int bitsPerPixel, const ContextSettings& contextSettings)
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    EglContextImpl::ensureInit();

    // Determine the number of available configs
    EGLint configCount = 0;
    eglCheck(eglGetConfigs(display, nullptr, 0, &configCount));

    // Retrieve the list of available configs
    const auto configs = std::make_unique<EGLConfig[]>(static_cast<std::size_t>(configCount));

    eglCheck(eglGetConfigs(display, configs.get(), configCount, &configCount));

    // Evaluate all the returned configs, and pick the best one
    int       bestScore = 0x7FFFFFFF;
    EGLConfig bestConfig{};

    for (std::size_t i = 0; i < static_cast<std::size_t>(configCount); ++i)
    {
        // Check mandatory attributes
        int surfaceType    = 0;
        int renderableType = 0;
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_SURFACE_TYPE, &surfaceType));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_RENDERABLE_TYPE, &renderableType));
        if (!(surfaceType & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) || !(renderableType & EGL_OPENGL_ES3_BIT))
            continue;

        // Extract the components of the current config
        int red           = 0;
        int green         = 0;
        int blue          = 0;
        int alpha         = 0;
        int depth         = 0;
        int stencil       = 0;
        int multiSampling = 0;
        int samples       = 0;
        int caveat        = 0;
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &red));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &green));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &blue));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_ALPHA_SIZE, &alpha));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depth));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &stencil));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_SAMPLE_BUFFERS, &multiSampling));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_SAMPLES, &samples));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_CONFIG_CAVEAT, &caveat));

        // Evaluate the config
        const int color = red + green + blue + alpha;
        const int score = GlContext::evaluateFormat(bitsPerPixel,
                                                    contextSettings,
                                                    color,
                                                    depth,
                                                    stencil,
                                                    multiSampling ? samples : 0,
                                                    caveat == EGL_NONE,
                                                    false);

        // If it's better than the current best, make it the new best
        if (score < bestScore)
        {
            bestScore  = score;
            bestConfig = configs[i];
        }
    }

    SFML_BASE_ASSERT(bestScore < 0x7FFFFFFF && "Failed to calculate best config");

    return bestConfig;
#else
    // Set our video contextSettings constraint
    const EGLint attributes[] =
        {EGL_BUFFER_SIZE,
         static_cast<EGLint>(bitsPerPixel),
         EGL_RED_SIZE,
         8,
         EGL_BLUE_SIZE,
         8,
         EGL_GREEN_SIZE,
         8,
         EGL_DEPTH_SIZE,
         static_cast<EGLint>(contextSettings.depthBits),
         EGL_STENCIL_SIZE,
         static_cast<EGLint>(contextSettings.stencilBits),
         EGL_SAMPLE_BUFFERS,
         static_cast<EGLint>(contextSettings.antiAliasingLevel),
         EGL_SURFACE_TYPE,
         EGL_RENDERABLE_TYPE,
         EGL_OPENGL_ES_BIT,
         EGL_NONE,
         EGL_NONE};

    EGLint    configCount;
    EGLConfig config;

    // Ask EGL for the best config matching our video contextSettings
    eglCheck(eglChooseConfig(display, attributes, &config, 1, &configCount));

    if (configCount == 0)
        err() << "Failed to get any EGL frame buffer configurations";

    return config;
#endif
}


////////////////////////////////////////////////////////////
void EglContext::updateSettings()
{
    m_settings.majorVersion      = 1u;
    m_settings.minorVersion      = 1u;
    m_settings.attributeFlags    = ContextSettings::Attribute::Default;
    m_settings.depthBits         = 0u;
    m_settings.stencilBits       = 0u;
    m_settings.antiAliasingLevel = 0u;

    EGLBoolean result = EGL_FALSE;
    EGLint     tmp    = 0;

    // Update the internal context contextSettings with the current config
    eglCheck(result = eglGetConfigAttrib(m_display, m_config, EGL_DEPTH_SIZE, &tmp));

    if (result != EGL_FALSE)
        m_settings.depthBits = static_cast<unsigned int>(tmp);

    eglCheck(result = eglGetConfigAttrib(m_display, m_config, EGL_STENCIL_SIZE, &tmp));

    if (result != EGL_FALSE)
        m_settings.stencilBits = static_cast<unsigned int>(tmp);

    eglCheck(result = eglGetConfigAttrib(m_display, m_config, EGL_SAMPLE_BUFFERS, &tmp));

    if ((result != EGL_FALSE) && tmp)
    {
        eglCheck(result = eglGetConfigAttrib(m_display, m_config, EGL_SAMPLES, &tmp));

        if (result != EGL_FALSE)
            m_settings.antiAliasingLevel = static_cast<unsigned int>(tmp);
    }
}


#if defined(SFML_SYSTEM_LINUX) && !defined(SFML_USE_DRM)
////////////////////////////////////////////////////////////
XVisualInfo EglContext::selectBestVisual(::Display* xDisplay, unsigned int bitsPerPixel, const ContextSettings& contextSettings)
{
    EglContextImpl::ensureInit();

    // Get the initialized EGL display
    EGLDisplay display = EglContextImpl::getInitializedDisplay();

    // Get the best EGL config matching the default video contextSettings
    EGLConfig config = getBestConfig(display, bitsPerPixel, contextSettings);

    // Retrieve the visual id associated with this EGL config
    EGLint nativeVisualId = 0;

    eglCheck(eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &nativeVisualId));

    if (nativeVisualId == 0)
    {
        // Should never happen...
        priv::err() << "No EGL visual found. You should check your graphics driver";

        return {};
    }

    XVisualInfo vTemplate;
    vTemplate.visualid = static_cast<VisualID>(nativeVisualId);

    // Get X11 visuals compatible with this EGL config
    int visualCount = 0;
    const auto availableVisuals = X11Ptr<XVisualInfo[]>(XGetVisualInfo(xDisplay, VisualIDMask, &vTemplate, &visualCount));

    if (visualCount == 0)
    {
        // Can't happen...
        priv::err() << "No X11 visual found. Bug in your EGL implementation ?";

        return {};
    }

    // Pick up the best one
    return availableVisuals.get()[0];
}
#endif

} // namespace sf::priv
