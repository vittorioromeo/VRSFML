#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_strcmp)

////////////////////////////////////////////////////////////
#define SFML_BASE_STRCMP __builtin_strcmp

#else

#include <cstring>

////////////////////////////////////////////////////////////
#define SFML_BASE_STRCMP ::std::strcmp

#endif
