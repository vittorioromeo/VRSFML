#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__remove_const)

    ////////////////////////////////////////////////////////////
    #define ZB_REMOVE_CONST(...) __remove_const(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveConstImpl          { using type = T; };
template <typename T> struct RemoveConstImpl<const T> { using type = T; };
// clang-format on

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_REMOVE_CONST(...) typename ::zb::priv::RemoveConstImpl<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveConst = ZB_REMOVE_CONST(T);

} // namespace zb
