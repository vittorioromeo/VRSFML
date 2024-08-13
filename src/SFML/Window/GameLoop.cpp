#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Window/GameLoop.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Traits/IsRvalueReference.hpp>

#ifdef SFML_SYSTEM_EMSCRIPTEN
#include <SFML/Window/Emscripten/EmscriptenImpl.hpp>

#include <emscripten.h>
#endif

void killWindow(); // TODO P0:

namespace sf::priv
{
////////////////////////////////////////////////////////////
GameLoop::ControlFlow GameLoop::continueLoop()
{
    return ControlFlow::Continue;
}


////////////////////////////////////////////////////////////
GameLoop::ControlFlow GameLoop::breakLoop()
{
    return ControlFlow::Break;
}


////////////////////////////////////////////////////////////
void GameLoop::runImpl(GameLoop::ControlFlow (*func)())
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    struct FuncHolder
    {
        ControlFlow (*heldFunc)();
    } funcHolder{func};

    emscripten_set_main_loop_arg(
        [](void* arg)
        {
            auto& [heldFunc] = *static_cast<FuncHolder*>(arg);
            EmscriptenImpl::invokeAndClearVSyncEnabler();

            if (heldFunc() == ControlFlow::Break)
            {
                EmscriptenImpl::killWindow();
                emscripten_cancel_main_loop();
            }
        },
        /* arg */ &funcHolder,
        /* fps */ 0,
        /* infinite loop */ true);
#else
    while (true)
        if (func() == ControlFlow::Break)
            break;
#endif
}

} // namespace sf::priv
