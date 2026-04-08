#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp" // IWYU pragma: keep

#include "SFML/GLUtils/GLBufferObject.hpp"
#include "SFML/GLUtils/GLUniqueResource.hpp"

#include "SFML/Base/SizeT.hpp"

#ifdef SFML_OPENGL_ES
    #include "SFML/System/Err.hpp"

    #include "SFML/Base/Abort.hpp"
#else
    #include "SFML/GLUtils/GLCheck.hpp"
    #include "SFML/GLUtils/Glad.hpp"

    #include "SFML/Base/Assert.hpp"
    #include "SFML/Base/Builtin/Memcpy.hpp"
    #include "SFML/Base/Macros.hpp"
    #include "SFML/Base/MinMaxMacros.hpp"
#endif


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Manages a persistently mapped OpenGL buffer
/// \ingroup glutils
///
/// Wraps an OpenGL buffer object that is allocated with immutable
/// storage (`glNamedBufferStorage`) and persistently mapped for writing
/// from the CPU side. By keeping the mapping alive across frames the
/// CPU can update buffer contents through a plain pointer, avoiding the
/// overhead of explicit `glBufferSubData` calls and the implicit
/// synchronization they involve.
///
/// The class is responsible for:
/// - Lazily allocating and mapping the underlying storage on first use
/// - Growing the buffer geometrically (capacity * 1.5) on reservation
///   requests that exceed the current capacity, copying any existing
///   contents into the freshly allocated storage
/// - Unmapping the buffer on demand
/// - Flushing explicit ranges of writes to the GPU
///
/// The buffer is mapped with the following flags:
/// `GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
/// GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
/// GL_MAP_FLUSH_EXPLICIT_BIT`. The caller is responsible for any
/// CPU/GPU synchronization (typically via fences or memory barriers)
/// before the GPU reads from regions that the CPU has just written.
///
/// Persistent mapping requires desktop OpenGL 4.4+ functionality and is
/// **not** available in OpenGL ES; calling `unmapIfNeeded`,
/// `flushWritesToGPU`, or causing a reservation under
/// `SFML_OPENGL_ES` will abort the program with a fatal error.
///
/// \tparam TBufferObject The type of the underlying buffer object
///                       (e.g. `GLVertexBufferObject`,
///                       `GLElementBufferObject`)
///
////////////////////////////////////////////////////////////
template <typename TBufferObject>
class [[nodiscard]] GLPersistentBuffer
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Ensure the buffer can hold at least `byteCount` bytes
    ///
    /// If the current capacity is already sufficient the call is a
    /// no-op (the fast path). Otherwise a new, larger buffer is
    /// allocated and mapped, the existing contents (if any) are copied
    /// into it, and `obj` is replaced with the new buffer object.
    ///
    /// \param obj       The buffer object to (re)allocate. Will be
    ///                  move-assigned a fresh instance on growth.
    /// \param byteCount Minimum number of bytes to make available
    ///
    /// \return `true` if a reallocation occurred, `false` if the
    ///         existing storage was already large enough
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] bool reserve(TBufferObject& obj, const base::SizeT byteCount)
    {
        if (m_capacity >= byteCount) [[likely]]
            return false;

        reserveImpl(obj, byteCount);
        return true;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get a write pointer to the persistently mapped storage
    ///
    /// \return Pointer to the mapped buffer memory, or `nullptr` if no
    ///         storage has been reserved yet
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] void* data()
    {
        return m_mappedPtr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get a read pointer to the persistently mapped storage
    ///
    /// \return Pointer to the mapped buffer memory, or `nullptr` if no
    ///         storage has been reserved yet
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] const void* data() const
    {
        return m_mappedPtr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Unmap the buffer if it is currently mapped
    ///
    /// Calls `glUnmapNamedBuffer` on the underlying buffer object and
    /// clears the cached mapped pointer. Safe to call when no mapping
    /// exists (no-op).
    ///
    /// \param obj The buffer object whose mapping should be released
    ///
    /// \warning Aborts the program when compiled for OpenGL ES.
    ///
    ////////////////////////////////////////////////////////////
    void unmapIfNeeded([[maybe_unused /* not available in EGL */]] TBufferObject& obj)
    {
#ifdef SFML_OPENGL_ES
        priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
        base::abort();
#else
        if (m_mappedPtr == nullptr)
            return;

        m_mappedPtr = nullptr;

        obj.bind();

        [[maybe_unused]] const bool rc = glCheck(glUnmapNamedBuffer(obj.getId()));
        SFML_BASE_ASSERT(rc);
#endif
    }


    ////////////////////////////////////////////////////////////
    /// \brief Flush a CPU-written range so that it becomes visible to the GPU
    ///
    /// Wraps `glFlushMappedNamedBufferRange`. Because the buffer is
    /// mapped with `GL_MAP_FLUSH_EXPLICIT_BIT`, the driver will not
    /// propagate CPU writes to GPU-visible memory until this function
    /// is called. The flushed region is `[offset, offset + count) *
    /// unitSize` bytes within the buffer.
    ///
    /// This function does **not** issue a memory barrier; the caller
    /// must still ensure proper synchronization (e.g. `glMemoryBarrier`
    /// or fence sync) before the GPU consumes the data.
    ///
    /// \param obj      The buffer object containing the persistent mapping
    /// \param unitSize Size in bytes of a single element
    /// \param count    Number of elements in the range to flush
    /// \param offset   Offset (in elements) of the first element to flush
    ///
    /// \warning Aborts the program when compiled for OpenGL ES.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushWritesToGPU([[maybe_unused]] const TBufferObject& obj,
                                                 [[maybe_unused]] const base::SizeT    unitSize,
                                                 [[maybe_unused]] const base::SizeT    count,
                                                 [[maybe_unused]] const base::SizeT    offset) const
    {
#ifdef SFML_OPENGL_ES
        priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
        base::abort();
#else
        const auto objId = obj.getId();

        SFML_BASE_ASSERT(objId != 0u);
        SFML_BASE_ASSERT(m_mappedPtr != nullptr);

        glCheck(glFlushMappedNamedBufferRange(objId,
                                              static_cast<GLintptr>(unitSize * offset),
                                              static_cast<GLsizeiptr>(unitSize * count)));
#endif
    }


