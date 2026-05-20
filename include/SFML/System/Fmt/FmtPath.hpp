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
enum class FmtResult;
struct FmtSpec;
class FmtSink;
} // namespace sf::base


namespace sf::priv
{
struct PathDebugFormatter;
} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief `Fmt` ADL hook for `sf::Path`. Emits the path as UTF-8.
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API base::FmtResult fmtArg(base::FmtSink& sink, const Path& path, const base::FmtSpec& spec);

} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief `Fmt` ADL hook for the `PathDebugFormatter` shim.
///
/// Renders two lines of debug information:
///     Provided path: <input>
///     Absolute path: <canonical or "<unavailable>">
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API base::FmtResult fmtArg(base::FmtSink&            sink,
                                                     const PathDebugFormatter& dbg,
                                                     const base::FmtSpec&      spec);

} // namespace sf::priv
