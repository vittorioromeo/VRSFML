#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
    // SFML_BASE_ASSERT(m_glContext != nullptr); // TODO P1: restore?
}


////////////////////////////////////////////////////////////
GLContextSaver::~GLContextSaver()
{
    if (m_glContext == nullptr)
    {
        WindowContext::disableGlContext();
        return;
    }

    if (!WindowContext::setActiveThreadLocalGlContext(*m_glContext, true))
        err() << "Could not restore context in `GLContextSaver::~GLContextSaver()`";
}

} // namespace sf::priv