private:
    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void reserveImpl([[maybe_unused]] TBufferObject&    obj,
                                                  [[maybe_unused]] const base::SizeT byteCount)
    {
#ifdef SFML_OPENGL_ES
        priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
        base::abort();
#else
        SFML_BASE_ASSERT(m_capacity < byteCount);

        const auto geometricGrowthTarget = m_capacity + (m_capacity / 2u); // Equivalent to `capacity * 1.5`
        const auto newCapacity           = SFML_BASE_MAX(byteCount, geometricGrowthTarget);

        auto newObj = tryCreateGLUniqueResource<TBufferObject>().value();
        newObj.bind();

        glCheck(glNamedBufferStorage(newObj.getId(),
                                     static_cast<GLsizeiptr>(newCapacity),
                                     /* data */ nullptr,
                                     GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));

        void* const newMappedPtr = glCheck(
            glMapNamedBufferRange(newObj.getId(),
                                  /* offset */ 0u,
                                  /* length */ static_cast<GLsizeiptr>(newCapacity),
                                  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                      GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
                                      GL_MAP_FLUSH_EXPLICIT_BIT));

        SFML_BASE_ASSERT(newMappedPtr != nullptr);

        if (m_mappedPtr != nullptr)
        {
            SFML_BASE_MEMCPY(newMappedPtr, m_mappedPtr, m_capacity);
            unmapIfNeeded(obj);
        }

        obj = SFML_BASE_MOVE(newObj);
        obj.bind();

        m_mappedPtr = newMappedPtr;
        m_capacity  = newCapacity;
#endif
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    void*       m_mappedPtr{nullptr}; //!< Write-only mapped pointer
    base::SizeT m_capacity{0u};       //!< Currently allocated capacity of the buffer
};


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class GLPersistentBuffer<GLVertexBufferObject>;
extern template class GLPersistentBuffer<GLElementBufferObject>;

} // namespace sf
