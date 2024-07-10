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
#include <SFML/System/Err.hpp>
#include <SFML/System/Optional.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <glad/gl.h>

#include <atomic>
#include <memory>
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
[[nodiscard]] std::vector<std::string> loadExtensions()
{
    std::vector<std::string> result; // Use a single local variable for NRVO

    auto glGetErrorFunc    = reinterpret_cast<glGetErrorFuncType>(DerivedGlContextType::getFunction("glGetError"));
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(
        DerivedGlContextType::getFunction("glGetIntegerv"));
    auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(DerivedGlContextType::getFunction("glGetString"));

    if (!glGetErrorFunc || !glGetIntegervFunc || !glGetStringFunc)
        return result; // Empty vector

    // Check whether a >= 3.0 context is available
    int majorVersion = 0;
    glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion);

    auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(DerivedGlContextType::getFunction("glGetStringi"));

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
    std::uint64_t        id{};
    sf::priv::GlContext* ptr{};
    unsigned int         transientCount{};
} activeGlContext;

} // namespace


////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    explicit Impl(GraphicsContext& graphicsContext) :
    sharedGlContext(graphicsContext, nextThreadLocalGlContextId.fetch_add(1u), nullptr),
    extensions(
        [this]
        {
            if (!sharedGlContext.initialize(ContextSettings{}))
                priv::err() << "Could not initialize context in SharedContext()" << priv::errEndl;

            return loadExtensions();
        }())
    {
        if (!setActiveThreadLocalGlContext(sharedGlContext, false))
            priv::err() << "Could not disable context in SharedContext()" << priv::errEndl;
    }

    std::atomic<std::uint64_t> nextThreadLocalGlContextId{1u};

    DerivedGlContextType sharedGlContext; //!< The hidden, inactive context that will be shared with all other contexts
    std::recursive_mutex sharedContextMutex;

    const std::vector<std::string> extensions; //!< Supported OpenGL extensions

    ////////////////////////////////////////////////////////////
    std::mutex unsharedObjectsMutex;

    struct UnsharedGlObject
    {
        std::uint64_t         contextId{};
        std::shared_ptr<void> object;
    };

    std::vector<UnsharedGlObject> unsharedObjects;
};


////////////////////////////////////////////////////////////
void GraphicsContext::registerUnsharedGlObject(void* objectSharedPtr)
{
    SFML_ASSERT(activeGlContext.ptr != nullptr);

    const std::lock_guard lock(m_impl->unsharedObjectsMutex);

    m_impl->unsharedObjects.emplace_back(activeGlContext.id,
                                         SFML_MOVE(*static_cast<std::shared_ptr<void>*>(objectSharedPtr)));
}


////////////////////////////////////////////////////////////
void GraphicsContext::unregisterUnsharedGlObject(void* objectSharedPtr)
{
    SFML_ASSERT(activeGlContext.ptr != nullptr);

    const std::lock_guard lock(m_impl->unsharedObjectsMutex);

    // Find the object in unshared objects and remove it if its associated context is currently active
    // This will trigger the destructor of the object since shared_ptr
    // in unshared objects should be the only one existing
    const auto iter = priv::findIf(m_impl->unsharedObjects.begin(),
                                   m_impl->unsharedObjects.end(),
                                   [&](const Impl::UnsharedGlObject& obj) {
                                       return obj.contextId == activeGlContext.id &&
                                              obj.object == *static_cast<std::shared_ptr<void>*>(objectSharedPtr);
                                   });

    if (iter != m_impl->unsharedObjects.end())
        m_impl->unsharedObjects.erase(iter);
}


