// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/System/SizeT.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct PNewMarker
{
};

} // namespace sf::priv


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline void* operator new(sf::priv::SizeT, sf::priv::PNewMarker, void* ptr)
{
    return ptr;
}

////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::const]] inline void operator delete(void*, sf::priv::PNewMarker, void*) noexcept
{
}

#define SFML_PRIV_PLACEMENT_NEW(...) ::new (::sf::priv::PNewMarker{}, __VA_ARGS__)
