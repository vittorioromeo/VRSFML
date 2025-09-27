#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Glsl.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class InputStream;
class Path;
class RenderTarget;
class Texture;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Shader class (vertex, geometry and fragment)
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Shader
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Types of shaders
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] Type : unsigned char
    {
        Vertex,   //!< %Vertex shader
        Geometry, //!< Geometry shader
        Fragment  //!< Fragment (pixel) shader
    };

    ////////////////////////////////////////////////////////////
    /// \brief Special type that can be passed to setUniform(),
    ///        and that represents the texture of the object being drawn
    ///
    /// \see `setUniform(base::StringView, CurrentTextureType)`
    ///
    ////////////////////////////////////////////////////////////
    struct CurrentTextureType
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Represents the texture of the object being drawn
    ///
    /// \see `setUniform(base::StringView, CurrentTextureType)`
    ///
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(readability-identifier-naming)
    static inline constexpr CurrentTextureType CurrentTexture{};

    ////////////////////////////////////////////////////////////
    /// \brief Type-safe wrapper over a non-null shader uniform location
    ///
    ////////////////////////////////////////////////////////////
    class [[nodiscard]] UniformLocation
    {
        friend Shader;

    private:
        [[nodiscard]] explicit UniformLocation(int location);
        int m_value;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Shader();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Shader(const Shader&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Shader& operator=(const Shader&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Shader(Shader&& source) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Shader& operator=(Shader&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct LoadFromFileSettings
    {
        Path vertexPath{};   // NOLINT(readability-redundant-member-init)
        Path fragmentPath{}; // NOLINT(readability-redundant-member-init)
        Path geometryPath{}; // NOLINT(readability-redundant-member-init)
    };

    ////////////////////////////////////////////////////////////
    /// \brief Load the vertex, geometry and fragment shaders from files
    ///
    /// This function loads the vertex, geometry and fragment
    /// shaders. If one of them fails to load, the shader is left
    /// empty (the valid shader is unloaded).
    /// The sources must be text files containing valid shaders
    /// in GLSL language. GLSL is a C-like language dedicated to
    /// OpenGL shaders; you'll probably need to read a good documentation
    /// for it before writing your own shaders.
    ///
    /// \param vertexShaderFilename   Path of the vertex shader file to load
    /// \param geometryShaderFilename Path of the geometry shader file to load
    /// \param fragmentShaderFilename Path of the fragment shader file to load
    ///
    /// \return Shader if loading succeeded, `base::nullOpt` if it failed
    ///
    /// \see `loadFromMemory`, `loadFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Shader> loadFromFile(const LoadFromFileSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct LoadFromMemorySettings
    {
        base::StringView vertexCode{};   // NOLINT(readability-redundant-member-init)
        base::StringView fragmentCode{}; // NOLINT(readability-redundant-member-init)
        base::StringView geometryCode{}; // NOLINT(readability-redundant-member-init)
    };

    ////////////////////////////////////////////////////////////
    /// \brief Load the vertex, geometry and fragment shaders from source codes in memory
    ///
    /// This function loads the vertex, geometry and fragment
    /// shaders. If one of them fails to load, the shader is left
    /// empty (the valid shader is unloaded).
    /// The sources must be valid shaders in GLSL language. GLSL is
    /// a C-like language dedicated to OpenGL shaders; you'll
    /// probably need to read a good documentation for it before
    /// writing your own shaders.
    ///
    /// \param vertexShader   String containing the source code of the vertex shader
    /// \param geometryShader String containing the source code of the geometry shader
    /// \param fragmentShader String containing the source code of the fragment shader
    ///
    /// \return Shader if loading succeeded, `base::nullOpt` if it failed
    ///
    /// \see `loadFromFile`, `loadFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Shader> loadFromMemory(const LoadFromMemorySettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct LoadFromStreamSettings
    {
        InputStream* vertexStream{nullptr};
        InputStream* fragmentStream{nullptr};
        InputStream* geometryStream{nullptr};
    };

    ////////////////////////////////////////////////////////////
    /// \brief Load the vertex, geometry and fragment shaders from custom streams
    ///
    /// This function loads the vertex, geometry and fragment
    /// shaders. If one of them fails to load, the shader is left
    /// empty (the valid shader is unloaded).
    /// The source codes must be valid shaders in GLSL language.
    /// GLSL is a C-like language dedicated to OpenGL shaders;
    /// you'll probably need to read a good documentation for
    /// it before writing your own shaders.
    ///
    /// \param vertexShaderStream   Source stream to read the vertex shader from
    /// \param geometryShaderStream Source stream to read the geometry shader from
    /// \param fragmentShaderStream Source stream to read the fragment shader from
    ///
    /// \return Shader if loading succeeded, `base::nullOpt` if it failed
    ///
    /// \see `loadFromFile`, `loadFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Shader> loadFromStream(const LoadFromStreamSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Get the location ID of a shader uniform
    ///
    /// \param name Name of the uniform variable to search
    ///
    /// \return Location ID of the uniform, or `sf::base::nullOpt` if not found
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<UniformLocation> getUniformLocation(base::StringView uniformName) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p float uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param x    Value of the float scalar
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, float x) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p vec2 uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the vec2
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, Glsl::Vec2 vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p vec3 uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the vec3
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Vec3& vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p vec4 uniform
    ///
    /// This overload can also be called with `sf::Color` objects
    /// that are converted to `sf::Glsl::Vec4`.
    ///
    /// It is important to note that the components of the color are
    /// normalized before being passed to the shader. Therefore,
    /// they are converted from range [0 .. 255] to range [0 .. 1].
    /// For example, a `sf::Color(255, 127, 0, 255)` will be transformed
    /// to a `vec4(1.0, 0.5, 0.0, 1.0)` in the shader.
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the vec4
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Vec4& vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p int uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param x    Value of the int scalar
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, int x) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p ivec2 uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the ivec2
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, Glsl::Ivec2 vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p ivec3 uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the ivec3
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Ivec3& vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p ivec4 uniform
    ///
    /// This overload can also be called with `sf::Color` objects
    /// that are converted to `sf::Glsl::Ivec4`.
    ///
    /// If color conversions are used, the ivec4 uniform in GLSL
    /// will hold the same values as the original `sf::Color`
    /// instance. For example, `sf::Color(255, 127, 0, 255)` is
    /// mapped to `ivec4(255, 127, 0, 255)`.
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the ivec4
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Ivec4& vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p bool uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param x    Value of the bool scalar
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, bool x) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p bvec2 uniform
    ///
    /// \param name Name of the uniform variable in GLSL
    /// \param vec  Value of the bvec2
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, Glsl::Bvec2 vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p bvec3 uniform
    ///
    /// \param name   Name of the uniform variable in GLSL
    /// \param vec Value of the bvec3
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Bvec3& vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p bvec4 uniform
    ///
    /// \param name   Name of the uniform variable in GLSL
    /// \param vec Value of the bvec4
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Bvec4& vec) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p mat3 matrix
    ///
    /// \param name   Name of the uniform variable in GLSL
    /// \param matrix Value of the mat3 matrix
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Mat3& matrix) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify value for \p mat4 matrix
    ///
    /// \param name   Name of the uniform variable in GLSL
    /// \param matrix Value of the mat4 matrix
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Glsl::Mat4& matrix) const;

    ////////////////////////////////////////////////////////////
    /// \brief Specify a texture as \p sampler2D uniform
    ///
    /// \a name is the name of the variable to change in the shader.
    /// The corresponding parameter in the shader must be a 2D texture
    /// (\p sampler2D GLSL type).
    ///
    /// Example:
    /// \code
    /// uniform sampler2D the_texture; // this is the variable in the shader
    /// \endcode
    /// \code
    /// sf::Texture texture;
    /// ...
    /// shader.setUniform("the_texture", texture);
    /// \endcode
    /// It is important to note that `texture` must remain alive as long
    /// as the shader uses it, no copy is made internally.
    ///
    /// To use the texture of the object being drawn, which cannot be
    /// known in advance, you can pass the special value
    /// `sf::Shader::CurrentTexture`:
    /// \code
    /// shader.setUniform("the_texture", sf::Shader::CurrentTexture).
    /// \endcode
    ///
    /// \param name    Name of the texture in the shader
    /// \param texture Texture to assign
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setUniform(UniformLocation location, const Texture& texture) const;

    ////////////////////////////////////////////////////////////
    /// \brief Disallow setting from a temporary texture
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, const Texture&& texture) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Specify current texture as \p sampler2D uniform
    ///
    /// This overload maps a shader texture variable to the
    /// texture of the object being drawn, which cannot be
    /// known in advance. The second argument must be
    /// `sf::Shader::CurrentTexture`.
    /// The corresponding parameter in the shader must be a 2D texture
    /// (\p sampler2D GLSL type).
    ///
    /// Example:
    /// \code
    /// uniform sampler2D current; // this is the variable in the shader
    /// \endcode
    /// \code
    /// shader.setUniform("current", sf::Shader::CurrentTexture);
    /// \endcode
    ///
    /// \param name Name of the texture in the shader
    ///
    ////////////////////////////////////////////////////////////
    void setUniform(UniformLocation location, CurrentTextureType);

    ////////////////////////////////////////////////////////////
    /// \brief Specify values for \p float[] array uniform
    ///
    /// \param name        Name of the uniform variable in GLSL
    /// \param scalarArray pointer to array of \p float values
    /// \param length      Number of elements in the array
    ///
    ////////////////////////////////////////////////////////////
    void setUniformArray(UniformLocation location, const float* scalarArray, base::SizeT length);

    ////////////////////////////////////////////////////////////
    /// \brief Specify values for \p vec2[] array uniform
    ///
    /// \param name        Name of the uniform variable in GLSL
    /// \param vecArray pointer to array of \p vec2 values
    /// \param length      Number of elements in the array
    ///
    ////////////////////////////////////////////////////////////
    void setUniformArray(UniformLocation location, const Glsl::Vec2* vecArray, base::SizeT length);

    ////////////////////////////////////////////////////////////
    /// \brief Specify values for \p vec3[] array uniform
    ///
    /// \param name        Name of the uniform variable in GLSL
    /// \param vecArray pointer to array of \p vec3 values
    /// \param length      Number of elements in the array
    ///
    ////////////////////////////////////////////////////////////
    void setUniformArray(UniformLocation location, const Glsl::Vec3* vecArray, base::SizeT length);

    ////////////////////////////////////////////////////////////
    /// \brief Specify values for \p vec4[] array uniform
    ///
    /// \param name        Name of the uniform variable in GLSL
    /// \param vecArray pointer to array of \p vec4 values
    /// \param length      Number of elements in the array
    ///
    ////////////////////////////////////////////////////////////
    void setUniformArray(UniformLocation location, const Glsl::Vec4* vecArray, base::SizeT length);

    ////////////////////////////////////////////////////////////
    /// \brief Specify values for \p mat3[] array uniform
    ///
    /// \param name        Name of the uniform variable in GLSL
    /// \param matrixArray pointer to array of \p mat3 values
    /// \param length      Number of elements in the array
    ///
    ////////////////////////////////////////////////////////////
    void setUniformArray(UniformLocation location, const Glsl::Mat3* matrixArray, base::SizeT length);

    ////////////////////////////////////////////////////////////
    /// \brief Specify values for \p mat4[] array uniform
    ///
    /// \param name        Name of the uniform variable in GLSL
    /// \param matrixArray pointer to array of \p mat4 values
    /// \param length      Number of elements in the array
    ///
    ////////////////////////////////////////////////////////////
    void setUniformArray(UniformLocation location, const Glsl::Mat4* matrixArray, base::SizeT length);

    ////////////////////////////////////////////////////////////
    /// \brief Get the underlying OpenGL handle of the shader.
    ///
    /// You shouldn't need to use this function, unless you have
    /// very specific stuff to implement that SFML doesn't support,
    /// or implement a temporary workaround until a bug is fixed.
    ///
    /// \return OpenGL handle of the shader or 0 if not yet loaded
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getNativeHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Bind a shader for rendering
    ///
    /// This function is not part of the graphics API, it mustn't be
    /// used when drawing SFML entities. It must be used only if you
    /// mix `sf::Shader` with OpenGL code.
    ///
    /// \code
    /// sf::Shader s1, s2;
    /// ...
    /// sf::Shader::bind(&s1);
    /// // draw OpenGL stuff that use s1...
    /// sf::Shader::bind(&s2);
    /// // draw OpenGL stuff that use s2...
    /// sf::Shader::bind(nullptr);
    /// // draw OpenGL stuff that use no shader...
    /// \endcode
    ///
    /// \param shader Shader to bind, can be null to use no shader
    ///
    ////////////////////////////////////////////////////////////
    void bind() const;

    ////////////////////////////////////////////////////////////
    /// \brief Unbind any bound shader
    ///
    ////////////////////////////////////////////////////////////
    static void unbind();

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether or not the system supports geometry shaders
    ///
    /// This function should always be called before using
    /// the geometry shader features. If it returns `false`, then
    /// any attempt to use `sf::Shader` geometry shader features will fail.
    ///
    /// Note: The first call to this function, whether by your
    /// code or SFML will result in a context switch.
    ///
    /// \return `true` if geometry shaders are supported, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isGeometryAvailable();

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct from shader program
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Shader(base::PassKey<Shader>&&, unsigned int shaderProgram);

