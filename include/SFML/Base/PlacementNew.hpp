#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
struct PNewMarker
{
};

} // namespace sf::base::priv


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr void* operator new(decltype(sizeof(int)),
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
