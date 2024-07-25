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
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <glad/gl.h>

#include <atomic>
#include <iostream>
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

// TODO P0: Repetition with glcontext.cpp
// These functions are expected to fail, but we don't want to pollute the state of `glGetError` so we have to call it anyway
#define SFML_PRIV_GLCHECK_INNER(...)                             \
    do                                                           \
    {                                                            \
        SFML_BASE_ASSERT(glGetErrorFunc() == GL_NO_ERROR);       \
                                                                 \
        __VA_ARGS__;                                             \
                                                                 \
        if (glGetErrorFunc() != GL_NO_ERROR)                     \
        {                                                        \
            /* err() << "Inner GL error: '" #__VA_ARGS__ "'"; */ \
        };                                                       \
                                                                 \
    } while (false)

#define SFML_PRIV_GLCHECK_INNER_EXPR(...)                        \
    [&]                                                          \
    {                                                            \
        SFML_BASE_ASSERT(glGetErrorFunc() == GL_NO_ERROR);       \
                                                                 \
        auto result = __VA_ARGS__;                               \
                                                                 \
        if (glGetErrorFunc() != GL_NO_ERROR)                     \
        {                                                        \
            /* err() << "Inner GL error: '" #__VA_ARGS__ "'"; */ \
        };                                                       \
                                                                 \
        return result;                                           \
    }()

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
    SFML_PRIV_GLCHECK_INNER(glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions));

    if (numExtensions == 0)
        return result; // Empty vector

    for (int i = 0; i < numExtensions; ++i)
        if (const auto* extensionString = reinterpret_cast<const char*>(
                SFML_PRIV_GLCHECK_INNER_EXPR(glGetStringiFunc(GL_EXTENSIONS, static_cast<unsigned int>(i)))))
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
constinit bool graphicsContextAlive{false};


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

    std::cout << "---------------" << '\n' << "Debug message (" << id << "): " << message << '\n';

    // clang-format off
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application";     break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other";           break;
    }
    // clang-format on

    std::cout << '\n';

    // clang-format off
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error";                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability";          break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance";          break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker";               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group";           break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group";            break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other";                break;
    }
    // clang-format on

    std::cout << '\n';

    // clang-format off
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high";         break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium";       break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low";          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    }
    // clang-format on

    std::cout << '\n' << std::endl;
}

} // namespace


////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    ////////////////////////////////////////////////////////////
    struct ClearAliveFlag
    {
        ~ClearAliveFlag()
        {
            SFML_BASE_ASSERT(graphicsContextAlive);
            graphicsContextAlive = false;
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
        SFML_BASE_ASSERT(!graphicsContextAlive &&
                         "An `sf::GraphicsContext` object is already alive, only one can exist at a time");

        graphicsContextAlive = true;
    }
};


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext() : m_impl(base::makeUnique<Impl>(*this, 1u /* id */, nullptr /* shared */))
{
    SFML_BASE_ASSERT(!hasActiveThreadLocalOrSharedGlContext());

    if (!setActiveThreadLocalGlContextToSharedContext(true))
        priv::err() << "Could not enable shared context in GraphicsContext()";

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    if (!m_impl->sharedGlContext.initialize(m_impl->sharedGlContext, ContextSettings{}))
        priv::err() << "Could not initialize shared context in GraphicsContext()";

    m_impl->extensions = loadExtensions(m_impl->sharedGlContext);

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    // TODO P0: better shader lifetime management
    // m_impl->builtInShader = createBuiltInShader(*this);
    // SFML_BASE_ASSERT(m_impl->builtInShader.hasValue());

    ensureExtensionsInit();

    // TODO P0: needed for emscripten shader example, why ?
    while (glGetError() != GL_NO_ERROR)
        ;

    SFML_BASE_ASSERT(glGetError() == GL_NO_ERROR);
}


////////////////////////////////////////////////////////////
void GraphicsContext::ensureExtensionsInit() const
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

