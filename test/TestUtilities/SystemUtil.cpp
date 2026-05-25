#include "SystemUtil.hpp"

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
#include "SFML/Base/Trait/IsFloatingPoint.hpp"

#include <Doctest.hpp>


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


////////////////////////////////////////////////////////////
template <typename T>
sf::base::String numToString(const T value)
{
    if constexpr (sf::base::isFloatingPoint<T>)
        return sysFloatToString(static_cast<float>(value));
    else
        return sysIntToString(value);
}

} // namespace


namespace tst
{
////////////////////////////////////////////////////////////
sf::base::String StringMaker<sf::Angle>::convert(const sf::Angle& angle)
{
    return sysFloatToString(angle.asDegrees()) + sf::base::String{" deg"};
}


////////////////////////////////////////////////////////////
sf::base::String StringMaker<sf::AutoWrapAngle>::convert(const sf::AutoWrapAngle& angle)
{
    return sysFloatToString(angle.asDegrees()) + sf::base::String{" deg"};
}


////////////////////////////////////////////////////////////
sf::base::String StringMaker<sf::Utf8String>::convert(const sf::Utf8String& string)
{
    return sf::base::String{string.data(), static_cast<sf::base::SizeT>(string.byteSize())};
}


////////////////////////////////////////////////////////////
sf::base::String StringMaker<sf::Time>::convert(const sf::Time time)
{
    return sysIntToString(time.asMicroseconds()) + sf::base::String{"us"};
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::String StringMaker<sf::Vec2<T>>::convert(const sf::Vec2<T>& vec)
{
    return sf::base::String{"("} + numToString(vec.x) + sf::base::String{", "} + numToString(vec.y) +
           sf::base::String{")"};
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::String StringMaker<sf::Vec3<T>>::convert(const sf::Vec3<T>& vec)
{
    return sf::base::String{"("} + numToString(vec.x) + sf::base::String{", "} + numToString(vec.y) +
           sf::base::String{", "} + numToString(vec.z) + sf::base::String{")"};
}


////////////////////////////////////////////////////////////
// Use the `stringify<T>` entry point (with its `if constexpr`
// fallback) rather than `StringMaker<T>::convert` directly -- the
// latter would require every wrapped type's StringMaker
// specialization to be visible in this TU, which is not always the
// case (e.g. `sf::Color`'s specialization lives in
// `GraphicsUtil.hpp`).
////////////////////////////////////////////////////////////
// Inline dispatch (mirrors the now-removed `tst::stringify` helper).
// Some `T` (e.g. `sf::Color`) only have their `StringMaker`
// specialization in scope in other utility headers; fall back to the
// generic placeholder for those cases rather than requiring the
// caller TU to drag in every dependency.
template <typename T>
sf::base::String StringMaker<::Approx<T>>::convert(const ::Approx<T>& approx)
{
    if constexpr (requires { ::tst::StringMaker<T>{}; })
        return ::tst::StringMaker<T>::convert(approx.value);
    else
        return ::tst::detail::defaultStringification();
}


////////////////////////////////////////////////////////////
// Explicit instantiations for the types actually used by tests.
template struct StringMaker<sf::Vec2<int>>;
template struct StringMaker<sf::Vec2<unsigned int>>;
template struct StringMaker<sf::Vec2<float>>;

template struct StringMaker<sf::Vec3<int>>;
template struct StringMaker<sf::Vec3<unsigned int>>;
template struct StringMaker<sf::Vec3<float>>;

template struct StringMaker<::Approx<float>>;
template struct StringMaker<::Approx<sf::Angle>>;
template struct StringMaker<::Approx<sf::Vec2<float>>>;
template struct StringMaker<::Approx<sf::Vec3<float>>>;

} // namespace tst


////////////////////////////////////////////////////////////
// Forward-declared graphics-side Approx specializations needed by the
// graphics tests' explicit-instantiation block in `GraphicsUtil.cpp`.
// The template body lives here (SystemUtil), so the instantiations
// need to be here too -- a `.cpp` instantiation outside the file that
// defines the template body produces no symbol.
////////////////////////////////////////////////////////////
namespace sf
{
struct Color;
struct Transform;
template <typename>
class Rect2;
} // namespace sf

namespace tst
{
template struct StringMaker<::Approx<sf::Color>>;
template struct StringMaker<::Approx<sf::Transform>>;
template struct StringMaker<::Approx<sf::Rect2<float>>>;
} // namespace tst


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
