#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_isnan)

    ////////////////////////////////////////////////////////////
    #define ZA_ISNAN __builtin_isnan

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define ZA_ISNAN ::std::isnan

#endif
