// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/GLContextSaver.hpp"

#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Base/Assert.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
GLContextSaver::GLContextSaver() : m_glContext(WindowContext::getActiveThreadLocalGlContextPtr())
{
    ZA_ASSERT(m_glContext != nullptr);
}


////////////////////////////////////////////////////////////
GLContextSaver::~GLContextSaver()
{
    ZA_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContext(*m_glContext, true))
        errMsg("Could not restore context in `GLContextSaver::~GLContextSaver()`");
}

} // namespace za::priv
