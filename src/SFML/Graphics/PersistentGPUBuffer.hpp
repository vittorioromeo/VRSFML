#pragma once
#include "OpenGLRAII.hpp"

#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] PersistentGPUBuffer
{
public:
    [[nodiscard]] explicit PersistentGPUBuffer(GLenum type, OpenGLRAII& obj) : m_type{type}, m_obj{&obj}
    {
    }

    ~PersistentGPUBuffer()
    {
        unmapIfNeeded();
    }

    PersistentGPUBuffer(const PersistentGPUBuffer&)            = delete;
    PersistentGPUBuffer& operator=(const PersistentGPUBuffer&) = delete;

    PersistentGPUBuffer(PersistentGPUBuffer&& rhs) noexcept :
    m_type{rhs.m_type},
    m_obj{rhs.m_obj},
    m_mappedPtr{rhs.m_mappedPtr},
    m_capacity{rhs.m_capacity}
    {
        rhs.m_obj       = nullptr;
        rhs.m_mappedPtr = nullptr;
    }

    PersistentGPUBuffer& operator=(PersistentGPUBuffer&& rhs) noexcept
    {
        if (&rhs == this)
            return *this;

        unmapIfNeeded();

        m_type      = rhs.m_type;
        m_obj       = rhs.m_obj;
        m_mappedPtr = rhs.m_mappedPtr;
        m_capacity  = rhs.m_capacity;

        rhs.m_obj       = nullptr;
        rhs.m_mappedPtr = nullptr;

        return *this;
    }

    [[gnu::always_inline]] void reserve(const base::SizeT byteCount)
    {
        if (m_capacity >= byteCount) [[likely]]
            return;

        reserveImpl(byteCount);
    }

    [[gnu::always_inline, gnu::flatten]] void memcpyToBuffer(const void* data, const base::SizeT byteCount)
    {
        SFML_BASE_MEMCPY(m_mappedPtr, data, byteCount);
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] void* data()
    {
        return m_mappedPtr;
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] const void* data() const
    {
        return m_mappedPtr;
    }

private:
    [[gnu::always_inline, gnu::flatten]] void unmapIfNeeded() const
    {
        if (m_mappedPtr == nullptr)
            return;

        [[maybe_unused]] const bool rc = glCheck(glUnmapBuffer(m_type));
        SFML_BASE_ASSERT(rc);
    }

    [[gnu::cold]] void reserveImpl(const base::SizeT byteCount)
    {
        SFML_BASE_ASSERT(m_capacity < byteCount);

        const auto newCapacity = (m_capacity * 3u / 2u) + byteCount;

        m_obj->bind();

        unmapIfNeeded();

        m_obj->reallocate();
        m_obj->bind();

        glCheck(
            glBufferStorage(m_type, static_cast<GLsizeiptr>(newCapacity), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));

        m_mappedPtr = glCheck(glMapBufferRange(m_type,
                                               0u,
                                               newCapacity,
                                               GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                                   GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

        m_capacity = newCapacity;
    }

    GLenum      m_type;               //!< Buffer type
    OpenGLRAII* m_obj;                //!< Associated GL object handle
    void*       m_mappedPtr{nullptr}; //!< Write-only mapped pointer
    base::SizeT m_capacity{0u};       //!< Currently allocated capacity of the buffer
};

} // namespace sf
