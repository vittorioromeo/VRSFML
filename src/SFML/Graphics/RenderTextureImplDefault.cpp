#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/RenderTextureImplDefault.hpp>
#include <SFML/Graphics/TextureSaver.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GlContext.hpp>

#include <SFML/Base/UniquePtr.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
RenderTextureImplDefault::RenderTextureImplDefault(GraphicsContext& graphicsContext) :
m_graphicsContext(&graphicsContext)
{
}


////////////////////////////////////////////////////////////
RenderTextureImplDefault::~RenderTextureImplDefault() = default;


////////////////////////////////////////////////////////////
RenderTextureImplDefault::RenderTextureImplDefault(RenderTextureImplDefault&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTextureImplDefault& RenderTextureImplDefault::operator=(RenderTextureImplDefault&&) noexcept = default;


////////////////////////////////////////////////////////////
unsigned int RenderTextureImplDefault::getMaximumAntialiasingLevel()
{
    // If the system is so old that it doesn't support FBOs, chances are it is
    // also using either a software renderer or some CPU emulated support for AA
    // In order to not cripple performance in this rare case, we just return 0 here
    return 0;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplDefault::create(Vector2u size, unsigned int, const ContextSettings& contextSettings)
{
    // Store the dimensions
    m_size = size;

    // Create the in-memory OpenGL context
    m_glContext = m_graphicsContext->createGlContext(contextSettings, size);
    SFML_BASE_ASSERT(m_glContext != nullptr);

    return true;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplDefault::activate(bool active)
{
    return m_graphicsContext->setActiveThreadLocalGlContext(*m_glContext, active);
}


////////////////////////////////////////////////////////////
bool RenderTextureImplDefault::isSrgb() const
{
    return m_glContext->getSettings().sRgbCapable;
}


////////////////////////////////////////////////////////////
void RenderTextureImplDefault::updateTexture(unsigned int textureId) const
{
    // Make sure that the current texture binding will be preserved
    const TextureSaver save;

    // Copy the rendered pixels to the texture
    glCheck(glBindTexture(GL_TEXTURE_2D, textureId));
    glCheck(
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, static_cast<GLsizei>(m_size.x), static_cast<GLsizei>(m_size.y)));
}

} // namespace sf::priv
