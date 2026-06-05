#include "Tst/Tst.hpp"

#include "Zancle/Base/SourceLocation.hpp"

#include "Zancle/Base/Strcmp.hpp"
#include "Zancle/Base/Strstr.hpp"


TEST_CASE("[Base] Base/SourceLocation.hpp")
{
    SECTION("current() returns valid location")
    {
        constexpr auto loc = za::SourceLocation::current();

        CHECK(loc.line() > 0);
        CHECK(loc.column() > 0);
        CHECK(loc.fileName() != nullptr);
        CHECK(loc.functionName() != nullptr);
    }

    SECTION("line() matches actual source line")
    {
        const auto lineBeforeCall = __LINE__;
        const auto loc            = za::SourceLocation::current();

        CHECK(loc.line() == static_cast<unsigned int>(lineBeforeCall + 1));
    }

    SECTION("fileName() contains this test file name")
    {
        constexpr auto loc = za::SourceLocation::current();

        CHECK(ZA_STRSTR(loc.fileName(), "SourceLocation.test.cpp") != nullptr);
    }

    SECTION("functionName() is non-empty")
    {
        constexpr auto loc = za::SourceLocation::current();

        CHECK(ZA_STRCMP(loc.functionName(), "") != 0);
    }

    SECTION("consteval evaluation")
    {
        // Verify the builtin struct layout matches at compile time
        static constexpr auto loc = za::SourceLocation::current();

        STATIC_CHECK(loc.line() > 0);
        STATIC_CHECK(loc.fileName() != nullptr);
        STATIC_CHECK(loc.functionName() != nullptr);
    }

    SECTION("propagation through function parameter default")
    {
        const auto check = [](za::SourceLocation loc = za::SourceLocation::current()) { return loc; };

        const auto callerLine = __LINE__;
        const auto loc        = check();

        // The default argument is evaluated at the call site
        CHECK(loc.line() == static_cast<unsigned int>(callerLine + 1));
    }
}
