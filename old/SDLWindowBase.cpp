#include <Zancle/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION
#include <string>


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/ContextSettings.hpp"
#include "Zancle/Window/Cursor.hpp"
#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/SDLWindowBase.hpp"
#include "Zancle/Window/Vulkan.hpp"
#include "Zancle/Window/WindowHandle.hpp"
#include "Zancle/Window/WindowImpl.hpp"

#include "Zancle/System/String.hpp"
#include "Zancle/System/Vector2.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Clamp.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/UniquePtr.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <climits>
#include <cstdlib>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] za::SDLWindowBase::Settings nullifyContextSettings(za::SDLWindowBase::Settings windowSettings)
{
    windowSettings.contextSettings = za::ContextSettings{.depthBits         = 0,
                                                         .stencilBits       = 0,
                                                         .antiAliasingLevel = 0,
                                                         .majorVersion      = 0,
                                                         .minorVersion      = 0,
                                                         .attributeFlags = za::ContextSettings::Attribute{0xFF'FF'FF'FFu},
                                                         .sRgbCapable = false};

    return windowSettings;
}


} // namespace


namespace za
{
////////////////////////////////////////////////////////////
struct SDLWindowBase::Impl
{
    SDL_Window* sdlWindow;

    explicit Impl(const Settings& windowSettings) :
    sdlWindow{SDL_CreateWindow(windowSettings.title.toAnsiString<std::string>().data(),
                               static_cast<int>(windowSettings.size.x),
                               static_cast<int>(windowSettings.size.y),
                               SDL_WindowFlags{})}
    {
    }

    explicit Impl(const WindowHandle handle)
    {
        auto props = SDL_CreateProperties();

#if defined(ZA_SYSTEM_WINDOWS)

        SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, handle);

#elif defined(ZA_SYSTEM_LINUX_OR_BSD)

        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER, handle);

#elif defined(ZA_SYSTEM_MACOS)

        SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, handle);

#elif defined(ZA_SYSTEM_IOS)

        // return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr);

#elif defined(ZA_SYSTEM_ANDROID)

        // return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);

#elif defined(ZA_SYSTEM_EMSCRIPTEN)

        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_EMSCRIPTEN_CANVAS_ID, handle);

#endif

        sdlWindow = SDL_CreateWindowWithProperties(props);
    }

    ~Impl()
    {
        SDL_DestroyWindow(sdlWindow);
    }

    [[nodiscard]] Vector2u getSize() const
    {
        int w{};
        int h{};
        SDL_GetWindowSize(sdlWindow, &w, &h);
        return {static_cast<unsigned int>(w), static_cast<unsigned int>(h)};
    }

    void setPosition(const Vector2i position) const
    {
        SDL_SetWindowPosition(sdlWindow, position.x, position.y);
    }

    void setMinimumSize(const zb::Optional<Vector2u> minimumSize) const
    {
        if (minimumSize.hasValue())
            SDL_SetWindowMinimumSize(sdlWindow, static_cast<int>(minimumSize->x), static_cast<int>(minimumSize->y));
        else
            SDL_SetWindowMinimumSize(sdlWindow, 0, 0);
    }

    void setMaximumSize(const zb::Optional<Vector2u> maximumSize) const
    {
        if (maximumSize.hasValue())
            SDL_SetWindowMaximumSize(sdlWindow, static_cast<int>(maximumSize->x), static_cast<int>(maximumSize->y));
        else
            SDL_SetWindowMaximumSize(sdlWindow, 0, 0);
    }

    [[nodiscard]] Vector2i getPosition() const
    {
        int x{};
        int y{};
        SDL_GetWindowPosition(sdlWindow, &x, &y);
        return {x, y};
    }

    [[nodiscard]] zb::Optional<Vector2u> getMinimumSize() const
    {
        int w{};
        int h{};
        SDL_GetWindowMinimumSize(sdlWindow, &w, &h);
        return zb::makeOptional<Vector2u>(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    }

    [[nodiscard]] zb::Optional<Vector2u> getMaximumSize() const
    {
        int w{};
        int h{};
        SDL_GetWindowMaximumSize(sdlWindow, &w, &h);
        return zb::makeOptional<Vector2u>(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    }

    void setSize(const Vector2u size) const
    {
        SDL_SetWindowSize(sdlWindow, static_cast<int>(size.x), static_cast<int>(size.y));
    }

    void requestFocus() const
    {
        SDL_RaiseWindow(sdlWindow);
    }

    void setVisible(const bool visible) const
    {
        if (visible)
            SDL_ShowWindow(sdlWindow);
        else
            SDL_HideWindow(sdlWindow);
    }

    void setMouseCursorVisible(const bool visible) const
    {
        if (visible)
            SDL_ShowCursor();
        else
            SDL_HideCursor();
    }

    [[nodiscard]] bool hasFocus() const
    {
        return SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_INPUT_FOCUS;
    }

    [[nodiscard]] float getDPIAwareScalingFactor() const
    {
        return SDL_GetWindowDisplayScale(sdlWindow);
    }

    void setMouseCursorGrabbed(const bool grabbed) const
    {
        SDL_SetWindowMouseGrab(sdlWindow, grabbed);
    }

    [[nodiscard]] WindowHandle getNativeHandle() const
    {
#if defined(ZA_SYSTEM_WINDOWS)

        return static_cast<HWND__*>(
            SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

#elif defined(ZA_SYSTEM_LINUX_OR_BSD)

        return SDL_GetNumberProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);

#elif defined(ZA_SYSTEM_MACOS)

        return SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);

#elif defined(ZA_SYSTEM_IOS)

        return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr);

#elif defined(ZA_SYSTEM_ANDROID)

        return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);

