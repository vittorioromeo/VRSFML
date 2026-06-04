#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"
#include "Zancle/Window/Sensor.hpp"

#if defined(ZA_SYSTEM_WINDOWS) || defined(ZA_SYSTEM_LINUX) || defined(ZA_SYSTEM_FREEBSD) || \
    defined(ZA_SYSTEM_OPENBSD) || defined(ZA_SYSTEM_NETBSD) || defined(ZA_SYSTEM_MACOS) || defined(ZA_SYSTEM_EMSCRIPTEN)

    #include "Zancle/Window/Stub/StubSensorImpl.hpp"


namespace za::priv
{
using SensorImpl = StubSensorImpl;
}

#elif defined(ZA_SYSTEM_IOS)

    #include "Zancle/Window/iOS/SensorImpl.hpp"

#elif defined(ZA_SYSTEM_ANDROID)

    #include "Zancle/Window/Android/SensorImpl.hpp"

#endif
