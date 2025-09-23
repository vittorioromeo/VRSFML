#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/GLUtils/GLBufferObject.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/SizeT.hpp"

#ifdef SFML_OPENGL_ES
    #include "SFML/System/Err.hpp"

    #include "SFML/Base/Abort.hpp"
#endif


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Manages a persistently mapped OpenGL buffer.
/// \ingroup glutils
///
/// This class provides a convenient way to work with OpenGL buffer objects
/// that are persistently mapped for writing. This allows direct memory access
/// to the buffer's storage from the CPU, potentially improving performance
/// for frequently updated buffers by avoiding explicit `glBufferSubData` calls.
///
/// It handles the creation, mapping, resizing (reallocation), and unmapping
/// of the buffer. The buffer memory is mapped with write access and persistent
/// mapping flags. Synchronization is typically required externally using fences
/// or barriers before the GPU reads from the updated buffer regions.
///
/// \tparam TBufferObject The type of the underlying buffer object (e.g., `GLVertexBufferObject`).
///
////////////////////////////////////////////////////////////
template <typename TBufferObject>
class [[nodiscard]] GLPersistentBuffer
{
public:
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GLPersistentBuffer(TBufferObject& obj) : m_obj{&obj}
    {
    }

    ////////////////////////////////////////////////////////////
    ~GLPersistentBuffer()
    {
#ifndef SFML_OPENGL_ES
        unmapIfNeeded();
#endif
    }

    ////////////////////////////////////////////////////////////
    GLPersistentBuffer(const GLPersistentBuffer&)            = delete;
    GLPersistentBuffer& operator=(const GLPersistentBuffer&) = delete;

    ////////////////////////////////////////////////////////////
    GLPersistentBuffer(GLPersistentBuffer&& rhs) noexcept :
        m_obj{rhs.m_obj},
        m_mappedPtr{rhs.m_mappedPtr},
        m_capacity{rhs.m_capacity}
    {
        rhs.m_obj       = nullptr;
        rhs.m_mappedPtr = nullptr;
    }

    ////////////////////////////////////////////////////////////
    GLPersistentBuffer& operator=(GLPersistentBuffer&& rhs) noexcept
    {
        if (&rhs == this)
            return *this;

#ifndef SFML_OPENGL_ES
        unmapIfNeeded();
#endif

        m_obj       = rhs.m_obj;
        m_mappedPtr = rhs.m_mappedPtr;
        m_capacity  = rhs.m_capacity;

        rhs.m_obj       = nullptr;
        rhs.m_mappedPtr = nullptr;

        return *this;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserve(const base::SizeT byteCount)
    {
        if (m_capacity >= byteCount) [[likely]]
            return;

        reserveImpl(byteCount);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] void* data()
    {
        return m_mappedPtr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] const void* data() const
    {
        return m_mappedPtr;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unmapIfNeeded()
    {
#ifdef SFML_OPENGL_ES
        priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
        base::abort();
#else
        if (m_mappedPtr == nullptr)
            return;

        m_mappedPtr = nullptr;

        SFML_BASE_ASSERT(m_obj != nullptr);
        m_obj->bind();

        [[maybe_unused]] const bool rc = glCheck(glUnmapNamedBuffer(m_obj->getId()));
        SFML_BASE_ASSERT(rc);
#endif
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void adjustObjPointer(TBufferObject& obj)
    {
        // This is needed to avoid dangling pointers when the object is moved as part
        // of persistent GPU batch.
        m_obj = &obj;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void flushWritesToGPU([[maybe_unused]] const base::SizeT unitSize,
                                                               [[maybe_unused]] const base::SizeT count,
                                                               [[maybe_unused]] const base::SizeT offset) const
    {
#ifdef SFML_OPENGL_ES
        priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
        base::abort();
#else
        const auto objId = m_obj->getId();

        SFML_BASE_ASSERT(objId != 0u);
        SFML_BASE_ASSERT(m_mappedPtr != nullptr);

        glCheck(glFlushMappedNamedBufferRange(objId,
                                              static_cast<GLintptr>(unitSize * offset),
                                              static_cast<GLsizeiptr>(unitSize * count)));
#endif
    }

private:
    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void reserveImpl([[maybe_unused]] const base::SizeT byteCount)
    {
#ifdef SFML_OPENGL_ES
        priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
        base::abort();
#else
        SFML_BASE_ASSERT(m_capacity < byteCount);
        SFML_BASE_ASSERT(m_obj != nullptr);

        const auto geometricGrowthTarget = m_capacity + (m_capacity / 2u); // Equivalent to `capacity * 1.5`
        const auto newCapacity           = SFML_BASE_MAX(byteCount, geometricGrowthTarget);

        m_obj->bind();

        unmapIfNeeded();

        m_obj->reallocate();
        m_obj->bind();

        const auto objId = m_obj->getId();
        SFML_BASE_ASSERT(objId != 0u);

        glCheck(glNamedBufferStorage(objId,
                                     static_cast<GLsizeiptr>(newCapacity),
                                     /* data */ nullptr,
                                     GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));

        m_mappedPtr = glCheck(
            glMapNamedBufferRange(objId,
                                  /* offset */ 0u,
                                  /* length */ static_cast<GLsizeiptr>(newCapacity),
                                  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                      GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                                      GL_MAP_FLUSH_EXPLICIT_BIT));

        m_capacity = newCapacity;
#endif
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    TBufferObject* m_obj;                //!< Associated GL object handle
    void*          m_mappedPtr{nullptr}; //!< Write-only mapped pointer
    base::SizeT    m_capacity{0u};       //!< Currently allocated capacity of the buffer
};

////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class GLPersistentBuffer<GLVertexBufferObject>;
extern template class GLPersistentBuffer<GLElementBufferObject>;

} // namespace sf
