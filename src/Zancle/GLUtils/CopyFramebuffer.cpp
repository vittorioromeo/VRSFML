// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/CopyFramebuffer.hpp"

#include "Zancle/Config.hpp" // IWYU pragma: keep

#include "Zancle/GLUtils/BlitFramebuffer.hpp"
#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/Glad.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Assert.hpp"


namespace za::priv
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

    ZB_ASSERT(glCheck(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE);
    ZB_ASSERT(glCheck(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE);

    blitFramebuffer(invertYAxis, size, srcPos, dstPos);
}

} // namespace za::priv