// TODO P0:
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
GraphicsContext::~GraphicsContext()
{
    SFML_BASE_ASSERT(m_impl->unsharedFrameBuffers.empty());

    SFML_BASE_ASSERT(hasActiveThreadLocalOrSharedGlContext());

    SFML_BASE_ASSERT(builtInShaderState != 2);

    if (builtInShaderState == 1)
    {
        SFML_BASE_ASSERT(builtInShaderDestroyFn != nullptr);
        builtInShaderState = 2;

        builtInShaderDestroyFn();
    }

    activeGlContext.id  = 0u;
    activeGlContext.ptr = nullptr;

    SFML_BASE_ASSERT(!hasActiveThreadLocalOrSharedGlContext());
}


////////////////////////////////////////////////////////////
void GraphicsContext::registerUnsharedFrameBuffer(std::uint64_t glContextId, unsigned int frameBufferId, UnsharedDeleteFn deleteFn)
{
    SFML_BASE_ASSERT(getActiveThreadLocalGlContextId() == glContextId);

    const std::lock_guard lock(m_impl->unsharedFrameBuffersMutex);
    m_impl->unsharedFrameBuffers.emplace_back(glContextId, frameBufferId, deleteFn);
}


////////////////////////////////////////////////////////////
void GraphicsContext::unregisterUnsharedFrameBuffer(std::uint64_t glContextId, unsigned int frameBufferId)
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
void GraphicsContext::cleanupUnsharedFrameBuffers(priv::GlContext& glContext)
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
const priv::GlContext* GraphicsContext::getActiveThreadLocalGlContextPtr() const
{
    SFML_BASE_ASSERT(graphicsContextAlive);
    return activeGlContext.ptr;
}


