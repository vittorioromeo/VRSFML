#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_signbit)

    ////////////////////////////////////////////////////////////
    #define ZB_SIGNBIT __builtin_signbit

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define ZB_SIGNBIT ::std::signbit

#endif
