// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/SFApplicationDelegate.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

////////////////////////////////////////////////////////////
@implementation SFApplicationDelegate

////////////////////////////////////////////////////////////
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
    (void)sender;
    // Generate close event for each SFML window
    [NSApp makeWindowsPerform:@selector(sfClose) inOrder:NO];
    return NSTerminateCancel;
}

////////////////////////////////////////////////////////////
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
    (void)theApplication;
    return YES;
}

@end
