#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"


namespace sf
{
class WindowBase;
}

namespace sf::priv::InputImpl
{
////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::isKeyPressed(Key)
///
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Key key);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::isKeyPressed(Scancode)
///
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Scancode code);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::localize
///
////////////////////////////////////////////////////////////
Keyboard::Key localize(Keyboard::Scancode code);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::delocalize
///
////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(Keyboard::Key key);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::getDescription
///
////////////////////////////////////////////////////////////
String getDescription(Keyboard::Scancode code);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::setVirtualKeyboardVisible
///
////////////////////////////////////////////////////////////
void setVirtualKeyboardVisible(bool visible);

////////////////////////////////////////////////////////////
/// \brief Check if a mouse button is pressed
///
/// \param button Button to check
///
/// \return `true` if the button is pressed, `false` otherwise
///
////////////////////////////////////////////////////////////
bool isMouseButtonPressed(Mouse::Button button);

////////////////////////////////////////////////////////////
/// \brief Get the current position of the mouse in desktop coordinates
///
/// This function returns the current position of the mouse
/// cursor, in global (desktop) coordinates.
///
/// \return Current position of the mouse
///
////////////////////////////////////////////////////////////
Vector2i getMousePosition();

////////////////////////////////////////////////////////////
/// \brief Get the current position of the mouse in window coordinates
///
/// This function returns the current position of the mouse
/// cursor, relative to the given window.
/// If no window is used, it returns desktop coordinates.
///
/// \param relativeTo Reference window
///
/// \return Current position of the mouse
///
////////////////////////////////////////////////////////////
Vector2i getMousePosition(const WindowBase& relativeTo);

////////////////////////////////////////////////////////////
/// \brief Set the current position of the mouse in desktop coordinates
///
/// This function sets the current position of the mouse
/// cursor in global (desktop) coordinates.
/// If no window is used, it sets the position in desktop coordinates.
///
/// \param position New position of the mouse
///
////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position);

////////////////////////////////////////////////////////////
/// \brief Set the current position of the mouse in window coordinates
///
/// This function sets the current position of the mouse
/// cursor, relative to the given window.
/// If no window is used, it sets the position in desktop coordinates.
///
/// \param position New position of the mouse
/// \param relativeTo Reference window
///
////////////////////////////////////////////////////////////
void setMousePosition(Vector2i position, const WindowBase& relativeTo);

////////////////////////////////////////////////////////////
/// \brief Check if a touch event is currently down
///
/// \param finger Finger index
///
/// \return `true` if \a finger is currently touching the screen, `false` otherwise
///
////////////////////////////////////////////////////////////
bool isTouchDown(unsigned int finger);

////////////////////////////////////////////////////////////
/// \brief Get the current position of a touch in desktop coordinates
///
/// This function returns the current touch position
/// in global (desktop) coordinates.
///
/// \param finger Finger index
///
/// \return Current position of \a finger, or undefined if it's not down
///
////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger);

////////////////////////////////////////////////////////////
/// \brief Get the current position of a touch in window coordinates
///
/// This function returns the current touch position
/// in global (desktop) coordinates.
///
/// \param finger Finger index
/// \param relativeTo Reference window
///
/// \return Current position of \a finger, or undefined if it's not down
///
////////////////////////////////////////////////////////////
Vector2i getTouchPosition(unsigned int finger, const WindowBase& relativeTo);

} // namespace sf::priv::InputImpl
