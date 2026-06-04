// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/GLContextSaver.hpp"

#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/System/Err.hpp"

#include "ZancleBase/Assert.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
GLContextSaver::GLContextSaver() : m_glContext(WindowContext::getActiveThreadLocalGlContextPtr())
{
    ZB_ASSERT(m_glContext != nullptr);
}


////////////////////////////////////////////////////////////
GLContextSaver::~GLContextSaver()
{
    ZB_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContext(*m_glContext, true))
        errMsg("Could not restore context in `GLContextSaver::~GLContextSaver()`");
}

} // namespace za::priv
