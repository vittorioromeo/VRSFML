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

#ifdef SFML_OPENGL_ES
    #include "SFML/GLUtils/TransferScratch.hpp"

    #include "SFML/System/Err.hpp"
#endif

#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"


namespace
{
#ifdef SFML_OPENGL_ES
////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents with vertical flipping using reusable scratch storage
///
/// Copies source framebuffer contents to destination while vertically flipping
/// the image. On OpenGL ES this uses a reusable intermediate texture/FBO pair.
///
/// \param sRgb   Whether the scratch texture should use sRGB storage
/// \param size   Dimensions of the region to copy
/// \param srcFBO Source framebuffer ID
/// \param dstFBO Destination framebuffer ID
/// \param srcPos Source region starting position (default: 0,0)
/// \param dstPos Destination region starting position (default: 0,0)
///
/// \return True if copy succeeded, false otherwise
///
////////////////////////////////////////////////////////////
bool copyFlippedFramebufferViaTransferScratch(
    const bool         sRgb,
    const sf::Vec2u    size,
    const unsigned int srcFBO,
    const unsigned int dstFBO,
    const sf::Vec2u    srcPos,
    const sf::Vec2u    dstPos)
{
    const unsigned int intermediateFBO        = sf::priv::getTransferScratchFlipFramebuffer();
    const unsigned int tmpTextureNativeHandle = sf::priv::ensureTransferScratchFlipTexture(size, sRgb);

    if (intermediateFBO == 0u || tmpTextureNativeHandle == 0u)
        return false;

    const sf::priv::FramebufferSaver framebufferSaver;

    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO));
    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmpTextureNativeHandle, 0));

    if (glCheck(glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        sf::priv::err() << "Failure to complete intermediate FBO in `copyFlippedFramebuffer`";
        return false;
    }

    sf::priv::copyFramebuffer(/* invertYAxis */ false, size, srcFBO, intermediateFBO, srcPos, {});
    sf::priv::copyFramebuffer(/* invertYAxis */ true, size, intermediateFBO, dstFBO, {}, dstPos);

    return true;
}
#endif


#ifndef SFML_OPENGL_ES
////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents with vertical flipping
///
/// Copies source framebuffer contents to destination while vertically flipping
/// the image.
///
/// \param size   Dimensions of the region to copy
/// \param srcFBO Source framebuffer ID
/// \param dstFBO Destination framebuffer ID
/// \param srcPos Source region starting position (default: 0,0)
/// \param dstPos Destination region starting position (default: 0,0)
///
////////////////////////////////////////////////////////////
void copyFlippedFramebufferViaDirectBlit(
    [[maybe_unused]] const bool sRgb,
    const sf::Vec2u             size,
    const unsigned int          srcFBO,
    const unsigned int          dstFBO,
    const sf::Vec2u             srcPos,
    const sf::Vec2u             dstPos)
{
    const sf::priv::FramebufferSaver framebufferSaver;
    sf::priv::copyFramebuffer(/* invertYAxis */ true, size, srcFBO, dstFBO, srcPos, dstPos);
}
#endif

} // namespace


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
bool copyFlippedFramebuffer(const bool         sRgb,
                            const Vec2u        size,
                            const unsigned int srcFBO,
                            const unsigned int dstFBO,
                            const Vec2u        srcPos,
                            const Vec2u        dstPos)
{
#ifdef SFML_OPENGL_ES
    return copyFlippedFramebufferViaTransferScratch(sRgb, size, srcFBO, dstFBO, srcPos, dstPos);
#else
    copyFlippedFramebufferViaDirectBlit(sRgb, size, srcFBO, dstFBO, srcPos, dstPos);
    return true;
#endif
}

} // namespace sf::priv
