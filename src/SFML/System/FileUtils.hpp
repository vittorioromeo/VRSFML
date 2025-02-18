#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/StringView.hpp"

#include <cstdio>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Path;
} // namespace sf


namespace sf
{
[[nodiscard]] SFML_SYSTEM_API std::FILE* openFile(const Path& filename, base::StringView mode);
} // namespace sf
