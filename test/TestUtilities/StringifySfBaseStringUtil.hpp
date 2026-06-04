#pragma once

#include "Tst/Detail/StringifyValue.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"


namespace sf::base
{
// Found by ADL when an `sf::base::String` operand needs rendering for a
// failed assertion.
[[gnu::always_inline]] inline sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const String& s) noexcept
{
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace sf::base
