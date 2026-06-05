// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/GLSharedContextGuard.hpp"

#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Diagnostic/Assert.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
GLSharedContextGuard::GLSharedContextGuard() : m_glContext(WindowContext::getActiveThreadLocalGlContextPtr())
{
    ZA_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContextToSharedContext())
        errMsg("Could not enable shared GL context in `GLSharedContextGuard::GLSharedContextGuard()`");
}


////////////////////////////////////////////////////////////
GLSharedContextGuard::~GLSharedContextGuard()
{
    ZA_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContext(*m_glContext, true))
        errMsg("Could not restore context in `GLSharedContextGuard::~GLSharedContextGuard()`");
}

} // namespace za::priv
