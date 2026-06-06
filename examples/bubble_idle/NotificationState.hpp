#pragma once

#include "ExampleUtils/Progress.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Container/Vector.hpp"


////////////////////////////////////////////////////////////
struct NotificationData
{
    const char* title;
    za::String  content;
};


////////////////////////////////////////////////////////////
struct NotificationState
{
    za::Vector<NotificationData> queue;
    TimedCountdown               countdown{.duration = 750.f};
};
