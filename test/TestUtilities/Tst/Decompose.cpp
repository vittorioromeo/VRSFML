// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#include "Tst/Detail/Decompose.hpp"
#include "Tst/Detail/State.hpp"
#include "Tst/Detail/StringifyValue.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: keep
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


////////////////////////////////////////////////////////////
// Out-of-line glue for assertion handling. Builds the failure message
// (including the rendered `<lhs> <op> <rhs>` expansion captured by
// `recordDecomposition`), updates the run statistics, and -- for
// `REQUIRE` failures -- unwinds the test body via an exception caught by
// the runner. See the design rationale at the top of `Decompose.hpp`.
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
        default:
            SFML_BASE_ASSERT(k == AssertKind::Warn);
            return "WARN";
    }
}


////////////////////////////////////////////////////////////
bool isRequire(AssertKind k) noexcept
{
    return k == AssertKind::Require || k == AssertKind::RequireFalse;
}


////////////////////////////////////////////////////////////
const char* decomposeNonNull(const char* s) noexcept
{
    return s != nullptr ? s : "<anonymous>";
}


////////////////////////////////////////////////////////////
void emitActiveSubcases(const ContextState& ctx)
{
    for (sf::base::SizeT i = 0u; i < ctx.traversal.activeSubcases.size(); ++i)
    {
        const auto& subcase = ctx.traversal.activeSubcases.data()[i];
        (void)sf::base::printErrLn("  subcase: {} ({}:{})",
                                   decomposeNonNull(subcase.name),
                                   decomposeNonNull(subcase.file),
                                   subcase.line);
    }
}


////////////////////////////////////////////////////////////
void emitDecomposition(ContextState& ctx)
{
    if (!ctx.hasDecomp)
        return;

    (void)sf::base::printErrLn("  expansion: {}", sf::base::StringView{ctx.decompBuf, ctx.decompLen});
    ctx.hasDecomp = false;
}


////////////////////////////////////////////////////////////
void emitFailureBanner(AssertKind kind, const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();

    (void)sf::base::printErrLn("{}:{}: FAILED: {}({})", file, line, kindLabel(kind), exprStr);
    (void)sf::base::printErrLn("  test case: {} ({}:{})",
                               decomposeNonNull(ctx.currentTestName),
                               decomposeNonNull(ctx.currentTestFile),
                               ctx.currentTestLine);
    emitActiveSubcases(ctx);

    emitDecomposition(ctx);

    // Replay scoped INFO messages (newest at the back, but emit oldest-first).
    for (sf::base::SizeT i = 0u; i < ctx.infoStack.size(); ++i)
        (void)sf::base::printErrLn("  info: {}", ctx.infoStack.data()[i].toStringView());
}


////////////////////////////////////////////////////////////
void emitWarningBanner(const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();

    (void)sf::base::printErrLn("{}:{}: WARNING: WARN({})", file, line, exprStr);
    (void)sf::base::printErrLn("  test case: {} ({}:{})",
                               decomposeNonNull(ctx.currentTestName),
                               decomposeNonNull(ctx.currentTestFile),
                               ctx.currentTestLine);
    emitActiveSubcases(ctx);

    emitDecomposition(ctx);

    for (sf::base::SizeT i = 0u; i < ctx.infoStack.size(); ++i)
        (void)sf::base::printErrLn("  info: {}", ctx.infoStack.data()[i].toStringView());
}

} // namespace


////////////////////////////////////////////////////////////
bool handleAssertion(Result res, AssertKind kind, const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();

    // Like doctest, WARN is diagnostic-only: failed warnings are printed but
    // must not affect assertion totals or the test-case result.
    if (kind == AssertKind::Warn)
    {
        if (!res.passed)
            emitWarningBanner(file, line, exprStr);

        return res.passed;
    }

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


////////////////////////////////////////////////////////////
void recordDecomposition(const char* opStr, const void* lhs, StringifyFn lhsFn, const void* rhs, StringifyFn rhsFn) noexcept
{
    auto&                 ctx  = contextState();
    char* const           base = ctx.decompBuf;
    const sf::base::SizeT cap  = sizeof(ctx.decompBuf);
    sf::base::SizeT       len  = 0u;

    len += lhsFn(base + len, cap - len, lhs);

    for (const char* p = opStr; *p != '\0' && len < cap; ++p)
        base[len++] = *p;

    if (len < cap)
        len += rhsFn(base + len, cap - len, rhs);

    ctx.decompLen = len;
    ctx.hasDecomp = true;
}


////////////////////////////////////////////////////////////
void recordDecompositionUnary(const void* lhs, StringifyFn lhsFn) noexcept
{
    auto& ctx = contextState();

    ctx.decompLen = lhsFn(ctx.decompBuf, sizeof(ctx.decompBuf), lhs);
    ctx.hasDecomp = true;
}

} // namespace tst::detail
