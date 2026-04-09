#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUniqueResource.hpp"
#include "SFML/GLUtils/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal policy struct exposing OpenGL functions for Vertex Array Objects (VAOs)
///
/// Acts as a policy class for `GLUniqueResource`, providing the static
/// `create`, `destroy`, `bind` and `get` functions tailored to OpenGL
/// Vertex Array Objects.
///
/// Unlike buffer objects, VAOs are container objects whose state is
/// **not** shareable across OpenGL contexts. As a result, this policy
/// performs all operations on the currently active context -- there is
/// no shared-context guard. See `GLVAOGroup` for a helper that handles
/// per-context VAO management on top of shared VBO/EBO storage.
///
////////////////////////////////////////////////////////////
struct GLVertexArrayObjectFuncs
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void create(unsigned int& id)
    {
        glCheck(glGenVertexArrays(1, &id));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void destroy(unsigned int& id)
    {
        glCheck(glDeleteVertexArrays(1, &id));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void bind(unsigned int id)
    {
        glCheck(glBindVertexArray(id));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void get(unsigned int& id)
    {
        glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint*>(&id)));
    }
};

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief RAII wrapper for an OpenGL Vertex Array Object (VAO)
/// \ingroup glutils
///
/// Manages the lifecycle (creation, destruction) and binding of an
/// OpenGL Vertex Array Object on the currently active context.
///
/// VAOs are not shareable across contexts: a VAO created on one context
/// cannot be bound from another. If you need to use the same vertex
/// layout from multiple contexts, see `GLVAOGroup`, which lazily
/// creates one VAO per context while sharing the underlying VBO/EBO.
///
////////////////////////////////////////////////////////////
struct GLVertexArrayObject : GLUniqueResource<priv::GLVertexArrayObjectFuncs>
{
    using GLUniqueResource<priv::GLVertexArrayObjectFuncs>::GLUniqueResource;
};

} // namespace sf
