#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Export.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Path;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Stateless helpers for GLSL shader preprocessing
///
/// Provides `#include "..."` directive support for GLSL
/// shaders loaded from files. Included files are resolved
/// relative to the including shader's directory.
///
/// `#line` directives are emitted so that OpenGL error
/// messages report the correct source line numbers.
///
////////////////////////////////////////////////////////////
class ZA_GRAPHICS_API ShaderUtils
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Emit a `#line N` directive into the buffer
    ///
    /// Appends `#line <lineNumber>\n` to `buffer`.
    ///
    /// \param buffer     Character buffer to append to
    /// \param lineNumber Line number to emit
    ///
    ////////////////////////////////////////////////////////////
    static void emitLineDirective(zb::Vector<char>& buffer, unsigned int lineNumber);

    ////////////////////////////////////////////////////////////
    /// \brief Parse a `#include "filename"` directive from a line
    ///
    /// \return `zb::nullOpt` if the line is a malformed `#include`,
    ///         an `Optional` holding an empty `StringView` if the
    ///         line is not an `#include` at all, or an `Optional`
    ///         holding the filename on success.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static zb::Optional<zb::StringView> parseIncludeDirective(zb::StringView line);

    ////////////////////////////////////////////////////////////
    /// \brief Preprocess `#include` directives in a GLSL shader source
    ///
    /// Recursively expands `#include "filename"` directives,
    /// resolving paths relative to \a shaderPath. Emits `#line`
    /// directives to preserve correct line numbers. Detects
    /// circular includes and enforces a maximum depth of 32.
    ///
    /// \param source     Shader source code
    /// \param shaderPath Path of the shader file (used as base for relative includes)
    /// \param output     Buffer that receives the preprocessed source
    ///
    /// \return `true` on success, `false` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool preprocessGlslIncludes(zb::StringView source, const Path& shaderPath, zb::Vector<char>& output);
};

} // namespace za
