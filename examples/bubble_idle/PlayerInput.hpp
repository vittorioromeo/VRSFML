#pragma once

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
struct Cat;


////////////////////////////////////////////////////////////
struct PlayerInputState
{
    bool escWasPressed{false};

    zb::Optional<za::Vec2f> dragPosition;
    float                   scroll{0.f};
    float                   actualScroll{0.f};

    za::Vec2f lastMousePos;

    float                   catDragPressDuration{0.f};
    zb::Optional<za::Vec2f> catDragOrigin;
    zb::Vector<Cat*>        draggedCats;
    bool                    draggedCatsStartedWithTouch{false};
    bool                    draggedCatsStartedFromAOESelection{false};
    Cat*                    catToPlace{nullptr};

    zb::Vector<zb::Optional<za::Vec2f>> fingerPositions;
};


////////////////////////////////////////////////////////////
struct FrameInput
{
    zb::Optional<za::Vec2f> clickPosition;
    zb::Vector<za::Vec2f>   downFingers;
    za::Vec2i               windowSpaceMouseOrFingerPos;
    za::Vec2f               mousePos;
};


////////////////////////////////////////////////////////////
struct FrameUpdateState
{
    zb::I64 elapsedUs{0};
    float   cursorGrow{0.f};
};
