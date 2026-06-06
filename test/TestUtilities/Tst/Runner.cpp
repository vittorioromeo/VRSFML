// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Detail/State.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp" // IWYU pragma: keep

#include "Zancle/String/String.hpp"

#include "Zancle/Container/Vector.hpp"

#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Base/Strcmp.hpp"
#include "Zancle/Base/Strlen.hpp"
#include "Zancle/Base/Strstr.hpp"

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
bool nameMatchesAny(const char* name, const za::Vector<za::String>& patterns) noexcept
{
    for (za::SizeT i = 0u; i < patterns.size(); ++i)
    {
        const auto& p = patterns.data()[i];
        if (p.empty())
            continue;

        if (ZA_STRSTR(name, p.cStr()) != nullptr)
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
void writeStderr(const char* data, za::SizeT size) noexcept
{
    // `stderr` is unbuffered by default, so this reaches the terminal
    // immediately -- important on the fatal-signal path below.
    std::fwrite(data, 1u, size, stderr);
}


////////////////////////////////////////////////////////////
void writeStderr(const char* s) noexcept
{
    writeStderr(s, ZA_STRLEN(s));
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

    writeStderr(out, static_cast<za::SizeT>((buf + sizeof(buf)) - out));
}


////////////////////////////////////////////////////////////
void writeActiveSubcases(const ContextState& ctx) noexcept
{
    for (za::SizeT i = 0u; i < ctx.traversal.activeSubcases.size(); ++i)
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
    za::printErrLn("Test runner options:");
    za::printErrLn("  --test-case=<substring>     Run only tests whose name contains <substring>");
    za::printErrLn("  --test-case-exclude=<substr> Skip tests matching <substr>");
    za::printErrLn("  --list-test-cases           List registered test cases and exit");
    za::printErrLn("  --verbose                   Report each test case before it runs");
    za::printErrLn("  --reporters=<name>          Accepted for source compatibility (ignored)");
    za::printErrLn("  --help, -h                  Print this help");
}


////////////////////////////////////////////////////////////
void parseOptions(int argc, char** argv, ContextState& ctx, bool& outShouldExit)
{
    outShouldExit = false;

    for (int i = 1; i < argc; ++i)
    {
        const char* a = argv[i];

        if (ZA_STRCMP(a, "--help") == 0 || ZA_STRCMP(a, "-h") == 0)
        {
            printHelp();
            outShouldExit = true;
            return;
        }

        if (ZA_STRCMP(a, "--verbose") == 0)
        {
            ctx.verbose = true;
            continue;
        }

        if (ZA_STRCMP(a, "--list-test-cases") == 0)
        {
            ctx.listOnly = true;
            continue;
        }

        if (startsWith(a, "--test-case="))
        {
            ctx.filterInclude.pushBack(za::String{a + sizeof("--test-case=") - 1u});
            continue;
        }

        if (startsWith(a, "--tc="))
        {
            ctx.filterInclude.pushBack(za::String{a + sizeof("--tc=") - 1u});
            continue;
        }

        if (startsWith(a, "--test-case-exclude="))
        {
            ctx.filterExclude.pushBack(za::String{a + sizeof("--test-case-exclude=") - 1u});
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

    for (za::SizeT i = 0u; i < list.size(); ++i)
    {
        const auto& tc = list.data()[i];
        if (!shouldRun(tc, ctx))
            continue;

        za::printLn("{}", runnerNonNull(tc.name));
    }
}


////////////////////////////////////////////////////////////
void runSingleTestCase(const TestCaseInfo& tc, ContextState& ctx)
{
    ctx.currentTestFailed = false;
    ctx.currentTestName   = tc.name;
    ctx.currentTestFile   = tc.file;
    ctx.currentTestLine   = tc.line;
    ctx.firstFailureFile  = nullptr;
    ctx.firstFailureLine  = 0;

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
            if (!ctx.currentTestFailed)
            {
                ctx.firstFailureFile = tc.file;
                ctx.firstFailureLine = tc.line;
            }
            ctx.currentTestFailed = true;
            ++ctx.failedAssertions;
            za::printErrLn("{}:{}: FAILED: uncaught exception in {}", tc.file, tc.line, runnerNonNull(tc.name));
        }
    } while (ctx.traversal.advance());

    if (ctx.currentTestFailed)
    {
        ++ctx.failedTestCases;
        // Prefer the first failed assertion's location; fall back to the
        // TEST_CASE site if nothing was captured (defensive -- shouldn't happen).
        const char* file = ctx.firstFailureFile != nullptr ? ctx.firstFailureFile : tc.file;
        const int   line = ctx.firstFailureFile != nullptr ? ctx.firstFailureLine : tc.line;
        ctx.failedTestCaseList.pushBack(FailedTestCaseRecord{tc.name, file, line});
    }
}


////////////////////////////////////////////////////////////
void printSummary(const ContextState& ctx)
{
    za::printLn("");

    za::printLn("[tst] test cases:  {} | passed: {} | failed: {} | skipped: {}",
                ctx.totalTestCases,
                ctx.totalTestCases - ctx.failedTestCases - ctx.skippedTestCases,
                ctx.failedTestCases,
                ctx.skippedTestCases);

    za::printLn("[tst] assertions:  {} | passed: {} | failed: {}",
                ctx.totalAssertions,
                ctx.totalAssertions - ctx.failedAssertions,
                ctx.failedAssertions);

    if (!ctx.failedTestCaseList.empty())
    {
        za::printLn("");
        za::printLn("[tst] failed test cases:");

        for (za::SizeT i = 0u; i < ctx.failedTestCaseList.size(); ++i)
        {
            const auto& f = ctx.failedTestCaseList.data()[i];
            za::printLn("  - {} ({}:{})", runnerNonNull(f.name), runnerNonNull(f.file), f.line);
        }
    }
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

    for (za::SizeT i = 0u; i < list.size(); ++i)
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
            za::printErrLn("[tst] running: {} ({}:{})", detail::runnerNonNull(tc.name), tc.file, tc.line);

        detail::runSingleTestCase(tc, ctx);
    }

    detail::printSummary(ctx);

    return ctx.failedTestCases == 0u ? 0 : 1;
}

} // namespace tst
