#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/WindowContext.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <glad/gl.h>

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include <cstdint>


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

    if (glGetErrorFunc() == GL_INVALID_ENUM || !majorVersion || !glGetStringiFunc)
    {
        // Try to load the < 3.0 way
        const auto* extensionString = reinterpret_cast<const char*>(glGetStringFunc(GL_EXTENSIONS));

        if (extensionString == nullptr)
            return result; // Empty vector

        do
        {
            const char* extension = extensionString;

            while (*extensionString && (*extensionString != ' '))
                ++extensionString;

            result.emplace_back(extension, extensionString);
        } while (*extensionString++);

        return result;
    }

    // Try to load the >= 3.0 way
    int numExtensions = 0;
    glCheckIgnoreWithFunc(glGetErrorFunc, glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions));

    if (numExtensions == 0)
        return result; // Empty vector

    for (int i = 0; i < numExtensions; ++i)
        if (const auto* extensionString = reinterpret_cast<const char*>(
                glCheckIgnoreExprWithFunc(glGetErrorFunc, glGetStringiFunc(GL_EXTENSIONS, static_cast<unsigned int>(i)))))
            result.emplace_back(extensionString);

    return result;
}

////////////////////////////////////////////////////////////
thread_local constinit struct
{
    std::uint64_t        id{0u};
    sf::priv::GlContext* ptr{nullptr};
} activeGlContext;


////////////////////////////////////////////////////////////
constinit bool windowContextAlive{false};


////////////////////////////////////////////////////////////
void extensionSanityCheck()
{
    static const auto check = [](int& flag, auto... entryPoints)
    {
        // If a required entry point is missing, flag the whole extension as unavailable
        if (!(entryPoints && ...))
            flag = 0;
    };
#ifdef SFML_OPENGL_ES
    check(GLEXT_multitexture_dependencies);
    check(GLEXT_vertex_buffer_object_dependencies);
    check(GLEXT_EXT_blend_minmax_dependencies);
#else
    check(GLEXT_blend_minmax_dependencies);
    check(GLEXT_multitexture_dependencies);
    check(GLEXT_blend_func_separate_dependencies);
    check(GLEXT_vertex_buffer_object_dependencies);
    check(GLEXT_shader_objects_dependencies);
    check(GLEXT_blend_equation_separate_dependencies);
    check(GLEXT_framebuffer_object_dependencies);
    check(GLEXT_framebuffer_blit_dependencies);
    check(GLEXT_framebuffer_multisample_dependencies);
    check(GLEXT_copy_buffer_dependencies);
#endif
}


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

    priv::err(true /* multiLine */) << "---------------" << '\n' << "Debug message (" << id << "): " << message << '\n';

    // clang-format off
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             priv::err(true /* multiLine */) << "Source: API";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   priv::err(true /* multiLine */) << "Source: Window System";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: priv::err(true /* multiLine */) << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     priv::err(true /* multiLine */) << "Source: Third Party";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     priv::err(true /* multiLine */) << "Source: Application";     break;
        case GL_DEBUG_SOURCE_OTHER:           priv::err(true /* multiLine */) << "Source: Other";           break;
    }
    // clang-format on

    priv::err(true /* multiLine */) << '\n';

    // clang-format off
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               priv::err(true /* multiLine */) << "Type: Error";                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: priv::err(true /* multiLine */) << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  priv::err(true /* multiLine */) << "Type: Undefined Behaviour";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         priv::err(true /* multiLine */) << "Type: Portability";          break;
        case GL_DEBUG_TYPE_PERFORMANCE:         priv::err(true /* multiLine */) << "Type: Performance";          break;
        case GL_DEBUG_TYPE_MARKER:              priv::err(true /* multiLine */) << "Type: Marker";               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          priv::err(true /* multiLine */) << "Type: Push Group";           break;
        case GL_DEBUG_TYPE_POP_GROUP:           priv::err(true /* multiLine */) << "Type: Pop Group";            break;
        case GL_DEBUG_TYPE_OTHER:               priv::err(true /* multiLine */) << "Type: Other";                break;
    }
    // clang-format on

    priv::err(true /* multiLine */) << '\n';

    // clang-format off
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         priv::err(true /* multiLine */) << "Severity: high";         break;
        case GL_DEBUG_SEVERITY_MEDIUM:       priv::err(true /* multiLine */) << "Severity: medium";       break;
        case GL_DEBUG_SEVERITY_LOW:          priv::err(true /* multiLine */) << "Severity: low";          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: priv::err(true /* multiLine */) << "Severity: notification"; break;
    }
    // clang-format on

    priv::err() << '\n';
}

} // namespace


