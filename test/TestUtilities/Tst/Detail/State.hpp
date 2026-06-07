#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/String/String.hpp"

#include "Zancle/Container/Vector.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace tst::detail
{
////////////////////////////////////////////////////////////
struct TestCaseInfo
{
    void (*fn)();
    const char* file;
    int         line;
    const char* name;
    const char* typeStr; // for templated test cases; nullptr otherwise
    bool        skipped;
};


////////////////////////////////////////////////////////////
struct SubcaseSignature
{
    const char* name;
    const char* file;
    int         line;

    [[nodiscard]] bool operator==(const SubcaseSignature& other) const noexcept;
};


////////////////////////////////////////////////////////////
struct DecisionPoint
{
    // The list of distinct subcases discovered at this decision depth in
    // the current TEST_CASE body. `branchCount` mirrors `subcases.size()`
    // but kept separate for clarity with the doctest algorithm.
    za::Vector<SubcaseSignature> subcases;
    za::SizeT                    branchCount = 0u;
};


////////////////////////////////////////////////////////////
struct TraversalState
{
    // The currently-chosen path through the subcase tree on this run.
    // `decisionPath[d]` is the sibling index picked at decision depth `d`.
    za::Vector<za::SizeT> decisionPath;

    // What we've actually discovered while running -- one entry per
    // decision depth, holding all sibling subcases observed.
    za::Vector<DecisionPoint> discoveredDecisionPath;

    // Stack of decision depths corresponding to the currently active
    // (entered) subcases. Used to restore `decisionDepth` on unwind.
    za::Vector<za::SizeT> enteredSubcaseDepths;

    // Active subcase path, used only for doctest-style diagnostics.
    za::Vector<SubcaseSignature> activeSubcases;

    za::SizeT activeSubcaseDepth = 0u;
    za::SizeT decisionDepth      = 0u;

    void resetForTestCase() noexcept;
    void resetForRun() noexcept;
    bool advance() noexcept;
    bool tryEnterSubcase(const SubcaseSignature& sig);
    void leaveSubcase() noexcept;
};


////////////////////////////////////////////////////////////
struct FailedTestCaseRecord
{
    const char* name;
    const char* file;
    int         line;

    bool uncaughtException; // true when the failure path was the runner's catch-all
};


////////////////////////////////////////////////////////////
struct ContextState
{
    TraversalState traversal;

    // Active INFO messages (scoped). Most-recent at the back.
    za::Vector<za::String> infoStack;

    // Total stats across the whole run.
    za::SizeT totalAssertions  = 0u;
    za::SizeT failedAssertions = 0u;
    za::SizeT totalTestCases   = 0u;
    za::SizeT failedTestCases  = 0u;
    za::SizeT skippedTestCases = 0u;

    // Test cases that failed, so the runner can list them at the end of the
    // summary. One entry per failed test case (not per failed assertion).
    za::Vector<FailedTestCaseRecord> failedTestCaseList;

    // Per-test-case state.
    bool        currentTestFailed = false;
    const char* currentTestName   = nullptr;
    const char* currentTestFile   = nullptr;
    int         currentTestLine   = 0;

    // File / line of the FIRST failing assertion seen in the current test
    // case, used by the end-of-run "failed test cases:" summary so it points
    // at the actual failure site rather than the TEST_CASE declaration.
    const char* firstFailureFile = nullptr;
    int         firstFailureLine = 0;

    // Rendered operands of the most recent failed assertion (cold path).
    char      decompBuf[512];
    za::SizeT decompLen = 0u;
    bool      hasDecomp = false;

    // Sentinel thrown by `REQUIRE` failure to unwind the test body.
    struct RequireFailedException
    {
    };

    bool verbose  = false;
    bool listOnly = false;

    // Filter substrings. Tests with a name *containing* one of these run.
    za::Vector<za::String> filterInclude;
    za::Vector<za::String> filterExclude;
};


////////////////////////////////////////////////////////////
[[nodiscard]] ContextState& contextState() noexcept;


////////////////////////////////////////////////////////////
[[nodiscard]] za::Vector<TestCaseInfo>& registeredTestCases() noexcept;

} // namespace tst::detail


////////////////////////////////////////////////////////////
/// \file
/// Internal-only header. Holds the singleton state used by the runner,
/// subcase traversal, and assertion reporting. Test-side code never
/// includes this -- only `Tst/*.cpp` does.
////////////////////////////////////////////////////////////
