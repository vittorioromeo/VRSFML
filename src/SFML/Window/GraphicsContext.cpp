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

#include <SFML/Window/Context.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <glad/gl.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>


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
    std::vector<std::string> result;

    auto glGetErrorFunc    = reinterpret_cast<glGetErrorFuncType>(ContextType::getFunction("glGetError"));
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(ContextType::getFunction("glGetIntegerv"));
    auto glGetStringFunc   = reinterpret_cast<glGetStringFuncType>(ContextType::getFunction("glGetString"));

    if (!glGetErrorFunc || !glGetIntegervFunc || !glGetStringFunc)
        return result; // Empty vector

    // Check whether a >= 3.0 context is available
    int majorVersion = 0;
    glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion);

    auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(ContextType::getFunction("glGetStringi"));

    if (glGetErrorFunc() == GL_INVALID_ENUM || !majorVersion || !glGetStringiFunc)
    {
        // Try to load the < 3.0 way
        const char* extensionString = reinterpret_cast<const char*>(glGetStringFunc(GL_EXTENSIONS));

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

    for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
        if (const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i)))
            result.emplace_back(extensionString);

    return result;
}

struct SharedContext
{
    [[nodiscard]] SharedContext(const std::lock_guard<std::recursive_mutex>&) :
    context{std::in_place, nullptr},
    extensions{[&]
               {
                   if (!context->initialize(ContextSettings{}))
                       priv::err() << "Could not initialize context in SharedContext()" << priv::errEndl;

                   return loadExtensions();
               }()}
    {
        if (!context->setActive(false))
            priv::err() << "Could not disable context in SharedContext()" << priv::errEndl;
    }

    // AMD drivers have issues with internal synchronization
    // We need to make sure that no operating system context
    // or pixel format operations are performed simultaneously
    // This mutex is also used to protect the shared context
    // from being locked on multiple threads

    // The hidden, inactive context that will be shared with all other contexts
    std::optional<ContextType> context;

    // Supported OpenGL extensions
    const std::vector<std::string> extensions;
};

} // namespace


////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    [[nodiscard]] explicit Impl() : sharedContext(std::lock_guard{mutex})
    {
    }

    std::recursive_mutex mutex;
    SharedContext        sharedContext;
};


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext() : m_impl(priv::makeUnique<Impl>())
{
    if (!setActive(true))
        priv::err() << "Failed to enable graphics context in GraphicsContext()" << priv::errEndl;
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext() = default;


////////////////////////////////////////////////////////////
bool GraphicsContext::setActive(bool active)
{
    assert(m_impl->sharedContext.context.has_value());
    return m_impl->sharedContext.context->setActive(active);
}


////////////////////////////////////////////////////////////
GraphicsContext::Guard GraphicsContext::lock()
{
    return Guard{*this};
}


////////////////////////////////////////////////////////////
std::recursive_mutex& GraphicsContext::getMutex()
{
    return m_impl->mutex;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::makeContextType()
{
    return priv::makeUnique<ContextType>(&m_impl->sharedContext.context.value());
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::makeContextType(const ContextSettings&  settings,
                                                                  const priv::WindowImpl& owner,
                                                                  unsigned int            bitsPerPixel)
{
    return priv::makeUnique<ContextType>(&m_impl->sharedContext.context.value(), settings, owner, bitsPerPixel);
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::makeContextType(const ContextSettings& settings, const Vector2u& size)
{
    return priv::makeUnique<ContextType>(&m_impl->sharedContext.context.value(), settings, size);
}


////////////////////////////////////////////////////////////
bool GraphicsContext::isExtensionAvailable(const char* name)
{
    return priv::find(m_impl->sharedContext.extensions.begin(), m_impl->sharedContext.extensions.end(), name) !=
           m_impl->sharedContext.extensions.end();
}


////////////////////////////////////////////////////////////
GlFunctionPointer GraphicsContext::getFunction(const char* name)
{
    return m_impl->sharedContext.context->getFunction(name);
}


////////////////////////////////////////////////////////////
GraphicsContext::Guard::Guard(GraphicsContext& parent) : m_parent(parent)
{
    if (!m_parent.setActive(true))
        priv::err() << "Failed to activate transient context 2" << priv::errEndl;
}


////////////////////////////////////////////////////////////
GraphicsContext::Guard::~Guard()
{
    if (!m_parent.setActive(false))
        priv::err() << "Failed to deactivate transient context 2" << priv::errEndl;
}

} // namespace sf
