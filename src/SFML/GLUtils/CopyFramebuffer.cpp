// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/CopyFramebuffer.hpp"

#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/GLUtils/BlitFramebuffer.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Vec2.hpp"

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

} // namespace sf::priv
