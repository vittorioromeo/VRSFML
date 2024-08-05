#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/Optional.hpp>

#ifdef SFML_ENABLE_STACK_TRACES
#include <cpptrace/cpptrace.hpp>
#endif

#include <cstdio>
#include <cstdlib>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void throwIfNotEngaged()
{
    std::printf("\n[[SFML OPTIONAL FAILURE]]: not engaged!\n");

// TODO P1: avoid repetition
#ifdef SFML_ENABLE_STACK_TRACES
    std::puts("");
    cpptrace::generate_trace().print();
#endif

    throw BadOptionalAccess{};
}

} // namespace sf::base::priv
