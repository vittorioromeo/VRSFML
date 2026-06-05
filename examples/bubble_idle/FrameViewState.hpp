#pragma once

#include "Zancle/Graphics/View.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
struct FrameViewState
{
    za::Vec2f resolution;
    za::View  scaledTopGameView;
    za::View  gameBackgroundView;
};
