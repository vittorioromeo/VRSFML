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

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"

#include <cstdlib>


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
[[nodiscard]] sf::Shader createBuiltInShader(sf::GraphicsContext& graphicsContext, const char* vertexSrc, const char* fragmentSrc)
{
    sf::Shader shader = sf::Shader::loadFromMemory(graphicsContext, vertexSrc, fragmentSrc).value();
    SFML_BASE_ASSERT(glCheck(glIsProgram(shader.getNativeHandle())));

    if (const sf::base::Optional ulTexture = shader.getUniformLocation("sf_u_texture"))
        shader.setUniform(*ulTexture, sf::Shader::CurrentTexture);

    return shader;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct GraphicsContext::Impl
{
    base::Optional<Shader>  builtInShader;
    base::Optional<Texture> builtInWhiteDotTexture;
};


////////////////////////////////////////////////////////////
GraphicsContext::GraphicsContext()
{
    m_impl->builtInShader.emplace(createBuiltInShader(*this, builtInShaderVertexSrc, builtInShaderFragmentSrc));
    m_impl->builtInWhiteDotTexture = Texture::loadFromImage(*this, *Image::create({1u, 1u}, Color::White));
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

} // namespace sf
