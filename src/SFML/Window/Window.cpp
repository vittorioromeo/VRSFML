#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/VideoModeUtils.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Window/WindowContext.hpp>
#include <SFML/Window/WindowImpl.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>

#include <SFML/Base/Macros.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
struct Window::Window::Impl
{
    WindowContext*                   windowContext;
    base::UniquePtr<priv::GlContext> glContext;      //!< Platform-specific implementation of the OpenGL context
    Clock                            clock;          //!< Clock for measuring the elapsed time between frames
    Time                             frameTimeLimit; //!< Current framerate limit

    explicit Impl(WindowContext& theWindowContext, base::UniquePtr<priv::GlContext>&& theContext) :
    windowContext(&theWindowContext),
    glContext(SFML_BASE_MOVE(theContext))
    {
    }
};


////////////////////////////////////////////////////////////
template <typename TWindowBaseArg>
Window::Window(WindowContext&         windowContext,
               const ContextSettings& settings,
               TWindowBaseArg&&       windowBaseArg,
               unsigned int           bitsPerPixel) :
WindowBase(SFML_BASE_FORWARD(windowBaseArg)),
m_impl(windowContext, windowContext.createGlContext(settings, getWindowImpl(), bitsPerPixel))
{
    // Perform common initializations
    SFML_BASE_ASSERT(m_impl->glContext != nullptr);

    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVerticalSyncEnabled(false);
    setFramerateLimit(0);

    // Activate the window
    if (!setActive())
        priv::err() << "Failed to set window as active during initialization";
}


////////////////////////////////////////////////////////////
Window::Window(WindowContext&         windowContext,
               VideoMode              mode,
               const String&          title,
               Style                  style,
               State                  state,
               const ContextSettings& settings) :
Window(windowContext, settings, priv::WindowImpl::create(mode, title, style, state, settings), mode.bitsPerPixel)
{
}


////////////////////////////////////////////////////////////
Window::Window(WindowContext& windowContext, VideoMode mode, const String& title, State state, const ContextSettings& settings) :
Window(windowContext, mode, title, sf::Style::Default, state, settings)
{
}


////////////////////////////////////////////////////////////
Window::Window(WindowContext& windowContext, WindowHandle handle, const ContextSettings& settings) :
Window(windowContext, settings, handle, VideoModeUtils::getDesktopMode().bitsPerPixel)
{
}


////////////////////////////////////////////////////////////
Window::Window(WindowContext&         windowContext,
               VideoMode              mode,
               const char*            title,
               Style                  style,
               State                  state,
               const ContextSettings& settings) :
Window(windowContext, mode, String(title), style, state, settings)
{
}


////////////////////////////////////////////////////////////
Window::Window(WindowContext& windowContext, VideoMode mode, const char* title, State state, const ContextSettings& settings) :
Window(windowContext, mode, String(title), state, settings)
{
}


////////////////////////////////////////////////////////////
Window::~Window()
{
    // Need to activate window context during destruction to avoid GL errors
    [[maybe_unused]] const bool rc = setActive(true);
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
Window::Window(Window&&) noexcept = default;


////////////////////////////////////////////////////////////
Window& Window::operator=(Window&&) noexcept = default;


////////////////////////////////////////////////////////////
const ContextSettings& Window::getSettings() const
{
    SFML_BASE_ASSERT(m_impl->glContext != nullptr);
    return m_impl->glContext->getSettings();
}


////////////////////////////////////////////////////////////
void Window::setVerticalSyncEnabled(bool enabled)
{
    if (setActive())
        m_impl->glContext->setVerticalSyncEnabled(enabled);
}


////////////////////////////////////////////////////////////
void Window::setFramerateLimit(unsigned int limit)
{
    m_impl->frameTimeLimit = limit > 0 ? seconds(1.f / static_cast<float>(limit)) : Time::Zero;
}


////////////////////////////////////////////////////////////
bool Window::setActive(bool active) const
{
    SFML_BASE_ASSERT(m_impl->glContext != nullptr);

    if (m_impl->windowContext->setActiveThreadLocalGlContext(*m_impl->glContext, active))
        return true;

    priv::err() << "Failed to activate the window's context";
    return false;
}


////////////////////////////////////////////////////////////
void Window::display()
{
    // Display the backbuffer on screen
    if (setActive())
        m_impl->glContext->display();

    // Limit the framerate if needed
    if (m_impl->frameTimeLimit != Time::Zero)
    {
        sleep(m_impl->frameTimeLimit - m_impl->clock.getElapsedTime());
        m_impl->clock.restart();
    }
}

} // namespace sf
