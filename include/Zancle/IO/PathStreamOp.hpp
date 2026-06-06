#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "Zancle/Config.hpp"

#include "Zancle/IO/Path.hpp"

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
