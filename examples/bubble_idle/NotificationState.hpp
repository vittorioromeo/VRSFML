#pragma once

#include "Countdown.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
struct NotificationData
{
    const char*      title;
    sf::base::String content;
};


////////////////////////////////////////////////////////////
struct NotificationState
{
    sf::base::Vector<NotificationData> queue;
    TargetedCountdown                  countdown{.startingValue = 750.f};
};
