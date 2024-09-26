#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
class GPUSyncGuard
{
public:
    [[gnu::always_inline, gnu::flatten]] explicit GPUSyncGuard() :
    m_sync(glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)))
    {
    }

    [[gnu::always_inline, gnu::flatten]] ~GPUSyncGuard()
    {
        while (true)
        {
            const GLenum waitReturn = glCheck(glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1u));

            SFML_BASE_ASSERT(waitReturn != GL_WAIT_FAILED);

            if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
                return;
        }

        glCheck(glDeleteSync(m_sync));
    }

    GPUSyncGuard(const GPUSyncGuard&)            = delete;
    GPUSyncGuard& operator=(const GPUSyncGuard&) = delete;

    GPUSyncGuard(GPUSyncGuard&&)            = delete;
    GPUSyncGuard& operator=(GPUSyncGuard&&) = delete;

private:
    GLsync m_sync;
};

} // namespace sf
