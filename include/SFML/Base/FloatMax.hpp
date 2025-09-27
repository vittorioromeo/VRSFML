#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#ifndef __FLT_MAX__
    #include <cfloat>
#endif


////////////////////////////////////////////////////////////
#ifdef __FLT_MAX__
    #define SFML_BASE_FLOAT_MAX __FLT_MAX__
#else
    #define SFML_BASE_FLOAT_MAX FLT_MAX
#endif
