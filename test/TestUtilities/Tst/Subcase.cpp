// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Detail/State.hpp"
#include "Tst/Detail/Subcase.hpp"
#include "Tst/TstFwd.hpp"

#include "SFML/Base/Builtin/Strcmp.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Subcase tree traversal. Algorithm mirrors doctest's:
//
//   1. The test body is re-run until `advance()` returns false.
//
//   2. On each run, `TraversalState` walks the subcase tree and records
//      the sibling indices at each decision depth.
//
//   3. `tryEnterSubcase` returns true if THIS subcase's sibling index
//      matches the current decision-path entry.
//
//   4. After the run, `advance()` increments the deepest yet-unexhausted
//      decision-path index, dropping any deeper entries (siblings of
//      that depth get a fresh discovery cycle on the next run).
//
////////////////////////////////////////////////////////////


namespace
{
////////////////////////////////////////////////////////////
bool sameCStr(const char* a, const char* b) noexcept
{
    if (a == b)
        return true;

    if (a == nullptr || b == nullptr)
        return false;

    return SFML_BASE_STRCMP(a, b) == 0;
}

} // namespace


namespace tst::detail
{
////////////////////////////////////////////////////////////
bool SubcaseSignature::operator==(const SubcaseSignature& other) const noexcept
{
    return line == other.line && sameCStr(file, other.file) && sameCStr(name, other.name);
}


////////////////////////////////////////////////////////////
void TraversalState::resetForTestCase() noexcept
{
    decisionPath.clear();
    discoveredDecisionPath.clear();
    enteredSubcaseDepths.clear();
    activeSubcases.clear();
    activeSubcaseDepth = 0u;
    decisionDepth      = 0u;
}


////////////////////////////////////////////////////////////
void TraversalState::resetForRun() noexcept
{
    activeSubcaseDepth = 0u;
    decisionDepth      = 0u;
    enteredSubcaseDepths.clear();
    activeSubcases.clear();
    discoveredDecisionPath.clear();
}


////////////////////////////////////////////////////////////
bool TraversalState::advance() noexcept
{
    const sf::base::SizeT maxDepth = decisionPath.size() < discoveredDecisionPath.size() ? decisionPath.size()
                                                                                         : discoveredDecisionPath.size();

    for (sf::base::SizeT depth = maxDepth; depth > 0u; --depth)
    {
        const sf::base::SizeT index = depth - 1u;
        const auto&           dp    = discoveredDecisionPath.data()[index];

        if (decisionPath.data()[index] + 1u < dp.branchCount)
        {
            ++decisionPath.data()[index];

            // Truncate deeper entries -- they'll be rediscovered on the next run.
            while (decisionPath.size() > depth)
                decisionPath.popBack();

            return true;
        }
    }

    return false;
}


////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
DecisionPoint& ensureDecisionPoint(TraversalState& s)
{
    const sf::base::SizeT depth = s.decisionDepth;

    if (s.discoveredDecisionPath.size() == depth)
    {
        s.discoveredDecisionPath.emplaceBack();

        if (s.decisionPath.size() == depth)
            s.decisionPath.pushBack(static_cast<sf::base::SizeT>(0u));
    }

    return s.discoveredDecisionPath.data()[depth];
}

} // namespace


////////////////////////////////////////////////////////////
bool TraversalState::tryEnterSubcase(const SubcaseSignature& sig)
{
    DecisionPoint& point = ensureDecisionPoint(*this);

    auto&                 subcases     = point.subcases;
    sf::base::SizeT       siblingIndex = 0u;
    const sf::base::SizeT subcasesSize = subcases.size();

    for (; siblingIndex < subcasesSize; ++siblingIndex)
    {
        if (subcases.data()[siblingIndex] == sig)
            break;
    }

    if (siblingIndex == subcasesSize)
        subcases.pushBack(sig);

    point.branchCount = subcases.size();

    if (siblingIndex != decisionPath.data()[decisionDepth])
        return false;

    enteredSubcaseDepths.pushBack(decisionDepth);
    activeSubcases.pushBack(sig);
    ++activeSubcaseDepth;
    ++decisionDepth;
    return true;
}


////////////////////////////////////////////////////////////
void TraversalState::leaveSubcase() noexcept
{
    decisionDepth = enteredSubcaseDepths.data()[enteredSubcaseDepths.size() - 1u];
    enteredSubcaseDepths.popBack();
    activeSubcases.popBack();
    --activeSubcaseDepth;
}


////////////////////////////////////////////////////////////
SubcaseScope::SubcaseScope(const char* name, const char* file, int line) noexcept
{
    SubcaseSignature sig{name, file, line};
    m_entered = contextState().traversal.tryEnterSubcase(sig);
}


////////////////////////////////////////////////////////////
SubcaseScope::~SubcaseScope()
{
    if (m_entered)
        contextState().traversal.leaveSubcase();
}

} // namespace tst::detail
