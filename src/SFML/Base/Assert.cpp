#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Assert.hpp"

#ifdef SFML_DEBUG

#include "SFML/Base/StackTrace.hpp"

#include <cstdio>


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
void assertFailure(const char* code, const char* file, const int line)
{
    std::printf("\n[[SFML ASSERTION FAILURE]]\n- %s:%d\n- SFML_BASE_ASSERT(%s);\n", file, line, code);
    printStackTrace();

    base::abort();
}

} // namespace sf::base::priv

#endif
