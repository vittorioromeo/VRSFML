////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Assert.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/UniquePtr.hpp>

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
    glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion);

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
    glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions);

    if (numExtensions == 0)
        return result; // Empty vector

    for (int i = 0; i < numExtensions; ++i)
        if (const auto* extensionString = reinterpret_cast<const char*>(
                glGetStringiFunc(GL_EXTENSIONS, static_cast<unsigned int>(i))))
            result.emplace_back(extensionString);

    return result;
}

////////////////////////////////////////////////////////////
constinit thread_local struct
{
    std::uint64_t        id{0u};
    sf::priv::GlContext* ptr{nullptr};
} activeGlContext;

} // namespace


////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    std::atomic<std::uint64_t> nextThreadLocalGlContextId{2u}; // 1 is reserved for shared context

    DerivedGlContextType sharedGlContext; //!< The hidden, inactive context that will be shared with all other contexts
    std::recursive_mutex sharedGlContextMutex;

    std::vector<std::string> extensions; //!< Supported OpenGL extensions

    ////////////////////////////////////////////////////////////
    std::mutex unsharedObjectsMutex;

    struct UnsharedGlObject
    {
        std::uint64_t glContextId{};
        unsigned int  frameBufferId;
    };

    std::vector<UnsharedGlObject> unsharedObjects;

    template <typename... SharedGlContextArgs>
    explicit Impl(SharedGlContextArgs&&... args) : sharedGlContext(SFML_FORWARD(args)...)
    {
    }
};


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext() : m_impl(priv::makeUnique<Impl>(*this, 1u, nullptr))
{
    SFML_ASSERT(!hasAnyActiveGlContext());

    if (!setActiveThreadLocalGlContext(m_impl->sharedGlContext, true))
        priv::err() << "Could not enable shared context in GraphicsContext()";

    SFML_ASSERT(isActiveGlContextSharedContext());

    if (!m_impl->sharedGlContext.initialize(m_impl->sharedGlContext, ContextSettings{}))
        priv::err() << "Could not initialize shared context in GraphicsContext()";

    m_impl->extensions = loadExtensions(m_impl->sharedGlContext);

    if (!setActiveThreadLocalGlContext(m_impl->sharedGlContext, false))
        priv::err() << "Could not disable shared context in GraphicsContext()";

    SFML_ASSERT(!hasAnyActiveGlContext());

    if (!setActiveThreadLocalGlContext(m_impl->sharedGlContext, true))
    {
        priv::err() << "Failed to enable shared GL context in `GraphicsContext::GraphicsContext`";
        SFML_ASSERT(false);
    }

    priv::ensureExtensionsInit(*this);
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext()
{
    SFML_ASSERT(hasAnyActiveGlContext());

    activeGlContext.id  = 0u;
    activeGlContext.ptr = nullptr;

    SFML_ASSERT(!hasAnyActiveGlContext());
}


////////////////////////////////////////////////////////////
void GraphicsContext::registerUnsharedGlObject(std::uint64_t glContextId, unsigned int frameBufferId)
{
    const std::lock_guard lock(m_impl->unsharedObjectsMutex);

    SFML_ASSERT(getActiveThreadLocalGlContextId() == glContextId);
    m_impl->unsharedObjects.emplace_back(glContextId, frameBufferId);
}


////////////////////////////////////////////////////////////
void GraphicsContext::unregisterUnsharedGlObject(std::uint64_t glContextId, unsigned int frameBufferId)
{
    const std::lock_guard lock(m_impl->unsharedObjectsMutex);

    SFML_ASSERT(getActiveThreadLocalGlContextId() == glContextId);

    // Find the object in unshared objects and remove it if its associated context is currently active
    // This will trigger the destructor of the object since shared_ptr in unshared objects should be the
    // only one existing
    const auto iter = priv::findIf(m_impl->unsharedObjects.begin(),
                                   m_impl->unsharedObjects.end(),
                                   [&](const Impl::UnsharedGlObject& obj)
                                   { return obj.glContextId == glContextId && obj.frameBufferId == frameBufferId; });

    if (iter != m_impl->unsharedObjects.end())
        m_impl->unsharedObjects.erase(iter);
}


////////////////////////////////////////////////////////////
void GraphicsContext::cleanupUnsharedResources(priv::GlContext& glContext)
{
    // Scope for lock guard
    {
        const std::lock_guard lock(m_impl->unsharedObjectsMutex);

        if (!priv::anyOf(m_impl->unsharedObjects.begin(),
                         m_impl->unsharedObjects.end(),
                         [&](const Impl::UnsharedGlObject& obj) { return obj.glContextId == activeGlContext.id; }))
        {
            return;
        }
    }

    SFML_ASSERT(false); // TODO: if this fails, need to delete framebuffer here

    // Save the current context so we can restore it later
    priv::GlContext* contextToRestore = activeGlContext.ptr;

    // Destroy the unshared objects contained in this context
    for (auto iter = m_impl->unsharedObjects.begin(); iter != m_impl->unsharedObjects.end();)
    {
        if (iter->glContextId == glContext.m_id)
        {
            SFML_ASSERT(false); // TODO: if this fails, need to delete framebuffer here
            iter = m_impl->unsharedObjects.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    // Make this context active so resources can be freed
    if (!setActiveThreadLocalGlContext(glContext, true))
        priv::err() << "Could not enable GL context in GlContext::cleanupUnsharedResources()";

    // Scope for lock guard
    {
        const std::lock_guard lock(m_impl->unsharedObjectsMutex);

        // Destroy the unshared objects contained in this context
        for (auto iter = m_impl->unsharedObjects.begin(); iter != m_impl->unsharedObjects.end();)
        {
            if (iter->glContextId == glContext.m_id)
            {
                SFML_ASSERT(false); // TODO: if this fails, need to delete framebuffer here
                iter = m_impl->unsharedObjects.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    // Make the originally active context active again
    if (contextToRestore != nullptr)
    {
        if (!setActiveThreadLocalGlContext(*contextToRestore, true))
            priv::err() << "Could not restore context in GlContext::cleanupUnsharedResources()";
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
    return activeGlContext.ptr;
}


////////////////////////////////////////////////////////////
std::uint64_t GraphicsContext::getActiveThreadLocalGlContextId() const
{
    return activeGlContext.id;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::hasActiveThreadLocalGlContext() const
{
    return activeGlContext.id != 0;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active)
{
    // If `glContext` is already the active one on this thread, don't do anything
    if (active && glContext.m_id == activeGlContext.id)
    {
        SFML_ASSERT(activeGlContext.ptr == &glContext);
        return true;
    }

    // If `glContext` is not the active one on this thread, don't do anything
    if (!active && glContext.m_id != activeGlContext.id)
    {
        SFML_ASSERT(activeGlContext.ptr != &glContext);
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
void GraphicsContext::onGlContextDestroyed(priv::GlContext& glContext)
{
    // If `glContext` is not the active one on this thread, don't do anything
    if (glContext.m_id != activeGlContext.id)
        return;

    if (!setActiveThreadLocalGlContext(m_impl->sharedGlContext, true))
    {
        priv::err() << "Failed to enable shared GL context in `GraphicsContext::onGlContextDestroyed`";
        SFML_ASSERT(false);
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool GraphicsContext::hasAnyActiveGlContext() const
{
    priv::err() << "pizza" << 1 << 2 << 3 << "banana";

    return activeGlContext.id != 0u && activeGlContext.ptr != nullptr;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool GraphicsContext::isActiveGlContextSharedContext() const
{
    return activeGlContext.id == 1u && activeGlContext.ptr == &m_impl->sharedGlContext;
}


////////////////////////////////////////////////////////////
template <typename... GLContextArgs>
[[nodiscard]] priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContextImpl(const ContextSettings& contextSettings,
                                                                                    GLContextArgs&&... args)
{
    // TODO: ?
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

    if (!setActiveThreadLocalGlContext(m_impl->sharedGlContext, true))
        priv::err() << "Error enabling shared GL context in GraphicsContext::createGlContext()";

    auto glContext = priv::makeUnique<DerivedGlContextType>(*this,
                                                            m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                            &m_impl->sharedGlContext,
                                                            SFML_FORWARD(args)...);

    if (!setActiveThreadLocalGlContext(m_impl->sharedGlContext, false))
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
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext()
{
    return createGlContextImpl(ContextSettings{});
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings&  contextSettings,
                                                                  const priv::WindowImpl& owner,
                                                                  unsigned int            bitsPerPixel)
{
    return createGlContextImpl(contextSettings, contextSettings, owner, bitsPerPixel);
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings& contextSettings, const Vector2u& size)
{
    return createGlContextImpl(contextSettings, contextSettings, size);
}


////////////////////////////////////////////////////////////
bool GraphicsContext::isExtensionAvailable(const char* name) const
{
    return priv::find(m_impl->extensions.begin(), m_impl->extensions.end(), name) != m_impl->extensions.end();
}


////////////////////////////////////////////////////////////
GlFunctionPointer GraphicsContext::getFunction(const char* name) const
{
    return m_impl->sharedGlContext.getFunction(name);
}

} // namespace sf
