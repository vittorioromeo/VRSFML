#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_strlen)

////////////////////////////////////////////////////////////
#define SFML_BASE_STRLEN __builtin_strlen

#else

#include <cstring>

////////////////////////////////////////////////////////////
#define SFML_BASE_STRLEN ::std::strlen

#endif
