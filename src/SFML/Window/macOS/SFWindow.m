// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/SFWindow.h>


@implementation SFWindow

////////////////////////////////////////////////////////
- (BOOL)acceptsFirstResponder
{
    return YES;
}


////////////////////////////////////////////////////////
- (BOOL)canBecomeKeyWindow
{
    return YES;
}


////////////////////////////////////////////////////////
- (BOOL)canBecomeMainWindow
{
    return YES;
}


////////////////////////////////////////////////////////
- (void)keyDown:(NSEvent*)theEvent
{
    // Do nothing except preventing a system alert each time a key is pressed
    //
    // Special Consideration :
    // -----------------------
    // Consider overriding NSResponder -keyDown: message in a Cocoa view/window
    // that contains a SFML rendering area. Doing so will prevent a system
    // alert to be thrown every time the user presses a key.
    (void)theEvent;
}


////////////////////////////////////////////////////////
- (void)performClose:(id)sender
{
    // From Apple documentation:
    //
    // > If the window's delegate or the window itself implements windowShouldClose:,
    // > that message is sent with the window as the argument. (Only one such message is sent;
    // > if both the delegate and the NSWindow object implement the method, only the delegate
    // > receives the message.) If the windowShouldClose: method returns NO, the window isn't
    // > closed. If it returns YES, or if it isn't implemented, performClose: invokes the
    // > close method to close the window.
    // >
    // > If the window doesn't have a close button or can't be closed (for example, if the
    // > delegate replies NO to a windowShouldClose: message), the system emits the alert sound.
    //
    // The last paragraph is problematic for SFML fullscreen window since they don't have
    // a close button (style is NSBorderlessWindowMask). So we reimplement this function.

    BOOL shouldClose = NO;

    if ([self delegate] && [[self delegate] respondsToSelector:@selector(windowShouldClose:)])
        shouldClose = [[self delegate] windowShouldClose:sender];
    // else if ([self respondsToSelector:@selector(windowShouldClose:)])
    //     shouldClose = [self windowShouldClose:sender];
    // error: no visible @interface for 'SFWindow' declares the selector 'windowShouldClose:'

    if (shouldClose)
        [self close];
}


////////////////////////////////////////////////////////
- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
    return [menuItem action] == @selector(performClose:) || [super validateMenuItem:menuItem];
}


@end


@implementation NSWindow (SFML)

////////////////////////////////////////////////////////////
- (id)sfClose
{
    [self performClose:nil];
    return nil;
}

@end
