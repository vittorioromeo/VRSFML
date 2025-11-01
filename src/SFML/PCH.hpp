#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

#ifndef SFML_ENABLE_PCH
    #error "PCH header included, but `SFML_ENABLE_PCH` was not defined"
#else // SFML_ENABLE_PCH

////////////////////////////////////////////////////////////
// Precompiled Headers
////////////////////////////////////////////////////////////

    #include "SFML/Config.hpp"

    #ifdef SFML_SYSTEM_WINDOWS

        #include "SFML/System/Win32/WindowsHeader.hpp"

        #include <dinput.h>
        #include <mmsystem.h>

    #endif // SFML_SYSTEM_WINDOWS

    #include "SFML/System/Err.hpp"
    #include "SFML/System/Path.hpp"
    #include "SFML/System/Rect2.hpp"
    #include "SFML/System/String.hpp"
    #include "SFML/System/Time.hpp"
    #include "SFML/System/Vec2.hpp"

    #include "SFML/Base/Assert.hpp"
    #include "SFML/Base/Builtin/Memcpy.hpp"
    #include "SFML/Base/InPlacePImpl.hpp"
    #include "SFML/Base/IntTypes.hpp"
    #include "SFML/Base/Macros.hpp"
    #include "SFML/Base/Optional.hpp"
    #include "SFML/Base/UniquePtr.hpp"

    #include <atomic>
    #include <chrono>
    #include <mutex>
    #include <string>

    #include <cstdlib>

    #ifdef SFML_BUILD_EXAMPLES
        #include <random>

        #include <cmath>
    #endif // SFML_BUILD_EXAMPLES

    #ifdef SFML_BUILD_TEST_SUITE
        #include <doctest/parts/doctest_fwd.h>
    #endif // SFML_BUILD_TEST_SUITE

    #ifdef SFML_BUILD_AUDIO
    // #include <miniaudio.h> // TODO P2: restore, needs CMake to be fixed
    #endif

    #ifdef SFML_BUILD_WINDOW
        #include "SFML/Window/Event.hpp"
    #endif

#endif // SFML_ENABLE_PCH
