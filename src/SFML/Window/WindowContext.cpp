#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/GlContext.hpp"
#include "SFML/Window/GlContextTypeImpl.hpp"
#include "SFML/Window/Glad.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/SensorManager.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include <csignal>


namespace sf
{
namespace
{
////////////////////////////////////////////////////////////
/// \brief Load our extensions vector with the supported extensions
///
////////////////////////////////////////////////////////////
[[nodiscard]] std::vector<std::string> loadExtensions(DerivedGlContextType& glContext)
{
    std::vector<std::string> result; // Use a single local variable for NRVO

    auto glGetErrorFunc    = reinterpret_cast<glGetErrorFuncType>(glContext.getFunction("glGetError"));
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(glContext.getFunction("glGetIntegerv"));
    auto glGetStringFunc   = reinterpret_cast<glGetStringFuncType>(glContext.getFunction("glGetString"));

    if (!glGetErrorFunc || !glGetIntegervFunc || !glGetStringFunc)
        return result; // Empty vector

    // Check whether a >= 3.0 context is available
    int majorVersion = 0;
    glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion); // intentionally not checked, will be checked below

    auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(glContext.getFunction("glGetStringi"));

    // Try to load the >= 3.0 way
    int numExtensions = 0;
    glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions));

    if (numExtensions == 0)
        return result; // Empty vector

    for (int i = 0; i < numExtensions; ++i)
        if (const auto* extensionString = reinterpret_cast<const char*>(
                glCheckIgnoreWithFunc(glGetErrorFunc, glGetStringiFunc(GL_EXTENSIONS, static_cast<unsigned int>(i)))))
            result.emplace_back(extensionString);

    return result;
}

////////////////////////////////////////////////////////////
thread_local constinit struct
{
    unsigned int         id{0u};
    sf::priv::GlContext* ptr{nullptr};
} activeGlContext;


////////////////////////////////////////////////////////////
constinit WindowContext* installedWindowContext{nullptr};


////////////////////////////////////////////////////////////
[[maybe_unused]] void GLAPIENTRY debugGLMessageCallback(
    GLenum       source,
    GLenum       type,
    unsigned int id,
    GLenum       severity,
    GLsizei /* length */,
    const char* message,
    const void* /* userParam */)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    auto& multiLineErr = priv::err(true /* multiLine */);

    multiLineErr << "---------------" << '\n' << "Debug message (" << id << "): " << message << "\nSource: ";

    // clang-format off
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             multiLineErr << "API";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   multiLineErr << "Window System";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: multiLineErr << "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     multiLineErr << "Third Party";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     multiLineErr << "Application";     break;
        case GL_DEBUG_SOURCE_OTHER:           multiLineErr << "Other";           break;
    }
    // clang-format on

    multiLineErr << "\nType: ";

    // clang-format off
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               multiLineErr << "Error";                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: multiLineErr << "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  multiLineErr << "Undefined Behaviour";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         multiLineErr << "Portability";          break;
        case GL_DEBUG_TYPE_PERFORMANCE:         multiLineErr << "Performance";          break;
        case GL_DEBUG_TYPE_MARKER:              multiLineErr << "Marker";               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          multiLineErr << "Push Group";           break;
        case GL_DEBUG_TYPE_POP_GROUP:           multiLineErr << "Pop Group";            break;
        case GL_DEBUG_TYPE_OTHER:               multiLineErr << "Other";                break;
    }
    // clang-format on

    multiLineErr << "\nSeverity: ";

    // clang-format off
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         multiLineErr << "High";         break;
        case GL_DEBUG_SEVERITY_MEDIUM:       multiLineErr << "Medium";       break;
        case GL_DEBUG_SEVERITY_LOW:          multiLineErr << "Low";          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: multiLineErr << "Notification"; break;
    }
    // clang-format on

    priv::err() << '\n';
}

} // namespace


////////////////////////////////////////////////////////////
struct WindowContext::Impl
{
    ////////////////////////////////////////////////////////////
    struct ClearInstalledGuard
    {
        ~ClearInstalledGuard()
        {
            SFML_BASE_ASSERT(installedWindowContext != nullptr);
            installedWindowContext = nullptr;
        }
    };

    ////////////////////////////////////////////////////////////
    ClearInstalledGuard clearInstalledGuard;

    ////////////////////////////////////////////////////////////
    std::atomic<unsigned int> nextThreadLocalGlContextId{2u}; // 1 is reserved for shared context

