#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_standard_layout)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_STANDARD_LAYOUT(...) __is_standard_layout(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_STANDARD_LAYOUT(...) ::std::is_standard_layout_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isStandardLayout = ZB_IS_STANDARD_LAYOUT(T);

} // namespace zb
