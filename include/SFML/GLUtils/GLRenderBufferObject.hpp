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
/// \brief Internal policy struct exposing OpenGL functions for Renderbuffer Objects (RBOs)
///
/// Acts as a policy class for `GLUniqueResource`, providing the static
/// `create`, `destroy`, `bind` and `get` functions tailored to OpenGL
/// Renderbuffer Objects.
///
/// Creation and destruction always happen on the shared GL context (via
/// `GLSharedContextGuard`) so that renderbuffers can be safely accessed
/// from any context that shares resources with it. Binding and querying
/// happen on whatever context is currently active.
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
/// \brief RAII wrapper for an OpenGL Renderbuffer Object (RBO)
/// \ingroup glutils
///
/// Manages the lifecycle (creation, destruction) and binding of an
/// OpenGL Renderbuffer Object. Renderbuffers are typically used as
/// storage for the depth, stencil, or multisampled color attachments
/// of a framebuffer.
///
////////////////////////////////////////////////////////////
struct GLRenderBufferObject : GLUniqueResource<priv::GLRenderBufferObjectFuncs>
{
    using GLUniqueResource<priv::GLRenderBufferObjectFuncs>::GLUniqueResource;
};

} // namespace sf
