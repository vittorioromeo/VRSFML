#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"


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
