#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"


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
[[nodiscard]] sf::Shader createBuiltInShader(const char* vertexSrc, const char* fragmentSrc)
{
    sf::Shader shader = sf::Shader::loadFromMemory(vertexSrc, fragmentSrc).value(); // TODO P1: propagate and use factory?
    SFML_BASE_ASSERT(glCheck(glIsProgram(shader.getNativeHandle())));

    if (const sf::base::Optional ulTexture = shader.getUniformLocation("sf_u_texture"))
        shader.setUniform(*ulTexture, sf::Shader::CurrentTexture);

    return shader;
}


///////////////////////////////////////////////////////////
constinit sf::GraphicsContext* installedGraphicsContext{nullptr};

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    ////////////////////////////////////////////////////////////
    struct ClearInstalledGuard
    {
        ~ClearInstalledGuard()
        {
            SFML_BASE_ASSERT(installedGraphicsContext != nullptr);
            installedGraphicsContext = nullptr;
        }
    };

    ////////////////////////////////////////////////////////////
    ClearInstalledGuard clearInstalledGuard;

    ////////////////////////////////////////////////////////////
    base::Optional<Shader>  builtInShader;
    base::Optional<Texture> builtInWhiteDotTexture;
};


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext()
{
    // Install graphics context:
    if (installedGraphicsContext != nullptr)
    {
        priv::err() << "Fatal error creating `sf::GraphicsContext`: a `sf::GraphicsContext` object already exists";
        base::abort();
    }

    installedGraphicsContext = this;

    m_impl->builtInShader.emplace(createBuiltInShader(builtInShaderVertexSrc, builtInShaderFragmentSrc));
    m_impl->builtInWhiteDotTexture = Texture::loadFromImage(*Image::create({1u, 1u}, Color::White));
}


////////////////////////////////////////////////////////////
GraphicsContext::~GraphicsContext()
{
    // Need to activate shared context during destruction to avoid GL errors when destroying texture and shader
    [[maybe_unused]] const bool rc = setActiveThreadLocalGlContextToSharedContext(true);
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
[[nodiscard]] Shader& GraphicsContext::getBuiltInShader()
{
    return *m_impl->builtInShader;
}


////////////////////////////////////////////////////////////
[[nodiscard]] Texture& GraphicsContext::getBuiltInWhiteDotTexture()
{
    return *m_impl->builtInWhiteDotTexture;
}


////////////////////////////////////////////////////////////
const char* GraphicsContext::getBuiltInShaderVertexSrc() const
{
    return builtInShaderVertexSrc;
}


////////////////////////////////////////////////////////////
const char* GraphicsContext::getBuiltInShaderFragmentSrc() const
{
    return builtInShaderFragmentSrc;
}


////////////////////////////////////////////////////////////
GraphicsContext* GraphicsContext::getInstalled()
{
    return installedGraphicsContext;
}


////////////////////////////////////////////////////////////
GraphicsContext& GraphicsContext::ensureInstalled()
{
    if (installedGraphicsContext == nullptr) [[unlikely]]
    {
        priv::err() << "`sf::GraphicsContext` not installed -- did you forget to create one in `main`?";
        base::abort();
    }

    return *installedGraphicsContext;
}

} // namespace sf
