#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
