// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/iOS/SFViewController.hpp"


@implementation SFViewController

@synthesize orientationCanChange;


////////////////////////////////////////////////////////////
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return self.orientationCanChange;
}


////////////////////////////////////////////////////////////
- (BOOL)shouldAutorotate
{
    return self.orientationCanChange;
}

@end
