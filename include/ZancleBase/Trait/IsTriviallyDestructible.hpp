#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_trivially_destructible)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_DESTRUCTIBLE(...) __is_trivially_destructible(__VA_ARGS__)

#elif __has_builtin(__has_trivial_destructor)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_DESTRUCTIBLE(...) __has_trivial_destructor(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_DESTRUCTIBLE(...) ::std::is_trivially_destructible_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyDestructible = ZB_IS_TRIVIALLY_DESTRUCTIBLE(T);

} // namespace zb
