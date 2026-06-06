#include "SystemUtil.hpp"
#include "Tst/Detail/StringifyValue.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/String/String.hpp"
#include "Zancle/String/ToChars.hpp"
#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Geometry/Angle.hpp"
#include "Zancle/Geometry/AutoWrapAngle.hpp"
#include "Zancle/Geometry/Rect2.hpp"
#include "Zancle/Geometry/Vec2.hpp"
#include "Zancle/Geometry/Vec3.hpp"

#include "Zancle/Base/SizeT.hpp"


namespace
{
////////////////////////////////////////////////////////////
za::String sysFloatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = za::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<za::SizeT>(end - buf);
    return za::String{buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
za::String sysIntToString(const T value)
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
za::SizeT stringifyValue(char* buf, za::SizeT cap, const za::Angle& angle) noexcept
{
    const za::String s = sysFloatToString(angle.asDegrees()) + za::String{" deg"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
za::SizeT stringifyValue(char* buf, za::SizeT cap, const za::AutoWrapAngle& angle) noexcept
{
    const za::String s = sysFloatToString(angle.asDegrees()) + za::String{" deg"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
za::SizeT stringifyValue(char* buf, za::SizeT cap, const za::Utf8String& string) noexcept
{
    const za::String s{string.data(), static_cast<za::SizeT>(string.byteSize())};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
za::SizeT stringifyValue(char* buf, za::SizeT cap, const za::Time& time) noexcept
{
    const za::String s = sysIntToString(time.asMicroseconds()) + za::String{"us"};
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
za::SizeT stringifyValue(char* buf, za::SizeT cap, const ::Approx<T>& approx) noexcept
{
    za::SizeT len = ::tst::detail::copyInto(buf, cap, "Approx(", 7u);
    len += ::tst::detail::renderValue(buf + len, cap - len, approx.value);

    if (len < cap)
        buf[len++] = ')';

    return len;
}


////////////////////////////////////////////////////////////
// Explicit instantiations for the wrapped types actually used by tests.
template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<float>&) noexcept;
template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<za::Angle>&) noexcept;
template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<za::Vec2<float>>&) noexcept;
template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<za::Vec3<float>>&) noexcept;


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

template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<za::Color>&) noexcept;
template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<za::Transform>&) noexcept;
template za::SizeT stringifyValue(char*, za::SizeT, const ::Approx<za::Rect2<float>>&) noexcept;


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
