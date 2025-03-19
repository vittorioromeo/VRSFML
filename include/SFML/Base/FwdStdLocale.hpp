#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_include(<bits/localefwd.h>)

    #include <bits/localefwd.h>

#elif __has_include(<__fwd/locale.h>)

    #include <__fwd/locale.h>

#else

    #include <locale>

#endif