#elif defined(ZA_SYSTEM_EMSCRIPTEN)

        // Window handle is int on Emscripten
        using WindowHandle = int;

#endif
    }

    void setTitle(const String& title) const
    {
        SDL_SetWindowTitle(sdlWindow, title.toAnsiString<std::string>().data());
    }

    void setIcon(const Vector2u size, const zb::U8* pixels) const
    {
        SDL_Surface* iconSurface = SDL_CreateSurfaceFrom(static_cast<int>(size.x),
                                                         static_cast<int>(size.y),
                                                         SDL_PIXELFORMAT_RGBA32,
                                                         const_cast<void*>(static_cast<const void*>(pixels)),
                                                         static_cast<int>(size.x * 4));

        SDL_SetWindowIcon(sdlWindow, iconSurface);

        SDL_DestroySurface(iconSurface);
    }
};


////////////////////////////////////////////////////////////
SDLWindowBase::SDLWindowBase(zb::UniquePtr<Impl>&& impl) : m_impl(ZB_MOVE(impl))
{
    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVisible(true);
    setMouseCursorVisible(true);
    setKeyRepeatEnabled(true);

    // Get and cache the initial size of the window
    m_size = m_impl->getSize();
}


////////////////////////////////////////////////////////////
SDLWindowBase::SDLWindowBase(const Settings& windowSettings) :
SDLWindowBase(zb::makeUnique<Impl>(nullifyContextSettings(windowSettings)))
{
}


////////////////////////////////////////////////////////////
SDLWindowBase::SDLWindowBase(WindowHandle handle) : SDLWindowBase(zb::makeUnique<Impl>(handle))
{
}


////////////////////////////////////////////////////////////
SDLWindowBase::~SDLWindowBase() = default;


////////////////////////////////////////////////////////////
zb::Optional<Event> SDLWindowBase::pollEvent()
{
    return filterEvent(m_impl->pollEvent());
}


////////////////////////////////////////////////////////////
zb::Optional<Event> SDLWindowBase::waitEvent(Time timeout)
{
    return filterEvent(m_impl->waitEvent(timeout));
}


