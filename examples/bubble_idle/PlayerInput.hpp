#pragma once

#include "Cat.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
struct PlayerInputState
{
    bool escWasPressed{false};

    sf::base::Optional<sf::Vec2f> dragPosition;
    float                         scroll{0.f};
    float                         actualScroll{0.f};

    sf::Vec2f lastMousePos;

    float                         catDragPressDuration{0.f};
    sf::base::Optional<sf::Vec2f> catDragOrigin;
    sf::base::Vector<Cat*>        draggedCats;
    bool                          draggedCatsStartedWithTouch{false};
    bool                          draggedCatsStartedFromAOESelection{false};
    Cat*                          catToPlace{nullptr};

    sf::base::Vector<sf::base::Optional<sf::Vec2f>> fingerPositions;
};


////////////////////////////////////////////////////////////
struct FrameInput
{
    sf::base::Optional<sf::Vec2f> clickPosition;
    sf::base::Vector<sf::Vec2f>   downFingers;
    sf::Vec2i                     windowSpaceMouseOrFingerPos;
    sf::Vec2f                     mousePos;
};
