#pragma once

#include "Zancle/Graphics/Color.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
inline constexpr za::Vec2f gameScreenSize{1366.f, 768.f};
inline constexpr auto      gameScreenSizeUInt = gameScreenSize.toVec2u();

////////////////////////////////////////////////////////////
inline constexpr float nGameScreens = 9.f;

////////////////////////////////////////////////////////////
inline constexpr za::Vec2f boundaries{gameScreenSize.x * nGameScreens, gameScreenSize.y};

////////////////////////////////////////////////////////////
inline constexpr za::Color colorBlueOutline{50u, 84u, 135u};
