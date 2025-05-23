#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#ifndef __FLT_EPSILON__
    #include <cfloat>
#endif


////////////////////////////////////////////////////////////
#ifdef __FLT_EPSILON__
    #define SFML_BASE_FLOAT_EPSILON __FLT_EPSILON__
#else
    #define SFML_BASE_FLOAT_EPSILON FLT_EPSILON
#endif
