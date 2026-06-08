#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md

#ifndef ZA_ENABLE_PCH
    #error "PCH header included, but `ZA_ENABLE_PCH` was not defined"
#else // ZA_ENABLE_PCH

////////////////////////////////////////////////////////////
// Precompiled Headers
////////////////////////////////////////////////////////////

    #include "Zancle/Config.hpp"

    #ifdef ZA_SYSTEM_WINDOWS

        #include "Zancle/Base/WindowsHeader.hpp"

        #include <dinput.h>
        #include <mmsystem.h>

    #endif // ZA_SYSTEM_WINDOWS

    #include "Zancle/Err/Err.hpp"

    #include "Zancle/IO/Path.hpp"

    #include "Zancle/String/Utf8String.hpp"

    #include "Zancle/Chrono/Time.hpp"

    #include "Zancle/Geometry/Rect2.hpp"
    #include "Zancle/Geometry/Vec2.hpp"

    #include "Zancle/Vocabulary/InPlacePImpl.hpp"
    #include "Zancle/Vocabulary/Optional.hpp"
    #include "Zancle/Vocabulary/UniquePtr.hpp"

    #include "Zancle/Base/Assert.hpp"
    #include "Zancle/Base/IntTypes.hpp"
    #include "Zancle/Base/Macros.hpp"
    #include "Zancle/Base/Memcpy.hpp"

    #ifdef ZA_BUILD_AUDIO
    // #include <miniaudio.h> // TODO P2: restore, needs CMake to be fixed
    #endif

    #ifdef ZA_BUILD_WINDOW
        #include "Zancle/Window/Event.hpp"
    #endif

#endif // ZA_ENABLE_PCH
