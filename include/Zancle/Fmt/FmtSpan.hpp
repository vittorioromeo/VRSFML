#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Base/SizeT.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal format-string carrier (data pointer + length).
///
/// Used in place of `za::StringView` so Fmt's transitive include
/// closure stays free of `<Zancle/String/StringView.hpp>`. Same 16-byte
/// layout, no member functions -- callers read `data` and `size` directly.
///
////////////////////////////////////////////////////////////
struct FmtSpan
{
    const char* data;
    SizeT       size;
};

} // namespace za::priv
