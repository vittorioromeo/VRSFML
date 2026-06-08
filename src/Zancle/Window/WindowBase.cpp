// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/WindowBase.hpp"

#include "Zancle/Window/Cursor.hpp"
#include "Zancle/Window/Event.hpp"
#include "Zancle/Window/SDLWindowImpl.hpp"
#include "Zancle/Window/Vulkan.hpp"
#include "Zancle/Window/WindowHandle.hpp"
#include "Zancle/Window/WindowSettings.hpp"

#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/Macros.hpp"


namespace za
{
////////////////////////////////////////////////////////////
priv::SDLWindowImpl& WindowBase::getWindowImpl()
{
    return *m_impl;
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(za::PassKey<WindowBase>&&, za::UniquePtr<priv::SDLWindowImpl>&& impl) : m_impl(ZA_MOVE(impl))
{
    ZA_ASSERT(m_impl != nullptr);

    // Get and cache the initial size of the window
    m_size = m_impl->getSize();
}


////////////////////////////////////////////////////////////
za::Optional<WindowBase> WindowBase::create(const WindowSettings& windowSettings)
{
    auto impl = priv::SDLWindowImpl::create(windowSettings);

    return impl ? za::Optional<WindowBase>(za::inPlace, za::PassKey<WindowBase>{}, ZA_MOVE(impl)) : za::nullOpt;
}


////////////////////////////////////////////////////////////
za::Optional<WindowBase> WindowBase::create(const WindowHandle handle)
{
    auto impl = priv::SDLWindowImpl::create(handle);

    return impl ? za::Optional<WindowBase>(za::inPlace, za::PassKey<WindowBase>{}, ZA_MOVE(impl)) : za::nullOpt;
}


////////////////////////////////////////////////////////////
WindowBase::~WindowBase() = default;


////////////////////////////////////////////////////////////
WindowBase::WindowBase(WindowBase&&) noexcept = default;


////////////////////////////////////////////////////////////
WindowBase& WindowBase::operator=(WindowBase&&) noexcept = default;


////////////////////////////////////////////////////////////
za::Optional<Event> WindowBase::pollEvent()
{
    return filterEvent(m_impl->pollEvent());
}


////////////////////////////////////////////////////////////
za::Optional<Event> WindowBase::waitEvent()
{
    return filterEvent(m_impl->waitEvent(Time{}));
}


////////////////////////////////////////////////////////////
za::Optional<Event> WindowBase::waitEvent(const Time timeout)
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

    ZA_ASSERT(validateMinimumSize() && "Minimum size cannot be bigger than the maximum size along either axis");

    m_impl->setMinimumSize(za::makeOptional(minimumSize));
    setSize(getSize());
}

// TODO P1: minimumSize/maximumSize are not respected when the window is resized by the user. This is because we only
// clamp the size when `setSize` is called, but not when processing a resize event


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const za::Optional<Vec2u>& minimumSize)
{
    if (minimumSize.hasValue())
        setMinimumSize(*minimumSize);
    else
    {
        m_impl->setMinimumSize(za::nullOpt);
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

    ZA_ASSERT(validateMaximumSize() && "Maximum size cannot be smaller than the minimum size along either axis");

    m_impl->setMaximumSize(za::makeOptional(maximumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const za::Optional<Vec2u>& maximumSize)
{
    if (maximumSize.hasValue())
        setMaximumSize(*maximumSize);
    else
    {
        m_impl->setMaximumSize(za::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setTitle(const Utf8String& title)
{
    m_impl->setTitle(title);
}


////////////////////////////////////////////////////////////
void WindowBase::setIcon(const za::U8* const pixels, const Vec2u size)
{
    m_impl->setIcon(pixels, size);
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
float WindowBase::getDisplayScale() const
{
    return m_impl->getDisplayScale();
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
za::Optional<Event> WindowBase::filterEvent(const za::Optional<Event> event)
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

} // namespace za
