#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Clang detection
#ifdef __is_identifier
    #if !__is_identifier(_Float16)
        #define SFML_BASE_FLOAT16_DETECTED
    #endif

    #if !__is_identifier(_Float32)
        #define SFML_BASE_FLOAT32_DETECTED
    #endif

    #if !__is_identifier(_Float64)
        #define SFML_BASE_FLOAT64_DETECTED
    #endif

    #if !__is_identifier(_Float128)
        #define SFML_BASE_FLOAT128_DETECTED
    #endif
#endif


////////////////////////////////////////////////////////////
// Other compilers detection
#ifdef __STDCPP_FLOAT16_T__
    #define SFML_BASE_FLOAT16_DETECTED
#endif

#ifdef __STDCPP_FLOAT32_T__
    #define SFML_BASE_FLOAT32_DETECTED
#endif

#ifdef __STDCPP_FLOAT64_T__
    #define SFML_BASE_FLOAT64_DETECTED
#endif

#ifdef __STDCPP_FLOAT128_T__
    #define SFML_BASE_FLOAT128_DETECTED
#endif


namespace sf::base
{
////////////////////////////////////////////////////////////
// 16 bits floating point types
#ifdef SFML_BASE_FLOAT16_DETECTED
using F16 = decltype(0.f16);
#endif


////////////////////////////////////////////////////////////
// 32 bits floating point types
#ifdef SFML_BASE_FLOAT32_DETECTED
using F32 = decltype(0.f32);
#endif


////////////////////////////////////////////////////////////
// 64 bits floating point types
#ifdef SFML_BASE_FLOAT64_DETECTED
using F64 = decltype(0.f64);
#endif


////////////////////////////////////////////////////////////
// 128 bits floating point types
#ifdef SFML_BASE_FLOAT128_DETECTED
using F128 = decltype(0.f128);
#endif

} // namespace sf::base
