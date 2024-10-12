#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


#ifdef __clang__

////////////////////////////////////////////////////////////
#define SFML_BASE_TRIVIAL_ABI [[clang::trivial_abi]]

#else

////////////////////////////////////////////////////////////
#define SFML_BASE_TRIVIAL_ABI

#endif
