#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GLUniqueResource.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLVertexArrayObjectFuncs
{
    [[gnu::always_inline, gnu::flatten]] static void create(unsigned int& id)
    {
        glCheck(glGenVertexArrays(1, &id));
    }

    [[gnu::always_inline, gnu::flatten]] static void destroy(unsigned int& id)
    {
        glCheck(glDeleteVertexArrays(1, &id));
    }

    [[gnu::always_inline, gnu::flatten]] static void bind(unsigned int id)
    {
        glCheck(glBindVertexArray(id));
    }

    [[gnu::always_inline, gnu::flatten]] static void get(unsigned int& id)
    {
        glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint*>(&id)));
    }
};

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLVertexArrayObject : GLUniqueResource<priv::GLVertexArrayObjectFuncs>
{
    using GLUniqueResource<priv::GLVertexArrayObjectFuncs>::GLUniqueResource;
};

} // namespace sf
