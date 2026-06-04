#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_unsigned)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_UNSIGNED(...) __is_unsigned(__VA_ARGS__)

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include "ZancleBase/Trait/IsIntegral.hpp"


namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename T, bool = isIntegral<T>>
inline constexpr bool isUnsignedImpl = false;


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isUnsignedImpl<T, true> = T(0) < T(-1);

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_IS_UNSIGNED(...) ::zb::priv::isUnsignedImpl<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isUnsigned = ZB_IS_UNSIGNED(T);

} // namespace zb
