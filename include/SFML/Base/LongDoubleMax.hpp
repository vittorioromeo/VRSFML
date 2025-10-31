#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#ifndef __LDBL_MAX__
    #include <cfloat>
#endif


////////////////////////////////////////////////////////////
#ifdef __LDBL_MAX__
    #define SFML_BASE_LONG_DOUBLE_MAX __LDBL_MAX__
#else
    #define SFML_BASE_LONG_DOUBLE_MAX LDBL_MAX
#endif