////////////////////////////////////////////////////////////
struct WindowContext::Impl
{
    ////////////////////////////////////////////////////////////
    struct ClearAliveFlag
    {
        ~ClearAliveFlag()
        {
            SFML_BASE_ASSERT(windowContextAlive);
            windowContextAlive = false;
        }
    };

    ////////////////////////////////////////////////////////////
    ClearAliveFlag clearAliveFlag;

    ////////////////////////////////////////////////////////////
    std::atomic<std::uint64_t> nextThreadLocalGlContextId{2u}; // 1 is reserved for shared context

    ////////////////////////////////////////////////////////////
    DerivedGlContextType sharedGlContext; //!< The hidden, inactive context that will be shared with all other contexts
    std::recursive_mutex sharedGlContextMutex;

    ////////////////////////////////////////////////////////////
    std::vector<std::string> extensions; //!< Supported OpenGL extensions

    ////////////////////////////////////////////////////////////
    struct UnsharedFrameBuffer
    {
        std::uint64_t    glContextId{};
        unsigned int     frameBufferId;
        UnsharedDeleteFn deleteFn;
    };

    std::mutex                       unsharedFrameBuffersMutex;
    std::vector<UnsharedFrameBuffer> unsharedFrameBuffers;

    ////////////////////////////////////////////////////////////
    template <typename... SharedGlContextArgs>
    explicit Impl(SharedGlContextArgs&&... args) : sharedGlContext(SFML_BASE_FORWARD(args)...)
    {
        SFML_BASE_ASSERT(!windowContextAlive &&
                         "An `sf::WindowContext` object is already alive, only one can exist at a time");

        windowContextAlive = true;
    }
};


////////////////////////////////////////////////////////////
WindowContext::WindowContext() : m_impl(base::makeUnique<Impl>(*this, /* id */ 1u, /* shared */ nullptr))
{
    SFML_BASE_ASSERT(!hasActiveThreadLocalOrSharedGlContext());

    if (!setActiveThreadLocalGlContextToSharedContext(true))
        priv::err() << "Could not enable shared context in WindowContext()";

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    if (!m_impl->sharedGlContext.initialize(m_impl->sharedGlContext, ContextSettings{}))
        priv::err() << "Could not initialize shared context in WindowContext()";

    m_impl->extensions = loadExtensions(m_impl->sharedGlContext);

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    ensureExtensionsInit();

    // Need to drain errors here or subsequent assertion will fail on Emscripten, unsure why
    while (glGetError() != GL_NO_ERROR)
        ;

    SFML_BASE_ASSERT(glGetError() == GL_NO_ERROR);
}


////////////////////////////////////////////////////////////
void WindowContext::ensureExtensionsInit() const
{
    SFML_BASE_ASSERT(hasActiveThreadLocalOrSharedGlContext());

    static bool initialized = false;
    if (initialized)
        return;

    initialized = true;

    // Load OpenGL or OpenGL ES entry points using glad
    loadGLEntryPointsViaGLAD();

    // Some GL implementations don't fully follow extension specifications
    // and advertise support for extensions although not providing the
    // entry points specified for the corresponding extension.
    // In order to protect ourselves from such implementations, we perform
    // a sanity check to ensure an extension is _really_ supported, even
    // from an entry point perspective.
    extensionSanityCheck();

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
#endif
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
void WindowContext::registerUnsharedFrameBuffer(std::uint64_t glContextId, unsigned int frameBufferId, UnsharedDeleteFn deleteFn)
{
    SFML_BASE_ASSERT(getActiveThreadLocalGlContextId() == glContextId);

    const std::lock_guard lock(m_impl->unsharedFrameBuffersMutex);
    m_impl->unsharedFrameBuffers.emplace_back(glContextId, frameBufferId, deleteFn);
}


////////////////////////////////////////////////////////////
void WindowContext::unregisterUnsharedFrameBuffer(std::uint64_t glContextId, unsigned int frameBufferId)
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
    SFML_BASE_ASSERT(windowContextAlive);
    return activeGlContext.ptr;
}


