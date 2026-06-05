#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_isinf)

    ////////////////////////////////////////////////////////////
    #define ZA_ISINF __builtin_isinf

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define ZA_ISINF ::std::isinf

#endif