    ////////////////////////////////////////////////////////////
    DerivedGlContextType sharedGlContext; //!< The hidden, inactive context that will be shared with all other contexts
    std::recursive_mutex sharedGlContextMutex;

    ////////////////////////////////////////////////////////////
    std::vector<std::string> extensions; //!< Supported OpenGL extensions

    ////////////////////////////////////////////////////////////
    struct UnsharedFrameBuffer
    {
        unsigned int     glContextId{};
        unsigned int     frameBufferId;
        UnsharedDeleteFn deleteFn;
    };

    std::mutex                       unsharedFrameBuffersMutex;
    std::vector<UnsharedFrameBuffer> unsharedFrameBuffers;

    ////////////////////////////////////////////////////////////
    base::Optional<priv::JoystickManager> joystickManager;
    base::Optional<priv::SensorManager>   sensorManager;

    ////////////////////////////////////////////////////////////
    template <typename... SharedGlContextArgs>
    explicit Impl(SharedGlContextArgs&&... args) : sharedGlContext(SFML_BASE_FORWARD(args)...)
    {
    }
};


////////////////////////////////////////////////////////////
WindowContext::WindowContext() : m_impl(base::makeUnique<Impl>(/* id */ 1u, /* shared */ nullptr))
{
    // Install window context:
    if (installedWindowContext != nullptr)
    {
        priv::err() << "Fatal error creating `sf::WindowContext`: a `sf::WindowContext` object already exists";
        base::abort();
    }

    installedWindowContext = this;

    // Define fatal signal handlers for the user that will display a stack trace:
    std::signal(SIGSEGV, [](int) { priv::err() << "FATAL SIGNAL: SIGSEGV"; });
    std::signal(SIGILL, [](int) { priv::err() << "FATAL SIGNAL: SIGILL"; });
    std::signal(SIGABRT, [](int) { priv::err() << "FATAL SIGNAL: SIGABRT"; });
    std::signal(SIGFPE, [](int) { priv::err() << "FATAL SIGNAL: SIGFPE"; });

    SFML_BASE_ASSERT(!hasActiveThreadLocalOrSharedGlContext());

    if (!setActiveThreadLocalGlContextToSharedContext(true))
        priv::err() << "Could not enable shared context in WindowContext()";

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    // TODO P1: propagate and use factory?
    if (!m_impl->sharedGlContext.initialize(m_impl->sharedGlContext, ContextSettings{}))
        priv::err() << "Could not initialize shared context in WindowContext()";

    m_impl->extensions = loadExtensions(m_impl->sharedGlContext);

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    // Load OpenGL or OpenGL ES entry points using glad
    loadGLEntryPointsViaGLAD();

#ifndef SFML_SYSTEM_EMSCRIPTEN
    // TODO P0: maybe conditionally enable depending on graphicscontext's debug ctx param?
    // or for emscripten, try to enable without glcheck and then drain gl errors
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    glCheck(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    glCheck(glDebugMessageCallback(debugGLMessageCallback, nullptr));

    // Retrieve the context version number
    const auto majorVersion = priv::getGLInteger(GL_MAJOR_VERSION);
    const auto minorVersion = priv::getGLInteger(GL_MINOR_VERSION);

    if ((majorVersion < 1) || ((majorVersion == 1) && (minorVersion < 1)))
    {
        priv::err() << "sfml-graphics requires support for OpenGL 1.1 or greater" << '\n'
                    << "Ensure that hardware acceleration is enabled if available";
    }
#else
    // Need to drain errors here or subsequent assertion will fail on Emscripten, unsure why
    while (glGetError() != GL_NO_ERROR)
        ;
#endif

    SFML_BASE_ASSERT(glGetError() == GL_NO_ERROR);
}


////////////////////////////////////////////////////////////
WindowContext::~WindowContext()
{
    SFML_BASE_ASSERT(m_impl->unsharedFrameBuffers.empty());

    SFML_BASE_ASSERT(hasActiveThreadLocalOrSharedGlContext());

    activeGlContext.id  = 0u;
    activeGlContext.ptr = nullptr;

    SFML_BASE_ASSERT(!hasActiveThreadLocalOrSharedGlContext());
}


////////////////////////////////////////////////////////////
void WindowContext::registerUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId, UnsharedDeleteFn deleteFn)
{
    SFML_BASE_ASSERT(getActiveThreadLocalGlContextId() == glContextId);

    const std::lock_guard lock(m_impl->unsharedFrameBuffersMutex);
    m_impl->unsharedFrameBuffers.emplace_back(glContextId, frameBufferId, deleteFn);
}


