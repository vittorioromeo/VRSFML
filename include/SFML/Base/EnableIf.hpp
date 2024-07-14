// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
// clang-format off
template<bool, typename = void> struct EnableIfImpl          { };
template<typename T>            struct EnableIfImpl<true, T> { using type = T; };
// clang-format on

} // namespace sf::base::priv

namespace sf::base
{
////////////////////////////////////////////////////////////
template <bool B, typename T = void>
using EnableIf = typename priv::EnableIfImpl<B, T>::type;

} // namespace sf::base
