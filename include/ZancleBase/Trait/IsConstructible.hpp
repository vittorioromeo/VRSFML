#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_constructible)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_CONSTRUCTIBLE(...) __is_constructible(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_CONSTRUCTIBLE(...) ::std::is_constructible_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T, typename... Args>
inline constexpr bool isConstructible = ZB_IS_CONSTRUCTIBLE(T, Args...);

} // namespace zb
