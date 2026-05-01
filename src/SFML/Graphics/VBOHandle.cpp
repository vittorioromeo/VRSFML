// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/GLUtils/GLBufferObject.hpp"
#include "SFML/GLUtils/GLPersistentRingBuffer.hpp"
#include "SFML/GLUtils/GLUniqueResource.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct VBOHandle::Impl
{
    GLVertexBufferObject vbo;

#ifndef SFML_OPENGL_ES
    GLPersistentRingBuffer<GLVertexBufferObject> persistentRingBuffer;
#endif

    explicit Impl(GLVertexBufferObject&& theVbo) : vbo(SFML_BASE_MOVE(theVbo))
    {
    }

    ~Impl()
    {
#ifndef SFML_OPENGL_ES
        persistentRingBuffer.destroy(vbo);
#endif
    }

    Impl(const Impl&)            = delete;
    Impl& operator=(const Impl&) = delete;
    Impl(Impl&&) noexcept        = default;

    Impl& operator=(Impl&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

#ifndef SFML_OPENGL_ES
        persistentRingBuffer.destroy(vbo);
#endif

        vbo = SFML_BASE_MOVE(rhs.vbo);

#ifndef SFML_OPENGL_ES
        persistentRingBuffer = SFML_BASE_MOVE(rhs.persistentRingBuffer);
#endif

        return *this;
    }
};


////////////////////////////////////////////////////////////
VBOHandle::VBOHandle() : m_impl{tryCreateGLUniqueResource<GLVertexBufferObject>().value()}
{
}


////////////////////////////////////////////////////////////
VBOHandle::~VBOHandle() = default;


////////////////////////////////////////////////////////////
void VBOHandle::bind()
{
    m_impl->vbo.bind();
}


////////////////////////////////////////////////////////////
base::SizeT VBOHandle::uploadStreamingData(const void* const data, const base::SizeT byteCount)
{
    SFML_BASE_ASSERT(data != nullptr);
    SFML_BASE_ASSERT(byteCount > 0u);

#ifdef SFML_OPENGL_ES
    bind();
    glCheck(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(byteCount), data, GL_STREAM_DRAW));
    return 0u;
#else
    const auto byteOffset = m_impl->persistentRingBuffer.beginWrite(m_impl->vbo, byteCount);

    SFML_BASE_MEMCPY(static_cast<char*>(m_impl->persistentRingBuffer.data()) + byteOffset, data, byteCount);
    m_impl->persistentRingBuffer.flushBytesToGPU(m_impl->vbo, byteOffset, byteCount);

    bind();
    return byteOffset;
#endif
}


////////////////////////////////////////////////////////////
void VBOHandle::commitPendingUploads()
{
#ifndef SFML_OPENGL_ES
    m_impl->persistentRingBuffer.commit();
#endif
}


////////////////////////////////////////////////////////////
void VBOHandle::rollbackPendingUploads() noexcept
{
#ifndef SFML_OPENGL_ES
    m_impl->persistentRingBuffer.rollback();
#endif
}

} // namespace sf
