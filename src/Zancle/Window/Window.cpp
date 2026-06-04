// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Window.hpp"

#include "Zancle/Window/VideoMode.hpp"
#include "Zancle/Window/VideoModeUtils.hpp"
#include "Zancle/Window/WindowBase.hpp"
#include "Zancle/Window/WindowContext.hpp"
#include "Zancle/Window/WindowHandle.hpp"
#include "Zancle/Window/WindowSettings.hpp"

#include "Zancle/GLUtils/GlContext.hpp"

#include "Zancle/System/Clock.hpp"
#include "Zancle/System/Err.hpp"
#include "Zancle/System/Thread.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/UniquePtr.hpp"

#ifdef ZA_SYSTEM_EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/em_asm.h>
    #include <emscripten/html5.h>
#endif


namespace za
{
#ifdef ZA_SYSTEM_EMSCRIPTEN
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
EM_ASYNC_JS(void, zancle_yield_to_raf, (), {
    await new Promise(resolve => requestAnimationFrame(resolve));
});
// clang-format on

} // namespace
#endif


////////////////////////////////////////////////////////////
struct Window::Window::Impl
{
    zb::UniquePtr<priv::GlContext> glContext;      //!< Platform-specific implementation of the OpenGL context
    Clock                          clock;          //!< Clock for measuring the elapsed time between frames
    Time                           frameTimeLimit; //!< Current framerate limit

    explicit Impl(zb::UniquePtr<priv::GlContext>&& theContext) : glContext(ZB_MOVE(theContext))
    {
    }
};


////////////////////////////////////////////////////////////
Window::Window(zb::PassKey<Window>&&, WindowBase&& windowBase, const WindowSettings& windowSettings, unsigned int bitsPerPixel) :
    WindowBase(ZB_MOVE(windowBase)),
    m_impl(WindowContext::createGlContext(windowSettings.contextSettings, getWindowImpl(), bitsPerPixel))
{
    ZB_ASSERT(m_impl->glContext != nullptr && "Failed to create GL context for window");

    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVerticalSyncEnabled(windowSettings.vsync);
    setFramerateLimit(windowSettings.frametimeLimit);

    // Activate the window
    if (!setActive())
        priv::errMsg("Failed to set window as active during initialization");
}


////////////////////////////////////////////////////////////
zb::Optional<Window> Window::create(const WindowSettings& windowSettings)
{
    auto windowBase = WindowBase::create(windowSettings);

    return windowBase.hasValue()
               ? zb::Optional<Window>(zb::inPlace,
                                      zb::PassKey<Window>{},
                                      ZB_MOVE(*windowBase),
                                      windowSettings,
                                      windowSettings.bitsPerPixel)
               : zb::nullOpt;
}


////////////////////////////////////////////////////////////
zb::Optional<Window> Window::create(const WindowHandle handle, const ContextSettings& contextSettings)
{
    auto windowBase = WindowBase::create(handle);

    if (!windowBase.hasValue())
        return zb::nullOpt;

    return zb::Optional<Window>(zb::inPlace,
                                zb::PassKey<Window>{},
                                ZB_MOVE(*windowBase),
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
    ZB_ASSERT(rc);
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
    m_impl = ZB_MOVE(rhs.m_impl);
    WindowBase::operator=(ZB_MOVE(rhs));

    return *this;
}


////////////////////////////////////////////////////////////
const ContextSettings& Window::getSettings() const
{
    ZB_ASSERT(m_impl->glContext != nullptr);
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
    ZB_ASSERT(m_impl->glContext != nullptr);

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

#ifdef ZA_SYSTEM_EMSCRIPTEN
    // The browser drives frame timing, not the GL driver. Pick the yield
    // primitive based on whether the user requested vsync:
    //   - vsync enabled  -> `requestAnimationFrame` (display-refresh-aligned)
    //   - vsync disabled -> `setTimeout(0)` (run as fast as the JS task queue
    //     allows -- ~4ms minimum, so still capped, but not display-aligned)
    if (m_impl->glContext->isVerticalSyncEnabled())
        zancle_yield_to_raf();
    else
        emscripten_sleep(0u);
#endif
}

} // namespace za
