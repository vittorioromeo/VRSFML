#pragma once

#include "SFML/Graphics/Color.hpp"

#include "SFML/System/Vec2.hpp"


////////////////////////////////////////////////////////////
inline constexpr sf::Vec2f gameScreenSize{1366.f, 768.f};
inline constexpr auto      gameScreenSizeUInt = gameScreenSize.toVec2u();

////////////////////////////////////////////////////////////
inline constexpr float nGameScreens = 9.f;

////////////////////////////////////////////////////////////
inline constexpr sf::Vec2f boundaries{gameScreenSize.x * nGameScreens, gameScreenSize.y};

////////////////////////////////////////////////////////////
inline constexpr sf::Color colorBlueOutline{50u, 84u, 135u};
