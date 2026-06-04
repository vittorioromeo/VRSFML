#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#if __has_builtin(__remove_reference)

    ////////////////////////////////////////////////////////////
    #define ZB_REMOVE_REFERENCE(...) __remove_reference(__VA_ARGS__)

#else

namespace zb::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template <typename T> struct RemoveReferenceImpl      { using type = T; };
template <typename T> struct RemoveReferenceImpl<T&>  { using type = T; };
template <typename T> struct RemoveReferenceImpl<T&&> { using type = T; };
// clang-format on

} // namespace zb::priv

    ////////////////////////////////////////////////////////////
    #define ZB_REMOVE_REFERENCE(...) typename ::zb::priv::RemoveReferenceImpl<__VA_ARGS__>::type

#endif


namespace zb
{
////////////////////////////////////////////////////////////
template <typename T>
using RemoveReference = ZB_REMOVE_REFERENCE(T);

} // namespace zb
