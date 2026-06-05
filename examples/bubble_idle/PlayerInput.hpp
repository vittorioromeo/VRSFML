#pragma once

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Container/Vector.hpp"


////////////////////////////////////////////////////////////
struct Cat;


////////////////////////////////////////////////////////////
struct PlayerInputState
{
    bool escWasPressed{false};

    za::Optional<za::Vec2f> dragPosition;
    float                   scroll{0.f};
    float                   actualScroll{0.f};

    za::Vec2f lastMousePos;

    float                   catDragPressDuration{0.f};
    za::Optional<za::Vec2f> catDragOrigin;
    za::Vector<Cat*>        draggedCats;
    bool                    draggedCatsStartedWithTouch{false};
    bool                    draggedCatsStartedFromAOESelection{false};
    Cat*                    catToPlace{nullptr};

    za::Vector<za::Optional<za::Vec2f>> fingerPositions;
};


////////////////////////////////////////////////////////////
struct FrameInput
{
    za::Optional<za::Vec2f> clickPosition;
    za::Vector<za::Vec2f>   downFingers;
    za::Vec2i               windowSpaceMouseOrFingerPos;
    za::Vec2f               mousePos;
};


////////////////////////////////////////////////////////////
struct FrameUpdateState
{
    za::I64 elapsedUs{0};
    float   cursorGrow{0.f};
};
