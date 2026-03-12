// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/BlitFramebuffer.hpp"

#include "SFML/Config.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Base/Swap.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
void blitFramebuffer(const bool invertYAxis, const Rect2u src, const Rect2u dst)
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

} // namespace sf::priv
