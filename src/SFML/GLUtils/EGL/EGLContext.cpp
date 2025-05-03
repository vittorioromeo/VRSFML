#include <EGL/egl.h>
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/SDLWindowImpl.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/EGL/EGLCheck.hpp"
#include "SFML/GLUtils/EGL/EGLContext.hpp"
#include "SFML/GLUtils/EGL/EGLGlad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Vector.hpp"


#ifdef SFML_SYSTEM_ANDROID
    #include "SFML/System/Android/Activity.hpp"

    #include <mutex>
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

    static EGLDisplay display = []
    {
        const EGLDisplay result = eglCheck(eglGetDisplay(EGL_DEFAULT_DISPLAY));
        eglCheck(eglInitialize(result, nullptr, nullptr));
        return result;
    }();

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

////////////////////////////////////////////////////////////
int evaluateFormat(unsigned int               bitsPerPixel,
                   const sf::ContextSettings& contextSettings,
                   int                        colorBits,
                   int                        depthBits,
                   int                        stencilBits,
                   int                        antiAliasing,
                   bool                       accelerated,
                   bool                       sRgb)
{
    // Weight sub-scores so that better contextSettings don't score equally as bad as worse contextSettings
    const auto adjustNegativeScore = [](int x) { return static_cast<unsigned int>(x * (x > 0 ? 100'000 : -1)); };

    const auto colorDiff   = adjustNegativeScore(static_cast<int>(bitsPerPixel) - colorBits);
    const auto depthDiff   = adjustNegativeScore(static_cast<int>(contextSettings.depthBits) - depthBits);
    const auto stencilDiff = adjustNegativeScore(static_cast<int>(contextSettings.stencilBits) - stencilBits);
    const auto antiAliasingDiff = adjustNegativeScore(static_cast<int>(contextSettings.antiAliasingLevel) - antiAliasing);

    // Aggregate the scores
    return static_cast<int>(
        colorDiff + depthDiff + stencilDiff + antiAliasingDiff +
        // If the user wants an sRGB capable format, try really hard to get one
        ((contextSettings.sRgbCapable && !sRgb) ? 10'000'000 : 0) +
        // Make sure we prefer hardware acceleration over features
        (!accelerated ? 100'000'000 : 0));
}


////////////////////////////////////////////////////////////
EGLConfig getBestConfig(EGLDisplay display, unsigned int bitsPerPixel, const sf::ContextSettings& contextSettings)
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    EglContextImpl::ensureInit();

    // Determine the number of available configs
    EGLint configCount = 0;
    if (const auto rc = eglCheck(eglGetConfigs(display, nullptr, 0, &configCount)); rc == EGL_FALSE)
        sf::priv::err() << "Failed to get EGL configs (1st call)";

    // Retrieve the list of available configs
    sf::base::Vector<EGLConfig> configs(static_cast<sf::base::SizeT>(configCount));

    if (const auto rc = eglCheck(eglGetConfigs(display, configs.data(), configCount, &configCount)); rc == EGL_FALSE)
        sf::priv::err() << "Failed to get EGL configs (2nd call)";

    // Evaluate all the returned configs, and pick the best one
    int       bestScore = 0x7F'FF'FF'FF;
    EGLConfig bestConfig{};

    for (sf::base::SizeT i = 0; i < static_cast<sf::base::SizeT>(configCount); ++i)
    {
        // Check mandatory attributes
        int surfaceType    = 0;
        int renderableType = 0;
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_SURFACE_TYPE, &surfaceType));
        eglCheck(eglGetConfigAttrib(display, configs[i], EGL_RENDERABLE_TYPE, &renderableType));

        // The following check doesn't pass on Emscripten
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
        const int score = evaluateFormat(bitsPerPixel,
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

    SFML_BASE_ASSERT(bestScore < 0x7F'FF'FF'FF && "Failed to calculate best config");

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
        sf::priv::err() << "Failed to get any EGL frame buffer configurations";

    return config;
#endif
}

} // namespace EglContextImpl
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct EglContext::Impl
{
    EGLDisplay display{EGL_NO_DISPLAY}; //!< The internal EGL display
    EGLContext context{EGL_NO_CONTEXT}; //!< The internal EGL context
    EGLSurface surface{EGL_NO_SURFACE}; //!< The internal EGL surface
    EGLConfig  config{};                //!< The internal EGL config
};


////////////////////////////////////////////////////////////
EglContext::EglContext(unsigned int id, EglContext* shared, const ContextSettings& contextSettings) :
GlContext(id, contextSettings)
{
    EglContextImpl::ensureInit();

    // Get the initialized EGL display
    m_impl->display = EglContextImpl::getInitializedDisplay();

    // Get the best EGL config matching the given contextSettings
    m_impl->config = EglContextImpl::getBestConfig(m_impl->display, VideoModeUtils::getDesktopMode().bitsPerPixel, contextSettings);
    updateSettings();

#ifndef SFML_SYSTEM_EMSCRIPTEN
    // Note: The EGL specs say that attribList can be a null pointer when passed to eglCreatePbufferSurface,
    // but this is resulting in a segfault. Bug in Android?
    constexpr EGLint attribList[]{EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};

    m_impl->surface = eglCheck(eglCreatePbufferSurface(m_impl->display, m_impl->config, attribList));
    SFML_BASE_ASSERT(m_impl->surface != EGL_NO_SURFACE);
#else
    EGLNativeWindowType dummyWindow{};
    createSurface(&dummyWindow);
#endif

    // Create EGL context
    createContext(shared);
}


////////////////////////////////////////////////////////////
EglContext::EglContext(unsigned int                          id,
                       EglContext*                           shared,
                       const ContextSettings&                contextSettings,
                       [[maybe_unused]] const SDLWindowImpl& owner,
                       unsigned int                          bitsPerPixel) :
GlContext(id, contextSettings)
{
    EglContextImpl::ensureInit();

#ifdef SFML_SYSTEM_ANDROID

    // On Android, we must save the created context
    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);

    states.context = this;

#endif

    // Get the initialized EGL display
    m_impl->display = EglContextImpl::getInitializedDisplay();

    // Get the best EGL config matching the requested video context settings
    m_impl->config = EglContextImpl::getBestConfig(m_impl->display, bitsPerPixel, contextSettings);
    updateSettings();

    // Create EGL context
    createContext(shared);

#if !defined(SFML_SYSTEM_ANDROID)

    // Create EGL surface (except on Android because the window is created
    // asynchronously, its activity manager will call it for us)
    WindowHandle nativeHandle = owner.getNativeHandle();
    createSurface(static_cast<void*>(&nativeHandle));

#endif
}


////////////////////////////////////////////////////////////
EglContext::~EglContext()
{
    // Notify unshared OpenGL resources of context destruction
    WindowContext::cleanupUnsharedFrameBuffers(*this);

    // Deactivate the current context
    const EGLContext currentContext = eglCheck(eglGetCurrentContext());

    if (currentContext == m_impl->context)
        eglCheck(eglMakeCurrent(m_impl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

// Destroying the context on Emscripten seem to make every subsequent GL call fail,
// even if switching to another context...
#ifndef SFML_SYSTEM_EMSCRIPTEN
    if (m_impl->context != EGL_NO_CONTEXT)
        eglCheck(eglDestroyContext(m_impl->display, m_impl->context));
#endif

    if (m_impl->surface != EGL_NO_SURFACE)
        eglCheck(eglDestroySurface(m_impl->display, m_impl->surface));
}


////////////////////////////////////////////////////////////
GlFunctionPointer EglContext::getFunction(const char* name) const
{
    EglContextImpl::ensureInit();

    return eglGetProcAddress(name);
}


////////////////////////////////////////////////////////////
bool EglContext::makeCurrent(const bool activate)
{
    if (m_impl->surface == EGL_NO_SURFACE)
        return false;

    return activate
               ? eglCheck(eglMakeCurrent(m_impl->display, m_impl->surface, m_impl->surface, m_impl->context)) != EGL_FALSE
               : eglCheck(eglMakeCurrent(m_impl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) != EGL_FALSE;
}


////////////////////////////////////////////////////////////
void EglContext::display()
{
    if (m_impl->surface != EGL_NO_SURFACE)
        eglCheck(eglSwapBuffers(m_impl->display, m_impl->surface));
}


////////////////////////////////////////////////////////////
void EglContext::setVerticalSyncEnabled([[maybe_unused]] bool enabled)
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    eglCheck(eglSwapInterval(m_impl->display, enabled));
#endif
}


////////////////////////////////////////////////////////////
void EglContext::createContext(EglContext* shared)
{
#ifndef SFML_SYSTEM_EMSCRIPTEN
    constexpr EGLint contextAttribs[]{EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE};
#else
    constexpr EGLint contextAttribs[]{EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
#endif

    const EGLContext toShared = shared != nullptr ? shared->m_impl->context : EGL_NO_CONTEXT;

    if (toShared != EGL_NO_CONTEXT)
        eglCheck(eglMakeCurrent(m_impl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

    // Create EGL context
    m_impl->context = eglCheck(eglCreateContext(m_impl->display, m_impl->config, toShared, contextAttribs));
    SFML_BASE_ASSERT(m_impl->context != EGL_NO_CONTEXT);
}


////////////////////////////////////////////////////////////
void EglContext::createSurface(void* windowPtr)
{
    m_impl->surface = eglCheck(
        eglCreateWindowSurface(m_impl->display, m_impl->config, *static_cast<EGLNativeWindowType*>(windowPtr), nullptr));

    SFML_BASE_ASSERT(m_impl->surface != EGL_NO_SURFACE);
}


////////////////////////////////////////////////////////////
void EglContext::destroySurface()
{
    // Seems to only be called by `SDLWindowImplAndroid`

    if (!WindowContext::setActiveThreadLocalGlContext(*this, false))
        err() << "Failure to disable EGL context in `EglContext::destroySurface`";

    eglCheck(eglDestroySurface(m_impl->display, m_impl->surface));
    m_impl->surface = EGL_NO_SURFACE;
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

    EGLint tmp = 0;

    // Update the internal context contextSettings with the current config
    // Update the internal context settings with the current config
    if (eglCheck(eglGetConfigAttrib(m_impl->display, m_impl->config, EGL_DEPTH_SIZE, &tmp)) != EGL_FALSE)
        m_settings.depthBits = static_cast<unsigned int>(tmp);

    if (eglCheck(eglGetConfigAttrib(m_impl->display, m_impl->config, EGL_STENCIL_SIZE, &tmp)) != EGL_FALSE)
        m_settings.stencilBits = static_cast<unsigned int>(tmp);

    if (eglCheck(eglGetConfigAttrib(m_impl->display, m_impl->config, EGL_SAMPLE_BUFFERS, &tmp)) != EGL_FALSE && tmp &&
        eglCheck(eglGetConfigAttrib(m_impl->display, m_impl->config, EGL_SAMPLES, &tmp)) != EGL_FALSE)
        m_settings.antiAliasingLevel = static_cast<unsigned int>(tmp);
}

} // namespace sf::priv
