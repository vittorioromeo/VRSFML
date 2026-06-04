#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_trivially_assignable)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(...) __is_trivially_assignable(__VA_ARGS__&, const __VA_ARGS__&)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(...) ::std::is_trivially_copy_assignable_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isTriviallyCopyAssignable = ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(T);

} // namespace zb
