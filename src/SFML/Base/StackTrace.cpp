#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/StackTrace.hpp>

#ifdef SFML_ENABLE_STACK_TRACES
#include <cpptrace/cpptrace.hpp>
#endif

#include <cstdio>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void printStackTrace()
{
#ifdef SFML_ENABLE_STACK_TRACES
    std::puts("");
    cpptrace::generate_trace().print();
#endif
}

} // namespace sf::base::priv
