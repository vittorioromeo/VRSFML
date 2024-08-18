#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Mouse.hpp"
#import <SFML/Window/macOS/SFOpenGLView.h>

#import <AppKit/AppKit.h>


////////////////////////////////////////////////////////////
/// Here are defined a few private messages for keyboard
/// handling in SFOpenGLView.
///
////////////////////////////////////////////////////////////


@interface SFOpenGLView (keyboard_priv)

////////////////////////////////////////////////////////////
/// \brief Convert a key down NSEvent into an SFML key pressed event
///
/// The conversion is based on localizedKeys and nonLocalizedKeys functions.
///
/// \param event a key event
///
/// \return sf::Keyboard::Key::Unknown as Code if the key is unknown
///
////////////////////////////////////////////////////////////
+ (sf::Event::KeyPressed)convertNSKeyDownEventToSFMLEvent:(NSEvent*)event;

////////////////////////////////////////////////////////////
/// \brief Convert a key up NSEvent into an SFML key released event
///
/// The conversion is based on localizedKeys and nonLocalizedKeys functions.
///
/// \param event a key event
///
/// \return sf::Keyboard::Key::Unknown as Code if the key is unknown
///
////////////////////////////////////////////////////////////
+ (sf::Event::KeyReleased)convertNSKeyUpEventToSFMLEvent:(NSEvent*)event;

////////////////////////////////////////////////////////////
/// \brief Check if the event represent some Unicode text
///
/// The event is assumed to be a key down event.
/// False is returned if the event is either escape or a non text Unicode.
///
/// \param event a key down event
///
/// \return true if event represents a Unicode character, false otherwise
///
////////////////////////////////////////////////////////////
+ (BOOL)isValidTextUnicode:(NSEvent*)event;

@end
