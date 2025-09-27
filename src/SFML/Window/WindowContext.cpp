// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/SDLLayer.hpp" // TODO P0:
#include "SFML/Window/SensorManager.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLContextSaver.hpp"
#include "SFML/GLUtils/GLDebugCallback.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/GlContext.hpp"
#include "SFML/GLUtils/GlContextTypeImpl.hpp"
#include "SFML/GLUtils/GlFuncTypesImpl.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/SignalErrHandler.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

#include <atomic>
#include <mutex>


namespace sf
{
namespace
{
////////////////////////////////////////////////////////////
/// \brief Load our extensions vector with the supported extensions
///
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Vector<sf::base::StringView> loadExtensions(priv::DerivedGlContextType& glContext)
{
    sf::base::Vector<sf::base::StringView> result; // Use a single local variable for NRVO

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
            result.emplaceBack(extensionString);

    return result;
}


////////////////////////////////////////////////////////////
thread_local constinit struct
{
    unsigned int     id{0u};
    priv::GlContext* ptr{nullptr};
} activeGlContext;


////////////////////////////////////////////////////////////
struct UnsharedContextResources
{
    ////////////////////////////////////////////////////////////
    ankerl::unordered_dense::set<unsigned int> frameBufferIds;
    ankerl::unordered_dense::set<unsigned int> vaoIds;

    ////////////////////////////////////////////////////////////
    void clear()
    {
        frameBufferIds.clear();
        vaoIds.clear();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool allEmpty() const
    {
        return frameBufferIds.empty() && vaoIds.empty();
    }
};


////////////////////////////////////////////////////////////
class UnsharedContextResourcesManager
{
private:
    mutable std::mutex                                                   m_mutex;
    ankerl::unordered_dense::map<unsigned int, UnsharedContextResources> m_mapping;

    ////////////////////////////////////////////////////////////
    void registerImpl(auto idsPmr, const unsigned int glContextId, const unsigned int id)
    {
        std::lock_guard lock(m_mutex);

        auto& resources = m_mapping[glContextId];
        auto& idsSet    = (resources.*idsPmr);

        if (idsSet.contains(id))
            return;

        idsSet.emplace(id);
    }

    ////////////////////////////////////////////////////////////
    void unregisterImpl(auto idsPmr, const unsigned int glContextId, const unsigned int id)
    {
        std::lock_guard lock(m_mutex);

        auto& resources = m_mapping[glContextId];
        auto& idsSet    = (resources.*idsPmr);

        if (!idsSet.contains(id))
            return;

        glCheck(glDeleteFramebuffers(1, &id));
        idsSet.erase(id);
    }

public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool allEmpty() const
    {
        std::lock_guard lock(m_mutex);

        for (const auto& [glContextId, resources] : m_mapping)
            if (!resources.allEmpty())
                return false;

        return true;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool allNonSharedEmpty() const
    {
        std::lock_guard lock(m_mutex);

        for (const auto& [glContextId, resources] : m_mapping)
            if (glContextId != 1u && !resources.allEmpty())
                return true;

        return false;
    }

    ////////////////////////////////////////////////////////////
    void unregisterAllResources(const unsigned int glContextId)
    {
        std::lock_guard lock(m_mutex);

        auto& resources = m_mapping[glContextId];
        if (resources.allEmpty())
            return;

        for (const unsigned int frameBufferId : resources.frameBufferIds)
            glCheck(glDeleteFramebuffers(1, &frameBufferId));

        for (const unsigned int vaoId : resources.vaoIds)
            glCheck(glDeleteVertexArrays(1, &vaoId));

        resources.clear();
    }

    ////////////////////////////////////////////////////////////
    void registerFrameBuffer(const unsigned int glContextId, const unsigned int frameBufferId)
    {
        registerImpl(&UnsharedContextResources::frameBufferIds, glContextId, frameBufferId);
    }

    ////////////////////////////////////////////////////////////
    void unregisterFrameBuffer(const unsigned int glContextId, const unsigned int frameBufferId)
    {
        unregisterImpl(&UnsharedContextResources::frameBufferIds, glContextId, frameBufferId);
    }

    ////////////////////////////////////////////////////////////
    void registerVAO(const unsigned int glContextId, const unsigned int vaoId)
    {
        registerImpl(&UnsharedContextResources::vaoIds, glContextId, vaoId);
    }

    ////////////////////////////////////////////////////////////
    void unregisterVAO(const unsigned int glContextId, const unsigned int vaoId)
    {
        unregisterImpl(&UnsharedContextResources::vaoIds, glContextId, vaoId);
    }
};

} // namespace

////////////////////////////////////////////////////////////
struct WindowContextImpl
{
    ////////////////////////////////////////////////////////////
    std::atomic<unsigned int> nextThreadLocalGlContextId{2u}; // 1 is reserved for shared context

