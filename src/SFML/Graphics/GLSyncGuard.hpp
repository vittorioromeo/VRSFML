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
class GLSyncGuard
{
public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] explicit GLSyncGuard() :
    m_sync(glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)))
    {
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] ~GLSyncGuard()
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

    ////////////////////////////////////////////////////////////
    GLSyncGuard(const GLSyncGuard&)            = delete;
    GLSyncGuard& operator=(const GLSyncGuard&) = delete;

    ////////////////////////////////////////////////////////////
    GLSyncGuard(GLSyncGuard&&)            = delete;
    GLSyncGuard& operator=(GLSyncGuard&&) = delete;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    GLsync m_sync;
};

} // namespace sf
