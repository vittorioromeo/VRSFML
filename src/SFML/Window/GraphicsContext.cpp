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

#include <SFML/Graphics/Shader.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Optional.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <glad/gl.h>

#include <mutex>
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
    context{sf::inPlace, nullptr},
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
    sf::Optional<ContextType> context;

    // Supported OpenGL extensions
    const std::vector<std::string> extensions;
};


////////////////////////////////////////////////////////////
constexpr const char* defaultFragShader = R"(

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor = v_color * texture2D(texture, v_texCoord.st);
}

)";


////////////////////////////////////////////////////////////
constexpr const char* defaultVertexShader = R"(

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 projMatrix;
uniform mat4 textMatrix;
uniform mat4 viewMatrix;

attribute vec4 color;
attribute vec2 position;
attribute vec2 texCoord;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    gl_Position = projMatrix * viewMatrix * vec4(position, 0.0, 1.0);
    v_texCoord = (textMatrix * vec4(texCoord, 0.0, 1.0)).xy;
    v_color = color;
}

)";

////////////////////////////////////////////////////////////
Optional<Shader> createBuiltInShader(GraphicsContext& graphicsContext)
{
    sf::Optional shader = sf::Shader::loadFromMemory(graphicsContext, defaultVertexShader, defaultFragShader);
    assert(shader.hasValue());

    const sf::Optional ulTexture = shader->getUniformLocation("texture");
    assert(ulTexture.hasValue());

    shader->setUniform(*ulTexture, sf::Shader::CurrentTexture);

    assert(glIsProgram(shader->getNativeHandle()));
    return shader;
}

} // namespace


////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    [[nodiscard]] explicit Impl() : sharedContext(std::lock_guard{mutex})
    {
    }

    std::recursive_mutex       mutex;
    SharedContext              sharedContext;
    Optional<TransientContext> transientContext;
    Optional<Shader>           builtInShader;
};


////////////////////////////////////////////////////////////
struct GraphicsContext::TransientContext::Impl
{
    [[nodiscard]] explicit Impl(GraphicsContext& theGraphicsContext) :
    graphicsContext(theGraphicsContext),
    sharedContextLock(graphicsContext.m_impl->mutex)
    {
    }

    GraphicsContext&                       graphicsContext;
    std::unique_lock<std::recursive_mutex> sharedContextLock;
};


////////////////////////////////////////////////////////////
GraphicsContext::TransientContext::TransientContext(GraphicsContext& theGraphicsContext) : impl(theGraphicsContext)
{
    // TransientContext should never be created if there is
    // already a context active on the current thread
    assert(!hasActiveContext() && "Another context is active on the current thread");

    // Lock the shared context for temporary use
    if (!impl->graphicsContext.setActive(true))
        priv::err() << "Error enabling shared context in TransientContext()" << priv::errEndl;
}


////////////////////////////////////////////////////////////
GraphicsContext::TransientContext::~TransientContext()
{
    if (!impl->graphicsContext.setActive(false))
        priv::err() << "Error disabling shared context in ~TransientContext()" << priv::errEndl;
}


////////////////////////////////////////////////////////////
GraphicsContext::CurrentContext& GraphicsContext::CurrentContext::get()
{
    // TODO: to member of graphicscontext?
    thread_local CurrentContext currentContext;
    return currentContext;
}


////////////////////////////////////////////////////////////
GraphicsContext::CurrentContext::CurrentContext() = default;


////////////////////////////////////////////////////////////
void GraphicsContext::acquireTransientContext()
{
    auto& [id, ptr, transientCount] = CurrentContext::get();

    // Fast path if we already have a context active on this thread
    if (id != 0)
    {
        ++transientCount;
        return;
    }

    // assert(false); // TODO: do we ever get here...?

    // If we don't already have a context active on this thread the count should be 0
    assert(transientCount == 0 && "Transient count cannot be non-zero");

    // If currentContextId is not set, this must be the first
    // TransientContextLock on this thread, construct the state object
    assert(!m_impl->transientContext.hasValue());
    m_impl->transientContext.emplace(*this);

    // Make sure a context is active at this point
    assert(id != 0 && "Current context ID cannot be zero");
}


////////////////////////////////////////////////////////////
void GraphicsContext::releaseTransientContext()
{
    auto& [id, ptr, transientCount] = CurrentContext::get();

    // Make sure a context was left active after acquireTransientContext() was called
    assert(id != 0 && "Current context ID cannot be zero");

    // Fast path if we already had a context active on this thread before acquireTransientContext() was called
    if (transientCount > 0)
    {
        --transientCount;
        return;
    }

    // If currentContextId is set and currentContextTransientCount is 0,
    // this is the last TransientContextLock that is released, destroy the state object
    assert(m_impl->transientContext.hasValue());
    m_impl->transientContext.reset();
}


////////////////////////////////////////////////////////////
const priv::GlContext* GraphicsContext::getActiveContext()
{
    return CurrentContext::get().ptr;
}


////////////////////////////////////////////////////////////
std::uint64_t GraphicsContext::getActiveContextId()
{
    return CurrentContext::get().id;
}


