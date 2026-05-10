#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "GameConstants.hpp"
#include "Playthrough.hpp"
#include "Profile.hpp"


////////////////////////////////////////////////////////////
struct [[nodiscard]] MainGameStorage
{
    Profile       profile;
    GameConstants gameConstants;
    Playthrough   ptMain;
    Playthrough   ptSpeedrun;
};