    ////////////////////////////////////////////////////////////
    UnsharedContextResourcesManager unsharedContextResourcesManager;

    ////////////////////////////////////////////////////////////
    priv::DerivedGlContextType sharedGlContext; //!< The hidden, inactive context that will be shared with all other contexts
    std::recursive_mutex sharedGlContextMutex;

    ////////////////////////////////////////////////////////////
    base::Vector<sf::base::StringView> extensions; //!< Supported OpenGL extensions

    ////////////////////////////////////////////////////////////
    base::Optional<priv::JoystickManager> joystickManager;
    base::Optional<priv::SensorManager>   sensorManager;

    ////////////////////////////////////////////////////////////
    template <typename... SharedGlContextArgs>
    explicit WindowContextImpl(SharedGlContextArgs&&... args) : sharedGlContext(SFML_BASE_FORWARD(args)...)
    {
    }
};

namespace
{
////////////////////////////////////////////////////////////
constinit base::Optional<WindowContextImpl> installedWindowContext;
constinit std::atomic<unsigned int>         windowContextRC{0u};


////////////////////////////////////////////////////////////
WindowContextImpl& ensureInstalled()
{
    if (!installedWindowContext.hasValue()) [[unlikely]]
    {
        priv::err() << "`sf::WindowContext` not installed -- did you forget to create one in `main`?";
        base::abort();
    }

    return *installedWindowContext;
}

} // namespace

////////////////////////////////////////////////////////////
base::Optional<WindowContext> WindowContext::create()
{
    priv::getSDLLayerSingleton(); // TODO P0:

    const auto fail = [](const char* what)
    {
        priv::err() << "Error creating `sf::WindowContext`: " << what;
        return base::nullOpt;
    };

    //
    // Ensure window context is not already installed
    if (installedWindowContext.hasValue())
        return fail("a `sf::WindowContext` object already exists");

    //
    // Shared context settings
    const ContextSettings sharedContextSettings{};

    //
    // Install window context
    auto& wc = installedWindowContext.emplace(/* id */ 1u, /* shared */ nullptr, sharedContextSettings);

    //
    // Define fatal signal handlers for the user that will display a stack trace
    priv::installSignalErrHandler();

    //
    // Enable shader GL context
    SFML_BASE_ASSERT(!hasActiveThreadLocalGlContext());

    if (!setActiveThreadLocalGlContextToSharedContext())
        return fail("could not enable shared context");

    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    //
    // Try to initialize shared GL context
    if (!wc.sharedGlContext.initialize(wc.sharedGlContext, sharedContextSettings))
        return fail("could not initialize shared context");

    //
    // Load extensions and entrypoints
    SFML_BASE_ASSERT(isActiveGlContextSharedContext());
    wc.extensions = loadExtensions(wc.sharedGlContext);
    loadGLEntryPointsViaGLAD();

#ifndef SFML_SYSTEM_EMSCRIPTEN
    if (sharedContextSettings.isDebug())
        priv::setupGLDebugCallback();

    // Retrieve the context version number
    const auto majorVersion = priv::getGLInteger(GL_MAJOR_VERSION);
    const auto minorVersion = priv::getGLInteger(GL_MINOR_VERSION);

    if ((majorVersion < 1) || ((majorVersion == 1) && (minorVersion < 1)))
        return fail("support for OpenGL 1.1 or greater required, ensure hardware acceleration is enabled");
#else
    // Need to drain errors here or subsequent assertion will fail on Emscripten, unsure why
    while (glGetError() != GL_NO_ERROR)
        ;
#endif

    SFML_BASE_ASSERT(glGetError() == GL_NO_ERROR);

    return base::makeOptional<WindowContext>(base::PassKey<WindowContext>{});
}


////////////////////////////////////////////////////////////
WindowContext::WindowContext(base::PassKey<WindowContext>&&)
{
    windowContextRC.fetch_add(1u, std::memory_order::relaxed);
}


////////////////////////////////////////////////////////////
WindowContext::WindowContext(base::PassKey<GraphicsContext>&&) : WindowContext(base::PassKey<WindowContext>{})
{
}


////////////////////////////////////////////////////////////
WindowContext::WindowContext(WindowContext&&) noexcept : WindowContext(base::PassKey<WindowContext>{})
{
}


////////////////////////////////////////////////////////////
WindowContext::~WindowContext()
{
    if (windowContextRC.fetch_sub(1u, std::memory_order::relaxed) > 1u)
        return;

    SFML_BASE_ASSERT(!ensureInstalled().unsharedContextResourcesManager.allNonSharedEmpty());

    // All the FBOs on shared context should be destroyed later

    SFML_BASE_ASSERT(hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    disableSharedGlContext();
    SFML_BASE_ASSERT(!hasActiveThreadLocalGlContext());

    installedWindowContext.reset();
}


////////////////////////////////////////////////////////////
void WindowContext::registerUnsharedFrameBuffer(const unsigned int glContextId, const unsigned int frameBufferId)
{
    auto& wc = ensureInstalled();

    SFML_BASE_ASSERT(getActiveThreadLocalGlContextId() == glContextId);
    wc.unsharedContextResourcesManager.registerFrameBuffer(glContextId, frameBufferId);
}


////////////////////////////////////////////////////////////
void WindowContext::unregisterUnsharedFrameBuffer(const unsigned int glContextId, const unsigned int frameBufferId)
{
    auto& wc = ensureInstalled();

    // If we're not on the right context, wait for the cleanup later on
    if (getActiveThreadLocalGlContextId() != glContextId)
        return;

    wc.unsharedContextResourcesManager.unregisterFrameBuffer(glContextId, frameBufferId);
}


////////////////////////////////////////////////////////////
void WindowContext::registerUnsharedVAO(const unsigned int glContextId, const unsigned int vaoId)
{
    auto& wc = ensureInstalled();

    SFML_BASE_ASSERT(getActiveThreadLocalGlContextId() == glContextId);
    wc.unsharedContextResourcesManager.registerVAO(glContextId, vaoId);
}


////////////////////////////////////////////////////////////
void WindowContext::unregisterUnsharedVAO(const unsigned int glContextId, const unsigned int vaoId)
{
    auto& wc = ensureInstalled();

    // If we're not on the right context, wait for the cleanup later on
    if (getActiveThreadLocalGlContextId() != glContextId)
        return;

    wc.unsharedContextResourcesManager.unregisterVAO(glContextId, vaoId);
}


////////////////////////////////////////////////////////////
void WindowContext::cleanupUnsharedFrameBuffers(priv::GlContext& glContext)
{
    auto& wc = ensureInstalled();

    if (&glContext == &wc.sharedGlContext)
    {
        if (!setActiveThreadLocalGlContext(glContext, true))
            priv::err() << "Could not enable GL context in GlContext::cleanupUnsharedFrameBuffers()";

        wc.unsharedContextResourcesManager.unregisterAllResources(glContext.getId());
        SFML_BASE_ASSERT(wc.unsharedContextResourcesManager.allEmpty());

        disableSharedGlContext();
        return;
    }

    // Save the current context so we can restore it later
    priv::GLContextSaver glContextSaver;

    // Make this context active so resources can be freed
    if (!setActiveThreadLocalGlContext(glContext, true))
        priv::err() << "Could not enable GL context in GlContext::cleanupUnsharedFrameBuffers()";

    wc.unsharedContextResourcesManager.unregisterAllResources(glContext.getId());
}


////////////////////////////////////////////////////////////
priv::GlContext* WindowContext::getActiveThreadLocalGlContextPtr()
{
    ensureInstalled();
    return activeGlContext.ptr;
}


////////////////////////////////////////////////////////////
priv::GlContext& WindowContext::getSharedGlContext()
{
    return ensureInstalled().sharedGlContext;
}


////////////////////////////////////////////////////////////
priv::JoystickManager& WindowContext::getJoystickManager()
{
    return ensureInstalled().joystickManager.emplaceIfNeeded();
}


////////////////////////////////////////////////////////////
priv::SensorManager& WindowContext::getSensorManager()
{
    return ensureInstalled().sensorManager.emplaceIfNeeded();
}


////////////////////////////////////////////////////////////
unsigned int WindowContext::getActiveThreadLocalGlContextId()
{
    ensureInstalled();
    return activeGlContext.id;
}


////////////////////////////////////////////////////////////
bool WindowContext::hasActiveThreadLocalGlContext()
{
    ensureInstalled();
    return activeGlContext.id != 0u && activeGlContext.ptr != nullptr;
}


////////////////////////////////////////////////////////////
bool WindowContext::setActiveThreadLocalGlContext(priv::GlContext& glContext, const bool active)
{
    auto& wc = ensureInstalled();

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

    if (&glContext == &wc.sharedGlContext)
    {
        SFML_BASE_ASSERT(active);

        activeGlContext.id  = glContext.m_id;
        activeGlContext.ptr = &glContext;
    }
    else
    {
        // Revert to shared context if `glContext` is disabled

        activeGlContext.id  = active ? glContext.m_id : 1u;
        activeGlContext.ptr = active ? &glContext : &wc.sharedGlContext;
    }

    return true;
}


////////////////////////////////////////////////////////////
bool WindowContext::setActiveThreadLocalGlContextToSharedContext()
{
    auto& wc = ensureInstalled();
    return setActiveThreadLocalGlContext(wc.sharedGlContext, true);
}


////////////////////////////////////////////////////////////
void WindowContext::onGlContextDestroyed(priv::GlContext& glContext)
{
    ensureInstalled();

    // If `glContext` is not the active one on this thread, don't do anything
    if (glContext.m_id != activeGlContext.id)
        return;

    if (!setActiveThreadLocalGlContextToSharedContext())
    {
        priv::err() << "Failed to enable shared GL context in `WindowContext::onGlContextDestroyed`";
        SFML_BASE_ASSERT(false);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool WindowContext::isSharedContext(priv::GlContext& glContext)
{
    auto& wc = ensureInstalled();
    return &glContext == &wc.sharedGlContext;
}


////////////////////////////////////////////////////////////
bool WindowContext::isActiveGlContextSharedContext()
{
    auto& wc = ensureInstalled();
    return activeGlContext.id == 1u && activeGlContext.ptr == &wc.sharedGlContext;
}


////////////////////////////////////////////////////////////
void WindowContext::disableSharedGlContext()
{
    auto& wc = ensureInstalled();

    SFML_BASE_ASSERT(hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(isActiveGlContextSharedContext());

    if (!wc.sharedGlContext.makeCurrent(false))
    {
        priv::err() << "Could not disable shared GL context in `WindowContext::disableSharedGlContext()`";
        return;
    }

    activeGlContext.id  = 0u;
    activeGlContext.ptr = nullptr;
}


////////////////////////////////////////////////////////////
bool WindowContext::isInstalled()
{
    return installedWindowContext.hasValue();
}


////////////////////////////////////////////////////////////
void WindowContext::loadGLEntryPointsViaGLAD()
{
#ifdef SFML_OPENGL_ES
    gladLoadGLES2(&getFunction);
#else
    gladLoadGL(&getFunction);
#endif
}


////////////////////////////////////////////////////////////
template <typename... GLContextArgs>
base::UniquePtr<priv::GlContext> WindowContext::createGlContextImpl(const ContextSettings& contextSettings,
                                                                    GLContextArgs&&... args)
{
    auto& wc = ensureInstalled();

    const std::lock_guard lock(wc.sharedGlContextMutex);

    if (!setActiveThreadLocalGlContextToSharedContext())
        priv::err() << "Error enabling shared GL context in WindowContext::createGlContext()";

    auto glContext = base::makeUnique<priv::DerivedGlContextType>(wc.nextThreadLocalGlContextId.fetch_add(1u),
                                                                  &wc.sharedGlContext,
                                                                  contextSettings,
                                                                  SFML_BASE_FORWARD(args)...);

    if (!setActiveThreadLocalGlContext(*glContext, true))
    {
        priv::err() << "Error enabling newly created GL context in GlContext::initialize()";
        return nullptr;
    }

    if (!glContext->initialize(wc.sharedGlContext, contextSettings))
    {
        priv::err() << "Error initializing newly created GL context in WindowContext::createGlContext()";
        return nullptr;
    }

    if (contextSettings.isDebug())
        priv::setupGLDebugCallback();

    glContext->checkSettings(contextSettings);
    return glContext;
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> WindowContext::createGlContext(const ContextSettings& contextSettings)
{
    return createGlContextImpl(contextSettings);
}


////////////////////////////////////////////////////////////
base::UniquePtr<priv::GlContext> WindowContext::createGlContext(const ContextSettings&     contextSettings,
                                                                const priv::SDLWindowImpl& owner,
                                                                const unsigned int         bitsPerPixel)
{
    return createGlContextImpl(contextSettings, owner, bitsPerPixel);
}


////////////////////////////////////////////////////////////
bool WindowContext::isExtensionAvailable(const char* const name)
{
    auto& wc = ensureInstalled();
    return base::find(wc.extensions.begin(), wc.extensions.end(), name) != wc.extensions.end();
}


////////////////////////////////////////////////////////////
GlFunctionPointer WindowContext::getFunction(const char* const name)
{
    return ensureInstalled().sharedGlContext.getFunction(name);
}

} // namespace sf