////////////////////////////////////////////////////////////
bool GraphicsContext::hasActiveContext()
{
    return getActiveContextId() != 0;
}


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext() : m_impl(priv::makeUnique<Impl>())
{
    if (!setActive(true))
        priv::err() << "Failed to enable graphics context in GraphicsContext()" << priv::errEndl;

    m_impl->builtInShader = createBuiltInShader(*this);
    assert(m_impl->builtInShader.hasValue());
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext() = default;


////////////////////////////////////////////////////////////
bool GraphicsContext::setActive(bool active)
{
    assert(m_impl->sharedContext.context.hasValue());
    return m_impl->sharedContext.context->setActive(active);
}


////////////////////////////////////////////////////////////
GraphicsContext::Guard GraphicsContext::lock()
{
    return Guard{*this};
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext()
{
    const std::lock_guard lock(m_impl->mutex);

    // We don't use acquireTransientContext here since we have
    // to ensure we have exclusive access to the shared context
    // in order to make sure it is not active during context creation
    if (!setActive(true))
        priv::err() << "Error enablind shared context in GlContext::create()" << priv::errEndl;

    // Create the context
    auto context = priv::makeUnique<ContextType>(&m_impl->sharedContext.context.value());

    if (!setActive(false))
        priv::err() << "Error disabling shared  in GlContext::create()" << priv::errEndl;

    if (!context->initialize(ContextSettings{}))
    {
        priv::err() << "Could not initialize  context in GlContext::create()" << priv::errEndl;
        return nullptr;
    }

    return context;
}


////////////////////////////////////////////////////////////
priv::UniquePtr<priv::GlContext> GraphicsContext::createGlContext(const ContextSettings&  settings,
                                                                  const priv::WindowImpl& owner,
                                                                  unsigned int            bitsPerPixel)
{
    const std::lock_guard lock(m_impl->mutex);

    // TODO: ?
    // If use_count is 2 (GlResource + sharedContext) we know that we are inside sf::Context or sf::Window
    // Only in this situation we allow the user to indirectly re-create the shared context as a core context

    // // Check if we need to convert our shared context into a core context
    // if ((SharedContext::getUseCount() == 2) && (settings.attributeFlags & ContextSettings::Attribute::Core) &&
    //     !(sharedContext.context->m_settings.attributeFlags & ContextSettings::Attribute::Core))
    // {
    //     // Re-create our shared context as a core context
    //     const ContextSettings sharedSettings{/* depthBits */ 0,
    //                                          /* stencilBits */ 0,
    //                                          /* antialiasingLevel */ 0,
    //                                          settings.majorVersion,
    //                                          settings.minorVersion,
    //                                          settings.attributeFlags};

    //     sharedContext.context.emplace(nullptr, sharedSettings, Vector2u{1, 1});
    //     if (!sharedContext.context->initialize(sharedSettings))
    //     {
    //        priv::err() << "Could not initialize shared context in GlContext::create()" <<priv::errEndl;
    //         return nullptr;
    //     }

    //     // Reload our extensions vector
    //     sharedContext.loadExtensions();
    // }

    // We don't use acquireTransientContext here since we have
    // to ensure we have exclusive access to the shared context
    // in order to make sure it is not active during context creation
    if (!setActive(true))
        priv::err() << "Error enabling shared context in GlContext::create()" << priv::errEndl;

    // Create the context
    auto context = priv::makeUnique<ContextType>(&m_impl->sharedContext.context.value(), settings, owner, bitsPerPixel);

    if (!setActive(false))
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
    const std::lock_guard lock(m_impl->mutex);

    // TODO: ?
    // If use_count is 2 (GlResource + sharedContext) we know that we are inside sf::Context or sf::Window
    // Only in this situation we allow the user to indirectly re-create the shared context as a core context

    // Check if we need to convert our shared context into a core context
    // if ((SharedContext::getUseCount() == 2) && (settings.attributeFlags & ContextSettings::Attribute::Core) &&
    //     !(sharedContext.context->m_settings.attributeFlags & ContextSettings::Attribute::Core))
    // {
    //     // Re-create our shared context as a core context
    //     const ContextSettings sharedSettings{/* depthBits */ 0,
    //                                          /* stencilBits */ 0,
    //                                          /* antialiasingLevel */ 0,
    //                                          settings.majorVersion,
    //                                          settings.minorVersion,
    //                                          settings.attributeFlags};

    //     sharedContext.context.emplace(nullptr, sharedSettings, Vector2u{1, 1});
    //     if (!sharedContext.context->initialize(sharedSettings))
    //     {
    //        priv::err() << "Could not initialize shared context in GlContext::create()" <<priv::errEndl;
    //         return nullptr;
    //     }

    //     // Reload our extensions vector
    //     sharedContext.loadExtensions();
    // }

    // We don't use acquireTransientContext here since we have
    // to ensure we have exclusive access to the shared context
    // in order to make sure it is not active during context creation
    if (!setActive(true))
        priv::err() << "Error enabling shared context in GlContext::create()" << priv::errEndl;

    // Create the context
    auto context = priv::makeUnique<ContextType>(&m_impl->sharedContext.context.value(), settings, size);

    if (!setActive(false))
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

////////////////////////////////////////////////////////////
Shader& GraphicsContext::getBuiltInShader()
{
    return *m_impl->builtInShader;
}

} // namespace sf
