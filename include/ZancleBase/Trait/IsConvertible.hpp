#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__is_convertible)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_CONVERTIBLE(from, to) __is_convertible(from, to)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <type_traits>


    ////////////////////////////////////////////////////////////
    #define ZB_IS_CONVERTIBLE(from, to) ::std::is_convertible_v<from, to>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename From, typename To>
inline constexpr bool isConvertible = ZB_IS_CONVERTIBLE(From, To);

} // namespace zb
