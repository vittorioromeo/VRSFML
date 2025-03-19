#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_include(<bits/stringfwd.h>)

    #include <bits/stringfwd.h>

#elif __has_include(<__fwd/string.h>)

    #include <__fwd/string.h>

#else

    #include <string>

#endif
