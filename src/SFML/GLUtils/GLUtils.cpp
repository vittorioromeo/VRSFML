#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/GLUtils/FramebufferSaver.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"

#ifdef SFML_OPENGL_ES
    #include "SFML/System/Err.hpp"
#endif

#include "SFML/Base/Swap.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(const unsigned int parameterName)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    int result;

    glCheck(glGetIntegerv(parameterName, &result));
    return result;
}


////////////////////////////////////////////////////////////
void blitFramebuffer(const bool invertYAxis, const UIntRect src, const UIntRect dst)
{
    auto srcY0 = static_cast<GLint>(src.position.y);
    auto srcY1 = static_cast<GLint>(src.position.y + src.size.y);

    if (invertYAxis)
        base::swap(srcY0, srcY1);

    glCheck(glBlitFramebuffer(static_cast<GLint>(src.position.x),
                              srcY0,
                              static_cast<GLint>(src.position.x + src.size.x),
                              srcY1,
                              static_cast<GLint>(dst.position.x),
                              static_cast<GLint>(dst.position.y),
                              static_cast<GLint>(dst.position.x + dst.size.x),
                              static_cast<GLint>(dst.position.y + dst.size.y),
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST));
}


////////////////////////////////////////////////////////////
void blitFramebuffer(const bool invertYAxis, const Vec2u size, const Vec2u srcPos, const Vec2u dstPos)
{
    blitFramebuffer(invertYAxis, {srcPos, size}, {dstPos, size});
}


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


////////////////////////////////////////////////////////////
unsigned int generateAndBindFramebuffer()
{
    GLuint out{};
    glCheck(glGenFramebuffers(1, &out));

    if (out != 0u)
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, out));

    return out;
}


////////////////////////////////////////////////////////////
ScissorDisableGuard::ScissorDisableGuard() :
m_savedState([]
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    GLboolean scissorEnabled;
    glCheck(glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled));
    return scissorEnabled == GL_TRUE;
}())
{
    if (m_savedState)
        glCheck(glDisable(GL_SCISSOR_TEST));
}


////////////////////////////////////////////////////////////
ScissorDisableGuard::~ScissorDisableGuard()
{
    if (m_savedState)
        glCheck(glEnable(GL_SCISSOR_TEST));
}

} // namespace sf::priv
