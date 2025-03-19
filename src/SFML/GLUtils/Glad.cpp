#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef GLAD_GL_IMPLEMENTATION_INCLUDED
    #define GLAD_GL_IMPLEMENTATION_INCLUDED
    #define GLAD_GL_IMPLEMENTATION
    #include <glad/gl.h>
#endif
