// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Windows specific: we define the WinMain entry point,
// so that developers can use the standard main function
// even in a Win32 Application project, and thus keep a
// portable code
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#ifndef WINAPI
    #if defined(_ARM_)
        #define WINAPI
    #else
        #define WINAPI __stdcall
    #endif
#endif

#undef DECLARE_HANDLE
#define DECLARE_HANDLE(name) \
    struct name##__;         \
    typedef struct name##__* name

DECLARE_HANDLE(HINSTANCE);
using LPSTR = char*;
using INT   = int;

#include <cstdlib> // for `__argc` and `__argv`

extern int main(int argc, char* argv[]);

////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    return main(__argc, __argv);
#pragma GCC diagnostic pop
}