////////////////////////////////////////////////////////////
void WindowContext::unregisterUnsharedFrameBuffer(unsigned int glContextId, unsigned int frameBufferId)
{
    // If we're not on the right context, wait for the cleanup later on
    if (getActiveThreadLocalGlContextId() != glContextId)
        return;

    const std::lock_guard lock(m_impl->unsharedFrameBuffersMutex);

    // Find the object in unshared objects and remove it if its associated context is currently active
    // Assume that the object has already been deleted with the right OpenGL delete call
    const auto iter = base::findIf(m_impl->unsharedFrameBuffers.begin(),
                                   m_impl->unsharedFrameBuffers.end(),
                                   [&](const Impl::UnsharedFrameBuffer& obj)
                                   { return obj.glContextId == glContextId && obj.frameBufferId == frameBufferId; });

    if (iter != m_impl->unsharedFrameBuffers.end())
    {
        iter->deleteFn(iter->frameBufferId);
        m_impl->unsharedFrameBuffers.erase(iter);
    }
}


////////////////////////////////////////////////////////////
void WindowContext::cleanupUnsharedFrameBuffers(priv::GlContext& glContext)
{
    // Save the current context so we can restore it later
    priv::GlContext* glContextToRestore = activeGlContext.ptr;

    // Make this context active so resources can be freed
    if (!setActiveThreadLocalGlContext(glContext, true))
        priv::err() << "Could not enable GL context in GlContext::cleanupUnsharedFrameBuffers()";

    // Scope for lock guard
    {
        const std::lock_guard lock(m_impl->unsharedFrameBuffersMutex);

        // Destroy the unshared objects contained in this context
        for (auto iter = m_impl->unsharedFrameBuffers.begin(); iter != m_impl->unsharedFrameBuffers.end();)
        {
            if (iter->glContextId == glContext.m_id)
            {
                iter->deleteFn(iter->frameBufferId);
                iter = m_impl->unsharedFrameBuffers.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    // Make the originally active context active again
    if (glContextToRestore != nullptr)
    {
        if (!setActiveThreadLocalGlContext(*glContextToRestore, true))
            priv::err() << "Could not restore context in GlContext::cleanupUnsharedFrameBuffers()";
    }
    else
    {
        activeGlContext.id  = 0u;
        activeGlContext.ptr = nullptr;
    }
}


////////////////////////////////////////////////////////////
const priv::GlContext* WindowContext::getActiveThreadLocalGlContextPtr() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);
    return activeGlContext.ptr;
}


////////////////////////////////////////////////////////////
priv::JoystickManager& WindowContext::getJoystickManager()
{
    return const_cast<priv::JoystickManager&>(static_cast<const WindowContext&>(*this).getJoystickManager());
}


////////////////////////////////////////////////////////////
const priv::JoystickManager& WindowContext::getJoystickManager() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);

    if (!m_impl->joystickManager.hasValue())
        m_impl->joystickManager.emplace();

    return *m_impl->joystickManager;
}


////////////////////////////////////////////////////////////
priv::SensorManager& WindowContext::getSensorManager()
{
    return const_cast<priv::SensorManager&>(static_cast<const WindowContext&>(*this).getSensorManager());
}


////////////////////////////////////////////////////////////
const priv::SensorManager& WindowContext::getSensorManager() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);

    if (!m_impl->sensorManager.hasValue())
        m_impl->sensorManager.emplace();

    return *m_impl->sensorManager;
}


////////////////////////////////////////////////////////////
unsigned int WindowContext::getActiveThreadLocalGlContextId() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);
    return activeGlContext.id;
}


////////////////////////////////////////////////////////////
bool WindowContext::hasActiveThreadLocalGlContext() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);
    return activeGlContext.id != 0;
}


////////////////////////////////////////////////////////////
bool WindowContext::setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active)
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);

    // If `glContext` is already the active one on this thread, don't do anything
    if (active && glContext.m_id == activeGlContext.id)
    {
        SFML_BASE_ASSERT(activeGlContext.ptr == &glContext);
        return true;
    }

    // If `glContext` is not the active one on this thread, don't do anything
    if (!active && glContext.m_id != activeGlContext.id)
    {
        SFML_BASE_ASSERT(activeGlContext.ptr != &glContext);
        return true;
    }

    // Activate/deactivate the context
    if (!glContext.makeCurrent(active))
    {
        priv::err() << "`glContext.makeCurrent` failure in `WindowContext::setActiveThreadLocalGlContext`";
        return false;
    }

    activeGlContext.id  = active ? glContext.m_id : 0u;
    activeGlContext.ptr = active ? &glContext : nullptr;

    return true;
}


