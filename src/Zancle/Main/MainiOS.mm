// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// iOS specific: SFML needs to hook the main function, to
// launch the iOS application (event loop), and then call the
// user main from inside it.
//
// Our strategy is to rename the user main to 'zancleMain' with
// a macro (see Main.hpp), and call this modified main ourselves.
//
// Note that half of this trick (the zancleMain placeholders and
// the application delegate) is defined zancle-window; see there
// for the full implementation.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"

#include <Zancle/Window/iOS/SFAppDelegate.hpp>

#include <UIKit/UIKit.h>


////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    NSString* appDelegateClassName = {};
    @autoreleasepool
    {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([SFAppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
