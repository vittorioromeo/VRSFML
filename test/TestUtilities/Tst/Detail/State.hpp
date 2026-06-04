#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/Vector.hpp"


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
    zb::Vector<SubcaseSignature> subcases;
    zb::SizeT                    branchCount = 0u;
};


////////////////////////////////////////////////////////////
struct TraversalState
{
    // The currently-chosen path through the subcase tree on this run.
    // `decisionPath[d]` is the sibling index picked at decision depth `d`.
    zb::Vector<zb::SizeT> decisionPath;

    // What we've actually discovered while running -- one entry per
    // decision depth, holding all sibling subcases observed.
    zb::Vector<DecisionPoint> discoveredDecisionPath;

    // Stack of decision depths corresponding to the currently active
    // (entered) subcases. Used to restore `decisionDepth` on unwind.
    zb::Vector<zb::SizeT> enteredSubcaseDepths;

    // Active subcase path, used only for doctest-style diagnostics.
    zb::Vector<SubcaseSignature> activeSubcases;

    zb::SizeT activeSubcaseDepth = 0u;
    zb::SizeT decisionDepth      = 0u;

    void resetForTestCase() noexcept;
    void resetForRun() noexcept;
    bool advance() noexcept;
    bool tryEnterSubcase(const SubcaseSignature& sig);
    void leaveSubcase() noexcept;
};


////////////////////////////////////////////////////////////
struct ContextState
{
    TraversalState traversal;

    // Active INFO messages (scoped). Most-recent at the back.
    zb::Vector<zb::String> infoStack;

    // Total stats across the whole run.
    zb::SizeT totalAssertions  = 0u;
    zb::SizeT failedAssertions = 0u;
    zb::SizeT totalTestCases   = 0u;
    zb::SizeT failedTestCases  = 0u;
    zb::SizeT skippedTestCases = 0u;

    // Per-test-case state.
    bool        currentTestFailed = false;
    const char* currentTestName   = nullptr;
    const char* currentTestFile   = nullptr;
    int         currentTestLine   = 0;

    // Rendered operands of the most recent failed assertion (cold path).
    char      decompBuf[512];
    zb::SizeT decompLen = 0u;
    bool      hasDecomp = false;

    // Sentinel thrown by `REQUIRE` failure to unwind the test body.
    struct RequireFailedException
    {
    };

    bool verbose  = false;
    bool listOnly = false;

    // Filter substrings. Tests with a name *containing* one of these run.
    zb::Vector<zb::String> filterInclude;
    zb::Vector<zb::String> filterExclude;
};


////////////////////////////////////////////////////////////
[[nodiscard]] ContextState& contextState() noexcept;


////////////////////////////////////////////////////////////
[[nodiscard]] zb::Vector<TestCaseInfo>& registeredTestCases() noexcept;

} // namespace tst::detail


////////////////////////////////////////////////////////////
/// \file
/// Internal-only header. Holds the singleton state used by the runner,
/// subcase traversal, and assertion reporting. Test-side code never
/// includes this -- only `Tst/*.cpp` does.
////////////////////////////////////////////////////////////