////////////////////////////////////////////////////////////
std::uint64_t GraphicsContext::getActiveThreadLocalGlContextId() const
{
    SFML_BASE_ASSERT(graphicsContextAlive);
    return activeGlContext.id;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::hasActiveThreadLocalGlContext() const
{
    SFML_BASE_ASSERT(graphicsContextAlive);
    return activeGlContext.id != 0;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active)
{
    SFML_BASE_ASSERT(graphicsContextAlive);

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
        priv::err() << "`glContext.makeCurrent` failure in `GraphicsContext::setActiveThreadLocalGlContext`";
        return false;
    }

    activeGlContext.id  = active ? glContext.m_id : 0u;
    activeGlContext.ptr = active ? &glContext : nullptr;

    return true;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::setActiveThreadLocalGlContextToSharedContext(bool active)
{
    return setActiveThreadLocalGlContext(m_impl->sharedGlContext, active);
}


////////////////////////////////////////////////////////////
void GraphicsContext::onGlContextDestroyed(priv::GlContext& glContext)
{
    // If `glContext` is not the active one on this thread, don't do anything
    if (glContext.m_id != activeGlContext.id)
        return;

    if (!setActiveThreadLocalGlContextToSharedContext(true))
    {
        priv::err() << "Failed to enable shared GL context in `GraphicsContext::onGlContextDestroyed`";
        SFML_BASE_ASSERT(false);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool GraphicsContext::hasActiveThreadLocalOrSharedGlContext() const
{
    SFML_BASE_ASSERT(graphicsContextAlive);
    return activeGlContext.id != 0u && activeGlContext.ptr != nullptr;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool GraphicsContext::isActiveGlContextSharedContext() const
{
    SFML_BASE_ASSERT(graphicsContextAlive);
    return activeGlContext.id == 1u && activeGlContext.ptr == &m_impl->sharedGlContext;
}


////////////////////////////////////////////////////////////
void GraphicsContext::loadGLEntryPointsViaGLAD() const
{
#ifdef SFML_OPENGL_ES
    gladLoadGLES2(getGLLoadFn());
#else
    gladLoadGL(getGLLoadFn());
#endif
}


////////////////////////////////////////////////////////////
[[nodiscard]] GraphicsContext::GLLoadFn GraphicsContext::getGLLoadFn() const
{
    static const sf::GraphicsContext* lastGraphicsContext;
    lastGraphicsContext = this;

    return [](const char* name)
    {
        SFML_BASE_ASSERT(graphicsContextAlive);
        return lastGraphicsContext->getFunction(name);
    };
}


////////////////////////////////////////////////////////////
template <typename... GLContextArgs>
[[nodiscard]] base::UniquePtr<priv::GlContext> GraphicsContext::createGlContextImpl(const ContextSettings& contextSettings,
                                                                                    GLContextArgs&&... args)
{
    // TODO P0: maybe graphicscontext should take a contextsetttings for teh shared context??
    // If use_count is 2 (GlResource + sharedContext) we know that we are inside sf::Context or sf::Window
    // Only in this situation we allow the user to indirectly re-create the shared context as a core context

    // // Check if we need to convert our shared context into a core context
    // if ((SharedContext::getUseCount() == 2) && (settings.attributeFlags & ContextSettings::Attribute::Core) &&
    //     !(sharedGlContext.m_settings.attributeFlags & ContextSettings::Attribute::Core))
    // {
    //     // Re-create our shared context as a core context
    //     const ContextSettings sharedSettings{/* depthBits */ 0,
    //                                          /* stencilBits */ 0,
    //                                          /* antialiasingLevel */ 0,
    //                                          settings.majorVersion,
    //                                          settings.minorVersion,
    //                                          settings.attributeFlags};

    //     sharedGlContext.emplace(nullptr, sharedSettings, Vector2u{1, 1});
    //     if (!sharedGlContext.initialize(sharedSettings))
    //     {
    //        priv::err() << "Could not initialize shared context in GraphicsContext::createGlContext()";
    //         return nullptr;
    //     }

    //     // Reload our extensions vector
    //     sharedContext->loadExtensions();
    // }

    const std::lock_guard lock(m_impl->sharedGlContextMutex);

    if (!setActiveThreadLocalGlContextToSharedContext(true))
        priv::err() << "Error enabling shared GL context in GraphicsContext::createGlContext()";

    auto glContext = base::makeUnique<DerivedGlContextType>(*this,
                                                            m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                            &m_impl->sharedGlContext,
                                                            SFML_BASE_FORWARD(args)...);

    if (!setActiveThreadLocalGlContextToSharedContext(false))
        priv::err() << "Error disabling shared GL context in GraphicsContext::createGlContext()";

    if (!setActiveThreadLocalGlContext(*glContext, true))
    {
        priv::err() << "Error enabling newly created GL context in GlContext::initalize()";
        return nullptr;
    }

    if (!glContext->initialize(m_impl->sharedGlContext, contextSettings))
    {
        priv::err() << "Error initializing newly created GL context in GraphicsContext::createGlContext()";
        return nullptr;
    }

    glContext->checkSettings(contextSettings);
    return glContext;
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> GraphicsContext::createGlContext()
{
    return createGlContextImpl(ContextSettings{});
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings&  contextSettings,
                                                                  const priv::WindowImpl& owner,
                                                                  unsigned int            bitsPerPixel)
{
    return createGlContextImpl(contextSettings, contextSettings, owner, bitsPerPixel);
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings& contextSettings, const Vector2u& size)
{
    return createGlContextImpl(contextSettings, contextSettings, size);
}


////////////////////////////////////////////////////////////
bool GraphicsContext::isExtensionAvailable(const char* name) const
{
    return base::find(m_impl->extensions.begin(), m_impl->extensions.end(), name) != m_impl->extensions.end();
}


////////////////////////////////////////////////////////////
GlFunctionPointer GraphicsContext::getFunction(const char* name) const
{
    return m_impl->sharedGlContext.getFunction(name);
}

} // namespace sf
