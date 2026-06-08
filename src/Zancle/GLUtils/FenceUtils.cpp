// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/FenceUtils.hpp"

#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/GLFenceSync.hpp"
#include "Zancle/GLUtils/Glad.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Base/Abort.hpp"
#include "Zancle/Base/Assert.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline GLsync asNativeHandle(const za::priv::GLFenceSync& fence) noexcept
{
    return static_cast<GLsync>(const_cast<void*>(fence.getNativeHandle()));
}

} // namespace


namespace za::priv
{
////////////////////////////////////////////////////////////
GLFenceSync makeFence()
{
    GLsync fenceToCreate = glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

    if (fenceToCreate == nullptr) [[unlikely]]
    {
        priv::errMsg("FATAL ERROR: Error creating fence sync object");
        za::abort();
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

    const GLbitfield flags      = fenceToWaitOn.needsClientFlush() ? GL_SYNC_FLUSH_COMMANDS_BIT : 0u;
    const GLenum     waitResult = glCheck(glClientWaitSync(asNativeHandle(fenceToWaitOn), flags, 0u));
    fenceToWaitOn.markClientFlushConsumed();

    if (waitResult == GL_WAIT_FAILED) [[unlikely]]
    {
        priv::errMsg("FATAL ERROR: Error waiting on GPU fence");
        za::abort();
    }

    if (waitResult == GL_TIMEOUT_EXPIRED)
        return false;

    ZA_ASSERT(waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED);

    fenceToWaitOn.reset();

    return true;
}

////////////////////////////////////////////////////////////
void waitOnFence(GLFenceSync& fenceToWaitOn)
{
    if (!fenceToWaitOn)
        return;

    const GLbitfield flags      = fenceToWaitOn.needsClientFlush() ? GL_SYNC_FLUSH_COMMANDS_BIT : 0u;
    const GLenum     waitResult = glCheck(glClientWaitSync(asNativeHandle(fenceToWaitOn), flags, GL_TIMEOUT_IGNORED));
    fenceToWaitOn.markClientFlushConsumed();

    if (waitResult == GL_WAIT_FAILED) [[unlikely]]
    {
        priv::errMsg("FATAL ERROR: Error waiting on GPU fence");
        za::abort();
    }

    if (waitResult == GL_TIMEOUT_EXPIRED) [[unlikely]]
    {
        priv::errMsg("FATAL ERROR: Fence wait timed out");
        za::abort();
    }

    ZA_ASSERT(waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED);

    fenceToWaitOn.reset();
}


} // namespace za::priv
