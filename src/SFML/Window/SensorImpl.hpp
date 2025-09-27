#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Sensor.hpp"

#if defined(SFML_SYSTEM_WINDOWS) || defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD) || \
    defined(SFML_SYSTEM_OPENBSD) || defined(SFML_SYSTEM_NETBSD) || defined(SFML_SYSTEM_MACOS) ||  \
    defined(SFML_SYSTEM_EMSCRIPTEN)

    #include "SFML/Window/Stub/StubSensorImpl.hpp"


namespace sf::priv
{
using SensorImpl = StubSensorImpl;
}

#elif defined(SFML_SYSTEM_IOS)

    #include "SFML/Window/iOS/SensorImpl.hpp"

#elif defined(SFML_SYSTEM_ANDROID)

    #include "SFML/Window/Android/SensorImpl.hpp"

#endif
