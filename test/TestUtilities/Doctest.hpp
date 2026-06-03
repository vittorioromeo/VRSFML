#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Source-compatible front for the bespoke testing library. Every
// existing `.test.cpp` keeps `#include <Doctest.hpp>` and gets the
// macros + helpers it expects, but all bodies live under `Tst/`.
////////////////////////////////////////////////////////////


#include "DoctestFwd.hpp" // IWYU pragma: export
#include "Tst/Approx.hpp" // IWYU pragma: export
#include "Tst/Tst.hpp"    // IWYU pragma: export


////////////////////////////////////////////////////////////
// Catch2 / doctest legacy namespace aliases. The few tests that came
// over from Catch2 still spell things `Catch::Approx`; nothing in the
// new library lives in those namespaces, so we re-export.
////////////////////////////////////////////////////////////
namespace doctest
{
using ::tst::Approx;
using ::tst::skip;

namespace Matchers
{
using ::tst::Matchers::WithinAbs;
using ::tst::Matchers::WithinRel;
} // namespace Matchers
} // namespace doctest


namespace Catch
{
using Approx = ::tst::Approx;

// NOLINTNEXTLINE(misc-unused-alias-decls)
namespace Matchers = ::tst::Matchers;
} // namespace Catch
