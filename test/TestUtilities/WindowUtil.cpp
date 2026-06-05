#include "Tst/Detail/StringifyValue.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/VideoMode.hpp"

#include "Zancle/Base/SizeT.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/ToChars.hpp"

namespace
{
////////////////////////////////////////////////////////////
template <typename T>
za::String winIntToString(const T value)
{
    char       buf[32];
    char*      end = za::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<za::SizeT>(end - buf);
    return za::String{buf, len};
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
za::SizeT stringifyValue(char* buf, za::SizeT cap, const za::VideoMode& videoMode) noexcept
{
    const za::String s = winIntToString(videoMode.size.x) + za::String{"x"} + winIntToString(videoMode.size.y) +
                         za::String{"x"} + winIntToString(videoMode.bitsPerPixel);

    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace za
