#include <SFML/System/Angle.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <SystemUtil.hpp>

namespace sf
{
std::ostream& operator<<(std::ostream& os, const Angle& angle)
{
    os << std::fixed << std::setprecision(std::numeric_limits<float>::max_digits10);
    return os << angle.asDegrees() << " deg";
}

std::ostream& operator<<(std::ostream& os, const String& string)
{
    return os << string.toAnsiString();
}

std::ostream& operator<<(std::ostream& os, Time time)
{
    return os << time.asMicroseconds() << "us";
}
} // namespace sf

bool operator==(const float& lhs, const Approx<float>& rhs)
{
    return static_cast<double>(lhs) == Catch::Approx(static_cast<double>(rhs.value)).margin(1e-5);
}

bool operator==(const sf::Vector2f& lhs, const Approx<sf::Vector2f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}

bool operator==(const sf::Vector3f& lhs, const Approx<sf::Vector3f>& rhs)
{
    return (lhs - rhs.value).length() == Approx(0.f);
}

bool operator==(const sf::Angle& lhs, const Approx<sf::Angle>& rhs)
{
    return lhs.asDegrees() == Approx(rhs.value.asDegrees());
}
