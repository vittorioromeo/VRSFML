#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Exchange.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Move-only RAII wrapper for an OpenGL fence sync object
///
/// Stores the native sync handle opaquely to avoid exposing GL headers
/// from this header. Destruction releases the underlying sync object
/// if one is currently owned.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] GLFenceSync
{
public:
    ////////////////////////////////////////////////////////////
    GLFenceSync() = default;
    ~GLFenceSync();

    ////////////////////////////////////////////////////////////
    GLFenceSync(const GLFenceSync&)            = delete;
    GLFenceSync& operator=(const GLFenceSync&) = delete;

    ////////////////////////////////////////////////////////////
    GLFenceSync(GLFenceSync&& rhs) noexcept :
        m_handle{base::exchange(rhs.m_handle, nullptr)},
        m_needsClientFlush{base::exchange(rhs.m_needsClientFlush, true)}
    {
    }

    ////////////////////////////////////////////////////////////
    GLFenceSync& operator=(GLFenceSync&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        reset();
        m_handle           = base::exchange(rhs.m_handle, nullptr);
        m_needsClientFlush = base::exchange(rhs.m_needsClientFlush, true);

        return *this;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool hasValue() const noexcept
    {
        return m_handle != nullptr;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] const void* getNativeHandle() const noexcept
    {
        return m_handle;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool needsClientFlush() const noexcept
    {
        return m_needsClientFlush;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void markClientFlushConsumed() noexcept
    {
        m_needsClientFlush = false;
    }

    ////////////////////////////////////////////////////////////
    void reset() noexcept;

private:
    friend GLFenceSync makeFence();

    ////////////////////////////////////////////////////////////
    explicit GLFenceSync(void* nativeHandle) noexcept : m_handle{nativeHandle}
    {
    }

    ////////////////////////////////////////////////////////////
    void* m_handle{nullptr};
    bool  m_needsClientFlush{true};
};

} // namespace sf::priv
