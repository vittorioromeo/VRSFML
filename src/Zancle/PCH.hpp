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

        #include "Zancle/System/WindowsHeader.hpp"

        #include <dinput.h>
        #include <mmsystem.h>

    #endif // ZA_SYSTEM_WINDOWS

    #include "Zancle/System/Err.hpp"
    #include "Zancle/System/Path.hpp"
    #include "Zancle/System/Rect2.hpp"
    #include "Zancle/System/Time.hpp"
    #include "Zancle/System/Utf8String.hpp"
    #include "Zancle/System/Vec2.hpp"
    #include "ZancleBase/Assert.hpp"
    #include "ZancleBase/Builtin/Memcpy.hpp"
    #include "ZancleBase/InPlacePImpl.hpp"
    #include "ZancleBase/IntTypes.hpp"
    #include "ZancleBase/Macros.hpp"
    #include "ZancleBase/Optional.hpp"
    #include "ZancleBase/UniquePtr.hpp"

    #ifdef ZA_BUILD_AUDIO
    // #include <miniaudio.h> // TODO P2: restore, needs CMake to be fixed
    #endif

    #ifdef ZA_BUILD_WINDOW
        #include "Zancle/Window/Event.hpp"
    #endif

#endif // ZA_ENABLE_PCH
