#pragma once

#include "Tst/Detail/StringifyValue.hpp"

#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"


namespace zb
{
// Found by ADL when an `zb::String` operand needs rendering for a
// failed assertion.
[[gnu::always_inline]] inline zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const String& s) noexcept
{
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace zb
