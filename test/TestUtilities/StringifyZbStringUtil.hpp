#pragma once

#include "Tst/Detail/StringifyValue.hpp"

#include "Zancle/String/String.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace za
{
// Found by ADL when an `za::String` operand needs rendering for a
// failed assertion.
[[gnu::always_inline]] inline za::SizeT stringifyValue(char* buf, za::SizeT cap, const String& s) noexcept
{
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace za
