// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/BlitFramebuffer.hpp"
#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/Glad.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"
#include "ZancleBase/Swap.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
void blitFramebuffer(const bool invertYAxis, const Rect2u src, const Rect2u dst)
{
    auto srcY0 = static_cast<GLint>(src.position.y);
    auto srcY1 = static_cast<GLint>(src.position.y + src.size.y);

    if (invertYAxis)
        zb::genericSwap(srcY0, srcY1);

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

} // namespace za::priv
