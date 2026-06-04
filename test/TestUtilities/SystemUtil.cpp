#include "SystemUtil.hpp"
#include "Tst/Detail/StringifyValue.hpp"
#include "Tst/Tst.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/AutoWrapAngle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Utf8String.hpp"
#include "SFML/System/Vec2.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/ToChars.hpp"


namespace
{
////////////////////////////////////////////////////////////
sf::base::String sysFloatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<sf::base::SizeT>(end - buf);
    return sf::base::String{buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::String sysIntToString(const T value)
{
    char       buf[32];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<sf::base::SizeT>(end - buf);
    return sf::base::String{buf, len};
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Angle& angle) noexcept
{
    const sf::base::String s = sysFloatToString(angle.asDegrees()) + sf::base::String{" deg"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::AutoWrapAngle& angle) noexcept
{
    const sf::base::String s = sysFloatToString(angle.asDegrees()) + sf::base::String{" deg"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Utf8String& string) noexcept
{
    const sf::base::String s{string.data(), static_cast<sf::base::SizeT>(string.byteSize())};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Time& time) noexcept
{
    const sf::base::String s = sysIntToString(time.asMicroseconds()) + sf::base::String{"us"};
    return ::tst::detail::copyInto(buf, cap, s.data(), s.size());
}

} // namespace sf


////////////////////////////////////////////////////////////
// `::Approx<T>` renders as `Approx(<value>)`, recursing through the same
// ADL-enabled dispatch for its nested value. Wrapped types whose
// `stringifyValue` overload is not visible in this TU (e.g. `sf::Color`)
// render their value as "<?>" -- matching the prior behavior.
////////////////////////////////////////////////////////////
template <typename T>
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const ::Approx<T>& approx) noexcept
{
    sf::base::SizeT len = ::tst::detail::copyInto(buf, cap, "Approx(", 7u);
    len += ::tst::detail::renderValue(buf + len, cap - len, approx.value);

    if (len < cap)
        buf[len++] = ')';

    return len;
}


////////////////////////////////////////////////////////////
// Explicit instantiations for the wrapped types actually used by tests.
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<float>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<sf::Angle>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<sf::Vec2<float>>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<sf::Vec3<float>>&) noexcept;


////////////////////////////////////////////////////////////
// Graphics-side wrapped types: the value renders as "<?>" here (their
// `stringifyValue` overload lives in `GraphicsUtil`), but the `::Approx`
// instantiations must live in the TU that defines the template body.
////////////////////////////////////////////////////////////
namespace sf
{
struct Color;
struct Transform;
template <typename>
class Rect2;
} // namespace sf

template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<sf::Color>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<sf::Transform>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const ::Approx<sf::Rect2<float>>&) noexcept;


////////////////////////////////////////////////////////////
bool operator==(const float& lhs, const Approx<float>& rhs)
{
    return static_cast<double>(lhs) == ::tst::Approx(static_cast<double>(rhs.value)).epsilon(1e-3);
}


////////////////////////////////////////////////////////////
bool operator==(sf::Vec2f lhs, const Approx<sf::Vec2f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}


////////////////////////////////////////////////////////////
bool operator==(const sf::Vec3f& lhs, const Approx<sf::Vec3f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}


////////////////////////////////////////////////////////////
bool operator==(const sf::Angle& lhs, const Approx<sf::Angle>& rhs)
{
    return lhs.asRadians() == Approx(rhs.value.asRadians());
}


////////////////////////////////////////////////////////////
bool operator==(const sf::Rect2f& lhs, const Approx<sf::Rect2f>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}
