// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/FileUtils.hpp"

#include "SFML/System/Path.hpp"

#ifdef SFML_SYSTEM_WINDOWS
    #include <string> // TODO P1: we can probably remove this and use a wchar_t buffer
#endif

#include <cstdio>


namespace sf
{
////////////////////////////////////////////////////////////
std::FILE* openFile(const Path& filename, const char* const mode)
{
#ifdef SFML_SYSTEM_WINDOWS
    std::wstring wmode;
    for (const char* p = mode; *p != '\0'; ++p)
        wmode += static_cast<wchar_t>(*p);

    return _wfopen(filename.c_str(), wmode.c_str());
#else
    return std::fopen(filename.c_str(), mode);
#endif
}

} // namespace sf
