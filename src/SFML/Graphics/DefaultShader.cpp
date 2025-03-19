#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/DefaultShader.hpp"
#include "SFML/Graphics/Shader.hpp"

#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
[[nodiscard]] base::Optional<Shader> DefaultShader::create()
{
    auto result = Shader::loadFromMemory(srcVertex, srcFragment);

    if (result)
    {
        if (const base::Optional ulTexture = result->getUniformLocation("sf_u_texture"))
            result->setUniform(*ulTexture, Shader::CurrentTexture);
    }

    return result;
}

} // namespace sf
