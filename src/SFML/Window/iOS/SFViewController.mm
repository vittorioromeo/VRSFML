#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/iOS/SFViewController.hpp>


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
