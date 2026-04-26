#include "GraphicsUtil.hpp"
#include "SystemUtil.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/AutoWrapAngle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/Vec2.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"

#include <Doctest.hpp>


namespace
{
////////////////////////////////////////////////////////////
// Build a `doctest::String` from a float, without dragging `<ostream>`/`<format>`.
doctest::String floatToString(const float value, const int precision = 6)
{
    char       buf[64];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value, precision);
    const auto len = static_cast<doctest::String::size_type>(end - buf);
    return {buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
doctest::String intToString(const T value)
{
    char       buf[32];
    char*      end = sf::base::toChars(buf, buf + sizeof(buf), value);
    const auto len = static_cast<doctest::String::size_type>(end - buf);
    return {buf, len};
}


////////////////////////////////////////////////////////////
template <typename T>
doctest::String numToString(const T value)
{
    if constexpr (sf::base::isFloatingPoint<T>)
        return floatToString(static_cast<float>(value));
    else
        return intToString(value);
}

} // namespace


namespace doctest
{
////////////////////////////////////////////////////////////
String StringMaker<sf::Angle>::convert(const sf::Angle& angle)
{
    return floatToString(angle.asDegrees()) + " deg";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::AutoWrapAngle>::convert(const sf::AutoWrapAngle& angle)
{
    return floatToString(angle.asDegrees()) + " deg";
}


////////////////////////////////////////////////////////////
String StringMaker<sf::UnicodeString>::convert(const sf::UnicodeString& string)
{
    const auto ansi = string.toAnsiString<sf::base::String>();
    return {ansi.data(), static_cast<String::size_type>(ansi.size())};
}


////////////////////////////////////////////////////////////
String StringMaker<sf::Time>::convert(const sf::Time time)
{
    return intToString(time.asMicroseconds()) + "us";
}


////////////////////////////////////////////////////////////
template <typename T>
String StringMaker<sf::Vec2<T>>::convert(const sf::Vec2<T>& vec)
{
    return String("(") + numToString(vec.x) + ", " + numToString(vec.y) + ")";
}


////////////////////////////////////////////////////////////
template <typename T>
String StringMaker<sf::Vec3<T>>::convert(const sf::Vec3<T>& vec)
{
    return String("(") + numToString(vec.x) + ", " + numToString(vec.y) + ", " + numToString(vec.z) + ")";
}


////////////////////////////////////////////////////////////
// Explicit instantiations for the types actually used by tests.
template struct StringMaker<sf::Vec2<int>>;
template struct StringMaker<sf::Vec2<unsigned int>>;
template struct StringMaker<sf::Vec2<float>>;

template struct StringMaker<sf::Vec3<int>>;
template struct StringMaker<sf::Vec3<unsigned int>>;
template struct StringMaker<sf::Vec3<float>>;

} // namespace doctest


////////////////////////////////////////////////////////////
bool operator==(const float& lhs, const Approx<float>& rhs)
{
    return static_cast<double>(lhs) == doctest::Approx(static_cast<double>(rhs.value)).epsilon(1e-3);
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
