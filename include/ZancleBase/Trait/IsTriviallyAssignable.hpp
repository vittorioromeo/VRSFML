#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_trivially_assignable)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_ASSIGNABLE(...) __is_trivially_assignable(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_TRIVIALLY_ASSIGNABLE(...) ::std::is_trivially_assignable_v<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename Target, typename Source>
inline constexpr bool isTriviallyAssignable = ZB_IS_TRIVIALLY_ASSIGNABLE(Target, Source);

} // namespace zb
