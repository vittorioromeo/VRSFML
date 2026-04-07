#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#ifdef __CLANGD__

    #include <string> // IWYU pragma: export

#else

    #if __has_include(<bits/stringfwd.h>)

        #include <bits/stringfwd.h> // IWYU pragma: export

    #elif __has_include(<__fwd/string.h>)

        #include <__fwd/string.h> // IWYU pragma: export

    #else

        #include <string> // IWYU pragma: export

    #endif

#endif
