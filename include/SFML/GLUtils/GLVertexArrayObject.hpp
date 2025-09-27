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
/// \brief Internal helper struct defining OpenGL functions for Vertex Array Objects (VAOs).
///
/// This template acts as a policy class for `GLUniqueResource`, providing
/// the necessary static functions tailored to OpenGL Vertex Array Objects.
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
/// \brief RAII wrapper for an OpenGL Vertex Array Object (VAO).
/// \ingroup glutils
///
/// Manages the lifecycle (creation, destruction) and binding of an OpenGL Vertex Array Object.
///
////////////////////////////////////////////////////////////
struct GLVertexArrayObject : GLUniqueResource<priv::GLVertexArrayObjectFuncs>
{
    using GLUniqueResource<priv::GLVertexArrayObjectFuncs>::GLUniqueResource;
};

} // namespace sf
