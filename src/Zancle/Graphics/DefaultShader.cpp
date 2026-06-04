// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/DefaultShader.hpp"
#include "Zancle/Graphics/Shader.hpp"
#include "ZancleBase/Optional.hpp"


namespace za
{
////////////////////////////////////////////////////////////
[[nodiscard]] zb::Optional<Shader> DefaultShader::create()
{
    auto result = Shader::loadFromMemory({.vertexCode = srcVertex, .fragmentCode = srcFragment});

    if (result)
    {
        if (const zb::Optional ulTexture = result->getUniformLocation("za_u_texture"))
            result->setUniform(*ulTexture, Shader::CurrentTexture);
    }

    return result;
}

} // namespace za
