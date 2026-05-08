#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/System/Path.hpp"

#include <iosfwd>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Stream-insertion operator for `sf::Path`
///
/// Writes the path as UTF-8, avoiding locale-dependent filesystem streaming.
///
////////////////////////////////////////////////////////////
SFML_SYSTEM_API std::ostream& operator<<(std::ostream& os, const Path& path);

} // namespace sf
