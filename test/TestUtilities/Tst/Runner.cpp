// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#include "Tst/Detail/State.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Base/Builtin/Strcmp.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Builtin/Strstr.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Driver / runtime entry point.
//
// `run(argc, argv)`:
//   1. Parses a tiny CLI subset (`--test-case=`, `--exclude=`, `--success`,
//      `--list-test-cases`, `--no-colors`).
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
bool nameMatchesAny(const char* name, const sf::base::Vector<sf::base::String>& patterns) noexcept
{
    for (sf::base::SizeT i = 0u; i < patterns.size(); ++i)
    {
        const auto& p = patterns.data()[i];
        if (p.empty())
            continue;

        if (SFML_BASE_STRSTR(name, p.cStr()) != nullptr)
            return true;
    }
    return false;
}


////////////////////////////////////////////////////////////
const char* nonNull(const char* s) noexcept
{
    return s != nullptr ? s : "<anonymous>";
}


////////////////////////////////////////////////////////////
void printHelp()
{
    (void)sf::base::printErrLn("Test runner options:");
    (void)sf::base::printErrLn("  --test-case=<substring>     Run only tests whose name contains <substring>");
    (void)sf::base::printErrLn("  --test-case-exclude=<substr> Skip tests matching <substr>");
    (void)sf::base::printErrLn("  --list-test-cases           List registered test cases and exit");
    (void)sf::base::printErrLn("  --success                   Report successful assertions too");
    (void)sf::base::printErrLn("  --no-colors                 Disable ANSI color output");
    (void)sf::base::printErrLn("  --reporters=<name>          Accepted for source compatibility (ignored)");
    (void)sf::base::printErrLn("  --help, -h                  Print this help");
}


////////////////////////////////////////////////////////////
void parseOptions(int argc, char** argv, ContextState& ctx, bool& outShouldExit)
{
    outShouldExit = false;

    for (int i = 1; i < argc; ++i)
    {
        const char* a = argv[i];

        if (SFML_BASE_STRCMP(a, "--help") == 0 || SFML_BASE_STRCMP(a, "-h") == 0)
        {
            printHelp();
            outShouldExit = true;
            return;
        }

        if (SFML_BASE_STRCMP(a, "--success") == 0)
        {
            ctx.showSuccess = true;
            continue;
        }

        if (SFML_BASE_STRCMP(a, "--no-colors") == 0)
        {
            ctx.noColors = true;
            continue;
        }

        if (SFML_BASE_STRCMP(a, "--list-test-cases") == 0)
        {
            ctx.listOnly = true;
            continue;
        }

        if (startsWith(a, "--test-case="))
        {
            ctx.filterInclude.pushBack(sf::base::String{a + sizeof("--test-case=") - 1u});
            continue;
        }

        if (startsWith(a, "--tc="))
        {
            ctx.filterInclude.pushBack(sf::base::String{a + sizeof("--tc=") - 1u});
            continue;
        }

        if (startsWith(a, "--test-case-exclude="))
        {
            ctx.filterExclude.pushBack(sf::base::String{a + sizeof("--test-case-exclude=") - 1u});
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

    for (sf::base::SizeT i = 0u; i < list.size(); ++i)
    {
        const auto& tc = list.data()[i];
        if (!shouldRun(tc, ctx))
            continue;

        (void)sf::base::printLn("{}", nonNull(tc.name));
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
            (void)sf::base::printErrLn("{}:{}: FAILED: uncaught exception in {}", tc.file, tc.line, nonNull(tc.name));
        }
    } while (ctx.traversal.advance());

    if (ctx.currentTestFailed)
        ++ctx.failedTestCases;
}


////////////////////////////////////////////////////////////
void printSummary(const ContextState& ctx)
{
    (void)sf::base::printLn("");
    (void)sf::base::printLn("[tst] test cases:  {} | passed: {} | failed: {} | skipped: {}",
                            ctx.totalTestCases,
                            ctx.totalTestCases - ctx.failedTestCases - ctx.skippedTestCases,
                            ctx.failedTestCases,
                            ctx.skippedTestCases);
    (void)sf::base::printLn("[tst] assertions:  {} | passed: {} | failed: {}",
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

    for (sf::base::SizeT i = 0u; i < list.size(); ++i)
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
            (void)sf::base::printErrLn("[tst] running: {} ({}:{})", detail::nonNull(tc.name), tc.file, tc.line);

        detail::runSingleTestCase(tc, ctx);
    }

    detail::printSummary(ctx);

    return ctx.failedTestCases == 0u ? 0 : 1;
}

} // namespace tst
