// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Base/Assert.hpp"

#include "Zancle/Config.hpp" // IWYU pragma: keep

#ifdef ZA_DEBUG

    #include "Zancle/Base/Abort.hpp"
    #include "Zancle/Base/StackTrace.hpp"

    #include <cstdio>


namespace za::priv
{
////////////////////////////////////////////////////////////
void assertFailure(const char* code, const char* file, const int line)
{
    // NOLINTNEXTLINE(modernize-use-std-print)
    std::printf("\n[[ZANCLE ASSERTION FAILURE]]\n- %s:%d\n- ZA_ASSERT(%s);\n", file, line, code);

    printStackTrace();
    za::abort();
}

} // namespace za::priv

#endif
