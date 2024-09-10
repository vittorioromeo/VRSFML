#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderTextureImplFBO.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/Event.hpp"
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/GLExtensions.hpp"
#include "SFML/Window/WindowBase.hpp"


namespace
{
////////////////////////////////////////////////////////////
void retrieveWindowFrameBufferId(unsigned int& defaultFrameBuffer)
{
    // Retrieve the framebuffer ID we have to bind when targeting the window for rendering
    // We assume that this window's context is still active at this point
    glCheck(glGetIntegerv(GLEXT_GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&defaultFrameBuffer)));
}


} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext& graphicsContext, const WindowSettings& windowSettings) :
Window(graphicsContext, windowSettings),
RenderTarget(graphicsContext)
{
    retrieveWindowFrameBufferId(m_defaultFrameBuffer);
    RenderTarget::initialize(); // Just initialize the render target part
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext& graphicsContext, WindowHandle handle, const ContextSettings& contextSettings) :
Window(graphicsContext, handle, contextSettings),
RenderTarget(graphicsContext)
{
    retrieveWindowFrameBufferId(m_defaultFrameBuffer);
    RenderTarget::initialize(); // Just initialize the render target part
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(RenderWindow&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderWindow& RenderWindow::operator=(RenderWindow&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderWindow::~RenderWindow()
{
    // Need to activate window context during destruction to avoid GL errors
    [[maybe_unused]] const bool rc = setActive(true);
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
Vector2u RenderWindow::getSize() const
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
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, m_defaultFrameBuffer));
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
void RenderWindow::setSize(const Vector2u& size)
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
