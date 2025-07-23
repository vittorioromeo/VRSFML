#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// iOS specific: SFML needs to hook the main function, to
// launch the iOS application (event loop), and then call the
// user main from inside it.
//
// Our strategy is to rename the user main to 'sfmlMain' with
// a macro (see Main.hpp), and call this modified main ourselves.
//
// Note that half of this trick (the sfmlMain placeholders and
// the application delegate) is defined sfml-window; see there
// for the full implementation.
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include <SFML/Window/iOS/SFAppDelegate.hpp>

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
