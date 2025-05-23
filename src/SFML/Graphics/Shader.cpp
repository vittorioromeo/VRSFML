#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/DefaultShader.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLSharedContextGuard.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Vec2.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"


using GLhandle = GLuint;

#if defined(SFML_SYSTEM_MACOS) || defined(SFML_SYSTEM_IOS)

    #include "SFML/Base/PtrDiffT.hpp"

    #define castToGlHandle(x)   reinterpret_cast<GLEXT_GLhandle>(::sf::base::PtrDiffT{x})
    #define castFromGlHandle(x) static_cast<unsigned int>(reinterpret_cast<::sf::base::PtrDiffT>(x))

#else

    #define castToGlHandle(x)   (x)
    #define castFromGlHandle(x) (x)

#endif


namespace
{
////////////////////////////////////////////////////////////
// Retrieve the maximum number of texture units available
[[nodiscard]] sf::base::SizeT getMaxTextureUnits()
{
    static const auto maxUnits = static_cast<sf::base::SizeT>(sf::priv::getGLInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS));
    return maxUnits;
}


////////////////////////////////////////////////////////////
// Pair of indices into thread-local buffer
struct [[nodiscard]] BufferSlice
{
    sf::base::SizeT beginIdx;
    sf::base::SizeT count;

    [[nodiscard]] explicit BufferSlice(sf::base::SizeT b, sf::base::SizeT c) : beginIdx(b), count(c)
    {
    }

    [[nodiscard]] sf::base::StringView toView(const sf::base::Vector<char>& buffer) const
    {
        return {buffer.data() + beginIdx, count};
    }
};


////////////////////////////////////////////////////////////
// Read the contents of a file into an array of char
[[nodiscard]] sf::base::Optional<BufferSlice> appendFileContentsToVector(const sf::Path& filename, sf::base::Vector<char>& buffer)
{
    sf::InFileStream file(filename.c_str(), sf::FileOpenMode::bin);

    if (!file)
    {
        sf::priv::err() << "Failed to open shader file";
        return sf::base::nullOpt;
    }

    file.seekg(0, sf::SeekDir::end);
    const auto size = file.tellg();

    const sf::base::SizeT bufferSizeBeforeRead = buffer.size();

    if (size > 0)
    {
        file.seekg(0, sf::SeekDir::beg);
        buffer.resize(static_cast<sf::base::SizeT>(size) + bufferSizeBeforeRead);
        file.read(buffer.data() + bufferSizeBeforeRead, static_cast<sf::base::PtrDiffT>(size));
    }

    buffer.pushBack('\0');
    return sf::base::makeOptional<BufferSlice>(bufferSizeBeforeRead, buffer.size() - bufferSizeBeforeRead);
}


////////////////////////////////////////////////////////////
// Read the contents of a stream into an array of char
[[nodiscard]] sf::base::Optional<BufferSlice> appendStreamContentsToVector(sf::InputStream&        stream,
                                                                           sf::base::Vector<char>& buffer)
{
    const sf::base::Optional<sf::base::SizeT> size = stream.getSize();

    if (!size.hasValue() || size.value() == 0)
    {
        buffer.pushBack('\0');
        return sf::base::nullOpt;
    }

    const sf::base::SizeT bufferSizeBeforeRead = buffer.size();
    buffer.resize(*size + bufferSizeBeforeRead);

    if (!stream.seek(0).hasValue())
    {
        sf::priv::err() << "Failed to seek shader stream";
        return sf::base::nullOpt;
    }

    const sf::base::Optional<sf::base::SizeT> read = stream.read(buffer.data() + bufferSizeBeforeRead, *size);

    if (!read.hasValue() || *read != *size)
    {
        sf::priv::err() << "Failed to read stream contents into buffer";
        return sf::base::nullOpt;
    }

    buffer.pushBack('\0');
    return sf::base::makeOptional<BufferSlice>(bufferSizeBeforeRead, buffer.size() - bufferSizeBeforeRead);
}


