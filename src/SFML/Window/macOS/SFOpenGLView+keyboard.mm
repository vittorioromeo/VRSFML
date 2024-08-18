#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/macOS/HIDInputManager.hpp" // For localizedKeys and nonLocalizedKeys
#import <SFML/Window/macOS/SFKeyboardModifiersHelper.h>
#import <SFML/Window/macOS/SFOpenGLView+keyboard_priv.h>
#include "SFML/Window/macOS/WindowImplCocoa.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

////////////////////////////////////////////////////////////
/// In this file, we implement keyboard handling for SFOpenGLView
///
////////////////////////////////////////////////////////////


@implementation SFOpenGLView (keyboard)


////////////////////////////////////////////////////////
- (BOOL)acceptsFirstResponder
{
    // Accepts key event.
    return YES;
}


////////////////////////////////////////////////////////
- (BOOL)canBecomeKeyView
{
    // Accepts key event.
    return YES;
}


////////////////////////////////////////////////////////
- (void)enableKeyRepeat
{
    m_useKeyRepeat = YES;
}


////////////////////////////////////////////////////////
- (void)disableKeyRepeat
{
    m_useKeyRepeat = NO;
}


////////////////////////////////////////////////////////
- (void)keyDown:(NSEvent*)theEvent
{
    // Transmit to non-SFML responder
    [[self nextResponder] keyDown:theEvent];

    if (m_requester == nil)
        return;

    // Handle key down event
    if (m_useKeyRepeat || ![theEvent isARepeat])
    {
        const sf::Event::KeyPressed key = [SFOpenGLView convertNSKeyDownEventToSFMLEvent:theEvent];

        if ((key.code != sf::Keyboard::Key::Unknown) || (key.scancode != sf::Keyboard::Scan::Unknown))
            m_requester->keyDown(key);
    }


    // Handle text entered event:
    // Ignore event if we don't want repeated keystrokes
    if (m_useKeyRepeat || ![theEvent isARepeat])
    {
        // Ignore escape key and other non text keycode (See NSEvent.h)
        // because they produce a sound alert.
        if ([SFOpenGLView isValidTextUnicode:theEvent])
        {
            // Send the event to the hidden text view for processing
            [m_hiddenTextView interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
        }

        // Carefully handle backspace and delete..
        // Note: the event is intentionally sent to the hidden view
        //       even if we do something more specific below. This way
        //       key combination are correctly interpreted.

        unsigned short keycode = [theEvent keyCode];

        // Backspace
        if (keycode == 0x33)
        {
            // Send the correct Unicode value (i.e. 8) instead of 127 (which is 'delete')
            m_requester->textEntered(8);
        }

        // Delete
        else if ((keycode == 0x75) || (keycode == NSDeleteFunctionKey))
        {
            // Instead of the value 63272 we send 127.
            m_requester->textEntered(127);
        }

        // Otherwise, let's see what our hidden field has computed
        else
        {
            NSString* string = [m_hiddenTextView string];

            // Send each character to SFML event requester
            for (NSUInteger index = 0; index < [string length]; ++index)
                m_requester->textEntered([string characterAtIndex:index]);

            // Empty our hidden cache
            [m_hiddenTextView setString:@""];
        }
    }
}


////////////////////////////////////////////////////////
- (void)sfKeyUp:(NSEvent*)theEvent
{
    // For some mystic reasons, key released events don't work the same way
    // as key pressed events... We somewhat hijack the event chain of response
    // in -[SFApplication sendEvent:] and resume this chain with the next
    // responder.
    // This is workaround to make sure key released events are fired in
    // fullscreen window too.

    // Transmit to non-SFML responder
    [[self nextResponder] keyUp:theEvent];

    if (m_requester == nil)
        return;

    const sf::Event::KeyReleased key = [SFOpenGLView convertNSKeyUpEventToSFMLEvent:theEvent];

    if ((key.code != sf::Keyboard::Key::Unknown) || (key.scancode != sf::Keyboard::Scan::Unknown))
        m_requester->keyUp(key);
}


////////////////////////////////////////////////////////
- (void)flagsChanged:(NSEvent*)theEvent
{
    // Transmit to non-SFML responder
    [[self nextResponder] flagsChanged:theEvent];

    if (m_requester == nil)
        return;

    NSUInteger modifiers = [theEvent modifierFlags];
    handleModifiersChanged(modifiers, *m_requester);
}


////////////////////////////////////////////////////////
+ (sf::Event::KeyPressed)convertNSKeyDownEventToSFMLEvent:(NSEvent*)event
{
    // The scancode always depends on the hardware keyboard, not some OS setting.
    sf::Keyboard::Scancode code = sf::priv::HIDInputManager::nonLocalizedKey([event keyCode]);

    // Get the corresponding key under the current keyboard layout.
    sf::Keyboard::Key key = sf::Keyboard::localize(code);

    return keyPressedEventWithModifiers([event modifierFlags], key, code);
}


////////////////////////////////////////////////////////
+ (sf::Event::KeyReleased)convertNSKeyUpEventToSFMLEvent:(NSEvent*)event
{
    // The scancode always depends on the hardware keyboard, not some OS setting.
    sf::Keyboard::Scancode code = sf::priv::HIDInputManager::nonLocalizedKey([event keyCode]);

    // Get the corresponding key under the current keyboard layout.
    sf::Keyboard::Key key = sf::Keyboard::localize(code);

    return keyReleasedEventWithModifiers([event modifierFlags], key, code);
}


////////////////////////////////////////////////////////
+ (BOOL)isValidTextUnicode:(NSEvent*)event
{
    if ([event keyCode] == 0x35) // Escape
    {
        return false;
    }
    if ([[event characters] length] > 0)
    {
        unichar code = [[event characters] characterAtIndex:0];
        return (code < 0xF700) || (code > 0xF8FF);
    }

    return true;
}

@end
