#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>

#include <SFML/Base/Assert.hpp>

#ifdef SFML_DEBUG

#ifdef SFML_ENABLE_STACK_TRACES
#include <cpptrace/cpptrace.hpp>
#endif

#include <cstdio>
#include <cstdlib>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void assertFailure(const char* code, const char* file, const int line)
{
    std::printf("\n[[SFML ASSERTION FAILURE]]\n- %s:%d\n- SFML_BASE_ASSERT(%s);\n", file, line, code);

#ifdef SFML_ENABLE_STACK_TRACES
    std::puts("");
    cpptrace::generate_trace().print();
#endif

    std::abort();
}

} // namespace sf::base::priv

#endif
