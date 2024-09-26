#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_assume)

////////////////////////////////////////////////////////////
#define SFML_BASE_ASSUME __builtin_assume

#else

////////////////////////////////////////////////////////////
#define SFML_BASE_ASSUME

#endif
