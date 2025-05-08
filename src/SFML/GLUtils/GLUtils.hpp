#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Get GL integer and assert success
///
////////////////////////////////////////////////////////////
[[nodiscard]] int getGLInteger(unsigned int parameterName);

////////////////////////////////////////////////////////////
/// \brief Blit a rectangular region between framebuffers with optional Y-axis inversion
///
/// This function copies a specified rectangular region from the read framebuffer
/// to the draw framebuffer, allowing vertical flipping through Y-axis inversion.
///
/// \param invertYAxis Whether to flip the source region vertically during copy
/// \param src         Source region coordinates and dimensions
/// \param dst         Destination region coordinates and dimensions
///
/// \note Uses `GL_NEAREST` filtering - appropriate for same-size copies.
///
////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, UIntRect src, UIntRect dst);

////////////////////////////////////////////////////////////
/// \brief Blit equal-sized regions between framebuffers with optional Y-axis inversion
///
/// This function copies a region of specified size from source position to destination
/// position from the read framebuffer to the draw framebuffers, allowing vertical flipping.
///
/// \param invertYAxis Whether to flip the source region vertically during copy
/// \param size        Dimensions of the region to copy
/// \param srcPos      Source region starting position
/// \param dstPos      Destination region starting position
///
/// \note Uses `GL_NEAREST` filtering - appropriate for same-size copies.
///
////////////////////////////////////////////////////////////
void blitFramebuffer(bool invertYAxis, Vec2u size, Vec2u srcPos, Vec2u dstPos);

////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents between specified FBOs
///
/// Binds the given source and destination framebuffer objects (FBOs) and performs
/// a blit operation between them with optional vertical flipping.
///
/// \param invertYAxis Whether to flip the source vertically during copy
/// \param size        Dimensions of the region to copy
/// \param srcFBO      Source framebuffer ID
/// \param dstFBO      Destination framebuffer ID
/// \param srcPos      Source region starting position (default: 0,0)
/// \param dstPos      Destination region starting position (default: 0,0)
///
////////////////////////////////////////////////////////////
void copyFramebuffer(bool         invertYAxis,
                     Vec2u        size,
                     unsigned int srcFBO,
                     unsigned int dstFBO,
                     Vec2u        srcPos = {0u, 0u},
                     Vec2u        dstPos = {0u, 0u});

////////////////////////////////////////////////////////////
/// \brief Copy framebuffer contents with vertical flipping
///
/// Copies source framebuffer contents to destination while vertically flipping
/// the image. Uses an intermediate framebuffer for OpenGL ES compatibility.
///
/// \param tmpTextureNativeHandle Temporary texture handle for intermediate operations (OpenGL ES only)
/// \param size                   Dimensions of the region to copy
/// \param srcFBO                 Source framebuffer ID
/// \param dstFBO                 Destination framebuffer ID
/// \param srcPos                 Source region starting position (default: 0,0)
/// \param dstPos                 Destination region starting position (default: 0,0)
///
/// \return True if copy succeeded, false otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool copyFlippedFramebuffer(
    unsigned int tmpTextureNativeHandle,
    Vec2u        size,
    unsigned int srcFBO,
    unsigned int dstFBO,
    Vec2u        srcPos = {0u, 0u},
    Vec2u        dstPos = {0u, 0u});

////////////////////////////////////////////////////////////
/// \brief Generate and bind a new framebuffer object
///
/// Creates a new OpenGL framebuffer object, binds it to the current context,
/// and returns its ID. Returns 0 if framebuffer creation failed.
///
/// \return New framebuffer ID or 0 on failure
///
////////////////////////////////////////////////////////////
[[nodiscard]] unsigned int generateAndBindFramebuffer();

////////////////////////////////////////////////////////////
/// \brief RAII guard for temporary scissor test disabling
///
/// Automatically disables scissor test on construction and restores
/// the original scissor test state on destruction.
///
////////////////////////////////////////////////////////////
class ScissorDisableGuard
{
public:
    explicit ScissorDisableGuard();
    ~ScissorDisableGuard();

    ScissorDisableGuard(const ScissorDisableGuard&) = delete;
    ScissorDisableGuard(ScissorDisableGuard&&)      = delete;

    ScissorDisableGuard& operator=(const ScissorDisableGuard&) = delete;
    ScissorDisableGuard& operator=(ScissorDisableGuard&&)      = delete;

private:
    bool m_savedState;
};

} // namespace sf::priv
