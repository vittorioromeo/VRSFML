// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Assert.hpp"

#include "Zancle/Config.hpp" // IWYU pragma: keep

#ifdef ZA_DEBUG

    #include "ZancleBase/Abort.hpp"
    #include "ZancleBase/StackTrace.hpp"

    #include <cstdio>


namespace zb::priv
{
////////////////////////////////////////////////////////////
void assertFailure(const char* code, const char* file, const int line)
{
    std::printf("\n[[ZANCLE ASSERTION FAILURE]]\n- %s:%d\n- ZB_ASSERT(%s);\n", file, line, code);
    printStackTrace();

    zb::abort();
}

} // namespace zb::priv

#endif
