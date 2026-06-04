#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__underlying_type)

    ////////////////////////////////////////////////////////////
    #define ZB_UNDERLYING_TYPE(...) __underlying_type(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_UNDERLYING_TYPE(...) typename ::std::underlying_type<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
using UnderlyingType = ZB_UNDERLYING_TYPE(T);

} // namespace zb
