// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/GLFenceSync.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline GLsync asNativeHandle(const void* const handle) noexcept
{
    return static_cast<GLsync>(const_cast<void*>(handle));
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
GLFenceSync::~GLFenceSync()
{
    reset();
}


////////////////////////////////////////////////////////////
void GLFenceSync::reset() noexcept
{
    if (m_handle == nullptr)
        return;

    glCheck(glDeleteSync(asNativeHandle(m_handle)));
    m_handle = nullptr;
}

} // namespace sf::priv