////////////////////////////////////////////////////////////
bool WindowContext::setActiveThreadLocalGlContextToSharedContext(bool active)
{
    return setActiveThreadLocalGlContext(m_impl->sharedGlContext, active);
}


////////////////////////////////////////////////////////////
void WindowContext::onGlContextDestroyed(priv::GlContext& glContext)
{
    // If `glContext` is not the active one on this thread, don't do anything
    if (glContext.m_id != activeGlContext.id)
        return;

    if (!setActiveThreadLocalGlContextToSharedContext(true))
    {
        priv::err() << "Failed to enable shared GL context in `WindowContext::onGlContextDestroyed`";
        SFML_BASE_ASSERT(false);
    }
}


////////////////////////////////////////////////////////////
bool WindowContext::hasActiveThreadLocalOrSharedGlContext() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);
    return activeGlContext.id != 0u && activeGlContext.ptr != nullptr;
}


////////////////////////////////////////////////////////////
bool WindowContext::isActiveGlContextSharedContext() const
{
    SFML_BASE_ASSERT(installedWindowContext != nullptr);
    return activeGlContext.id == 1u && activeGlContext.ptr == &m_impl->sharedGlContext;
}


////////////////////////////////////////////////////////////
WindowContext* WindowContext::getInstalled()
{
    return installedWindowContext;
}


////////////////////////////////////////////////////////////
WindowContext& WindowContext::ensureInstalled()
{
    if (installedWindowContext == nullptr) [[unlikely]]
    {
        priv::err() << "`sf::WindowContext` not installed -- did you forget to create one in `main`?";
        base::abort();
    }

    return *installedWindowContext;
}


////////////////////////////////////////////////////////////
void WindowContext::loadGLEntryPointsViaGLAD() const
{
#ifdef SFML_OPENGL_ES
    gladLoadGLES2(getGLLoadFn());
#else
    gladLoadGL(getGLLoadFn());
#endif
}


////////////////////////////////////////////////////////////
WindowContext::GLLoadFn WindowContext::getGLLoadFn() const
{
    static const sf::WindowContext* lastWindowContext;
    lastWindowContext = this;

    return [](const char* name)
    {
        SFML_BASE_ASSERT(installedWindowContext != nullptr);
        return lastWindowContext->getFunction(name);
    };
}


////////////////////////////////////////////////////////////
template <typename... GLContextArgs>
base::UniquePtr<priv::GlContext> WindowContext::createGlContextImpl(const ContextSettings& contextSettings,
                                                                    GLContextArgs&&... args)
{
    const std::lock_guard lock(m_impl->sharedGlContextMutex);

    if (!setActiveThreadLocalGlContextToSharedContext(true))
        priv::err() << "Error enabling shared GL context in WindowContext::createGlContext()";

    auto glContext = base::makeUnique<DerivedGlContextType>(m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                            &m_impl->sharedGlContext,
                                                            SFML_BASE_FORWARD(args)...);

    if (!setActiveThreadLocalGlContextToSharedContext(false))
        priv::err() << "Error disabling shared GL context in WindowContext::createGlContext()";

    if (!setActiveThreadLocalGlContext(*glContext, true))
    {
        priv::err() << "Error enabling newly created GL context in GlContext::initialize()";
        return nullptr;
    }

    if (!glContext->initialize(m_impl->sharedGlContext, contextSettings))
    {
        priv::err() << "Error initializing newly created GL context in WindowContext::createGlContext()";
        return nullptr;
    }

    glContext->checkSettings(contextSettings);
    return glContext;
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> WindowContext::createGlContext()
{
    return createGlContextImpl(ContextSettings{});
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> WindowContext::createGlContext(const ContextSettings&  contextSettings,
                                                                const priv::WindowImpl& owner,
                                                                unsigned int            bitsPerPixel)
{
    return createGlContextImpl(contextSettings, contextSettings, owner, bitsPerPixel);
}


////////////////////////////////////////////////////////////
bool WindowContext::isExtensionAvailable(const char* name) const
{
    return base::find(m_impl->extensions.begin(), m_impl->extensions.end(), name) != m_impl->extensions.end();
}


////////////////////////////////////////////////////////////
GlFunctionPointer WindowContext::getFunction(const char* name) const
{
    return m_impl->sharedGlContext.getFunction(name);
}

} // namespace sf
