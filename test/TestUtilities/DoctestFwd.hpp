#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Detail/Stringify.hpp" // IWYU pragma: export
#include "Tst/TstFwd.hpp"           // IWYU pragma: export


namespace doctest
{
////////////////////////////////////////////////////////////
// Alias declaration -- only the forward declaration of String (in
// `TstFwd.hpp`) is needed at this point.
////////////////////////////////////////////////////////////
using String = ::sf::base::String;

} // namespace doctest


////////////////////////////////////////////////////////////
/// \file
///
/// Minimal forward header that maps the historic `doctest::` surface
/// onto the bespoke testing library. Pulled by the various
/// `Stringify*Util.hpp` shims and by a few legacy stringification
/// utilities -- it is INTENTIONALLY free of `<SFML/Base/String.hpp>`
/// so it imposes near-zero cost on the TUs that include it.
///
/// Includers that need the full `sf::base::String` definition (and
/// pretty much all of them do, because their inline `StringMaker`
/// bodies return `String` by value) must include
/// `<SFML/Base/String.hpp>` themselves.
////////////////////////////////////////////////////////////
