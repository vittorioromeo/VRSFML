#pragma once

#include "Tst/Detail/StringifyValue.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StringView.hpp"


namespace zb
{
// Found by ADL when a `StringView` operand needs rendering for a failed
// assertion. Bytes are copied straight into the runner's scratch buffer.
[[gnu::always_inline]] inline zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const StringView& sv) noexcept
{
    return ::tst::detail::copyInto(buf, cap, sv.data(), sv.size());
}

} // namespace zb
