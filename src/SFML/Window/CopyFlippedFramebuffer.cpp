#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Graphics/CopyFlippedFramebuffer.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/GLUtils.hpp"

#ifdef SFML_OPENGL_ES
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/System/Err.hpp"
#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
bool copyFlippedFramebuffer(
    [[maybe_unused]] GraphicsContext&         graphicsContext,
    [[maybe_unused]] base::Optional<Texture>& optTexture,
    [[maybe_unused]] bool                     srgb,
    Vector2u                                  size,
    unsigned int                              srcFBO,
    unsigned int                              dstFBO,
    Vector2u                                  srcPos,
    Vector2u                                  dstPos)
{
#ifndef SFML_OPENGL_ES

    priv::copyFramebuffer(/* invertYAxis */ true, size, srcFBO, dstFBO, srcPos, dstPos);
    return true;

#else

    if (!optTexture.hasValue())
    {
        if (!(optTexture = Texture::create(graphicsContext, size, srgb)))
        {
            priv::err() << "Failure to create intermediate texture in `copyFlippedFramebuffer`";
            return false;
        }
    }

    return copyFlippedFramebuffer(*optTexture, size, srcFBO, dstFBO, srcPos, dstPos);

#endif
}

////////////////////////////////////////////////////////////
bool copyFlippedFramebuffer([[maybe_unused]] Texture& tmpTexture,
                            Vector2u                  size,
                            unsigned int              srcFBO,
                            unsigned int              dstFBO,
                            Vector2u                  srcPos,
                            Vector2u                  dstPos)
{
#ifndef SFML_OPENGL_ES

    priv::copyFramebuffer(/* invertYAxis */ true, size, srcFBO, dstFBO, srcPos, dstPos);
    return true;

#else

    const GLuint intermediateFBO = priv::generateAndBindFramebuffer();
    if (!intermediateFBO)
    {
        priv::err() << "Failure to create intermediate FBO in `copyFlippedFramebuffer`";
        return false;
    }

    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmpTexture.getNativeHandle(), 0));

    if (glCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        priv::err() << "Failure to complete intermediate FBO in `copyFlippedFramebuffer`";
        return false;
    }

    priv::copyFramebuffer(/* invertYAxis */ false, size, srcFBO, intermediateFBO, srcPos, dstPos);
    priv::copyFramebuffer(/* invertYAxis */ true, size, intermediateFBO, dstFBO, srcPos, dstPos);

    glCheck(glDeleteFramebuffers(1, &intermediateFBO));
    return true;

#endif
}

} // namespace sf::priv
