// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/FenceUtils.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLFenceSync.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline GLsync asNativeHandle(const sf::priv::GLFenceSync& fence) noexcept
{
    return static_cast<GLsync>(const_cast<void*>(fence.getNativeHandle()));
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
GLFenceSync makeFence()
{
    GLsync fenceToCreate = glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

    if (fenceToCreate == nullptr) [[unlikely]]
    {
        priv::err() << "FATAL ERROR: Error creating fence sync object";
        base::abort();
    }

    return GLFenceSync{static_cast<void*>(fenceToCreate)};
}

////////////////////////////////////////////////////////////
void deleteFenceIfNeeded(GLFenceSync& fence) noexcept
{
    fence.reset();
}

////////////////////////////////////////////////////////////
bool tryWaitOnFence(GLFenceSync& fenceToWaitOn)
{
    if (!fenceToWaitOn)
        return true;

    const GLenum waitResult = glCheck(glClientWaitSync(asNativeHandle(fenceToWaitOn), GL_SYNC_FLUSH_COMMANDS_BIT, 0u));

    if (waitResult == GL_WAIT_FAILED) [[unlikely]]
    {
        sf::priv::err() << "FATAL ERROR: Error waiting on GPU fence";
        sf::base::abort();
    }

    if (waitResult == GL_TIMEOUT_EXPIRED)
        return false;

    SFML_BASE_ASSERT(waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED);

    fenceToWaitOn.reset();

    return true;
}

////////////////////////////////////////////////////////////
void waitOnFence(GLFenceSync& fenceToWaitOn)
{
    if (!fenceToWaitOn)
        return;

    const GLenum waitResult = glCheck(
        glClientWaitSync(asNativeHandle(fenceToWaitOn), GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED));

    if (waitResult == GL_WAIT_FAILED) [[unlikely]]
    {
        sf::priv::err() << "FATAL ERROR: Error waiting on GPU fence";
        sf::base::abort();
    }

    if (waitResult == GL_TIMEOUT_EXPIRED) [[unlikely]]
    {
        sf::priv::err() << "FATAL ERROR: Fence wait timed out";
        sf::base::abort();
    }

    SFML_BASE_ASSERT(waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED);

    fenceToWaitOn.reset();
}


} // namespace sf::priv
