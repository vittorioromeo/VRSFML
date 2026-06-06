// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Graphics/RenderWindow.hpp"

#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/Glad.hpp"

#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/RenderTarget.hpp"

#include "Zancle/Window/ContextSettings.hpp"
#include "Zancle/Window/Window.hpp"
#include "Zancle/Window/WindowHandle.hpp"
#include "Zancle/Window/WindowSettings.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"

#include "Zancle/Diagnostic/Assert.hpp"

#include "Zancle/Base/Macros.hpp"


namespace za
{
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(za::PassKey<RenderWindow>&&, Window&& window) : Window(ZA_MOVE(window))
{
    // Retrieve the framebuffer ID we have to bind when targeting the window for rendering
    // We assume that this window's context is still active at this point
    glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&m_defaultFrameBuffer)));
}


////////////////////////////////////////////////////////////
za::Optional<RenderWindow> RenderWindow::create(const WindowSettings& windowSettings)
{
    auto window = Window::create(windowSettings);

    return window.hasValue() ? za::Optional<RenderWindow>(za::inPlace, za::PassKey<RenderWindow>{}, ZA_MOVE(*window))
                             : za::nullOpt;
}


////////////////////////////////////////////////////////////
za::Optional<RenderWindow> RenderWindow::create(const WindowHandle handle, const ContextSettings& contextSettings)
{
    auto window = Window::create(handle, contextSettings);

    return window.hasValue() ? za::Optional<RenderWindow>(za::inPlace, za::PassKey<RenderWindow>{}, ZA_MOVE(*window))
                             : za::nullOpt;
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
    ZA_ASSERT(rc);
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
    // Make sure the right context is current before issuing any GL --
    // `RenderTexture::display()` already does this; symmetric here so a
    // user juggling two `RenderWindow`s without an explicit `setActive`
    // doesn't accidentally drive `flush()` against the wrong context.
    if (!setActive(true))
    {
        priv::errMsg("Failed to activate render window in `display`");
        return {};
    }

    const auto result = RenderTarget::flush();
    RenderTarget::syncGPUEndFrame();
    Window::display();
    return result;
}

} // namespace za
