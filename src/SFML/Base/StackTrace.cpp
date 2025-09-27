// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/StackTrace.hpp"

#include "SFML/Config.hpp"

#ifdef SFML_ENABLE_STACK_TRACES

    #ifdef SFML_SYSTEM_EMSCRIPTEN
        #include <emscripten.h>
    #else
        #include <cpptrace/basic.hpp>
    #endif

    #include <cstdio>

#endif


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void printStackTrace()
{
#ifdef SFML_ENABLE_STACK_TRACES
    std::puts("");

    #ifdef SFML_SYSTEM_EMSCRIPTEN
    char callstack[4096];
    emscripten_get_callstack(EM_LOG_NO_PATHS | EM_LOG_JS_STACK, callstack, sizeof(callstack));
    std::puts(callstack);
    #else
    cpptrace::generate_trace().print();
    #endif

#endif
}

} // namespace sf::base::priv
