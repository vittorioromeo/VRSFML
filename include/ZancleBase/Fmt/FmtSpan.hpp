#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/SizeT.hpp"


namespace zb::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal format-string carrier (data pointer + length).
///
/// Used in place of `zb::StringView` so Fmt's transitive include
/// closure stays free of `<ZancleBase/StringView.hpp>`. Same 16-byte
/// layout, no member functions -- callers read `data` and `size` directly.
///
////////////////////////////////////////////////////////////
struct FmtSpan
{
    const char* data;
    SizeT       size;
};

} // namespace zb::priv
