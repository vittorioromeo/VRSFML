#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__builtin_memset)

    ////////////////////////////////////////////////////////////
    #define ZB_MEMSET __builtin_memset

#else

    #include <cstring>

    ////////////////////////////////////////////////////////////
    #define ZB_MEMSET ::std::memset

#endif
