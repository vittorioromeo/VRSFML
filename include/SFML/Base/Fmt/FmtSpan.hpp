#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal format-string carrier (data pointer + length).
///
/// Used in place of `base::StringView` so Fmt's transitive include
/// closure stays free of `<SFML/Base/StringView.hpp>`. Same 16-byte
/// layout, no member functions -- callers read `data` and `size` directly.
///
////////////////////////////////////////////////////////////
struct FmtSpan
{
    const char* data;
    SizeT       size;
};

} // namespace sf::base::priv
