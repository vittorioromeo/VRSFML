// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Event.hpp"

#import <AppKit/AppKit.h>

namespace sf::priv
{
class WindowImplCocoa;
}

////////////////////////////////////////////////////////////
/// Keyboard Modifiers Helper
///
/// Handle left & right modifiers (cmd, ctrl, alt, shift)
/// events and send them back to the requester.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
/// \brief Initialize the global state (only if needed)
///
/// It needs to be called before any event, e.g. in the window constructor.
///
////////////////////////////////////////////////////////////
void initialiseKeyboardHelper();


////////////////////////////////////////////////////////////
/// \brief Set up a SFML key pressed event based on the given modifiers flags and key code
///
////////////////////////////////////////////////////////////
sf::Event::KeyPressed keyPressedEventWithModifiers(NSUInteger modifiers, sf::Keyboard::Key key, sf::Keyboard::Scancode code);


////////////////////////////////////////////////////////////
/// \brief Set up a SFML key released event based on the given modifiers flags and key code
///
////////////////////////////////////////////////////////////
sf::Event::KeyReleased keyReleasedEventWithModifiers(NSUInteger modifiers, sf::Keyboard::Key key, sf::Keyboard::Scancode code);


////////////////////////////////////////////////////////////
/// \brief Handle the state of modifiers keys
///
/// Send key released & pressed events to the requester.
///
////////////////////////////////////////////////////////////
void handleModifiersChanged(NSUInteger modifiers, sf::priv::WindowImplCocoa& requester);
