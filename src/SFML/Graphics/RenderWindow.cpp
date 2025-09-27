// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/WindowBase.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Base/Macros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename... TWindowArgs>
RenderWindow::RenderWindow(int /* disambiguator */, TWindowArgs&&... windowArgs) :
    Window(SFML_BASE_FORWARD(windowArgs)...),
    RenderTarget(View::fromRect({{0.f, 0.f}, getSize().toVec2f()}))
{
    // Retrieve the framebuffer ID we have to bind when targeting the window for rendering
    // We assume that this window's context is still active at this point
    glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&m_defaultFrameBuffer)));
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(const Settings& windowSettings) : RenderWindow(int{}, windowSettings)
{
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(WindowHandle handle, const ContextSettings& contextSettings) :
    RenderWindow(int{}, handle, contextSettings)
{
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
    setIcon(icon.getSize(), icon.getPixelsPtr());
}


////////////////////////////////////////////////////////////
bool RenderWindow::isSrgb() const
{
    return getSettings().sRgbCapable;
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
base::Optional<Event> RenderWindow::filterEvent(base::Optional<Event> event)
{
    if (event.hasValue() && event->getIf<Event::Resized>())
        onResize();

    return event;
}


////////////////////////////////////////////////////////////
base::Optional<Event> RenderWindow::pollEvent()
{
    return filterEvent(WindowBase::pollEvent());
}


////////////////////////////////////////////////////////////
base::Optional<Event> RenderWindow::waitEvent(Time timeout)
{
    return filterEvent(WindowBase::waitEvent(timeout));
}


////////////////////////////////////////////////////////////
RenderTarget::DrawStatistics RenderWindow::display()
{
    const auto result = RenderTarget::flush();
    RenderTarget::syncGPUEndFrame();
    Window::display();
    return result;
}


////////////////////////////////////////////////////////////
void RenderWindow::setSize(const Vec2u& size)
{
    WindowBase::setSize(size);
    onResize();
}


////////////////////////////////////////////////////////////
void RenderWindow::onResize()
{
    // Update the current view (recompute the viewport, which is stored in relative coordinates)
    setView(getView());
}

} // namespace sf
