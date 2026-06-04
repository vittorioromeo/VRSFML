#pragma once

#include "Zancle/Graphics/View.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"


////////////////////////////////////////////////////////////
struct FrameViewState
{
    za::Vec2f resolution;
    za::View  scaledTopGameView;
    za::View  gameBackgroundView;
};
