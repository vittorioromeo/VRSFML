#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/Vec2.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp"

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <SystemUtil.hpp>

#include <iomanip>
#include <limits>
#include <ostream>


namespace sf
{
void setStreamPrecision(std::ostream& os, int maxDigits10)
{
    os << std::fixed << std::setprecision(maxDigits10);
}

std::ostream& operator<<(std::ostream& os, const Angle& angle)
{
    setStreamPrecision(os, std::numeric_limits<float>::max_digits10);
    return os << angle.asDegrees() << " deg";
}

std::ostream& operator<<(std::ostream& os, const UnicodeString& string)
{
    return os << string.toAnsiString<base::String>();
}

std::ostream& operator<<(std::ostream& os, Time time)
{
    return os << time.asMicroseconds() << "us";
}

template <typename T>
std::ostream& operator<<(std::ostream& os, Vec2<T> vec)
{
    setStreamPrecision(os, std::numeric_limits<T>::max_digits10);
    return os << "(" << vec.x << ", " << vec.y << ")";
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vec3<T>& vec)
{
    setStreamPrecision(os, std::numeric_limits<T>::max_digits10);
    return os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

template std::ostream& operator<<(std::ostream&, Vec2<int>);
template std::ostream& operator<<(std::ostream&, Vec2<unsigned int>);
template std::ostream& operator<<(std::ostream&, Vec2<float>);

template std::ostream& operator<<(std::ostream&, const Vec3<int>&);
template std::ostream& operator<<(std::ostream&, const Vec3<unsigned int>&);
template std::ostream& operator<<(std::ostream&, const Vec3<float>&);
} // namespace sf

bool operator==(const float& lhs, const Approx<float>& rhs)
{
    return static_cast<double>(lhs) == doctest::Approx(static_cast<double>(rhs.value)).epsilon(1e-3);
}

bool operator==(sf::Vec2f lhs, const Approx<sf::Vec2f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}

bool operator==(const sf::Vec3f& lhs, const Approx<sf::Vec3f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}

bool operator==(const sf::Angle& lhs, const Approx<sf::Angle>& rhs)
{
    return lhs.asRadians() == Approx(rhs.value.asRadians());
}

bool operator==(const sf::Rect2f& lhs, const Approx<sf::Rect2f>& rhs)
{
    return lhs.position == Approx(rhs.value.position) && lhs.size == Approx(rhs.value.size);
}


template <typename T>
std::ostream& operator<<(std::ostream& os, const Approx<T>& approx)
{
    return os << approx.value;
}

template std::ostream& operator<<(std::ostream&, const Approx<float>&);
template std::ostream& operator<<(std::ostream&, const Approx<double>&);
template std::ostream& operator<<(std::ostream&, const Approx<sf::Vec2<float>>&);
template std::ostream& operator<<(std::ostream&, const Approx<sf::Vec3<float>>&);
template std::ostream& operator<<(std::ostream&, const Approx<sf::Transform>&);
template std::ostream& operator<<(std::ostream&, const Approx<sf::Rect2<float>>&);
template std::ostream& operator<<(std::ostream&, const Approx<sf::Angle>&);
