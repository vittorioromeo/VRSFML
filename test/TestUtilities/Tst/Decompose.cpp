// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


#include "Tst/Detail/Decompose.hpp"
#include "Tst/Detail/State.hpp"
#include "Tst/Detail/StringifyValue.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp" // IWYU pragma: keep
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"


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
            ZA_ASSERT(k == AssertKind::Warn);
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
    for (za::SizeT i = 0u; i < ctx.traversal.activeSubcases.size(); ++i)
    {
        const auto& subcase = ctx.traversal.activeSubcases.data()[i];
        (void)za::printErrLn("  subcase: {} ({}:{})",
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

    (void)za::printErrLn("  expansion: {}", za::StringView{ctx.decompBuf, ctx.decompLen});
    ctx.hasDecomp = false;
}


////////////////////////////////////////////////////////////
void emitFailureBanner(AssertKind kind, const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();

    (void)za::printErrLn("{}:{}: FAILED: {}({})", file, line, kindLabel(kind), exprStr);
    (void)za::printErrLn("  test case: {} ({}:{})",
                         decomposeNonNull(ctx.currentTestName),
                         decomposeNonNull(ctx.currentTestFile),
                         ctx.currentTestLine);
    emitActiveSubcases(ctx);

    emitDecomposition(ctx);

    // Replay scoped INFO messages (newest at the back, but emit oldest-first).
    for (za::SizeT i = 0u; i < ctx.infoStack.size(); ++i)
        (void)za::printErrLn("  info: {}", ctx.infoStack.data()[i].toStringView());
}


////////////////////////////////////////////////////////////
void emitWarningBanner(const char* file, int line, const char* exprStr)
{
    auto& ctx = contextState();

    (void)za::printErrLn("{}:{}: WARNING: WARN({})", file, line, exprStr);
    (void)za::printErrLn("  test case: {} ({}:{})",
                         decomposeNonNull(ctx.currentTestName),
                         decomposeNonNull(ctx.currentTestFile),
                         ctx.currentTestLine);
    emitActiveSubcases(ctx);

    emitDecomposition(ctx);

    for (za::SizeT i = 0u; i < ctx.infoStack.size(); ++i)
        (void)za::printErrLn("  info: {}", ctx.infoStack.data()[i].toStringView());
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
    auto&           ctx  = contextState();
    char* const     base = ctx.decompBuf;
    const za::SizeT cap  = sizeof(ctx.decompBuf);
    za::SizeT       len  = 0u;

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
