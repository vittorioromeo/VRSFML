#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Export.hpp"

#include "Zancle/System/Path.hpp"

#include <iosfwd>


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Stream-insertion operator for `za::Path`
///
/// Writes the path as UTF-8, avoiding locale-dependent filesystem streaming.
///
////////////////////////////////////////////////////////////
ZA_SYSTEM_API std::ostream& operator<<(std::ostream& os, const Path& path);

} // namespace za
