#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_unreachable)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNREACHABLE() __builtin_unreachable()

#elif __has_builtin(__assume)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNREACHABLE() __assume(false)

#else

    #include "SFML/Base/Assert.hpp"

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_UNREACHABLE() SFML_ASSERT(false)

#endif
