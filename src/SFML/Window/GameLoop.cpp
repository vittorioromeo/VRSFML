#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Window/GameLoop.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Traits/IsRvalueReference.hpp>

#ifdef SFML_SYSTEM_EMSCRIPTEN
#include <SFML/Window/Emscripten/WindowImplEmscripten.hpp>

#include <emscripten.h>
#endif

void killWindow(); // TODO P0:

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
    return priv::ControlFlow::Break;
}

} // namespace sf::GameLoop


namespace sf::GameLoop::priv
{
////////////////////////////////////////////////////////////
void runImpl(ControlFlow (*func)())
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    struct FuncHolder
    {
        ControlFlow (*heldFunc)();
        void (*vsyncEnablerFunc)();
    } funcHolder{func, sf::priv::WindowImplEmscripten::vsyncEnablerFn};

    emscripten_set_main_loop_arg(
        [](void* arg)
        {
            auto& [heldFunc, vsyncEnablerFunc] = *static_cast<FuncHolder*>(arg);

            if (vsyncEnablerFunc != nullptr) [[unlikely]]
            {
                vsyncEnablerFunc();
                vsyncEnablerFunc = nullptr;
            }

            if (heldFunc() == ControlFlow::Break)
            {
                killWindow(); // TODO P0:
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

} // namespace sf::GameLoop::priv
