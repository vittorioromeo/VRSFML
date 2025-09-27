#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
/// \brief Internal helper struct defining OpenGL functions for Renderbuffer Objects (RBOs).
///
/// This template acts as a policy class for `GLUniqueResource`, providing
/// the necessary static functions tailored to OpenGL Renderbuffer Objects.
///
/// Creation and destruction of renderbuffers is always done on the shared context.
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
/// \brief RAII wrapper for an OpenGL Renderbuffer Object (RBO).
/// \ingroup glutils
///
/// Manages the lifecycle (creation, destruction) and binding of an OpenGL Renderbuffer Object.
///
////////////////////////////////////////////////////////////
struct GLRenderBufferObject : GLUniqueResource<priv::GLRenderBufferObjectFuncs>
{
    using GLUniqueResource<priv::GLRenderBufferObjectFuncs>::GLUniqueResource;
};

} // namespace sf
