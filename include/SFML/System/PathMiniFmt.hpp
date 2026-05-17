#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Path;
} // namespace sf


namespace sf::base
{
struct FormatSpec;
class FormatSink;
} // namespace sf::base


namespace sf::priv
{
struct PathDebugFormatter;
} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief `MiniFmt` ADL hook for `sf::Path`. Emits the path as UTF-8.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void formatArg(base::FormatSink& sink, const Path& path, const base::FormatSpec& spec);

} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief `MiniFmt` ADL hook for the `PathDebugFormatter` shim.
///
/// Renders two lines of debug information:
///     Provided path: <input>
///     Absolute path: <canonical or "<unavailable>">
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void formatArg(base::FormatSink& sink, const PathDebugFormatter& dbg, const base::FormatSpec& spec);

} // namespace sf::priv
