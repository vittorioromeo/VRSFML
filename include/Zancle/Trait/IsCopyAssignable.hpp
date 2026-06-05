#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_assignable)

    ////////////////////////////////////////////////////////////
    #define ZA_IS_COPY_ASSIGNABLE(...) __is_assignable(__VA_ARGS__&, const __VA_ARGS__&)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZA_IS_COPY_ASSIGNABLE(...) ::std::is_copy_assignable_v<__VA_ARGS__>

#endif


namespace za
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isCopyAssignable = ZA_IS_COPY_ASSIGNABLE(T);

} // namespace za
