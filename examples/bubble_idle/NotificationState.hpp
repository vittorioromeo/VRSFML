#pragma once

#include "ExampleUtils/Progress.hpp"

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
    TimedCountdown                     countdown{.duration = 750.f};
};
