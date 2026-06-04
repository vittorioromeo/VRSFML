// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/DefaultShader.hpp"

#include "Zancle/Graphics/Shader.hpp"

#include "ZancleBase/Optional.hpp"


namespace za
{
////////////////////////////////////////////////////////////
[[nodiscard]] base::Optional<Shader> DefaultShader::create()
{
    auto result = Shader::loadFromMemory({.vertexCode = srcVertex, .fragmentCode = srcFragment});

    if (result)
    {
        if (const base::Optional ulTexture = result->getUniformLocation("za_u_texture"))
            result->setUniform(*ulTexture, Shader::CurrentTexture);
    }

    return result;
}

} // namespace za
