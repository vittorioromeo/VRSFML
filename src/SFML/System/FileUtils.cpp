#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/FileUtils.hpp"
#include "SFML/System/Path.hpp"

#include <string>

#include <cctype>
#include <cstdio>


namespace sf
{
std::FILE* openFile(const Path& filename, base::StringView mode)
{
#ifdef SFML_SYSTEM_WINDOWS
    const std::wstring wmode(mode.begin(), mode.end());
    return _wfopen(filename.c_str(), wmode.data());
#else
    return std::fopen(filename.c_str(), mode.data());
#endif
}

} // namespace sf
