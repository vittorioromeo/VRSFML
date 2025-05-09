#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/SDLWindowImpl.hpp"
#include "SFML/Window/Vulkan.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowHandle.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
priv::SDLWindowImpl& WindowBase::getWindowImpl()
{
    return *m_impl;
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(base::UniquePtr<priv::SDLWindowImpl>&& impl) : m_impl(SFML_BASE_MOVE(impl))
{
    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVisible(true);
    setMouseCursorVisible(true);
    setKeyRepeatEnabled(true);

    // Get and cache the initial size of the window
    m_size = m_impl->getSize();
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(const Settings& windowSettings) : WindowBase(priv::SDLWindowImpl::create(windowSettings))
{
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(WindowHandle handle) : WindowBase(priv::SDLWindowImpl::create(handle))
{
}


////////////////////////////////////////////////////////////
WindowBase::~WindowBase() = default;


////////////////////////////////////////////////////////////
WindowBase::WindowBase(WindowBase&&) noexcept = default;


////////////////////////////////////////////////////////////
WindowBase& WindowBase::operator=(WindowBase&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::pollEvent()
{
    return filterEvent(m_impl->pollEvent());
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::waitEvent(const Time timeout)
{
    return filterEvent(m_impl->waitEvent(timeout));
}


////////////////////////////////////////////////////////////
Vec2i WindowBase::getPosition() const
{
    return m_impl->getPosition();
}


////////////////////////////////////////////////////////////
void WindowBase::setPosition(Vec2i position)
{
    m_impl->setPosition(position);
}


////////////////////////////////////////////////////////////
Vec2u WindowBase::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void WindowBase::setSize(const Vec2u size)
{
    enum : unsigned int
    {
        uIntMax = static_cast<unsigned int>(-1)
    };

    // Constrain requested size within minimum and maximum bounds
    const auto minimumSize = m_impl->getMinimumSize().valueOr(Vec2u{});
    const auto maximumSize = m_impl->getMaximumSize().valueOr(Vec2u{uIntMax, uIntMax});

    // Do nothing if requested size matches current size
    const auto clampedSize = size.componentWiseClamp(minimumSize, maximumSize);
    if (clampedSize == m_size)
        return;

    m_impl->setSize(clampedSize);

    // Cache the new size
    m_size = clampedSize;
}


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const Vec2u& minimumSize)
{
    [[maybe_unused]] const auto validateMinimumSize = [&]
    {
        if (!m_impl->getMaximumSize().hasValue())
            return true;

        return minimumSize.x <= m_impl->getMaximumSize()->x && minimumSize.y <= m_impl->getMaximumSize()->y;
    };

    SFML_BASE_ASSERT(validateMinimumSize() && "Minimum size cannot be bigger than the maximum size along either axis");

    m_impl->setMinimumSize(base::makeOptional(minimumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const base::Optional<Vec2u>& minimumSize)
{
    if (minimumSize.hasValue())
        setMinimumSize(*minimumSize);
    else
    {
        m_impl->setMinimumSize(base::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const Vec2u& maximumSize)
{
    [[maybe_unused]] const auto validateMaximumSize = [&]
    {
        if (!m_impl->getMinimumSize().hasValue())
            return true;

        return maximumSize.x >= m_impl->getMinimumSize()->x && maximumSize.y >= m_impl->getMinimumSize()->y;
    };

    SFML_BASE_ASSERT(validateMaximumSize() && "Maximum size cannot be smaller than the minimum size along either axis");

    m_impl->setMaximumSize(base::makeOptional(maximumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const base::Optional<Vec2u>& maximumSize)
{
    if (maximumSize.hasValue())
        setMinimumSize(*maximumSize);
    else
    {
        m_impl->setMaximumSize(base::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setTitle(const String& title)
{
    m_impl->setTitle(title);
}


////////////////////////////////////////////////////////////
void WindowBase::setIcon(const Vec2u size, const base::U8* const pixels)
{
    m_impl->setIcon(size, pixels);
}


////////////////////////////////////////////////////////////
void WindowBase::setVisible(const bool visible)
{
    m_impl->setVisible(visible);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursorVisible(const bool visible)
{
    m_impl->setMouseCursorVisible(visible);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursorGrabbed(const bool grabbed)
{
    m_impl->setMouseCursorGrabbed(grabbed);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursor(const Cursor& cursor)
{
    m_impl->setMouseCursor(cursor.getImpl());
}


////////////////////////////////////////////////////////////
void WindowBase::setKeyRepeatEnabled(const bool enabled)
{
    m_impl->setKeyRepeatEnabled(enabled);
}


////////////////////////////////////////////////////////////
void WindowBase::setJoystickThreshold(const float threshold)
{
    m_impl->setJoystickThreshold(threshold);
}


////////////////////////////////////////////////////////////
void WindowBase::requestFocus()
{
    m_impl->requestFocus();
}


////////////////////////////////////////////////////////////
bool WindowBase::hasFocus() const
{
    return m_impl->hasFocus();
}


////////////////////////////////////////////////////////////
float WindowBase::getWindowDisplayScale() const
{
    return m_impl->getWindowDisplayScale();
}


////////////////////////////////////////////////////////////
WindowHandle WindowBase::getNativeHandle() const
{
    return m_impl->getNativeHandle();
}


////////////////////////////////////////////////////////////
SDL_Window* WindowBase::getSDLHandle() const
{
    return m_impl->getSDLHandle();
}


////////////////////////////////////////////////////////////
bool WindowBase::createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData)
{
    return m_impl->createVulkanSurface(vulkanSurfaceData);
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::filterEvent(const base::Optional<Event> event)
{
    // Cache the new size if needed
    if (event.hasValue() && event->getIf<Event::Resized>())
        m_size = event->getIf<Event::Resized>()->size;

    return event;
}


////////////////////////////////////////////////////////////
bool WindowBase::isFullscreen() const
{
    return m_impl->isFullscreen();
}


////////////////////////////////////////////////////////////
bool WindowBase::isResizable() const
{
    return m_impl->isResizable();
}


////////////////////////////////////////////////////////////
bool WindowBase::hasTitlebar() const
{
    return m_impl->hasTitlebar();
}


////////////////////////////////////////////////////////////
void WindowBase::setResizable(const bool resizable)
{
    m_impl->setResizable(resizable);
}


////////////////////////////////////////////////////////////
void WindowBase::setHasTitlebar(const bool hasTitleBar)
{
    m_impl->setHasTitlebar(hasTitleBar);
}

} // namespace sf
