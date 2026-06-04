#include "Tst/Detail/StringifyValue.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/VideoMode.hpp"

#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/ToChars.hpp"

namespace
{
////////////////////////////////////////////////////////////
template <typename T>
zb::String winIntToString(const T value)
{
    char       buf[32];
    char*      end = zb::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<zb::SizeT>(end - buf);
    return zb::String{buf, len};
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::VideoMode& videoMode) noexcept
{
    const zb::String s = winIntToString(videoMode.size.x) + zb::String{"x"} + winIntToString(videoMode.size.y) +
                               zb::String{"x"} + winIntToString(videoMode.bitsPerPixel);

    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace za
