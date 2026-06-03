#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/TstFwd.hpp"


////////////////////////////////////////////////////////////
// RAII helper that drives the subcase tree traversal. The runner sets
// up a per-test-case `TraversalState` (see `Subcase.cpp`); each
// `SUBCASE` macro instantiates a `SubcaseScope` whose ctor consults the
// state and decides whether the subcase body should execute on this run.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
class SubcaseScope
{
public:
    SubcaseScope(const char* name, const char* file, int line) noexcept;
    ~SubcaseScope();

    SubcaseScope(const SubcaseScope&)            = delete;
    SubcaseScope(SubcaseScope&&)                 = delete;
    SubcaseScope& operator=(const SubcaseScope&) = delete;
    SubcaseScope& operator=(SubcaseScope&&)      = delete;

    [[nodiscard, gnu::always_inline]] explicit operator bool() const noexcept
    {
        return m_entered;
    }

private:
    bool m_entered;
};

} // namespace tst::detail
