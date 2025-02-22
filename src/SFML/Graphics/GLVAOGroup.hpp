#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GLBufferObject.hpp"
#include "SFML/Graphics/GLVertexArrayObject.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct GLVAOGroup
{
    GLVertexArrayObject   vao; //!< Vertex array object
    GLVertexBufferObject  vbo; //!< Associated vertex buffer object
    GLElementBufferObject ebo; //!< Associated element index buffer object

    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        vao.bind();
        vbo.bind();
        ebo.bind();
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] unsigned int getId() const
    {
        return vao.getId();
    }
};

} // namespace sf
