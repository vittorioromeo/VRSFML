// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/InstanceAttributeBinder.hpp"

#include "SFML/Graphics/GlDataType.hpp"
#include "SFML/Graphics/Priv/EnumToGlEnumConversions.hpp"
#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
void InstanceAttributeBinder::bindVBO(VBOHandle& vboHandle)
{
    vboHandle.bind();
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::uploadData(const base::SizeT instanceCount, const void* const data, const base::SizeT stride)
{
    glCheck(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(stride * instanceCount), data, GL_STREAM_DRAW));
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::setup(
    const unsigned int location,
    const unsigned int size,
    const GlDataType   type,
    const bool         normalized,
    const base::SizeT  stride,
    const base::SizeT  fieldOffset)
{
    glCheck(glEnableVertexAttribArray(location));

    glCheck(glVertexAttribPointer(/*      index */ location,
                                  /*       size */ static_cast<GLint>(size),
                                  /*       type */ priv::dataTypeToOpenGLDataType(type),
                                  /* normalized */ normalized ? GL_TRUE : GL_FALSE,
                                  /*     stride */ static_cast<GLsizei>(stride),
                                  /*     offset */ reinterpret_cast<void*>(fieldOffset)));

    glCheck(glVertexAttribDivisor(location, 1));
}

} // namespace sf