////////////////////////////////////////////////////////////
// Return a thread-local vector for suitable use as a temporary buffer
// This function is non-reentrant
[[nodiscard]] sf::base::Vector<char>& getThreadLocalCharBuffer()
{
    static thread_local sf::base::Vector<char> result;
    return result;
}


////////////////////////////////////////////////////////////
// Transforms an array of vec2s into a contiguous array of scalars
[[nodiscard]] sf::base::Vector<float> flatten(const sf::Vec2f* vecArray, sf::base::SizeT length)
{
    constexpr sf::base::SizeT vecSize = 2u;

    sf::base::Vector<float> contiguous(vecSize * length);

    for (sf::base::SizeT i = 0; i < length; ++i)
    {
        contiguous[vecSize * i]     = vecArray[i].x;
        contiguous[vecSize * i + 1] = vecArray[i].y;
    }

    return contiguous;
}


////////////////////////////////////////////////////////////
// Transforms an array of vec3s into a contiguous array of scalars
[[nodiscard]] sf::base::Vector<float> flatten(const sf::Vec3f* vecArray, sf::base::SizeT length)
{
    constexpr sf::base::SizeT vecSize = 3u;

    sf::base::Vector<float> contiguous(vecSize * length);

    for (sf::base::SizeT i = 0; i < length; ++i)
    {
        contiguous[vecSize * i]     = vecArray[i].x;
        contiguous[vecSize * i + 1] = vecArray[i].y;
        contiguous[vecSize * i + 2] = vecArray[i].z;
    }

    return contiguous;
}


////////////////////////////////////////////////////////////
// Transforms an array of vec4s into a contiguous array of scalars
[[nodiscard]] sf::base::Vector<float> flatten(const sf::Glsl::Vec4* vecArray, sf::base::SizeT length)
{
    constexpr sf::base::SizeT vecSize = 4u;

    sf::base::Vector<float> contiguous(vecSize * length);

    for (sf::base::SizeT i = 0; i < length; ++i)
    {
        contiguous[vecSize * i]     = vecArray[i].x;
        contiguous[vecSize * i + 1] = vecArray[i].y;
        contiguous[vecSize * i + 2] = vecArray[i].z;
        contiguous[vecSize * i + 3] = vecArray[i].w;
    }

    return contiguous;
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::StringView adjustPreamble(sf::base::StringView src)
{
    constexpr sf::base::StringView preamble{
#if defined(SFML_SYSTEM_EMSCRIPTEN)

        // Emscripten/WebGL always requires `#version 300 es` and precision
        R"glsl(#version 300 es

precision highp float;

)glsl"

#elif defined(SFML_OPENGL_ES)

        // Desktop/mobile GLES can use `#version 310 es` and precision
        R"glsl(#version 310 es

precision highp float;

)glsl"

#else

        // Desktop GL can use `#version 430 core`
        R"glsl(#version 430 core

)glsl"

#endif
    };

    static thread_local sf::base::Vector<char> buffer; // Cannot reuse the other buffer here
    buffer.clear();

    buffer.emplaceRange(preamble.data(), preamble.size() - 1);
    buffer.emplaceRange(src.data(), src.size() - 1);

    return {buffer.data(), buffer.size()};
}


////////////////////////////////////////////////////////////
void printLinesWithNumbers(auto& os, sf::base::StringView text)
{
    sf::base::SizeT lineStart  = 0u;
    sf::base::SizeT lineNumber = 1u;

    while (lineStart < text.size())
    {
        // Find the position of the next newline character.
        sf::base::SizeT newlinePos = text.find('\n', lineStart);
        if (newlinePos == sf::base::StringView::nPos)
        {
            // Print the remaining text as the final line.
            os << lineNumber << " | " << text.substrByPosLen(lineStart) << "\n";
            break;
        }

        // Print the current line with its line number and pipe separator.
        os << lineNumber << " | " << text.substrByPosLen(lineStart, newlinePos - lineStart) << "\n";
        lineStart = newlinePos + 1; // Move past the newline.
        ++lineNumber;
    }
}


