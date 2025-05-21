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
/// \brief Provides access to SFML's built-in default shader sources and creation
///
/// `sf::DefaultShader` is a utility struct that serves two main purposes:
///
/// 1. It stores the GLSL source code for SFML's default vertex (`srcVertex`)
///    and fragment (`srcFragment`) shaders. These shaders are used internally
///    by SFML when no custom shader is specified, providing basic 2D texturing
///    and coloring.
///
/// 2. It offers a static factory method (`create()`) to easily compile and
///    link these built-in sources into a ready-to-use `sf::Shader` object.
///
/// This is useful for users who might want to inspect the default shader code
/// or create an instance of the default shader, perhaps as a base for their
/// own modifications or as a fallback.
///
////////////////////////////////////////////////////////////
struct DefaultShader
{
    ////////////////////////////////////////////////////////////
    /// \brief GLSL source code for the built-in default vertex shader
    ///
    /// This shader handles basic 2D transformations (model-view-projection matrix)
    /// and passes vertex color and texture coordinates to the fragment shader.
    /// Texture coordinates are normalized based on the texture size.
    ///
    /// Inputs (attributes):
    /// - `vec2 sf_a_position`: Vertex position
    /// - `vec4 sf_a_color`: Vertex color
    /// - `vec2 sf_a_texCoord`: Non-normalized vertex texture coordinates
    ///
    /// Uniforms:
    /// - `mat4 sf_u_mvpMatrix`: Combined model-view-projection matrix
    /// - `sampler2D sf_u_texture`: The texture to be sampled (used for `textureSize`)
    ///
    /// Outputs (varyings):
    /// - `vec4 sf_v_color`: Passed-through vertex color
    /// - `vec2 sf_v_texCoord`: Normalized texture coordinates
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
    /// \brief GLSL source code for the built-in default fragment shader
    ///
    /// This shader samples a texture at the given texture coordinates
    /// and multiplies the sampled color by the vertex color to produce
    /// the final fragment color.
    ///
    /// Inputs (varyings):
    /// - `vec4 sf_v_color`: Interpolated vertex color
    /// - `vec2 sf_v_texCoord`: Interpolated normalized texture coordinates
    ///
    /// Uniforms:
    /// - `sampler2D sf_u_texture`: The texture to be sampled
    ///
    /// Outputs (fragment color):
    /// - `vec4 sf_fragColor`: Final color of the fragment
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
    /// \brief Create an `sf::Shader` instance from the default shader sources
    ///
    /// This static method attempts to compile `srcVertex` and `srcFragment`
    /// and link them into a new `sf::Shader` object.
    ///
    /// This is a convenient way to get a ready-to-use instance of SFML's
    /// default shader program.
    ///
    /// \return An `sf::base::Optional<sf::Shader>` containing the compiled
    ///         shader if successful, or `sf::base::nullOpt` if compilation
    ///         or linking failed.
    ///
    /// \see sf::Shader::loadFromMemory
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Shader> create();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::DefaultShader
/// \ingroup graphics
///
/// `sf::DefaultShader` provides access to SFML's built-in
/// default vertex and fragment shader sources. It also offers
/// a convenient way to create an `sf::Shader` object from these
/// default sources.
///
/// This can be useful if you need a basic shader for rendering
/// textured and colored 2D objects, or if you want to inspect
/// or extend SFML's default rendering behavior.
///
/// Example:
/// \code
/// // Attempt to create an instance of the default shader
/// auto defaultShader = sf::DefaultShader::create();
///
/// if (defaultShader.hasValue())
/// {
///     // Use the default shader for rendering
///     // renderStates.shader = &(*defaultShader);
/// }
/// else
/// {
///     // Shader creation failed, handle error
/// }
///
/// // Accessing the source code directly
/// const char* vertexShaderSource = sf::DefaultShader::srcVertex;
/// const char* fragmentShaderSource = sf::DefaultShader::srcFragment;
/// \endcode
///
/// \see sf::Shader
///
////////////////////////////////////////////////////////////
