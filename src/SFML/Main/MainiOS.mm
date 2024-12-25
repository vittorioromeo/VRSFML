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

#include <UIKit/UIKit.h>


////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // Note: we intentionally drop command line arguments,
    // there's no such thing as a command line on an iOS device :)

    // Important: "SFAppDelegate" must always match the name of the
    // application delegate class defined in sfml-window

    return UIApplicationMain(argc, argv, nil, @"SFAppDelegate");
}