private:
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    /// \brief More efficient but less safe way of setting 4x4 matrix uniform
    ///
    ////////////////////////////////////////////////////////////
    void setMat4Uniform(UniformLocation location, const float* matrixPtr) const;

    ////////////////////////////////////////////////////////////
    /// \brief Compile the shader(s) and create the program
    ///
    /// If one of the arguments is a null pointer, the corresponding shader
    /// is not created.
    ///
    /// \param vertexShaderCode   Source code of the vertex shader
    /// \param geometryShaderCode Source code of the geometry shader
    /// \param fragmentShaderCode Source code of the fragment shader
    ///
    /// \return Shader on success, `base::nullOpt` if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Shader> compile(base::StringView vertexShaderCode,
                                                        base::StringView geometryShaderCode,
                                                        base::StringView fragmentShaderCode);

    ////////////////////////////////////////////////////////////
    /// \brief Bind all the textures used by the shader
    ///
    /// This function each texture to a different unit, and
    /// updates the corresponding variables in the shader accordingly.
    ///
    ////////////////////////////////////////////////////////////
    void bindTextures() const;

    ////////////////////////////////////////////////////////////
    /// \brief RAII object to save and restore the program
    ///        binding while uniforms are being set
    ///
    /// Implementation is private in the .cpp file.
    ///
    ////////////////////////////////////////////////////////////
    class [[nodiscard]] UniformBinder;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 192> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Shader
