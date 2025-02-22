#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"

#include <atomic>


namespace sf
{
namespace
{
////////////////////////////////////////////////////////////
constexpr const char* builtInShaderVertexSrc = R"glsl(

layout(location = 0) uniform mat4 sf_u_mvpMatrix;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    gl_Position = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color = sf_a_color;
    sf_v_texCoord = sf_a_texCoord;
}

)glsl";


////////////////////////////////////////////////////////////
constexpr const char* builtInShaderFragmentSrc = R"glsl(

layout(location = 2) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = sf_v_color * texture(sf_u_texture, sf_v_texCoord / vec2(textureSize(sf_u_texture, 0)));
}

)glsl";


////////////////////////////////////////////////////////////
[[nodiscard]] base::Optional<Shader> createBuiltInShader(const char* vertexSrc, const char* fragmentSrc)
{
    auto result = Shader::loadFromMemory(vertexSrc, fragmentSrc);

    if (result)
    {
        SFML_BASE_ASSERT(glCheck(glIsProgram(result->getNativeHandle())));

        if (const base::Optional ulTexture = result->getUniformLocation("sf_u_texture"))
            result->setUniform(*ulTexture, Shader::CurrentTexture);
    }

    return result;
}

} // namespace


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
constinit std::atomic<unsigned int> graphicsContextRC{0u};


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
base::Optional<GraphicsContext> GraphicsContext::create()
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
                                                      : WindowContext::create().value();

    //
    // Initialize built-in shader
    auto shader = createBuiltInShader(builtInShaderVertexSrc, builtInShaderFragmentSrc);
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
WindowContext(SFML_BASE_MOVE(windowContext))
{
    graphicsContextRC.fetch_add(1u, std::memory_order_relaxed);
}


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext(GraphicsContext&& rhs) noexcept : WindowContext(static_cast<WindowContext&&>(rhs))
{
    graphicsContextRC.fetch_add(1u, std::memory_order_relaxed);
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext()
{
    if (graphicsContextRC.fetch_sub(1u, std::memory_order_relaxed) > 1u)
        return;

    // Need to activate shared context during destruction to avoid GL errors when destroying texture and shader
    [[maybe_unused]] const bool rc = WindowContext::setActiveThreadLocalGlContextToSharedContext(true);
    SFML_BASE_ASSERT(rc);

    installedGraphicsContext.reset();
}


////////////////////////////////////////////////////////////
[[nodiscard]] Shader& GraphicsContext::getBuiltInShader() const
{
    return ensureInstalled().builtInShader;
}


////////////////////////////////////////////////////////////
[[nodiscard]] Texture& GraphicsContext::getBuiltInWhiteDotTexture() const
{
    return ensureInstalled().builtInWhiteDotTexture;
}


////////////////////////////////////////////////////////////
const char* GraphicsContext::getBuiltInShaderVertexSrc()
{
    return builtInShaderVertexSrc;
}


////////////////////////////////////////////////////////////
const char* GraphicsContext::getBuiltInShaderFragmentSrc()
{
    return builtInShaderFragmentSrc;
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
