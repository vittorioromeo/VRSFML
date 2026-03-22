#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#ifdef __CLANGD__
    #include <chrono> // IWYU pragma: export
#endif


////////////////////////////////////////////////////////////
#if __has_include(<bits/chrono.h>)
    #include <bits/chrono.h> // IWYU pragma: export
#else
    #include <chrono> // IWYU pragma: export
#endif
