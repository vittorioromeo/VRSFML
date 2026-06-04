#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_isinf)

    ////////////////////////////////////////////////////////////
    #define ZB_ISINF __builtin_isinf

#else

    #include <cmath>

    ////////////////////////////////////////////////////////////
    #define ZB_ISINF ::std::isinf

#endif
