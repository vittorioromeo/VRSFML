#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za::priv
{
////////////////////////////////////////////////////////////
template <bool>
struct SelectImpl
{
    template <typename TTrue, typename TFalse>
    using type = TFalse;
};


////////////////////////////////////////////////////////////
template <>
struct SelectImpl<true>
{
    template <typename TTrue, typename TFalse>
    using type = TTrue;
};

} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
template <bool B, typename TTrue, typename TFalse>
using Conditional = typename priv::SelectImpl<B>::template type<TTrue, TFalse>;

} // namespace za
