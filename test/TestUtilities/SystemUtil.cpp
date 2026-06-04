#include "SystemUtil.hpp"
#include "Tst/Detail/StringifyValue.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/System/Angle.hpp"
#include "Zancle/System/AutoWrapAngle.hpp"
#include "Zancle/System/Rect2.hpp"
#include "Zancle/System/Time.hpp"
#include "Zancle/System/Utf8String.hpp"
#include "Zancle/System/Vec2.hpp"
#include "Zancle/System/Vec3.hpp"

#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/ToChars.hpp"


namespace
{
////////////////////////////////////////////////////////////
zb::String sysFloatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = zb::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<zb::SizeT>(end - buf);
    return zb::String{buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
zb::String sysIntToString(const T value)
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
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Angle& angle) noexcept
{
    const zb::String s = sysFloatToString(angle.asDegrees()) + zb::String{" deg"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::AutoWrapAngle& angle) noexcept
{
    const zb::String s = sysFloatToString(angle.asDegrees()) + zb::String{" deg"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Utf8String& string) noexcept
{
    const zb::String s{string.data(), static_cast<zb::SizeT>(string.byteSize())};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Time& time) noexcept
{
    const zb::String s = sysIntToString(time.asMicroseconds()) + zb::String{"us"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace za


////////////////////////////////////////////////////////////
// `::Approx<T>` renders as `Approx(<value>)`, recursing through the same
// ADL-enabled dispatch for its nested value. Wrapped types whose
// `stringifyValue` overload is not visible in this TU (e.g. `za::Color`)
// render their value as "<?>" -- matching the prior behavior.
////////////////////////////////////////////////////////////
template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const ::Approx<T>& approx) noexcept
{
    zb::SizeT len = ::tst::detail::copyInto(buf, cap, "Approx(", 7u);
    len += ::tst::detail::renderValue(buf + len, cap - len, approx.value);

    if (len < cap)
        buf[len++] = ')';

    return len;
}


////////////////////////////////////////////////////////////
// Explicit instantiations for the wrapped types actually used by tests.
template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<float>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<za::Angle>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<za::Vec2<float>>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<za::Vec3<float>>&) noexcept;


////////////////////////////////////////////////////////////
// Graphics-side wrapped types: the value renders as "<?>" here (their
// `stringifyValue` overload lives in `GraphicsUtil`), but the `::Approx`
// instantiations must live in the TU that defines the template body.
////////////////////////////////////////////////////////////
namespace za
{
struct Color;
struct Transform;
template <typename>
class Rect2;
} // namespace za

template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<za::Color>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<za::Transform>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const ::Approx<za::Rect2<float>>&) noexcept;


////////////////////////////////////////////////////////////
bool operator==(const float& lhs, const Approx<float>& rhs)
{
    return static_cast<double>(lhs) == ::tst::Approx(static_cast<double>(rhs.value)).epsilon(1e-3);
}


////////////////////////////////////////////////////////////
bool operator==(za::Vec2f lhs, const Approx<za::Vec2f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}


////////////////////////////////////////////////////////////
bool operator==(const za::Vec3f& lhs, const Approx<za::Vec3f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}


////////////////////////////////////////////////////////////
bool operator==(const za::Angle& lhs, const Approx<za::Angle>& rhs)
{
    return lhs.asRadians() == Approx(rhs.value.asRadians());
}


////////////////////////////////////////////////////////////
bool operator==(const za::Rect2f& lhs, const Approx<za::Rect2f>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}
