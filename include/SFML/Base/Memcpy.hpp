#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_memcpy)

////////////////////////////////////////////////////////////
#define SFML_BASE_MEMCPY __builtin_memcpy

#else

#include <cstring>

////////////////////////////////////////////////////////////
#define SFML_BASE_MEMCPY ::SFML_BASE_MEMCPY

#endif
