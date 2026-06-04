// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/iOS/SFViewController.hpp"


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
