#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"


namespace sf
{
class Path;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SFML_SYSTEM_API PathDebugFormatter
{
    const Path& path;
};

} // namespace sf::priv
