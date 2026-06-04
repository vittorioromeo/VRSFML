#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_trivially_copyable)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_COPYABLE(...) __is_trivially_copyable(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_COPYABLE(...) ::std::is_trivially_copyable_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyCopyable = ZB_IS_TRIVIALLY_COPYABLE(T);

} // namespace zb
