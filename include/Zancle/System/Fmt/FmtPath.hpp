#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Export.hpp"

#include "ZancleBase/Fmt/FmtResult.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Path;
} // namespace za


namespace zb
{
struct FmtSpec;
class FmtSink;
} // namespace zb


namespace za::priv
{
struct PathDebugFormatter;
} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
/// \brief `Fmt` ADL hook for `za::Path`. Emits the path as UTF-8.
////////////////////////////////////////////////////////////
[[nodiscard]] ZA_SYSTEM_API base::FmtResult fmtArg(base::FmtSink& sink, const Path& path, const base::FmtSpec& spec);

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
[[nodiscard]] ZA_SYSTEM_API base::FmtResult fmtArg(base::FmtSink&            sink,
                                                     const PathDebugFormatter& dbg,
                                                     const base::FmtSpec&      spec);

} // namespace za::priv
