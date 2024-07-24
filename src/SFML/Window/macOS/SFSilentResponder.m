#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

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
