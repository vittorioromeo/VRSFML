#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>


namespace sf
{
class Path;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct PathDebugFormatter
{
    const Path& path;

    explicit PathDebugFormatter(const Path& thePath) : path(thePath)
    {
    }
};

} // namespace sf::priv
