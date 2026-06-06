#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "Zancle/Config.hpp"

#include "Zancle/Fmt/FmtResult.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Path;
} // namespace za


namespace za
{
struct FmtSpec;
class FmtSink;
} // namespace za


namespace za::priv
{
struct PathDebugFormatter;
} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
/// \brief `Fmt` ADL hook for `za::Path`. Emits the path as UTF-8.
////////////////////////////////////////////////////////////
[[nodiscard]] ZA_SYSTEM_API za::FmtResult fmtArg(za::FmtSink& sink, const Path& path, const za::FmtSpec& spec);

} // namespace za


namespace za::priv
{
////////////////////////////////////////////////////////////
/// \brief `Fmt` ADL hook for the `PathDebugFormatter` shim.
///
/// Renders two lines of debug information:
///     Provided path: <input>
///     Absolute path: <canonical or "<unavailable>">
////////////////////////////////////////////////////////////
[[nodiscard]] ZA_SYSTEM_API za::FmtResult fmtArg(za::FmtSink& sink, const PathDebugFormatter& dbg, const za::FmtSpec& spec);

} // namespace za::priv
