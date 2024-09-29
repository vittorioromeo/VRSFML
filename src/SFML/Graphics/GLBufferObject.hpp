#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GLUniqueResource.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class GraphicsContext;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <GLenum BufferType, GLenum BindingType>
struct GLBufferObjectFuncs
{
    [[gnu::always_inline, gnu::flatten]] static void create(unsigned int& id)
    {
        glCheck(glGenBuffers(1, &id));
    }

    [[gnu::always_inline, gnu::flatten]] static void destroy(unsigned int& id)
    {
        glCheck(glDeleteBuffers(1, &id));
    }

    [[gnu::always_inline, gnu::flatten]] static void bind(unsigned int id)
    {
        glCheck(glBindBuffer(BufferType, id));
    }

    [[gnu::always_inline, gnu::flatten]] static void get(unsigned int& id)
    {
        glCheck(glGetIntegerv(BindingType, reinterpret_cast<GLint*>(&id)));
    }
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <GLenum BufferType, GLenum BindingType>
struct GLBufferObject : GLUniqueResource<GLBufferObjectFuncs<BufferType, BindingType>>
{
    enum : GLenum
    {
        bufferType  = BufferType,
        bindingType = BindingType
    };

    using GLUniqueResource<GLBufferObjectFuncs<BufferType, BindingType>>::GLUniqueResource;
};

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLVertexBufferObject : priv::GLBufferObject<GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING>
{
    using priv::GLBufferObject<GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING>::GLBufferObject;
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLElementBufferObject : priv::GLBufferObject<GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING>
{
    using priv::GLBufferObject<GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING>::GLBufferObject;
};

} // namespace sf
