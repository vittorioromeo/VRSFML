#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


////////////////////////////////////////////////////////////
/// This file just includes the OpenGL headers,
/// which have actually different paths on each system
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)

    // The Visual C++ version of gl.h uses WINGDIAPI and APIENTRY but doesn't define them
    #ifdef _MSC_VER
        #include "SFML/System/WindowsHeader.hpp"
    #endif

    #include <GL/gl.h>

#elif defined(SFML_SYSTEM_LINUX_OR_BSD)

    #if defined(SFML_OPENGL_ES)
        #include <GLES3/gl3.h>
    #else
        #include <GL/gl.h>
    #endif

#elif defined(SFML_SYSTEM_MACOS)

    #include <OpenGL/gl.h>

#elif defined(SFML_SYSTEM_IOS)

    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>

#elif defined(SFML_SYSTEM_ANDROID)

    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
    #include <GLES3/gl3platform.h>

#endif
