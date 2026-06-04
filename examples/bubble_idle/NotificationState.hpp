#pragma once

#include "ZancleBase/String.hpp"
#include "ZancleBase/Vector.hpp"

#include "ExampleUtils/Progress.hpp"


////////////////////////////////////////////////////////////
struct NotificationData
{
    const char* title;
    zb::String  content;
};


////////////////////////////////////////////////////////////
struct NotificationState
{
    zb::Vector<NotificationData> queue;
    TimedCountdown               countdown{.duration = 750.f};
};
