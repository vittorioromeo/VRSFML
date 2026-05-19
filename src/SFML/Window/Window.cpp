// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Window.hpp"

#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/GLUtils/GlContext.hpp"

#include "SFML/System/Clock.hpp"
#include "SFML/System/Err.hpp"
#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/UniquePtr.hpp"

#ifdef SFML_SYSTEM_EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/em_asm.h>
    #include <emscripten/html5.h>
#endif


namespace sf
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
namespace
{
////////////////////////////////////////////////////////////
// Yield the wasm thread until the next browser `requestAnimationFrame` tick.
// Used as a vsync-aligned alternative to `emscripten_sleep(0)` (which yields
// via `setTimeout`, with no display-refresh alignment and a ~4ms minimum
// delay -- causing the app to render far more frames than the display shows).
//
// `EM_ASYNC_JS` integrates with asyncify: the wasm side suspends until the
// returned JS promise resolves, which happens inside the browser's RAF
// callback -- i.e. on a display-refresh boundary.
////////////////////////////////////////////////////////////
// clang-format off
EM_ASYNC_JS(void, sfml_yield_to_raf, (), {
    await new Promise(resolve => requestAnimationFrame(resolve));
});
// clang-format on

} // namespace
#endif


////////////////////////////////////////////////////////////
struct Window::Window::Impl
{
    base::UniquePtr<priv::GlContext> glContext;      //!< Platform-specific implementation of the OpenGL context
    Clock                            clock;          //!< Clock for measuring the elapsed time between frames
    Time                             frameTimeLimit; //!< Current framerate limit

    explicit Impl(base::UniquePtr<priv::GlContext>&& theContext) : glContext(SFML_BASE_MOVE(theContext))
    {
    }
};


////////////////////////////////////////////////////////////
Window::Window(base::PassKey<Window>&&, WindowBase&& windowBase, const WindowSettings& windowSettings, unsigned int bitsPerPixel) :
    WindowBase(SFML_BASE_MOVE(windowBase)),
    m_impl(WindowContext::createGlContext(windowSettings.contextSettings, getWindowImpl(), bitsPerPixel))
{
    SFML_BASE_ASSERT(m_impl->glContext != nullptr && "Failed to create GL context for window");

    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVerticalSyncEnabled(windowSettings.vsync);
    setFramerateLimit(windowSettings.frametimeLimit);

    // Activate the window
    if (!setActive())
        priv::errMsg("Failed to set window as active during initialization");
}


////////////////////////////////////////////////////////////
base::Optional<Window> Window::create(const WindowSettings& windowSettings)
{
    auto windowBase = WindowBase::create(windowSettings);

    return windowBase.hasValue()
               ? base::Optional<Window>(base::inPlace,
                                        base::PassKey<Window>{},
                                        SFML_BASE_MOVE(*windowBase),
                                        windowSettings,
                                        windowSettings.bitsPerPixel)
               : base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Window> Window::create(const WindowHandle handle, const ContextSettings& contextSettings)
{
    auto windowBase = WindowBase::create(handle);

    if (!windowBase.hasValue())
        return base::nullOpt;

    return base::Optional<Window>(base::inPlace,
                                  base::PassKey<Window>{},
                                  SFML_BASE_MOVE(*windowBase),
                                  WindowSettings{.size{}, .contextSettings = contextSettings},
                                  VideoModeUtils::getDesktopMode().bitsPerPixel);
}


////////////////////////////////////////////////////////////
bool Window::isMovedFrom() const
{
    return m_impl->glContext == nullptr;
}


////////////////////////////////////////////////////////////
Window::~Window()
{
    if (isMovedFrom())
        return;

    // Need to activate window context during destruction to avoid GL errors
    [[maybe_unused]] const bool rc = setActive(true);
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
Window::Window(Window&&) noexcept = default;


////////////////////////////////////////////////////////////
Window& Window::operator=(Window&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    // Make sure the window is destroyed after the context,
    //  as SDL context activation requires the window to be alive
    m_impl = SFML_BASE_MOVE(rhs.m_impl);
    WindowBase::operator=(SFML_BASE_MOVE(rhs));

    return *this;
}


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
bool Window::isVerticalSyncEnabled() const
{
    return m_impl->glContext->isVerticalSyncEnabled();
}


////////////////////////////////////////////////////////////
void Window::setFramerateLimit(unsigned int limit)
{
    m_impl->frameTimeLimit = limit > 0 ? seconds(1.f / static_cast<float>(limit)) : Time{};
}


////////////////////////////////////////////////////////////
bool Window::setActive(bool active) const
{
    SFML_BASE_ASSERT(m_impl->glContext != nullptr);

    if (WindowContext::setActiveThreadLocalGlContext(*m_impl->glContext, active))
        return true;

    priv::errMsg("Failed to activate the window's context");
    return false;
}


////////////////////////////////////////////////////////////
void Window::display()
{
    // Display the backbuffer on screen
    if (setActive())
        m_impl->glContext->display();

    // Limit the framerate if needed
    if (m_impl->frameTimeLimit != Time{})
    {
        ThisThread::sleepFor(m_impl->frameTimeLimit - m_impl->clock.getElapsedTime());
        m_impl->clock.restart();
    }

#ifdef SFML_SYSTEM_EMSCRIPTEN
    // The browser drives frame timing, not the GL driver. Pick the yield
    // primitive based on whether the user requested vsync:
    //   - vsync enabled  -> `requestAnimationFrame` (display-refresh-aligned)
    //   - vsync disabled -> `setTimeout(0)` (run as fast as the JS task queue
    //     allows -- ~4ms minimum, so still capped, but not display-aligned)
    if (m_impl->glContext->isVerticalSyncEnabled())
        sfml_yield_to_raf();
    else
        emscripten_sleep(0u);
#endif
}

} // namespace sf
