// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLContextSaver.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
GLContextSaver::GLContextSaver() : m_glContext(WindowContext::getActiveThreadLocalGlContextPtr())
{
    SFML_BASE_ASSERT(m_glContext != nullptr);
}


////////////////////////////////////////////////////////////
GLContextSaver::~GLContextSaver()
{
    SFML_BASE_ASSERT(m_glContext != nullptr);

    if (!WindowContext::setActiveThreadLocalGlContext(*m_glContext, true))
        err() << "Could not restore context in `GLContextSaver::~GLContextSaver()`";
}

} // namespace sf::priv
