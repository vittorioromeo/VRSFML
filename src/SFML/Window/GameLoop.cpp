#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Window/GameLoop.hpp>

#ifdef SFML_SYSTEM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
void yieldGameLoopFrame()
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    emscripten_sleep(0u);
#endif
}

} // namespace sf::priv
