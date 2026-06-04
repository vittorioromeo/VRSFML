// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/iOS/SFMain.hpp"


// zancleMain is called by the application delegate (SFAppDelegate).
//
// Since we don't know which prototype of `main` the user
// defines, we declare both versions of `zancleMain`, but with
// the `weak` attribute (GCC extension) so that the
// user-declared one will replace SFML's one at linking stage.
//
// If the user defines `main(argc, argv)` then it will be called
// directly, if they define `main()` then it will be called by
// our placeholder.
//
// The `zancleMain()` version is never called, it is just defined
// to avoid a linker error if the user directly defines the
// version with arguments.
//
// See the `zancle-main` module for the other half of this
// initialization trick.


////////////////////////////////////////////////////////////
__attribute__((weak)) int zancleMain(int, char**)
{
    return zancleMain();
}


////////////////////////////////////////////////////////////
__attribute__((weak)) int zancleMain()
{
    return 0;
}
