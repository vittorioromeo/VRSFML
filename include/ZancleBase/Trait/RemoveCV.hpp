#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#if __has_builtin(__remove_cv)

    ////////////////////////////////////////////////////////////
    #define ZB_REMOVE_CV(...) __remove_cv(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveCVImpl                   { using type = T; };
template <typename T> struct RemoveCVImpl<const T>          { using type = T; };
template <typename T> struct RemoveCVImpl<volatile T>       { using type = T; };
template <typename T> struct RemoveCVImpl<const volatile T> { using type = T; };
// clang-format on

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_REMOVE_CV(...) typename ::zb::priv::RemoveCVImpl<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveCV = ZB_REMOVE_CV(T);

} // namespace zb
