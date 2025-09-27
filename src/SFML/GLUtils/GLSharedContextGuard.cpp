// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLSharedContextGuard.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
GLSharedContextGuard::GLSharedContextGuard() : m_glContext(WindowContext::getActiveThreadLocalGlContextPtr())
{
    SFML_BASE_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContextToSharedContext())
        err() << "Could not enable shared GL context in `GLSharedContextGuard::GLSharedContextGuard()`";
}


////////////////////////////////////////////////////////////
GLSharedContextGuard::~GLSharedContextGuard()
{
    SFML_BASE_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContext(*m_glContext, true))
        err() << "Could not restore context in `GLSharedContextGuard::~GLSharedContextGuard()`";
}

} // namespace sf::priv
