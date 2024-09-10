#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_memcmp)

////////////////////////////////////////////////////////////
#define SFML_BASE_MEMCMP __builtin_memcmp

#else

#include <cstring>

////////////////////////////////////////////////////////////
#define SFML_BASE_MEMCMP ::std::memcmp

#endif
