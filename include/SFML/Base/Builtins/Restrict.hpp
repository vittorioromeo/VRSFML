#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#if defined(__GNUC__) || defined(__clang__)
#define SFML_BASE_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define SFML_BASE_RESTRICT __restrict
#else
#define SFML_BASE_RESTRICT
#endif