////////////////////////////////////////////////////////////
Vector2i SDLWindowBase::getPosition() const
{
    return m_impl->getPosition();
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setPosition(Vector2i position)
{
    m_impl->setPosition(position);
}


////////////////////////////////////////////////////////////
Vector2u SDLWindowBase::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setSize(Vector2u size)
{
    // Constrain requested size within minimum and maximum bounds
    const auto minimumSize = m_impl->getMinimumSize().valueOr(Vector2u{});
    const auto maximumSize = m_impl->getMaximumSize().valueOr(Vector2u{UINT_MAX, UINT_MAX});

    // Do nothing if requested size matches current size
    const auto clampedSize = size.componentWiseClamp(minimumSize, maximumSize);
    if (clampedSize == m_size)
        return;

    m_impl->setSize(clampedSize);

    // Cache the new size
    m_size = clampedSize;
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMinimumSize(const Vector2u& minimumSize)
{
    [[maybe_unused]] const auto validateMinimumSize = [&]
    {
        if (!m_impl->getMaximumSize().hasValue())
            return true;

        return minimumSize.x <= m_impl->getMaximumSize()->x && minimumSize.y <= m_impl->getMaximumSize()->y;
    };

    ZB_ASSERT(validateMinimumSize() && "Minimum size cannot be bigger than the maximum size along either axis");

    m_impl->setMinimumSize(zb::makeOptional(minimumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMinimumSize(const zb::Optional<Vector2u>& minimumSize)
{
    if (minimumSize.hasValue())
        setMinimumSize(*minimumSize);
    else
    {
        m_impl->setMinimumSize(zb::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMaximumSize(const Vector2u& maximumSize)
{
    [[maybe_unused]] const auto validateMaximumSize = [&]
    {
        if (!m_impl->getMinimumSize().hasValue())
            return true;

        return maximumSize.x >= m_impl->getMinimumSize()->x && maximumSize.y >= m_impl->getMinimumSize()->y;
    };

    ZB_ASSERT(validateMaximumSize() && "Maximum size cannot be smaller than the minimum size along either axis");

    m_impl->setMaximumSize(zb::makeOptional(maximumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMaximumSize(const zb::Optional<Vector2u>& maximumSize)
{
    if (maximumSize.hasValue())
        setMinimumSize(*maximumSize);
    else
    {
        m_impl->setMaximumSize(zb::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setTitle(const String& title)
{
    m_impl->setTitle(title);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setIcon(Vector2u size, const zb::U8* pixels)
{
    m_impl->setIcon(size, pixels);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setVisible(bool visible)
{
    m_impl->setVisible(visible);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMouseCursorVisible(bool visible)
{
    m_impl->setMouseCursorVisible(visible);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMouseCursorGrabbed(bool grabbed)
{
    m_impl->setMouseCursorGrabbed(grabbed);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setMouseCursor(const Cursor& cursor)
{
    // m_impl->setMouseCursor(cursor.getImpl());
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setKeyRepeatEnabled(bool enabled)
{
    // m_impl->setKeyRepeatEnabled(enabled);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::setJoystickThreshold(float threshold)
{
    // m_impl->setJoystickThreshold(threshold);
}


////////////////////////////////////////////////////////////
void SDLWindowBase::requestFocus()
{
    m_impl->requestFocus();
}


////////////////////////////////////////////////////////////
bool SDLWindowBase::hasFocus() const
{
    return m_impl->hasFocus();
}


////////////////////////////////////////////////////////////
float SDLWindowBase::getDPIAwareScalingFactor() const
{
    return m_impl->getDPIAwareScalingFactor();
}


////////////////////////////////////////////////////////////
WindowHandle SDLWindowBase::getNativeHandle() const
{
    return m_impl->getNativeHandle();
}


////////////////////////////////////////////////////////////
bool SDLWindowBase::createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData)
{
    return false;
    // return m_impl->createVulkanSurface(vulkanSurfaceData);
}


////////////////////////////////////////////////////////////
zb::Optional<Event> SDLWindowBase::filterEvent(zb::Optional<Event> event)
{
    // Cache the new size if needed
    if (event.hasValue() && event->getIf<Event::Resized>())
        m_size = event->getIf<Event::Resized>()->size;

    return event;
}

} // namespace za
