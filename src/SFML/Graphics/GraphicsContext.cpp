#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/DefaultShader.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"

#include <atomic>


namespace sf
{
///////////////////////////////////////////////////////////
struct GraphicsContextImpl
{
    Shader  builtInShader;
    Texture builtInWhiteDotTexture;
};

namespace
{
///////////////////////////////////////////////////////////
constinit sf::base::Optional<GraphicsContextImpl> installedGraphicsContext;
constinit std::atomic<unsigned int>               graphicsContextRC{0u};


////////////////////////////////////////////////////////////
GraphicsContextImpl& ensureInstalled()
{
    if (!installedGraphicsContext.hasValue()) [[unlikely]]
    {
        priv::err() << "`sf::GraphicsContext` not installed -- did you forget to create one in `main`?";
        base::abort();
    }

    return *installedGraphicsContext;
}

} // namespace

////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    explicit Impl(WindowContext&& theWindowContext) : windowContext(SFML_BASE_MOVE(theWindowContext))
    {
    }

    WindowContext windowContext;
};

////////////////////////////////////////////////////////////
base::Optional<GraphicsContext> GraphicsContext::create(const ContextSettings& sharedContextSettings)
{
    const auto fail = [](const char* what)
    {
        priv::err() << "Error creating `sf::GraphicsContext`: " << what;
        return base::nullOpt;
    };

    //
    // Ensure graphics context is not already installed
    if (installedGraphicsContext.hasValue())
        return fail("a `sf::GraphicsContext` object already exists");

    //
    // Install window context if necessary
    auto windowContext = WindowContext::isInstalled() ? WindowContext{base::PassKey<GraphicsContext>{}}
                                                      : WindowContext::create(sharedContextSettings).value();

    //
    // Initialize built-in shader
    auto shader = DefaultShader::create();
    if (!shader.hasValue())
        return fail("built-in shader initialization failure");

    //
    // Initialize built-in texture
    auto texture = Texture::loadFromImage(*Image::create({1u, 1u}, Color::White));
    if (!texture.hasValue())
        return fail("built-in texture initialization failure");

    //
    // Install graphics context
    installedGraphicsContext.emplace(*SFML_BASE_MOVE(shader), *SFML_BASE_MOVE(texture));

    return base::makeOptional<GraphicsContext>(base::PassKey<GraphicsContext>{}, SFML_BASE_MOVE(windowContext));
}


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext(base::PassKey<GraphicsContext>&&, WindowContext&& windowContext) :
m_impl(SFML_BASE_MOVE(windowContext))
{
    graphicsContextRC.fetch_add(1u, std::memory_order::relaxed);
}


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext(GraphicsContext&& rhs) noexcept :
m_impl(static_cast<WindowContext&&>(rhs.m_impl->windowContext))
{
    graphicsContextRC.fetch_add(1u, std::memory_order::relaxed);
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext()
{
    if (graphicsContextRC.fetch_sub(1u, std::memory_order::relaxed) > 1u)
        return;

    // Need to activate shared context during destruction to avoid GL errors when destroying texture and shader
    [[maybe_unused]] const bool rc = WindowContext::setActiveThreadLocalGlContextToSharedContext();
    SFML_BASE_ASSERT(rc);

    installedGraphicsContext.reset();
}


////////////////////////////////////////////////////////////
Shader& GraphicsContext::getBuiltInShader() const
{
    return ensureInstalled().builtInShader;
}


////////////////////////////////////////////////////////////
Texture& GraphicsContext::getBuiltInWhiteDotTexture() const
{
    return ensureInstalled().builtInWhiteDotTexture;
}


////////////////////////////////////////////////////////////
unsigned int GraphicsContext::getActiveThreadLocalGlContextId()
{
    return WindowContext::getActiveThreadLocalGlContextId();
}


////////////////////////////////////////////////////////////
bool GraphicsContext::hasActiveThreadLocalGlContext()
{
    return WindowContext::hasActiveThreadLocalGlContext();
}


////////////////////////////////////////////////////////////
void GraphicsContext::registerUnsharedFrameBuffer(const unsigned int glContextId, const unsigned int frameBufferId)
{
    WindowContext::registerUnsharedFrameBuffer(glContextId, frameBufferId);
}


////////////////////////////////////////////////////////////
void GraphicsContext::unregisterUnsharedFrameBuffer(const unsigned int glContextId, const unsigned int frameBufferId)
{
    WindowContext::unregisterUnsharedFrameBuffer(glContextId, frameBufferId);
}


////////////////////////////////////////////////////////////
void GraphicsContext::registerUnsharedVAO(const unsigned int glContextId, const unsigned int vaoId)
{
    WindowContext::registerUnsharedVAO(glContextId, vaoId);
}


////////////////////////////////////////////////////////////
void GraphicsContext::unregisterUnsharedVAO(const unsigned int glContextId, const unsigned int vaoId)
{
    WindowContext::unregisterUnsharedVAO(glContextId, vaoId);
}


////////////////////////////////////////////////////////////
bool GraphicsContext::isInstalled()
{
    return installedGraphicsContext.hasValue();
}


////////////////////////////////////////////////////////////
Shader& GraphicsContext::getInstalledBuiltInShader()
{
    SFML_BASE_ASSERT(installedGraphicsContext.hasValue());
    return installedGraphicsContext->builtInShader;
}


////////////////////////////////////////////////////////////
Texture& GraphicsContext::getInstalledBuiltInWhiteDotTexture()
{
    SFML_BASE_ASSERT(installedGraphicsContext.hasValue());
    return installedGraphicsContext->builtInWhiteDotTexture;
}

} // namespace sf
