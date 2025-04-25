#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLSharedContextGuard.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
GLSharedContextGuard::GLSharedContextGuard()
{
    if (!WindowContext::setActiveThreadLocalGlContextToSharedContext(true))
        err() << "Could not enable shared GL context in `GLSharedContextGuard::GLSharedContextGuard()`";
}


////////////////////////////////////////////////////////////
GLSharedContextGuard::~GLSharedContextGuard() = default;

} // namespace sf::priv
