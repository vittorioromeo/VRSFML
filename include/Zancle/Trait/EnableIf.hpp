#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za::priv
{
////////////////////////////////////////////////////////////
template <bool, typename T = void>
struct EnableIfImpl
{
};


////////////////////////////////////////////////////////////
template <typename T>
struct EnableIfImpl<true, T>
{
    using type = T;
};

} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
template <bool B, typename T = void>
using EnableIf = typename priv::EnableIfImpl<B, T>::type;

} // namespace za
