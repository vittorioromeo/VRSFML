#pragma once

#include "Tst/Detail/StringifyValue.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf::base
{
// Found by ADL when a `StringView` operand needs rendering for a failed
// assertion. Bytes are copied straight into the runner's scratch buffer.
[[gnu::always_inline]] inline sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const StringView& sv) noexcept
{
    return ::tst::detail::copyInto(buf, cap, sv.data(), sv.size());
}

} // namespace sf::base
