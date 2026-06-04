#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Export.hpp"


namespace za
{
class Path;
} // namespace za


namespace za::priv
{
////////////////////////////////////////////////////////////
struct ZA_SYSTEM_API PathDebugFormatter
{
    const Path& path;
};

} // namespace za::priv
