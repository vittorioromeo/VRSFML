#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Base/StackTrace.hpp"

#ifdef SFML_ENABLE_STACK_TRACES

#ifdef SFML_SYSTEM_EMSCRIPTEN
#include <emscripten.h>
#else
#include <cpptrace/cpptrace.hpp>
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
    emscripten_get_callstack(EM_LOG_C_STACK, callstack, sizeof(callstack));
    std::puts(callstack);
#else
    cpptrace::generate_trace().print();
#endif

#endif
}

} // namespace sf::base::priv
