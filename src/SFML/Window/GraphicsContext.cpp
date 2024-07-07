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
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <glad/gl.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>


#if defined(SFML_SYSTEM_WINDOWS)

#if defined(SFML_OPENGL_ES)

#include <SFML/Window/EglContext.hpp>
using ContextType = sf::priv::EglContext;

#else

#include <SFML/Window/Win32/WglContext.hpp>
using ContextType = sf::priv::WglContext;

#endif

#elif defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD) || defined(SFML_SYSTEM_OPENBSD) || \
    defined(SFML_SYSTEM_NETBSD)

#if defined(SFML_USE_DRM)

#include <SFML/Window/DRM/DRMContext.hpp>
using ContextType = sf::priv::DRMContext;

#elif defined(SFML_OPENGL_ES)

#include <SFML/Window/EglContext.hpp>
using ContextType = sf::priv::EglContext;

#else

#include <SFML/Window/Unix/GlxContext.hpp>
using ContextType = sf::priv::GlxContext;

#endif

#elif defined(SFML_SYSTEM_MACOS)

#include <SFML/Window/macOS/SFContext.hpp>
using ContextType = sf::priv::SFContext;

#elif defined(SFML_SYSTEM_IOS)

#include <SFML/Window/iOS/EaglContext.hpp>
using ContextType = sf::priv::EaglContext;

#elif defined(SFML_SYSTEM_ANDROID)

#include <SFML/Window/EglContext.hpp>
using ContextType = sf::priv::EglContext;

#endif

#if defined(SFML_SYSTEM_WINDOWS)

using glEnableFuncType      = void(APIENTRY*)(GLenum);
using glGetErrorFuncType    = GLenum(APIENTRY*)();
using glGetIntegervFuncType = void(APIENTRY*)(GLenum, GLint*);
using glGetStringFuncType   = const GLubyte*(APIENTRY*)(GLenum);
using glGetStringiFuncType  = const GLubyte*(APIENTRY*)(GLenum, GLuint);
using glIsEnabledFuncType   = GLboolean(APIENTRY*)(GLenum);

#else

using glEnableFuncType      = void (*)(GLenum);
using glGetErrorFuncType    = GLenum (*)();
using glGetIntegervFuncType = void (*)(GLenum, GLint*);
using glGetStringFuncType   = const GLubyte* (*)(GLenum);
using glGetStringiFuncType  = const GLubyte* (*)(GLenum, GLuint);
using glIsEnabledFuncType   = GLboolean (*)(GLenum);

#endif

#if !defined(GL_MULTISAMPLE)
#define GL_MULTISAMPLE 0x809D
#endif

#if !defined(GL_MAJOR_VERSION)
#define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
#define GL_MINOR_VERSION 0x821C
#endif

#if !defined(GL_NUM_EXTENSIONS)
#define GL_NUM_EXTENSIONS 0x821D
#endif

#if !defined(GL_CONTEXT_FLAGS)
#define GL_CONTEXT_FLAGS 0x821E
#endif

#if !defined(GL_FRAMEBUFFER_SRGB)
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

#if !defined(GL_CONTEXT_FLAG_DEBUG_BIT)
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#endif

#if !defined(GL_CONTEXT_PROFILE_MASK)
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif

#if !defined(GL_CONTEXT_CORE_PROFILE_BIT)
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#if !defined(GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif


namespace sf
{
namespace
{
struct SharedContext
{
    ////////////////////////////////////////////////////////////
    /// \brief Constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SharedContext() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Initialize shared context
    ///
    ////////////////////////////////////////////////////////////
    void initialize()
    {
        const std::lock_guard lock(mutex);

        context.emplace(nullptr);
        if (!context->initialize(ContextSettings{}))
            priv::err() << "Could not initialize context in SharedContext::initalize()" << priv::errEndl;

        loadExtensions();

        if (!context->setActive(false))
            priv::err() << "Could not disable context in SharedContext::initalize()" << priv::errEndl;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Load our extensions vector with the supported extensions
    ///
    ////////////////////////////////////////////////////////////
    void loadExtensions()
    {
        auto glGetErrorFunc    = reinterpret_cast<glGetErrorFuncType>(priv::GlContext::getFunction("glGetError"));
        auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(priv::GlContext::getFunction("glGetIntegerv"));
        auto glGetStringFunc   = reinterpret_cast<glGetStringFuncType>(priv::GlContext::getFunction("glGetString"));

        if (!glGetErrorFunc || !glGetIntegervFunc || !glGetStringFunc)
            return;

        // Check whether a >= 3.0 context is available
        int majorVersion = 0;
        glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion);

        auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(priv::GlContext::getFunction("glGetStringi"));

        if (glGetErrorFunc() == GL_INVALID_ENUM || !majorVersion || !glGetStringiFunc)
        {
            // Try to load the < 3.0 way
            const char* extensionString = reinterpret_cast<const char*>(glGetStringFunc(GL_EXTENSIONS));

            if (extensionString)
            {
                extensions.clear();

                do
                {
                    const char* extension = extensionString;

                    while (*extensionString && (*extensionString != ' '))
                        ++extensionString;

                    extensions.emplace_back(extension, extensionString);
                } while (*extensionString++);
            }
        }
        else
        {
            // Try to load the >= 3.0 way
            int numExtensions = 0;
            glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions);

            if (numExtensions)
            {
                extensions.clear();

                for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
                {
                    const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                    if (extensionString)
                        extensions.emplace_back(extensionString);
                }
            }
        }
    }

    // AMD drivers have issues with internal synchronization
    // We need to make sure that no operating system context
    // or pixel format operations are performed simultaneously
    // This mutex is also used to protect the shared context
    // from being locked on multiple threads
    std::recursive_mutex mutex;

    // Supported OpenGL extensions
    std::vector<std::string> extensions;

    // The hidden, inactive context that will be shared with all other contexts
    std::optional<ContextType> context;
};

} // namespace


////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    SharedContext sharedContext{};
};


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext() : m_impl(priv::makeUnique<Impl>())
{
    m_impl->sharedContext.initialize();
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
    return m_impl->sharedContext.mutex;
}


////////////////////////////////////////////////////////////
void GraphicsContext::makeContextType(priv::UniquePtr<priv::GlContext>& target)
{
    target = priv::makeUnique<ContextType>(m_impl->sharedContext.context.value());
}


////////////////////////////////////////////////////////////
void GraphicsContext::makeContextType(priv::UniquePtr<priv::GlContext>& target,
                                      const ContextSettings&            settings,
                                      const priv::WindowImpl&           owner,
                                      unsigned int                      bitsPerPixel)
{
    target = priv::makeUnique<ContextType>(m_impl->sharedContext.context.value(), settings, owner, bitsPerPixel);
}


////////////////////////////////////////////////////////////
void GraphicsContext::makeContextType(priv::UniquePtr<priv::GlContext>& target,
                                      const ContextSettings&            settings,
                                      const Vector2u&                   size)
{
    target = priv::makeUnique<ContextType>(m_impl->sharedContext.context.value(), settings, size);
}


////////////////////////////////////////////////////////////
bool GraphicsContext::isExtensionAvailable(const char* name)
{
    return priv::find(m_impl->sharedContext.extensions.begin(), m_impl->sharedContext.extensions.end(), name) !=
           m_impl->sharedContext.extensions.end();
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
