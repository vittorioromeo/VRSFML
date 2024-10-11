#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLUtils.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/Base/Algorithm.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(unsigned int parameterName)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    int result;

    glCheck(glGetIntegerv(parameterName, &result));
    return result;
}


////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, UIntRect src, UIntRect dst)
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
void blitFramebuffer(bool invertYAxis, Vector2u size, Vector2u srcPos, Vector2u dstPos)
{
    blitFramebuffer(invertYAxis, {srcPos, size}, {dstPos, size});
}


////////////////////////////////////////////////////////////
ScissorDisableGuard::ScissorDisableGuard() :
m_savedState(
    []
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
