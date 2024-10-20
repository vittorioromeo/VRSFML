#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if __has_builtin(__builtin_assume)

////////////////////////////////////////////////////////////
#define SFML_BASE_ASSUME(...) __builtin_assume(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
#define SFML_BASE_ASSUME(...)

#endif
