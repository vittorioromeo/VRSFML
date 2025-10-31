#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#ifndef __DBL_MAX__
    #include <cfloat>
#endif


////////////////////////////////////////////////////////////
#ifdef __DBL_MAX__
    #define SFML_BASE_DOUBLE_MAX __DBL_MAX__
#else
    #define SFML_BASE_DOUBLE_MAX DBL_MAX
#endif
