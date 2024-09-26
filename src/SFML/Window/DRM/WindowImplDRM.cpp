#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/DRM/DRMContext.hpp"
#include "SFML/Window/DRM/WindowImplDRM.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/WindowImpl.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{

// Defined in DRM/InputImpl.cpp because they require access to that file's global state
namespace InputImpl
{
////////////////////////////////////////////////////////////
/// \brief Fetch input event from event queue
///
/// \return False if event queue is empty
///
////////////////////////////////////////////////////////////
base::Optional<Event> checkEvent();

////////////////////////////////////////////////////////////
/// \brief Backup terminal configuration and disable console feedback
///
////////////////////////////////////////////////////////////
void setTerminalConfig();

////////////////////////////////////////////////////////////
/// \brief Restore terminal configuration from backup
///
////////////////////////////////////////////////////////////
void restoreTerminalConfig();
} // namespace InputImpl


////////////////////////////////////////////////////////////
WindowImplDRM::WindowImplDRM(WindowContext& windowContext, WindowHandle /*handle*/) : WindowImpl(windowContext)
{
    InputImpl::setTerminalConfig();
}


////////////////////////////////////////////////////////////
WindowImplDRM::WindowImplDRM(WindowContext& windowContext, const WindowSettings& windowSettings) :
WindowImpl(windowContext),
m_size(windowSettings.size)
{
    InputImpl::setTerminalConfig();
}


////////////////////////////////////////////////////////////
WindowImplDRM::~WindowImplDRM()
{
    InputImpl::restoreTerminalConfig();
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplDRM::getNativeHandle() const
{
    const Drm& drm = DRMContext::getDRM();
    return static_cast<WindowHandle>(drm.fileDescriptor);
}

////////////////////////////////////////////////////////////
Vector2i WindowImplDRM::getPosition() const
{
    return {0, 0};
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setPosition(Vector2i /*position*/)
{
}


////////////////////////////////////////////////////////////
Vector2u WindowImplDRM::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setSize(Vector2u /*size*/)
{
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setMinimumSize(const base::Optional<Vector2u>& /* minimumSize */)
{
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setMaximumSize(const base::Optional<Vector2u>& /* maximumSize */)
{
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setTitle(const String& /*title*/)
{
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setIcon(Vector2u /*size*/, const base::U8* /*pixels*/)
{
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setVisible(bool /*visible*/)
{
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setMouseCursorVisible(bool /*visible*/)
{
    // TODO P2: not implemented
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setMouseCursorGrabbed(bool /*grabbed*/)
{
    // TODO P2: not implemented
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setMouseCursor(const CursorImpl& /*cursor*/)
{
    // TODO P2: not implemented
}


////////////////////////////////////////////////////////////
void WindowImplDRM::setKeyRepeatEnabled(bool /*enabled*/)
{
    // TODO P2: not implemented
}


////////////////////////////////////////////////////////////
void WindowImplDRM::requestFocus()
{
    // Not applicable
}


////////////////////////////////////////////////////////////
bool WindowImplDRM::hasFocus() const
{
    return true;
}

void WindowImplDRM::processEvents()
{
    while (const base::Optional event = InputImpl::checkEvent())
        pushEvent(*event);
}

} // namespace sf::priv
