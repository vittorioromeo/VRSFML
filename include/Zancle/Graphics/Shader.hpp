#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Priv/ShaderBase.hpp" // IWYU pragma: export

#include "Zancle/IO/Path.hpp" // IWYU pragma: export

#include "Zancle/String/StringView.hpp" // IWYU pragma: export

#include "Zancle/Vocabulary/Optional.hpp" // IWYU pragma: export


namespace za
{
////////////////////////////////////////////////////////////
struct Shader::LoadFromFileSettings
{
    Path vertexPath{}; //!< Path of the vertex shader file (empty to skip)   // NOLINT(readability-redundant-member-init)
    Path fragmentPath{}; //!< Path of the fragment shader file (empty to skip) // NOLINT(readability-redundant-member-init)
    Path geometryPath{}; //!< Path of the geometry shader file (empty to skip) // NOLINT(readability-redundant-member-init)
};

////////////////////////////////////////////////////////////
struct Shader::LoadFromMemorySettings
{
    za::StringView vertexCode{}; //!< Source code of the vertex shader (empty to skip)   // NOLINT(readability-redundant-member-init)
    za::StringView fragmentCode{}; //!< Source code of the fragment shader (empty to skip) // NOLINT(readability-redundant-member-init)
    za::StringView geometryCode{}; //!< Source code of the geometry shader (empty to skip) // NOLINT(readability-redundant-member-init)
};

} // namespace za
