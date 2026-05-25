// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#include "Tst/Detail/Decompose.hpp"
#include "Tst/Detail/State.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"


////////////////////////////////////////////////////////////
// Out-of-line glue for assertion handling. Builds the failure message,
// updates the run statistics, and -- for `REQUIRE` failures -- unwinds
// the test body via an exception caught by the runner.
//
// No value decomposition is performed: see the design rationale at the
// top of `Decompose.hpp`.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
const char* kindLabel(AssertKind k) noexcept
{
    switch (k)
    {
        case AssertKind::Check:
            return "CHECK";
        case AssertKind::CheckFalse:
            return "CHECK_FALSE";
        case AssertKind::Require:
            return "REQUIRE";
        case AssertKind::RequireFalse:
            return "REQUIRE_FALSE";
        case AssertKind::Warn:
            return "WARN";
    }
    return "ASSERT";
}


////////////////////////////////////////////////////////////
bool isRequire(AssertKind k) noexcept
{
    return k == AssertKind::Require || k == AssertKind::RequireFalse;
}


////////////////////////////////////////////////////////////
void emitFailureBanner(AssertKind kind, const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();

    (void)sf::base::printErrLn("{}:{}: FAILED: {}({})", file, line, kindLabel(kind), exprStr);

    // Replay scoped INFO messages (newest at the back, but emit oldest-first).
    for (sf::base::SizeT i = 0u; i < ctx.infoStack.size(); ++i)
        (void)sf::base::printErrLn("  info: {}", ctx.infoStack.data()[i].toStringView());
}

} // namespace


////////////////////////////////////////////////////////////
bool handleAssertion(Result res, AssertKind kind, const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();
    ++ctx.totalAssertions;

    if (res.passed)
        return true;

    ++ctx.failedAssertions;
    ctx.currentTestFailed = true;

    emitFailureBanner(kind, file, line, exprStr);

    if (isRequire(kind))
        throw ContextState::RequireFailedException{};

    return false;
}

} // namespace tst::detail
