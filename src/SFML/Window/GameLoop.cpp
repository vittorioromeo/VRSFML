#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Window/GameLoop.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Traits/IsRvalueReference.hpp>

#ifdef SFML_SYSTEM_EMSCRIPTEN
#include <emscripten.h>
#endif


namespace sf::GameLoop
{
////////////////////////////////////////////////////////////
priv::ControlFlow continueLoop()
{
    return priv::ControlFlow::Continue;
}


////////////////////////////////////////////////////////////
priv::ControlFlow breakLoop()
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    emscripten_cancel_main_loop();
#endif

    return priv::ControlFlow::Break;
}

} // namespace sf::GameLoop


namespace sf::GameLoop::priv
{
////////////////////////////////////////////////////////////
void runImpl(ControlFlow (*func)())
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    thread_local ControlFlow (*pinnedFunc)();
    pinnedFunc = func;

    emscripten_set_main_loop([] { (void)pinnedFunc(); }, 0 /* fps */, true /* infinite loop */);
#else
    while (true)
        if (func() == ControlFlow::Break)
            break;
#endif
}

} // namespace sf::GameLoop::priv