////////////////////////////////////////////////////////////
std::uint64_t WindowContext::getActiveThreadLocalGlContextId() const
{
    SFML_BASE_ASSERT(windowContextAlive);
    return activeGlContext.id;
}


////////////////////////////////////////////////////////////
bool WindowContext::hasActiveThreadLocalGlContext() const
{
    SFML_BASE_ASSERT(windowContextAlive);
    return activeGlContext.id != 0;
}


////////////////////////////////////////////////////////////
bool WindowContext::setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active)
{
    SFML_BASE_ASSERT(windowContextAlive);

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
[[nodiscard]] bool WindowContext::hasActiveThreadLocalOrSharedGlContext() const
{
    SFML_BASE_ASSERT(windowContextAlive);
    return activeGlContext.id != 0u && activeGlContext.ptr != nullptr;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool WindowContext::isActiveGlContextSharedContext() const
{
    SFML_BASE_ASSERT(windowContextAlive);
    return activeGlContext.id == 1u && activeGlContext.ptr == &m_impl->sharedGlContext;
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
[[nodiscard]] WindowContext::GLLoadFn WindowContext::getGLLoadFn() const
{
    static const sf::WindowContext* lastWindowContext;
    lastWindowContext = this;

    return [](const char* name)
    {
        SFML_BASE_ASSERT(windowContextAlive);
        return lastWindowContext->getFunction(name);
    };
}


////////////////////////////////////////////////////////////
template <typename... GLContextArgs>
[[nodiscard]] base::UniquePtr<priv::GlContext> WindowContext::createGlContextImpl(const ContextSettings& contextSettings,
                                                                                  GLContextArgs&&... args)
{
    // TODO P0: maybe graphicscontext should take a contextsetttings for teh shared context??
    // If use_count is 2 (GlResource + sharedContext) we know that we are inside sf::Context or sf::Window
    // Only in this situation we allow the user to indirectly re-create the shared context as a core context

    // // Check if we need to convert our shared context into a core context
    // if ((SharedContext::getUseCount() == 2) && (contextSettings.attributeFlags & ContextSettings::Attribute::Core) &&
    //     !(sharedGlContext.m_settings.attributeFlags & ContextSettings::Attribute::Core))
    // {
    //     // Re-create our shared context as a core context
    //     const ContextSettings sharedSettings{.depthBits = 0,
    //                                          .stencilBits = 0,
    //                                          .antialiasingLevel = 0,
    //                                          .majorVersion = contextSettings.majorVersion,
    //                                          .minorVersion = contextSettings.minorVersion,
    //                                          .attributeFlags = contextSettings.attributeFlags};

    //     sharedGlContext.emplace(nullptr, sharedSettings, Vector2u{1, 1});
    //     if (!sharedGlContext.initialize(sharedSettings))
    //     {
    //        priv::err() << "Could not initialize shared context in WindowContext::createGlContext()";
    //         return nullptr;
    //     }

    //     // Reload our extensions vector
    //     sharedContext->loadExtensions();
    // }

    const std::lock_guard lock(m_impl->sharedGlContextMutex);

    if (!setActiveThreadLocalGlContextToSharedContext(true))
        priv::err() << "Error enabling shared GL context in WindowContext::createGlContext()";

    auto glContext = base::makeUnique<DerivedGlContextType>(*this,
                                                            m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                            &m_impl->sharedGlContext,
                                                            SFML_BASE_FORWARD(args)...);

    if (!setActiveThreadLocalGlContextToSharedContext(false))
        priv::err() << "Error disabling shared GL context in WindowContext::createGlContext()";

    if (!setActiveThreadLocalGlContext(*glContext, true))
    {
        priv::err() << "Error enabling newly created GL context in GlContext::initalize()";
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
base::UniquePtr<priv::GlContext> WindowContext::createGlContext(const ContextSettings& contextSettings, const Vector2u& size)
{
    return createGlContextImpl(contextSettings, contextSettings, size);
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
