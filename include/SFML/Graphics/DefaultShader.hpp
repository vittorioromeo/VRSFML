#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Shader.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
template <typename T>
class Optional;
}

namespace sf
{
class Shader;
}


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct DefaultShader
{
    ////////////////////////////////////////////////////////////
    /// \brief Built-in default vertex shader source code
    ///
    ////////////////////////////////////////////////////////////
    static inline constexpr const char* srcVertex = R"glsl(

layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 1) uniform sampler2D sf_u_texture;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    gl_Position = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color = sf_a_color;
    sf_v_texCoord = sf_a_texCoord / vec2(textureSize(sf_u_texture, 0));
}

)glsl";

    ////////////////////////////////////////////////////////////
    /// \brief Built-in default fragment shader source code
    ///
    ////////////////////////////////////////////////////////////
    static inline constexpr const char* srcFragment = R"glsl(

layout(location = 1) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = sf_v_color * texture(sf_u_texture, sf_v_texCoord);
}

)glsl";

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Shader> create();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::DefaultShader
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see `sf::Shader`
///
////////////////////////////////////////////////////////////
