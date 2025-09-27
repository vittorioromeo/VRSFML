#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
#if __has_include(<bits/localefwd.h>)

    #include <bits/localefwd.h>

#elif __has_include(<__fwd/locale.h>)

    #include <__fwd/locale.h>

#else

    #include <locale>

#endif
