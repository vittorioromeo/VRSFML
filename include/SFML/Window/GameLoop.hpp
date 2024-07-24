#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

// TODO P0: cleanup

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Traits/IsRvalueReference.hpp>

#ifdef SFML_SYSTEM_EMSCRIPTEN
#include <emscripten.h>
#endif


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
} // namespace sf


namespace sf::GameLoop::priv
{
////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] ControlFlow
{
    Continue,
    Break
};

////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
struct Runner
{
    template <typename F>
    void operator|(F&& func);
};

} // namespace sf::GameLoop::priv


namespace sf::GameLoop
{


////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline priv::ControlFlow continueLoop()
{
    return priv::ControlFlow::Continue;
}


////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
[[nodiscard]] inline priv::ControlFlow breakLoop()
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    emscripten_cancel_main_loop();
#endif

    return priv::ControlFlow::Break;
}


////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
template <typename F>
void run(F&& func)
{
#ifdef SFML_SYSTEM_EMSCRIPTEN
    if constexpr (base::isRvalueReference<F>)
    {
        thread_local F pinnedFunc;
        pinnedFunc = SFML_BASE_MOVE(func);

        emscripten_set_main_loop([] { (void)pinnedFunc(); }, 0 /* fps */, true /* infinite loop */);
    }
    else
    {
        thread_local F* pinnedFunc;
        pinnedFunc = &func;

        emscripten_set_main_loop([] { (void)(*pinnedFunc)(); }, 0 /* fps */, true /* infinite loop */);
    }
#else
    while (true)
        if (func() == priv::ControlFlow::Break)
            break;
#endif
}

} // namespace sf::GameLoop


namespace sf::GameLoop::priv
{
////////////////////////////////////////////////////////////
template <typename F>
void Runner::operator|(F&& func)
{
    run(SFML_BASE_FORWARD(func));
}

} // namespace sf::GameLoop::priv

// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SFML_GAME_LOOP ::sf::GameLoop::priv::Runner{} | [&]

#define SFML_GAME_LOOP_BREAK return ::sf::GameLoop::breakLoop()

#define SFML_GAME_LOOP_CONTINUE return ::sf::GameLoop::continueLoop()


////////////////////////////////////////////////////////////
/// \namespace sf::GameLoop
/// \ingroup window
///
/// TODO
///
/// \see TODO
///
////////////////////////////////////////////////////////////
