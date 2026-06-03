#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"


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
    sf::base::Vector<SubcaseSignature> subcases;
    sf::base::SizeT                    branchCount = 0u;
};


////////////////////////////////////////////////////////////
struct TraversalState
{
    // The currently-chosen path through the subcase tree on this run.
    // `decisionPath[d]` is the sibling index picked at decision depth `d`.
    sf::base::Vector<sf::base::SizeT> decisionPath;

    // What we've actually discovered while running -- one entry per
    // decision depth, holding all sibling subcases observed.
    sf::base::Vector<DecisionPoint> discoveredDecisionPath;

    // Stack of decision depths corresponding to the currently active
    // (entered) subcases. Used to restore `decisionDepth` on unwind.
    sf::base::Vector<sf::base::SizeT> enteredSubcaseDepths;

    // Active subcase path, used only for doctest-style diagnostics.
    sf::base::Vector<SubcaseSignature> activeSubcases;

    sf::base::SizeT activeSubcaseDepth = 0u;
    sf::base::SizeT decisionDepth      = 0u;

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
    sf::base::Vector<sf::base::String> infoStack;

    // Total stats across the whole run.
    sf::base::SizeT totalAssertions  = 0u;
    sf::base::SizeT failedAssertions = 0u;
    sf::base::SizeT totalTestCases   = 0u;
    sf::base::SizeT failedTestCases  = 0u;
    sf::base::SizeT skippedTestCases = 0u;

    // Per-test-case state.
    bool        currentTestFailed = false;
    const char* currentTestName   = nullptr;
    const char* currentTestFile   = nullptr;
    int         currentTestLine   = 0;

    // Rendered operands of the most recent failed assertion (cold path).
    char            decompBuf[512];
    sf::base::SizeT decompLen = 0u;
    bool            hasDecomp = false;

    // Sentinel thrown by `REQUIRE` failure to unwind the test body.
    struct RequireFailedException
    {
    };

    bool verbose  = false;
    bool listOnly = false;

    // Filter substrings. Tests with a name *containing* one of these run.
    sf::base::Vector<sf::base::String> filterInclude;
    sf::base::Vector<sf::base::String> filterExclude;
};


////////////////////////////////////////////////////////////
[[nodiscard]] ContextState& contextState() noexcept;


////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Vector<TestCaseInfo>& registeredTestCases() noexcept;

} // namespace tst::detail


////////////////////////////////////////////////////////////
/// \file
/// Internal-only header. Holds the singleton state used by the runner,
/// subcase traversal, and assertion reporting. Test-side code never
/// includes this -- only `Tst/*.cpp` does.
////////////////////////////////////////////////////////////