////////////////////////////////////////////////////////////
void GraphicsContext::cleanupUnsharedResources()
{
    if (activeGlContext.ptr == nullptr)
        return;

    // Save the current context so we can restore it later
    priv::GlContext* contextToRestore = activeGlContext.ptr;

    // If this context is already active there is no need to save it
    if (activeGlContext.ptr->m_id == activeGlContext.id)
        contextToRestore = nullptr;

    // Make this context active so resources can be freed
    if (!setActiveThreadLocalGlContext(*activeGlContext.ptr, true))
        priv::err() << "Could not enable context in GlContext::cleanupUnsharedResources()" << priv::errEndl;

    {
        const std::lock_guard lock(m_impl->unsharedObjectsMutex);

        // Destroy the unshared objects contained in this context
        for (auto iter = m_impl->unsharedObjects.begin(); iter != m_impl->unsharedObjects.end();)
        {
            if (iter->contextId == activeGlContext.ptr->m_id)
            {
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
        if (!setActiveThreadLocalGlContext(*contextToRestore, true))
            priv::err() << "Could not restore context in GlContext::cleanupUnsharedResources()" << priv::errEndl;
}


////////////////////////////////////////////////////////////
const priv::GlContext* GraphicsContext::getActiveThreadLocalGlContext()
{
    return activeGlContext.ptr;
}


////////////////////////////////////////////////////////////
std::uint64_t GraphicsContext::getActiveThreadLocalGlContextId()
{
    return activeGlContext.id;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::hasActiveThreadLocalGlContext()
{
    return activeGlContext.id != 0;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::setActiveThreadLocalGlContext(priv::GlContext& glContext, bool active)
{
    // If `glContext` is already the active one on this thread, don't do anything
    if (active && glContext.m_id == activeGlContext.id)
        return true;

    // If `glContext` is not the active one on this thread, don't do anything
    if (!active && glContext.m_id != activeGlContext.id)
        return true;

    // Activate/deactivate the context
    if (!glContext.makeCurrent(active))
    {
        priv::err() << "`glContext.makeCurrent` failure in `GraphicsContext::setActiveThreadLocalGlContext`"
                    << priv::errEndl;

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

    if (!setSharedContextAsActiveThreadLocalGlContext(true))
        priv::err() << "Failed to enable graphics context in GraphicsContext()" << priv::errEndl;
}


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext() : m_impl(priv::makeUnique<Impl>(*this))
{
    if (!setSharedContextAsActiveThreadLocalGlContext(true))
        priv::err() << "Failed to enable graphics context in GraphicsContext()" << priv::errEndl;

    priv::ensureExtensionsInit(*this);
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext() = default;


////////////////////////////////////////////////////////////
bool GraphicsContext::setSharedContextAsActiveThreadLocalGlContext(bool active)
{
    return setActiveThreadLocalGlContext(m_impl->sharedGlContext, active);
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext()
{
    const std::lock_guard lock(m_impl->sharedContextMutex);

    // We don't use acquireTransientContext here since we have
    // to ensure we have exclusive access to the shared context
    // in order to make sure it is not active during context creation
    if (!setSharedContextAsActiveThreadLocalGlContext(true))
        priv::err() << "Error enabling shared context in GlContext::create()" << priv::errEndl;

    // Create the context
    auto context = priv::makeUnique<DerivedGlContextType>(*this,
                                                          m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                          &m_impl->sharedGlContext);

    if (!setSharedContextAsActiveThreadLocalGlContext(false))
        priv::err() << "Error disabling shared in GlContext::create()" << priv::errEndl;

    if (!context->initialize(ContextSettings{}))
    {
        priv::err() << "Could not initialize context in GlContext::create()" << priv::errEndl;
        return nullptr;
    }

    return context;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings&  settings,
                                                                  const priv::WindowImpl& owner,
                                                                  unsigned int            bitsPerPixel)
{
    const std::lock_guard lock(m_impl->sharedContextMutex);

    // TODO: ?
    // If use_count is 2 (GlResource + sharedContext) we know that we are inside sf::Context or sf::Window
    // Only in this situation we allow the user to indirectly re-create the shared context as a core context

    // // Check if we need to convert our shared context into a core context
    // if ((SharedContext::getUseCount() == 2) && (settings.attributeFlags & ContextSettings::Attribute::Core) &&
    //     !(sharedGlContext->m_settings.attributeFlags & ContextSettings::Attribute::Core))
    // {
    //     // Re-create our shared context as a core context
    //     const ContextSettings sharedSettings{/* depthBits */ 0,
    //                                          /* stencilBits */ 0,
    //                                          /* antialiasingLevel */ 0,
    //                                          settings.majorVersion,
    //                                          settings.minorVersion,
    //                                          settings.attributeFlags};

    //     sharedGlContext.emplace(nullptr, sharedSettings, Vector2u{1, 1});
    //     if (!sharedGlContext->initialize(sharedSettings))
    //     {
    //        priv::err() << "Could not initialize shared context in GlContext::create()" <<priv::errEndl;
    //         return nullptr;
    //     }

    //     // Reload our extensions vector
    //     sharedContext->loadExtensions();
    // }

    // We don't use acquireTransientContext here since we have
    // to ensure we have exclusive access to the shared context
    // in order to make sure it is not active during context creation
    if (!setSharedContextAsActiveThreadLocalGlContext(true))
        priv::err() << "Error enabling shared context in GlContext::create()" << priv::errEndl;

    // Create the context
    auto context = priv::makeUnique<DerivedGlContextType>(*this,
                                                          m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                          &m_impl->sharedGlContext,
                                                          settings,
                                                          owner,
                                                          bitsPerPixel);

    if (!setSharedContextAsActiveThreadLocalGlContext(false))
        priv::err() << "Error disabling shared context in GlContext::create()" << priv::errEndl;

    if (!context->initialize(settings))
    {
        priv::err() << "Could not initialize context in GlContext::create()" << priv::errEndl;
        return nullptr;
    }

    context->checkSettings(settings);

    return context;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings& settings, const Vector2u& size)
{
    const std::lock_guard lock(m_impl->sharedContextMutex);

    // TODO: ?
    // If use_count is 2 (GlResource + sharedContext) we know that we are inside sf::Context or sf::Window
    // Only in this situation we allow the user to indirectly re-create the shared context as a core context

    // Check if we need to convert our shared context into a core context
    // if ((SharedContext::getUseCount() == 2) && (settings.attributeFlags & ContextSettings::Attribute::Core) &&
    //     !(sharedGlContext->m_settings.attributeFlags & ContextSettings::Attribute::Core))
    // {
    //     // Re-create our shared context as a core context
    //     const ContextSettings sharedSettings{/* depthBits */ 0,
    //                                          /* stencilBits */ 0,
    //                                          /* antialiasingLevel */ 0,
    //                                          settings.majorVersion,
    //                                          settings.minorVersion,
    //                                          settings.attributeFlags};

    //     sharedGlContext.emplace(nullptr, sharedSettings, Vector2u{1, 1});
    //     if (!sharedGlContext->initialize(sharedSettings))
    //     {
    //        priv::err() << "Could not initialize shared context in GlContext::create()" <<priv::errEndl;
    //         return nullptr;
    //     }

    //     // Reload our extensions vector
    //     sharedContext->loadExtensions();
    // }

    // We don't use acquireTransientContext here since we have
    // to ensure we have exclusive access to the shared context
    // in order to make sure it is not active during context creation
    if (!setSharedContextAsActiveThreadLocalGlContext(true))
        priv::err() << "Error enabling shared context in GlContext::create()" << priv::errEndl;

    // Create the context
    auto context = priv::makeUnique<DerivedGlContextType>(*this,
                                                          m_impl->nextThreadLocalGlContextId.fetch_add(1u),
                                                          &m_impl->sharedGlContext,
                                                          settings,
                                                          size);

    if (!setSharedContextAsActiveThreadLocalGlContext(false))
        priv::err() << "Error disabling shared context in GlContext::create()" << priv::errEndl;

    if (!context->initialize(settings))
    {
        priv::err() << "Could not initialize  context in GlContext::create()" << priv::errEndl;
        return nullptr;
    }

    context->checkSettings(settings);

    return context;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::isExtensionAvailable(const char* name)
{
    return priv::find(m_impl->extensions.begin(), m_impl->extensions.end(), name) != m_impl->extensions.end();
}


////////////////////////////////////////////////////////////
GlFunctionPointer GraphicsContext::getFunction(const char* name)
{
    return DerivedGlContextType::getFunction(name);
}

} // namespace sf
