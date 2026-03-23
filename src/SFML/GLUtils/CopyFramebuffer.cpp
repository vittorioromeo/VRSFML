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

#include "SFML/System/Err.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"


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
bool copyFlippedFramebufferViaIntermediateFBO(
    const unsigned int intermediateFBO,
    const unsigned int tmpTextureNativeHandle,
    const Vec2u        size,
    const unsigned int srcFBO,
    const unsigned int dstFBO,
    const Vec2u        srcPos,
    const Vec2u        dstPos)
{
#ifndef SFML_OPENGL_ES
    err() << "Should only be called on OpenGL ES";
    base::abort();
#endif

    SFML_BASE_ASSERT(intermediateFBO != 0u);
    SFML_BASE_ASSERT(tmpTextureNativeHandle != 0u);

    const FramebufferSaver framebufferSaver;

    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO));
    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmpTextureNativeHandle, 0));

    if (glCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        err() << "Failure to complete intermediate FBO in `copyFlippedFramebuffer`";
        return false;
    }

    copyFramebuffer(/* invertYAxis */ false, size, srcFBO, intermediateFBO, srcPos, dstPos);
    copyFramebuffer(/* invertYAxis */ true, size, intermediateFBO, dstFBO, srcPos, dstPos);
    return true;
}


////////////////////////////////////////////////////////////
void copyFlippedFramebufferViaDirectBlit(const Vec2u        size,
                                         const unsigned int srcFBO,
                                         const unsigned int dstFBO,
                                         const Vec2u        srcPos,
                                         const Vec2u        dstPos)
{
#ifdef SFML_OPENGL_ES
    err() << "Should only be called on desktop OpenGL";
    base::abort();
#endif

    const FramebufferSaver framebufferSaver;
    copyFramebuffer(/* invertYAxis */ true, size, srcFBO, dstFBO, srcPos, dstPos);
}

} // namespace sf::priv
