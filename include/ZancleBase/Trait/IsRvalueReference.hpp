#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__is_rvalue_reference)

    ////////////////////////////////////////////////////////////
    #define ZB_IS_RVALUE_REFERENCE(...) __is_rvalue_reference(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReferenceImpl = false;


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReferenceImpl<T&&> = true;

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_IS_RVALUE_REFERENCE(...) ::zb::priv::isRvalueReferenceImpl<__VA_ARGS__>

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr bool isRvalueReference = ZB_IS_RVALUE_REFERENCE(T);

} // namespace zb
