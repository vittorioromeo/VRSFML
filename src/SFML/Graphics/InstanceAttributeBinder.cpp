// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/InstanceAttributeBinder.hpp"

#include "SFML/Graphics/GlDataType.hpp"
#include "SFML/Graphics/Priv/EnumToGlEnumConversions.hpp"
#include "SFML/Graphics/VAOHandle.hpp"
#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
InstanceAttributeBinder::InstanceAttributeBinder(const base::SizeT instanceCount, VAOHandle& vaoHandle) :
    m_vaoHandle(vaoHandle),
    m_instanceCount(instanceCount)
{
    SFML_BASE_ASSERT(instanceCount > 0u);
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::bindVBO(VBOHandle& vboHandle)
{
    vboHandle.bind();

    m_vboBound = true;
    m_uploaded = false;
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::uploadData(const void* const data, const base::SizeT stride)
{
    SFML_BASE_ASSERT(data != nullptr);
    SFML_BASE_ASSERT(stride > 0u);

    // Auto-bind the next VBO from the VAOHandle's internal pool
    m_vaoHandle.bindNextVBO();
    m_vboBound = true;

    glCheck(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(stride * m_instanceCount), data, GL_STREAM_DRAW));

    m_uploaded = true;
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::setup(
    const unsigned int location,
    const unsigned int size,
    const GlDataType   type,
    const bool         normalized,
    const base::SizeT  stride,
    const base::SizeT  fieldOffset) const
{
    SFML_BASE_ASSERT(size >= 1u && size <= 4u);
    SFML_BASE_ASSERT(stride > 0u);
    SFML_BASE_ASSERT(fieldOffset < stride);
    SFML_BASE_ASSERT(m_vboBound);
    SFML_BASE_ASSERT(m_uploaded);

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
