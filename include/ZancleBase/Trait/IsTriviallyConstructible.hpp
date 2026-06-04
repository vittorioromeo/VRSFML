#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_trivially_constructible)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_CONSTRUCTIBLE(...) __is_trivially_constructible(__VA_ARGS__)

#elif __has_builtin(__has_trivial_constructor)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_CONSTRUCTIBLE(...) __has_trivial_constructor(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_CONSTRUCTIBLE(...) ::std::is_trivially_constructible_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyConstructible = ZB_IS_TRIVIALLY_CONSTRUCTIBLE(T);

} // namespace zb
