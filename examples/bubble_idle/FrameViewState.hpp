#pragma once

#include "SFML/Graphics/View.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
struct FrameViewState
{
    sf::Vec2f resolution;
    sf::View  scaledTopGameView;
    sf::View  gameBackgroundView;
};
