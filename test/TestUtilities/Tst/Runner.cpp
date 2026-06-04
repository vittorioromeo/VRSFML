// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Detail/State.hpp"
#include "Tst/Tst.hpp"

#include "ZancleBase/Builtin/Strcmp.hpp"
#include "ZancleBase/Builtin/Strlen.hpp"
#include "ZancleBase/Builtin/Strstr.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp" // IWYU pragma: keep
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/Vector.hpp"

#include <csignal>
#include <cstdio>


////////////////////////////////////////////////////////////
// Driver / runtime entry point.
//
// `run(argc, argv)`:
//   1. Parses a tiny CLI subset (`--test-case=`, `--test-case-exclude=`,
//      `--list-test-cases`, `--verbose`).
//   2. Iterates the registered test cases.
//   3. For each test case, repeatedly invokes the function until the
//      subcase tree's `advance()` returns false.
//   4. Catches the `REQUIRE` sentinel exception and any other escape from
//      the body, marking the test as failed.
//   5. Emits a per-test header at start and a summary at the end.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
ContextState& contextState() noexcept
{
    static ContextState instance;
    return instance;
}


////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
bool startsWith(const char* s, const char* prefix) noexcept
{
    while (*prefix != '\0')
    {
        if (*s != *prefix)
            return false;
        ++s;
        ++prefix;
    }
    return true;
}


////////////////////////////////////////////////////////////
bool nameMatchesAny(const char* name, const zb::Vector<zb::String>& patterns) noexcept
{
    for (zb::SizeT i = 0u; i < patterns.size(); ++i)
    {
        const auto& p = patterns.data()[i];
        if (p.empty())
            continue;

        if (ZB_STRSTR(name, p.cStr()) != nullptr)
            return true;
    }
    return false;
}


////////////////////////////////////////////////////////////
const char* runnerNonNull(const char* s) noexcept
{
    return s != nullptr ? s : "<anonymous>";
}


////////////////////////////////////////////////////////////
const char* signalName(const int sig) noexcept
{
    switch (sig)
    {
        case SIGABRT:
            return "SIGABRT";
        case SIGSEGV:
            return "SIGSEGV";
        case SIGILL:
            return "SIGILL";
        case SIGFPE:
            return "SIGFPE";
    }

    return "signal";
}


////////////////////////////////////////////////////////////
void writeStderr(const char* data, zb::SizeT size) noexcept
{
    // `stderr` is unbuffered by default, so this reaches the terminal
    // immediately -- important on the fatal-signal path below.
    (void)std::fwrite(data, 1u, size, stderr);
}


////////////////////////////////////////////////////////////
void writeStderr(const char* s) noexcept
{
    writeStderr(s, ZB_STRLEN(s));
}


////////////////////////////////////////////////////////////
void writeStderrInt(const int value) noexcept
{
    char  buf[16];
    char* out = buf + sizeof(buf);

    const bool negative = value < 0;
    const unsigned int magnitude = negative ? static_cast<unsigned int>(-(value + 1)) + 1u : static_cast<unsigned int>(value);

    unsigned int remaining = magnitude;
    do
    {
        *--out = static_cast<char>('0' + (remaining % 10u));
        remaining /= 10u;
    } while (remaining != 0u);

    if (negative)
        *--out = '-';

    writeStderr(out, static_cast<zb::SizeT>((buf + sizeof(buf)) - out));
}


////////////////////////////////////////////////////////////
void writeActiveSubcases(const ContextState& ctx) noexcept
{
    for (zb::SizeT i = 0u; i < ctx.traversal.activeSubcases.size(); ++i)
    {
        const auto& subcase = ctx.traversal.activeSubcases.data()[i];

        writeStderr("\n  subcase: ");
        writeStderr(runnerNonNull(subcase.name));
        writeStderr(" (");
        writeStderr(runnerNonNull(subcase.file));
        writeStderr(":");
        writeStderrInt(subcase.line);
        writeStderr(")");
    }
}


////////////////////////////////////////////////////////////
void fatalSignalHandler(const int sig) noexcept
{
    const auto& ctx = contextState();

    // Hard aborts bypass normal assertion reporting, so print the last test
    // case entered by the runner before handing control back to the platform.
    writeStderr("\n[tst] fatal ");
    writeStderr(signalName(sig));
    writeStderr(" while running test case:\n  ");
    writeStderr(runnerNonNull(ctx.currentTestName));
    writeStderr("\n  registered at: ");
    writeStderr(runnerNonNull(ctx.currentTestFile));
    writeStderr(":");
    writeStderrInt(ctx.currentTestLine);
    writeActiveSubcases(ctx);
    writeStderr("\n");

    std::signal(sig, SIG_DFL);
    std::raise(sig);
}


////////////////////////////////////////////////////////////
void installFatalSignalHandlers() noexcept
{
    std::signal(SIGABRT, fatalSignalHandler);
    std::signal(SIGSEGV, fatalSignalHandler);
    std::signal(SIGILL, fatalSignalHandler);
    std::signal(SIGFPE, fatalSignalHandler);
}


////////////////////////////////////////////////////////////
void printHelp()
{
    zb::printErrLn("Test runner options:");
    zb::printErrLn("  --test-case=<substring>     Run only tests whose name contains <substring>");
    zb::printErrLn("  --test-case-exclude=<substr> Skip tests matching <substr>");
    zb::printErrLn("  --list-test-cases           List registered test cases and exit");
    zb::printErrLn("  --verbose                   Report each test case before it runs");
    zb::printErrLn("  --reporters=<name>          Accepted for source compatibility (ignored)");
    zb::printErrLn("  --help, -h                  Print this help");
}


