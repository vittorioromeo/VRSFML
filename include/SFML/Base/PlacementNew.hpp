// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Base/SizeT.hpp>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
struct PNewMarker
{
};

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr void* operator new(sf::base::SizeT,
                                                                                  sf::base::priv::PNewMarker,
                                                                                  void* ptr) noexcept
{
    return ptr;
}

////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline constexpr void operator delete(void*, sf::base::priv::PNewMarker, void*) noexcept
{
}

////////////////////////////////////////////////////////////
#define SFML_BASE_PLACEMENT_NEW(...) ::new (::sf::base::priv::PNewMarker{}, __VA_ARGS__)
