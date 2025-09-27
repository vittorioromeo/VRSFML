#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
// 8 bits integer types
using I8 = signed char;
using U8 = unsigned char;


#if defined(__GNUC__) || defined(__clang__)

////////////////////////////////////////////////////////////
// 16 bits integer types
    #if __SIZEOF_SHORT__ == 2
using I16 = short;
using U16 = unsigned short;
    #elif __SIZEOF_INT__ == 2
using I16 = int;
using U16 = unsigned int;
    #else
        #error "Could not determine a 16-bit integer type (GCC/Clang)."
    #endif

////////////////////////////////////////////////////////////
// 32 bits integer types
    #if __SIZEOF_INT__ == 4
using I32 = int;
using U32 = unsigned int;
    #elif __SIZEOF_LONG__ == 4
using I32 = long;
using U32 = unsigned long;
    #else
        #error "Could not determine a 32-bit integer type (GCC/Clang)."
    #endif

////////////////////////////////////////////////////////////
// 64 bits integer types
    #if __SIZEOF_LONG_LONG__ == 8
using I64 = long long;
using U64 = unsigned long long;
    #elif __SIZEOF_LONG__ == 8
using I64 = long;
using U64 = unsigned long;
    #else
        #error "Could not determine a 64-bit integer type (GCC/Clang)."
    #endif


#else


// Assume standard Windows data models (ILP32, LLP64) where:
// char = 1, short = 2, int = 4, long = 4, long long = 8

////////////////////////////////////////////////////////////
// 16 bits integer types
using I16 = short;
using U16 = unsigned short;


////////////////////////////////////////////////////////////
// 32 bits integer types
using I32 = int;
using U32 = unsigned int;


////////////////////////////////////////////////////////////
// 64 bits integer types
using I64 = long long;
using U64 = unsigned long long;


#endif

} // namespace sf::base