////////////////////////////////////////////////////////////
void destroyProgramIfNeeded(const unsigned int program)
{
    if (!program)
        return;

    // Always delete programs and shaders on the shared context
    sf::priv::GLSharedContextGuard guard;

    SFML_BASE_ASSERT(sf::GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(glCheck(glIsProgram(castToGlHandle(program))));
    glCheck(glDeleteProgram(castToGlHandle(program)));
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Shader::Impl
{
    unsigned int shaderProgram{};    //!< OpenGL identifier for the program
    int          currentTexture{-1}; //!< Location of the current texture in the shader

    // TODO P1: protect with mutex? Change API?
    mutable ankerl::unordered_dense::map<int, const Texture*> textures; //!< Texture variables in the shader, mapped to their location

    explicit Impl(unsigned int theShaderProgram) : shaderProgram(theShaderProgram)
    {
    }

    explicit Impl(Impl&& rhs) noexcept :
    shaderProgram(base::exchange(rhs.shaderProgram, 0u)),
    currentTexture(base::exchange(rhs.currentTexture, -1)),
    textures(SFML_BASE_MOVE(rhs.textures))
    {
    }
};


////////////////////////////////////////////////////////////
Shader::UniformLocation::UniformLocation(int location) : m_value(location)
{
    SFML_BASE_ASSERT(m_value != -1);
}


////////////////////////////////////////////////////////////
class [[nodiscard]] Shader::UniformBinder
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor: set up state before uniform is set
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] explicit UniformBinder(unsigned int shaderProgram) :
    m_currentProgram(static_cast<GLhandle>(castToGlHandle(shaderProgram)))
    {
        SFML_BASE_ASSERT(m_currentProgram != 0);

        // Enable program object
        glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&m_savedProgram)));

        if (m_currentProgram != m_savedProgram)
            glCheck(glUseProgram(m_currentProgram));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destructor: restore state after uniform is set
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~UniformBinder()
    {
        // Disable program object
        if (m_currentProgram && (m_currentProgram != m_savedProgram))
            glCheck(glUseProgram(m_savedProgram));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    UniformBinder(const UniformBinder&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    UniformBinder& operator=(const UniformBinder&) = delete;

private:
    GLhandle m_currentProgram; //!< Handle to the program object of the modified `sf::Shader` instance
    GLhandle m_savedProgram{}; //!< Handle to the previously active program object
};


////////////////////////////////////////////////////////////
Shader::~Shader()
{
    destroyProgramIfNeeded(m_impl->shaderProgram);
}


////////////////////////////////////////////////////////////
Shader::Shader(Shader&& source) noexcept = default;


////////////////////////////////////////////////////////////
Shader& Shader::operator=(Shader&& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    destroyProgramIfNeeded(m_impl->shaderProgram);

    // Move the contents of rhs.
    m_impl->shaderProgram  = base::exchange(rhs.m_impl->shaderProgram, 0u);
    m_impl->currentTexture = base::exchange(rhs.m_impl->currentTexture, -1);
    m_impl->textures       = SFML_BASE_MOVE(rhs.m_impl->textures);

    return *this;
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromFile(const LoadFromFileSettings& settings)
{
    // Prepare thread-local buffer
    base::Vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Helper function
    const auto readIntoBufferSlice =
        [&](const char* typeStr, const Path& optPath, base::Optional<BufferSlice>& optBufferSlice)
    {
        if (optPath.empty())
            return true;

        optBufferSlice = appendFileContentsToVector(optPath, buffer);
        if (!optBufferSlice.hasValue())
        {
            priv::err() << "Failed to open " << typeStr << " shader file\n" << priv::PathDebugFormatter{optPath};
            return false;
        }

        return true;
    };

    // Read the vertex shader file (if path provided)
    base::Optional<BufferSlice> vertexShaderSlice;
    if (!readIntoBufferSlice("vertex", settings.vertexPath, vertexShaderSlice))
        return base::nullOpt;

    // Read the geometry shader file (if path provided)
    base::Optional<BufferSlice> geometryShaderSlice;
    if (!readIntoBufferSlice("geometry", settings.geometryPath, geometryShaderSlice))
        return base::nullOpt;

    // Read the fragment shader file (if path provided)
    base::Optional<BufferSlice> fragmentShaderSlice;
    if (!readIntoBufferSlice("fragment", settings.fragmentPath, fragmentShaderSlice))
        return base::nullOpt;

    return compile(vertexShaderSlice.hasValue() ? vertexShaderSlice->toView(buffer) : base::StringView{},
                   geometryShaderSlice.hasValue() ? geometryShaderSlice->toView(buffer) : base::StringView{},
                   fragmentShaderSlice.hasValue() ? fragmentShaderSlice->toView(buffer) : base::StringView{});
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromMemory(const LoadFromMemorySettings& settings)
{
    return compile(settings.vertexCode, settings.geometryCode, settings.fragmentCode);
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromStream(const LoadFromStreamSettings& settings)
{
    // Prepare thread-local buffer
    base::Vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Helper function
    const auto readIntoBufferSlice =
        [&](const char* typeStr, InputStream* optStream, base::Optional<BufferSlice>& optBufferSlice)
    {
        if (optStream == nullptr)
            return true;

        optBufferSlice = appendStreamContentsToVector(*optStream, buffer);
        if (!optBufferSlice.hasValue())
        {
            priv::err() << "Failed to open " << typeStr << " shader from stream\n";
            return false;
        }

        return true;
    };

    // Read the vertex shader code from the stream
    base::Optional<BufferSlice> vertexShaderSlice;
    if (!readIntoBufferSlice("vertex", settings.vertexStream, vertexShaderSlice))
        return base::nullOpt;

    // Read the geometry shader code from the stream
    base::Optional<BufferSlice> geometryShaderSlice;
    if (!readIntoBufferSlice("geometry", settings.geometryStream, geometryShaderSlice))
        return base::nullOpt;

    // Read the fragment shader code from the stream
    base::Optional<BufferSlice> fragmentShaderSlice;
    if (!readIntoBufferSlice("fragment", settings.fragmentStream, fragmentShaderSlice))
        return base::nullOpt;

    return compile(vertexShaderSlice.hasValue() ? vertexShaderSlice->toView(buffer) : base::StringView{},
                   geometryShaderSlice.hasValue() ? geometryShaderSlice->toView(buffer) : base::StringView{},
                   fragmentShaderSlice.hasValue() ? fragmentShaderSlice->toView(buffer) : base::StringView{});
}


////////////////////////////////////////////////////////////
base::Optional<Shader::UniformLocation> Shader::getUniformLocation(base::StringView uniformName) const
{
    enum : base::SizeT
    {
        maxUniformNameLength = 256
    };

    SFML_BASE_ASSERT(uniformName.size() < maxUniformNameLength && "Uniform name too long");

    // To get a a null-terminated string
    char uniformNameBuffer[maxUniformNameLength];
    SFML_BASE_MEMCPY(uniformNameBuffer, uniformName.data(), uniformName.size());
    uniformNameBuffer[uniformName.size()] = '\0';

    // Request the location from OpenGL
    const int location = glGetUniformLocation(castToGlHandle(m_impl->shaderProgram), uniformNameBuffer);
    return location == -1 ? base::nullOpt : base::makeOptional(UniformLocation{location});
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, float x) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform1f(location.m_value, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, Glsl::Vec2 v) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform2f(location.m_value, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Vec3& v) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform3f(location.m_value, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Vec4& v) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform4f(location.m_value, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, int x) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform1i(location.m_value, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, Glsl::Ivec2 v) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform2i(location.m_value, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Ivec3& v) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform3i(location.m_value, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Ivec4& v) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform4i(location.m_value, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, bool x) const
{
    setUniform(location, static_cast<int>(x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, Glsl::Bvec2 v) const
{
    setUniform(location, v.to<Glsl::Ivec2>());
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Bvec3& v) const
{
    setUniform(location, v.to<Glsl::Ivec3>());
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Bvec4& v) const
{
    setUniform(location, Glsl::Ivec4(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Mat3& matrix) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniformMatrix3fv(location.m_value, 1, GL_FALSE, matrix.array));
}


////////////////////////////////////////////////////////////
void Shader::setMat4Uniform(UniformLocation location, const float* matrixPtr) const
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniformMatrix4fv(location.m_value, 1, GL_FALSE, matrixPtr));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Mat4& matrix) const
{
    setMat4Uniform(location, matrix.array);
}


////////////////////////////////////////////////////////////
bool Shader::setUniform(UniformLocation location, const Texture& texture) const
{
    SFML_BASE_ASSERT(m_impl->shaderProgram);
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());

    // Store the location -> texture mapping
    if (auto* const it = m_impl->textures.find(location.m_value); it != m_impl->textures.end())
    {
        // Location already used, just replace the texture
        it->second = &texture;
        return true;
    }

    // New entry, make sure there are enough texture units
    if (m_impl->textures.size() + 1 >= getMaxTextureUnits())
    {
        priv::err() << "Impossible to use texture \"" << location.m_value << '"'
                    << " \"for shader: all available texture units are used";

        return false;
    }

    m_impl->textures[location.m_value] = &texture;
    return true;
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, CurrentTextureType)
{
    SFML_BASE_ASSERT(m_impl->shaderProgram);
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());

    // Find the location of the variable in the shader
    m_impl->currentTexture = location.m_value;
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const float* scalarArray, base::SizeT length)
{
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform1fv(location.m_value, static_cast<GLsizei>(length), scalarArray));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Vec2* vecArray, base::SizeT length)
{
    base::Vector<float> contiguous = flatten(vecArray, length);
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform2fv(location.m_value, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Vec3* vecArray, base::SizeT length)
{
    base::Vector<float> contiguous = flatten(vecArray, length);
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform3fv(location.m_value, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Vec4* vecArray, base::SizeT length)
{
    base::Vector<float> contiguous = flatten(vecArray, length);
    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniform4fv(location.m_value, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Mat3* matrixArray, base::SizeT length)
{
    const base::SizeT matrixSize = 3 * 3;

    base::Vector<float> contiguous(matrixSize * length);

    for (base::SizeT i = 0; i < length; ++i)
        priv::copyMatrix(matrixArray[i].array, matrixSize, &contiguous[matrixSize * i]);

    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniformMatrix3fv(location.m_value, static_cast<GLsizei>(length), GL_FALSE, contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Mat4* matrixArray, base::SizeT length)
{
    const base::SizeT matrixSize = 4 * 4;

    base::Vector<float> contiguous(matrixSize * length);

    for (base::SizeT i = 0; i < length; ++i)
        priv::copyMatrix(matrixArray[i].array, matrixSize, &contiguous[matrixSize * i]);

    const UniformBinder binder{m_impl->shaderProgram};
    glCheck(glUniformMatrix4fv(location.m_value, static_cast<GLsizei>(length), GL_FALSE, contiguous.data()));
}


////////////////////////////////////////////////////////////
unsigned int Shader::getNativeHandle() const
{
    return m_impl->shaderProgram;
}


////////////////////////////////////////////////////////////
void Shader::bind() const
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(m_impl->shaderProgram != 0u);

    // Enable the program
    SFML_BASE_ASSERT(glCheck(glIsProgram(castToGlHandle(m_impl->shaderProgram))));
    glCheck(glUseProgram(castToGlHandle(m_impl->shaderProgram)));

    // Bind the textures
    bindTextures();

    // Bind the current texture
    if (m_impl->currentTexture != -1)
        glCheck(glUniform1i(m_impl->currentTexture, 0));
}


////////////////////////////////////////////////////////////
void Shader::unbind()
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    glCheck(glUseProgram({}));
}


////////////////////////////////////////////////////////////
bool Shader::isGeometryAvailable()
{
#ifdef SFML_OPENGL_ES
    return false;
#else
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    return GL_VERSION_3_2;
#endif
}


////////////////////////////////////////////////////////////
Shader::Shader(base::PassKey<Shader>&&, unsigned int shaderProgram) : m_impl(shaderProgram)
{
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::compile(base::StringView vertexShaderCode,
                                       base::StringView geometryShaderCode,
                                       base::StringView fragmentShaderCode)
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());

    // Make sure we can use geometry shaders
    if (geometryShaderCode.data() != nullptr && !isGeometryAvailable())
    {
        priv::err() << "Failed to create a shader: your system doesn't support geometry shaders "
                    << "(you should test Shader::isGeometryAvailable() before trying to use geometry shaders)";

        return base::nullOpt;
    }

    // Always create programs and shaders on the shared context
    priv::GLSharedContextGuard guard;

    // Create the program
    const GLhandle shaderProgram = glCheck(glCreateProgram());
    SFML_BASE_ASSERT(glCheck(glIsProgram(shaderProgram)));

    const auto makeShader = [&](GLenum type, const char* typeStr, base::StringView shaderCode)
    {
        // Add `#version` (and float precision if required)
        const auto adjustedShaderCode = adjustPreamble(shaderCode);

        const GLhandle shader = glCheck(glCreateShader(type));

        const GLcharARB* sourceCode       = adjustedShaderCode.data();
        const auto       sourceCodeLength = static_cast<GLint>(adjustedShaderCode.size());

        glCheck(glShaderSource(shader, 1, &sourceCode, &sourceCodeLength));
        glCheck(glCompileShader(shader));
        SFML_BASE_ASSERT(glCheck(glIsShader(shader)));

        // Check the compile log
        GLint success = 0;
        glCheck(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024]{};
            glCheck(glGetShaderInfoLog(shader, sizeof(log), nullptr, log));

            priv::err() << "Failed to compile " << typeStr << " shader:" << '\n'
                        << static_cast<const char*>(log) << "\n\nSource code:\n";

            printLinesWithNumbers(priv::err(/* multiLine */ true), adjustedShaderCode);

            glCheck(glDeleteShader(shader));
            glCheck(glDeleteProgram(shaderProgram));

            return false;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(glAttachShader(shaderProgram, shader));
        glCheck(glDeleteShader(shader));

        return true;
    };

    if (vertexShaderCode.data() == nullptr)
        vertexShaderCode = DefaultShader::srcVertex;

    if (!makeShader(GL_VERTEX_SHADER, "vertex", vertexShaderCode))
        return base::nullOpt;


    // Create the geometry shader if needed
    if (geometryShaderCode.data())
    {
        if (!makeShader(GL_GEOMETRY_SHADER, "geometry", geometryShaderCode))
            return base::nullOpt;
    }

    if (fragmentShaderCode.data() == nullptr)
        fragmentShaderCode = DefaultShader::srcFragment;

    // Create the fragment shader
    if (!makeShader(GL_FRAGMENT_SHADER, "fragment", fragmentShaderCode))
        return base::nullOpt;

    // Link the program
    glCheck(glLinkProgram(shaderProgram));

    // Check the link log
    GLint success = 0;
    glCheck(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
    if (success == GL_FALSE)
    {
        char log[1024]{};
        glCheck(glGetProgramInfoLog(shaderProgram, sizeof(log), nullptr, log));
        priv::err() << "Failed to link shader:" << '\n'
                    << static_cast<const char*>(log) << "VERTEX SOURCE:\n"
                    << vertexShaderCode << "\n\nFRAGMENT SOURCE:\n"
                    << fragmentShaderCode << "\n\nGEOMETRY SOURCE:\n"
                    << geometryShaderCode;
        glCheck(glDeleteProgram(shaderProgram));
        return base::nullOpt;
    }

    // Force an OpenGL flush, so that the shader will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());

    return base::makeOptional<Shader>(base::PassKey<Shader>{}, castFromGlHandle(shaderProgram));
}


////////////////////////////////////////////////////////////
void Shader::bindTextures() const
{
    auto* it = m_impl->textures.begin();
    for (base::SizeT i = 0u; i < m_impl->textures.size(); ++i)
    {
        const auto index = static_cast<GLsizei>(i + 1);
        glCheck(glUniform1i(it->first, index));
        glCheck(glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(index)));
        it->second->bind();
        ++it;
    }

    // Make sure that the texture unit which is left active is the number 0
    glCheck(glActiveTexture(GL_TEXTURE0));
}

} // namespace sf

// TODO P2: add support for `#include` in shaders