/// \ingroup graphics
///
/// Shaders are programs written using a specific language,
/// executed directly by the graphics card and allowing
/// to apply real-time operations to the rendered entities.
///
/// There are three kinds of shaders:
/// \li %Vertex shaders, that process vertices
/// \li Geometry shaders, that process primitives
/// \li Fragment (pixel) shaders, that process pixels
///
/// A `sf::Shader` can be composed of either a vertex shader
/// alone, a geometry shader alone, a fragment shader alone,
/// or any combination of them. (see the variants of the
/// load functions).
///
/// Shaders are written in GLSL, which is a C-like
/// language dedicated to OpenGL shaders. You'll probably
/// need to learn its basics before writing your own shaders
/// for SFML.
///
/// Like any C/C++ program, a GLSL shader has its own variables
/// called _uniforms_ that you can set from your C++ application.
/// `sf::Shader` handles different types of uniforms:
/// \li scalars: \p float, \p int, \p bool
/// \li vectors (2, 3 or 4 components)
/// \li matrices (3x3 or 4x4)
/// \li samplers (textures)
///
/// Some SFML-specific types can be converted:
/// \li `sf::Color` as a 4D vector (\p vec4)
/// \li `sf::Transform` as matrices (\p mat3 or \p mat4)
///
/// Every uniform variable in a shader can be set through one of the
/// `setUniform()` or `setUniformArray()` overloads. For example, if you
/// have a shader with the following uniforms:
/// \code
/// uniform float offset;
/// uniform vec3 point;
/// uniform vec4 color;
/// uniform mat4 matrix;
/// uniform sampler2D overlay;
/// uniform sampler2D current;
/// \endcode
/// You can set their values from C++ code as follows, using the types
/// defined in the `sf::Glsl` namespace:
/// \code
/// shader.setUniform("offset", 2.f);
/// shader.setUniform("point", sf::Vec3f(0.5f, 0.8f, 0.3f));
/// shader.setUniform("color", sf::Glsl::Vec4(color));          // color is a sf::Color
/// shader.setUniform("matrix", sf::Glsl::Mat4(transform));     // transform is a sf::Transform
/// shader.setUniform("overlay", texture);                      // texture is a sf::Texture
/// shader.setUniform("current", sf::Shader::CurrentTexture);
/// \endcode
///
/// The special `Shader::CurrentTexture` argument maps the
/// given \p sampler2D uniform to the current texture of the
/// object being drawn (which cannot be known in advance).
///
/// To apply a shader to a drawable, you must pass it as an
/// additional parameter to the `RenderWindow::draw` function:
/// \code
/// window.draw(sprite, {.shader = &shader});
/// \endcode
///
/// In the code above we pass a pointer to the shader, because it may
/// be null (which means "no shader").
///
/// Shaders can be used on any drawable, but some combinations are
/// not interesting. For example, using a vertex shader on a `sf::Sprite`
/// is limited because there are only 4 vertices, the sprite would
/// have to be subdivided in order to apply wave effects.
/// Another bad example is a fragment shader with `sf::Text`: the texture
/// of the text is not the actual text that you see on screen, it is
/// a big texture containing all the characters of the font in an
/// arbitrary order; thus, texture lookups on pixels other than the
/// current one may not give you the expected result.
///
/// Shaders can also be used to apply global post-effects to the
/// current contents of the target.
/// This can be done in two different ways:
/// \li draw everything to a `sf::RenderTexture`, then draw it to
///     the main target using the shader
/// \li draw everything directly to the main target, then use
///     `sf::Texture::update(Window&)` to copy its contents to a texture
///     and draw it to the main target using the shader
///
/// The first technique is more optimized because it doesn't involve
/// retrieving the target's pixels to system memory, but the
/// second one doesn't impact the rendering process and can be
/// easily inserted anywhere without impacting all the code.
///
/// Like `sf::Texture` that can be used as a raw OpenGL texture,
/// `sf::Shader` can also be used directly as a raw shader for
/// custom OpenGL geometry.
/// \code
/// sf::Shader::bind(&shader);
/// ... render OpenGL geometry ...
/// sf::Shader::bind(nullptr);
/// \endcode
///
/// \see `sf::Glsl`
///
////////////////////////////////////////////////////////////
