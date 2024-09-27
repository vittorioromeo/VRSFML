#pragma once
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
/// \brief TODO P1: docs
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
        unmapIfNeeded();
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

        unmapIfNeeded();

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
    [[gnu::always_inline, gnu::flatten]] void memcpyToBuffer(const void* data, const base::SizeT byteCount)
    {
        SFML_BASE_MEMCPY(m_mappedPtr, data, byteCount);
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

private:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unmapIfNeeded() const
    {
        if (m_mappedPtr == nullptr)
            return;

        [[maybe_unused]] const bool rc = glCheck(glUnmapBuffer(TBufferObject::bufferType));
        SFML_BASE_ASSERT(rc);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::cold]] void reserveImpl(const base::SizeT byteCount)
    {
        SFML_BASE_ASSERT(m_capacity < byteCount);
        SFML_BASE_ASSERT(m_obj != nullptr);

        const auto newCapacity = (m_capacity * 3u / 2u) + byteCount;

        m_obj->bind();

        unmapIfNeeded();

        m_obj->reallocate();
        m_obj->bind();

        glCheck(glBufferStorage(TBufferObject::bufferType,
                                static_cast<GLsizeiptr>(newCapacity),
                                nullptr,
                                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));

        m_mappedPtr = glCheck(glMapBufferRange(TBufferObject::bufferType,
                                               0u,
                                               newCapacity,
                                               GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                                   GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

        m_capacity = newCapacity;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    TBufferObject* m_obj;                //!< Associated GL object handle
    void*          m_mappedPtr{nullptr}; //!< Write-only mapped pointer
    base::SizeT    m_capacity{0u};       //!< Currently allocated capacity of the buffer
};

} // namespace sf
