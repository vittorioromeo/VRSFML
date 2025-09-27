// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#import <SFML/Window/macOS/SFSilentResponder.h>

@implementation SFSilentResponder

- (void)doCommandBySelector:(SEL)sel
{
    // Just do nothing, to prevent sound alerts
    (void)sel;
}

@end
