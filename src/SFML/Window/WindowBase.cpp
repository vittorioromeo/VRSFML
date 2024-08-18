#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Vulkan.hpp"
#include "SFML/Window/WindowBase.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"

#include <climits>
#include <cstdlib>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::WindowSettings nullifyContextSettings(sf::WindowSettings windowSettings)
{
    windowSettings.contextSettings = sf::ContextSettings{.depthBits         = 0,
                                                         .stencilBits       = 0,
                                                         .antialiasingLevel = 0,
                                                         .majorVersion      = 0,
                                                         .minorVersion      = 0,
                                                         .attributeFlags = sf::ContextSettings::Attribute{0xFFFFFFFFu},
                                                         .sRgbCapable    = false};

    return windowSettings;
}


} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
struct WindowBase::Impl
{
    base::UniquePtr<priv::WindowImpl> impl; //!< Platform-specific implementation of the window
    Vector2u                          size; //!< Current size of the window
};


////////////////////////////////////////////////////////////
priv::WindowImpl& WindowBase::getWindowImpl()
{
    return *static_cast<priv::WindowImpl*>(m_impl->impl.get());
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(base::UniquePtr<priv::WindowImpl>&& impl) : m_impl(SFML_BASE_MOVE(impl), Vector2u{})
{
    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVisible(true);
    setMouseCursorVisible(true);
    setKeyRepeatEnabled(true);

    // Get and cache the initial size of the window
    m_impl->size = m_impl->impl->getSize();
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(const WindowSettings& windowSettings) :
WindowBase(priv::WindowImpl::create(nullifyContextSettings(windowSettings)))
{
}

////////////////////////////////////////////////////////////
WindowBase::WindowBase(WindowHandle handle) : WindowBase(priv::WindowImpl::create(handle))
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
    return filterEvent(m_impl->impl->pollEvent());
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::waitEvent(Time timeout)
{
    return filterEvent(m_impl->impl->waitEvent(timeout));
}


////////////////////////////////////////////////////////////
Vector2i WindowBase::getPosition() const
{
    return m_impl->impl->getPosition();
}


////////////////////////////////////////////////////////////
void WindowBase::setPosition(Vector2i position)
{
    m_impl->impl->setPosition(position);
}


////////////////////////////////////////////////////////////
Vector2u WindowBase::getSize() const
{
    return m_impl->size;
}


////////////////////////////////////////////////////////////
void WindowBase::setSize(Vector2u size)
{
    // Constrain requested size within minimum and maximum bounds
    const auto minimumSize = m_impl->impl->getMinimumSize().valueOr(Vector2u());
    const auto maximumSize = m_impl->impl->getMaximumSize().valueOr(Vector2u{UINT_MAX, UINT_MAX});

    const auto width  = base::clamp(size.x, minimumSize.x, maximumSize.x);
    const auto height = base::clamp(size.y, minimumSize.y, maximumSize.y);

    // Do nothing if requested size matches current size
    const Vector2u clampedSize(width, height);
    if (clampedSize == m_impl->size)
        return;

    m_impl->impl->setSize(clampedSize);

    // Cache the new size
    m_impl->size = clampedSize;
}


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const Vector2u& minimumSize)
{
    [[maybe_unused]] const auto validateMinimumSize = [&]
    {
        if (!m_impl->impl->getMaximumSize().hasValue())
            return true;

        return minimumSize.x <= m_impl->impl->getMaximumSize()->x && minimumSize.y <= m_impl->impl->getMaximumSize()->y;
    };

    SFML_BASE_ASSERT(validateMinimumSize() && "Minimum size cannot be bigger than the maximum size along either axis");

    m_impl->impl->setMinimumSize(base::makeOptional(minimumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const base::Optional<Vector2u>& minimumSize)
{
    if (minimumSize.hasValue())
        setMinimumSize(*minimumSize);
    else
    {
        m_impl->impl->setMinimumSize(base::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const Vector2u& maximumSize)
{
    [[maybe_unused]] const auto validateMaximumSize = [&]
    {
        if (!!m_impl->impl->getMinimumSize().hasValue())
            return true;

        return maximumSize.x >= m_impl->impl->getMinimumSize()->x && maximumSize.y >= m_impl->impl->getMinimumSize()->y;
    };

    SFML_BASE_ASSERT(validateMaximumSize() && "Maximum size cannot be smaller than the minimum size along either axis");

    m_impl->impl->setMaximumSize(base::makeOptional(maximumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const base::Optional<Vector2u>& maximumSize)
{
    if (maximumSize.hasValue())
        setMinimumSize(*maximumSize);
    else
    {
        m_impl->impl->setMaximumSize(base::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setTitle(const String& title)
{
    m_impl->impl->setTitle(title);
}


////////////////////////////////////////////////////////////
void WindowBase::setTitle(const char* title)
{
    setTitle(String(title));
}


////////////////////////////////////////////////////////////
void WindowBase::setIcon(Vector2u size, const std::uint8_t* pixels)
{
    m_impl->impl->setIcon(size, pixels);
}


////////////////////////////////////////////////////////////
void WindowBase::setVisible(bool visible)
{
    m_impl->impl->setVisible(visible);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursorVisible(bool visible)
{
    m_impl->impl->setMouseCursorVisible(visible);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursorGrabbed(bool grabbed)
{
    m_impl->impl->setMouseCursorGrabbed(grabbed);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursor(const Cursor& cursor)
{
    m_impl->impl->setMouseCursor(cursor.getImpl());
}


////////////////////////////////////////////////////////////
void WindowBase::setKeyRepeatEnabled(bool enabled)
{
    m_impl->impl->setKeyRepeatEnabled(enabled);
}


////////////////////////////////////////////////////////////
void WindowBase::setJoystickThreshold(float threshold)
{
    m_impl->impl->setJoystickThreshold(threshold);
}


////////////////////////////////////////////////////////////
void WindowBase::requestFocus()
{
    m_impl->impl->requestFocus();
}


////////////////////////////////////////////////////////////
bool WindowBase::hasFocus() const
{
    return m_impl->impl->hasFocus();
}


////////////////////////////////////////////////////////////
WindowHandle WindowBase::getNativeHandle() const
{
    return m_impl->impl->getNativeHandle();
}


////////////////////////////////////////////////////////////
bool WindowBase::createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData)
{
    return m_impl->impl->createVulkanSurface(vulkanSurfaceData);
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::filterEvent(base::Optional<Event> event)
{
    // Cache the new size if needed
    if (event.hasValue() && event->getIf<Event::Resized>())
        m_impl->size = event->getIf<Event::Resized>()->size;

    return event;
}

} // namespace sf
