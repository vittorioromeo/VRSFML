#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Priv/ShaderBase.hpp" // IWYU pragma: export

#include "SFML/System/Path.hpp" // IWYU pragma: export

#include "SFML/Base/Optional.hpp"   // IWYU pragma: export
#include "SFML/Base/StringView.hpp" // IWYU pragma: export


namespace sf
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
    base::StringView vertexCode{}; //!< Source code of the vertex shader (empty to skip)   // NOLINT(readability-redundant-member-init)
    base::StringView fragmentCode{}; //!< Source code of the fragment shader (empty to skip) // NOLINT(readability-redundant-member-init)
    base::StringView geometryCode{}; //!< Source code of the geometry shader (empty to skip) // NOLINT(readability-redundant-member-init)
};

} // namespace sf
