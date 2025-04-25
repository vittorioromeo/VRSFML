#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLSharedContextGuard.hpp"
#include "SFML/GLUtils/GLUniqueResource.hpp"
#include "SFML/GLUtils/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLRenderBufferObjectFuncs
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void create(unsigned int& id)
    {
        // Always create buffers on the shared context
        GLSharedContextGuard guard;
        glCheck(glGenRenderbuffers(1, &id));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void destroy(unsigned int& id)
    {
        // Always destroy buffers on the shared context
        GLSharedContextGuard guard;
        glCheck(glDeleteRenderbuffers(1, &id));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void bind(unsigned int id)
    {
        glCheck(glBindRenderbuffer(GL_RENDERBUFFER, id));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void get(unsigned int& id)
    {
        glCheck(glGetIntegerv(GL_RENDERBUFFER_BINDING, reinterpret_cast<GLint*>(&id)));
    }
};

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLRenderBufferObject : GLUniqueResource<priv::GLRenderBufferObjectFuncs>
{
    using GLUniqueResource<priv::GLRenderBufferObjectFuncs>::GLUniqueResource;
};

} // namespace sf