////////////////////////////////////////////////////////////
void parseOptions(int argc, char** argv, ContextState& ctx, bool& outShouldExit)
{
    outShouldExit = false;

    for (int i = 1; i < argc; ++i)
    {
        const char* a = argv[i];

        if (ZB_STRCMP(a, "--help") == 0 || ZB_STRCMP(a, "-h") == 0)
        {
            printHelp();
            outShouldExit = true;
            return;
        }

        if (ZB_STRCMP(a, "--verbose") == 0)
        {
            ctx.verbose = true;
            continue;
        }

        if (ZB_STRCMP(a, "--list-test-cases") == 0)
        {
            ctx.listOnly = true;
            continue;
        }

        if (startsWith(a, "--test-case="))
        {
            ctx.filterInclude.pushBack(zb::String{a + sizeof("--test-case=") - 1u});
            continue;
        }

        if (startsWith(a, "--tc="))
        {
            ctx.filterInclude.pushBack(zb::String{a + sizeof("--tc=") - 1u});
            continue;
        }

        if (startsWith(a, "--test-case-exclude="))
        {
            ctx.filterExclude.pushBack(zb::String{a + sizeof("--test-case-exclude=") - 1u});
            continue;
        }

        // Silently ignore unrecognized flags so legacy invocations still work
        // (the runner is invoked with `--reporters=console,progress` by the
        // existing test infrastructure).
    }
}


////////////////////////////////////////////////////////////
bool shouldRun(const TestCaseInfo& tc, const ContextState& ctx) noexcept
{
    if (tc.skipped)
        return false;

    if (tc.name == nullptr)
        return false;

    if (!ctx.filterInclude.empty() && !nameMatchesAny(tc.name, ctx.filterInclude))
        return false;

    if (nameMatchesAny(tc.name, ctx.filterExclude))
        return false;

    return true;
}


////////////////////////////////////////////////////////////
void listTestCases(const ContextState& ctx)
{
    auto& list = registeredTestCases();

    for (zb::SizeT i = 0u; i < list.size(); ++i)
    {
        const auto& tc = list.data()[i];
        if (!shouldRun(tc, ctx))
            continue;

        (void)zb::printLn("{}", runnerNonNull(tc.name));
    }
}


////////////////////////////////////////////////////////////
void runSingleTestCase(const TestCaseInfo& tc, ContextState& ctx)
{
    ctx.currentTestFailed = false;
    ctx.currentTestName   = tc.name;
    ctx.currentTestFile   = tc.file;
    ctx.currentTestLine   = tc.line;

    ctx.traversal.resetForTestCase();

    do
    {
        ctx.traversal.resetForRun();
        ctx.infoStack.clear();

        try
        {
            tc.fn();
        } catch (const ContextState::RequireFailedException&) // NOLINT(bugprone-empty-catch)
        {
            // Expected control flow on REQUIRE failure -- already reported.
        } catch (...)
        {
            ctx.currentTestFailed = true;
            ++ctx.failedAssertions;
            (void)zb::printErrLn("{}:{}: FAILED: uncaught exception in {}", tc.file, tc.line, runnerNonNull(tc.name));
        }
    } while (ctx.traversal.advance());

    if (ctx.currentTestFailed)
        ++ctx.failedTestCases;
}


////////////////////////////////////////////////////////////
void printSummary(const ContextState& ctx)
{
    (void)zb::printLn("");
    (void)zb::printLn("[tst] test cases:  {} | passed: {} | failed: {} | skipped: {}",
                            ctx.totalTestCases,
                            ctx.totalTestCases - ctx.failedTestCases - ctx.skippedTestCases,
                            ctx.failedTestCases,
                            ctx.skippedTestCases);
    (void)zb::printLn("[tst] assertions:  {} | passed: {} | failed: {}",
                            ctx.totalAssertions,
                            ctx.totalAssertions - ctx.failedAssertions,
                            ctx.failedAssertions);
}

} // namespace
} // namespace tst::detail


namespace tst
{
////////////////////////////////////////////////////////////
int run(int argc, char** argv)
{
    auto& ctx = detail::contextState();

    detail::installFatalSignalHandlers();

    bool shouldExit = false;
    detail::parseOptions(argc, argv, ctx, shouldExit);
    if (shouldExit)
        return 0;

    if (ctx.listOnly)
    {
        detail::listTestCases(ctx);
        return 0;
    }

    auto& list = detail::registeredTestCases();

    for (zb::SizeT i = 0u; i < list.size(); ++i)
    {
        const auto& tc = list.data()[i];

        if (tc.skipped)
        {
            ++ctx.skippedTestCases;
            ++ctx.totalTestCases;
            continue;
        }

        if (!detail::shouldRun(tc, ctx))
            continue;

        ++ctx.totalTestCases;

        if (ctx.verbose)
            (void)zb::printErrLn("[tst] running: {} ({}:{})", detail::runnerNonNull(tc.name), tc.file, tc.line);

        detail::runSingleTestCase(tc, ctx);
    }

    detail::printSummary(ctx);

    return ctx.failedTestCases == 0u ? 0 : 1;
}

} // namespace tst
