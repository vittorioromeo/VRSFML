#pragma once

#include "Tst/Detail/StringifyValue.hpp"

#include "Zancle/String/StringView.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace za
{
// Found by ADL when a `StringView` operand needs rendering for a failed
// assertion. Bytes are copied straight into the runner's scratch buffer.
[[gnu::always_inline]] inline za::SizeT stringifyValue(char* buf, za::SizeT cap, const StringView& sv) noexcept
{
    return ::tst::detail::copyInto(buf, cap, sv.data(), sv.size());
}

} // namespace za
