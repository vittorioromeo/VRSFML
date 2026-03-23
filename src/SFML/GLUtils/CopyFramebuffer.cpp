// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/CopyFramebuffer.hpp"

#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/GLUtils/BlitFramebuffer.hpp"
#include "SFML/GLUtils/FramebufferSaver.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"

#ifdef SFML_OPENGL_ES
    #include "SFML/GLUtils/GLUtils.hpp"

    #include "SFML/System/Err.hpp"
#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
void copyFramebuffer(const bool         invertYAxis,
                     const Vec2u        size,
                     const unsigned int srcFBO,
                     const unsigned int dstFBO,
                     const Vec2u        srcPos,
                     const Vec2u        dstPos)
{
    glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO));
    glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO));

    SFML_BASE_ASSERT(glCheck(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE);
    SFML_BASE_ASSERT(glCheck(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE);

    blitFramebuffer(invertYAxis, size, srcPos, dstPos);
}


////////////////////////////////////////////////////////////
bool copyFlippedFramebuffer([[maybe_unused]] const unsigned int tmpTextureNativeHandle,
                            const Vec2u                         size,
                            const unsigned int                  srcFBO,
                            const unsigned int                  dstFBO,
                            const Vec2u                         srcPos,
                            const Vec2u                         dstPos)
{
    const FramebufferSaver framebufferSaver;

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

    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmpTextureNativeHandle, 0));

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
