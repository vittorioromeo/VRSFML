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
/// \brief `FLT_EPSILON` macro, prefers the compiler builtin to avoid `<cfloat>`
///
////////////////////////////////////////////////////////////
#ifdef __FLT_EPSILON__
    #define ZB_FLOAT_EPSILON __FLT_EPSILON__
#else
    #define ZB_FLOAT_EPSILON FLT_EPSILON
#endif
