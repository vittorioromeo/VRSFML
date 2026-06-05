#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/String/StringView.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Stream-insertion operator for `za::StringView`
///
/// Writes the viewed character range to any stream-like type that
/// exposes a `write(const char*, long)` member.
///
////////////////////////////////////////////////////////////
template <typename StreamLike>
StreamLike& operator<<(StreamLike& stream, const StringView& stringView)
    requires(requires { stream.write(stringView.theData, static_cast<long>(stringView.theSize)); })
{
    stream.write(stringView.theData, static_cast<long>(stringView.theSize));
    return stream;
}

} // namespace za
