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
/// \brief Internal helper struct defining OpenGL functions for a specific buffer type.
///
/// This template acts as a policy class for `GLUniqueResource`, providing
/// the necessary static functions (`create`, `destroy`, `bind`, `get`)
/// tailored to a specific type of OpenGL buffer object (like VBO or EBO)
/// defined by the `BufferType` and `BindingType` template parameters.
///
/// Creation and destruction of buffers is always done on the shared context.
///
////////////////////////////////////////////////////////////
template <GLenum BufferType, GLenum BindingType>
struct GLBufferObjectFuncs
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void create(unsigned int& id)
    {
        // Always create buffers on the shared context
        GLSharedContextGuard guard;
        glCheck(glGenBuffers(1, &id));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void destroy(unsigned int& id)
    {
        // Always destroy buffers on the shared context
        GLSharedContextGuard guard;
        glCheck(glDeleteBuffers(1, &id));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void bind(unsigned int id)
    {
        glCheck(glBindBuffer(BufferType, id));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void get(unsigned int& id)
    {
        glCheck(glGetIntegerv(BindingType, reinterpret_cast<GLint*>(&id)));
    }
};

////////////////////////////////////////////////////////////
/// \brief RAII wrapper for a generic OpenGL buffer object (e.g., VBO, EBO).
///
/// Manages the lifecycle (creation, destruction) and binding of an
/// OpenGL buffer object specified by the `BufferType` (e.g., `GL_ARRAY_BUFFER`)
/// and its corresponding binding point `BindingType` (e.g., `GL_ARRAY_BUFFER_BINDING`).
/// Inherits from `GLUniqueResource` to handle the underlying OpenGL ID.
///
/// \see `GLUniqueResource`, `GLVertexBufferObject`, `GLElementBufferObject`
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
/// \brief Specialization of GLBufferObject for Vertex Buffer Objects (VBOs).
///
/// Provides RAII management for buffers typically used to store vertex data (`GL_ARRAY_BUFFER`).
///
////////////////////////////////////////////////////////////
struct GLVertexBufferObject : priv::GLBufferObject<GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING>
{
    using priv::GLBufferObject<GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING>::GLBufferObject;
};

////////////////////////////////////////////////////////////
/// \brief Specialization of GLBufferObject for Element/Index Buffer Objects (EBOs/IBOs).
///
/// Provides RAII management for buffers typically used to store vertex indices (`GL_ELEMENT_ARRAY_BUFFER`).
///
////////////////////////////////////////////////////////////
struct GLElementBufferObject : priv::GLBufferObject<GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING>
{
    using priv::GLBufferObject<GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING>::GLBufferObject;
};

} // namespace sf
