// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowHandle.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(base::PassKey<RenderWindow>&&, Window&& window) : Window(SFML_BASE_MOVE(window))
{
    // Retrieve the framebuffer ID we have to bind when targeting the window for rendering
    // We assume that this window's context is still active at this point
    glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&m_defaultFrameBuffer)));
}


////////////////////////////////////////////////////////////
base::Optional<RenderWindow> RenderWindow::create(const Settings& windowSettings)
{
    auto window = Window::create(windowSettings);

    return window.hasValue()
               ? base::Optional<RenderWindow>(base::inPlace, base::PassKey<RenderWindow>{}, SFML_BASE_MOVE(*window))
               : base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<RenderWindow> RenderWindow::create(const WindowHandle handle, const ContextSettings& contextSettings)
{
    auto window = Window::create(handle, contextSettings);

    return window.hasValue()
               ? base::Optional<RenderWindow>(base::inPlace, base::PassKey<RenderWindow>{}, SFML_BASE_MOVE(*window))
               : base::nullOpt;
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(RenderWindow&&) noexcept            = default;
RenderWindow& RenderWindow::operator=(RenderWindow&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderWindow::~RenderWindow()
{
    if (isMovedFrom())
        return;

    // Need to activate window context during destruction to avoid GL errors
    [[maybe_unused]] const bool rc = setActive(true);
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
Vec2u RenderWindow::getSize() const
{
    return Window::getSize();
}


////////////////////////////////////////////////////////////
void RenderWindow::setIcon(const Image& icon)
{
    setIcon(icon.getPixelsPtr(), icon.getSize());
}


////////////////////////////////////////////////////////////
bool RenderWindow::isSrgb() const
{
    return false;
}


////////////////////////////////////////////////////////////
bool RenderWindow::setActive(bool active)
{
    bool result = Window::setActive(active);

    // Update RenderTarget tracking
    if (result)
        result = RenderTarget::setActive(active);

    // If FBOs are available, make sure none are bound when we
    // try to draw to the default framebuffer of the RenderWindow
    if (active && result)
    {
        // TODO P2: many redundant calls, but this is needed when using rendertextures
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFrameBuffer));
        return true;
    }

    return result;
}


////////////////////////////////////////////////////////////
RenderTarget::DrawStatistics RenderWindow::display()
{
    const auto result = RenderTarget::flush();
    RenderTarget::syncGPUEndFrame();
    Window::display();
    return result;
}

} // namespace sf
