#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/Priv/ShaderBase.hpp" // IWYU pragma: export

#include "Zancle/System/Path.hpp" // IWYU pragma: export

#include "ZancleBase/Optional.hpp"   // IWYU pragma: export
#include "ZancleBase/StringView.hpp" // IWYU pragma: export


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
    zb::StringView vertexCode{}; //!< Source code of the vertex shader (empty to skip)   // NOLINT(readability-redundant-member-init)
    zb::StringView fragmentCode{}; //!< Source code of the fragment shader (empty to skip) // NOLINT(readability-redundant-member-init)
    zb::StringView geometryCode{}; //!< Source code of the geometry shader (empty to skip) // NOLINT(readability-redundant-member-init)
};

} // namespace za
