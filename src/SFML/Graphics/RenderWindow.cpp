#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTextureImplFBO.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowBase.hpp>

#include <SFML/System/String.hpp>


namespace
{
////////////////////////////////////////////////////////////
void retrieveWindowFrameBufferId(sf::GraphicsContext& graphicsContext, unsigned int& defaultFrameBuffer)
{
    if (sf::priv::RenderTextureImplFBO::isAvailable(graphicsContext))
    {
        // Retrieve the framebuffer ID we have to bind when targeting the window for rendering
        // We assume that this window's context is still active at this point
        glCheck(glGetIntegerv(GLEXT_GL_DRAW_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&defaultFrameBuffer)));
    }
}


} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext&       graphicsContext,
                           VideoMode              mode,
                           const String&          title,
                           Style                  style,
                           State                  state,
                           const ContextSettings& settings) :
Window(graphicsContext, mode, title, style, state, settings),
RenderTarget(graphicsContext)
{
    retrieveWindowFrameBufferId(getGraphicsContext(), m_defaultFrameBuffer);
    RenderTarget::initialize(); // Just initialize the render target part
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext&       graphicsContext,
                           VideoMode              mode,
                           const char*            title,
                           Style                  style,
                           State                  state,
                           const ContextSettings& settings) :
RenderWindow(graphicsContext, mode, String(title), style, state, settings)
{
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext&       graphicsContext,
                           VideoMode              mode,
                           const String&          title,
                           State                  state,
                           const ContextSettings& settings) :
RenderWindow(graphicsContext, mode, title, sf::Style::Default, state, settings)
{
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext&       graphicsContext,
                           VideoMode              mode,
                           const char*            title,
                           State                  state,
                           const ContextSettings& settings) :
RenderWindow(graphicsContext, mode, String(title), state, settings)
{
}


////////////////////////////////////////////////////////////
RenderWindow::RenderWindow(GraphicsContext& graphicsContext, WindowHandle handle, const ContextSettings& settings) :
Window(graphicsContext, handle, settings),
RenderTarget(graphicsContext)
{
    retrieveWindowFrameBufferId(getGraphicsContext(), m_defaultFrameBuffer);
    RenderTarget::initialize(); // Just initialize the render target part
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
    if (active && result && priv::RenderTextureImplFBO::isAvailable(getGraphicsContext()))
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
