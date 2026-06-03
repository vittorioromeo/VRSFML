// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Approx.hpp"

#include "SFML/Base/Math/Fabs.hpp"


namespace tst
{
////////////////////////////////////////////////////////////
namespace
{
////////////////////////////////////////////////////////////
constexpr double defaultEpsilon = 1e-12;
constexpr double defaultMargin  = 0.0;
constexpr double defaultScale   = 1.0;


////////////////////////////////////////////////////////////
[[nodiscard]] bool compareApprox(double lhs, double approxValue, double epsilon, double margin, double scale) noexcept
{
    const double absL    = sf::base::fabs(lhs);
    const double absR    = sf::base::fabs(approxValue);
    const double largest = absL > absR ? absL : absR;

    return sf::base::fabs(lhs - approxValue) < epsilon * (scale + largest) + margin;
}

} // namespace


////////////////////////////////////////////////////////////
Approx::Approx(double value) noexcept :
    m_epsilon{defaultEpsilon},
    m_margin{defaultMargin},
    m_scale{defaultScale},
    m_value{value}
{
}


////////////////////////////////////////////////////////////
Approx& Approx::epsilon(double newEpsilon) noexcept
{
    m_epsilon = newEpsilon;
    return *this;
}


////////////////////////////////////////////////////////////
Approx& Approx::scale(double newScale) noexcept
{
    m_scale = newScale;
    return *this;
}


////////////////////////////////////////////////////////////
Approx& Approx::margin(double newMargin) noexcept
{
    m_margin = newMargin;
    return *this;
}


////////////////////////////////////////////////////////////
double Approx::value() const noexcept
{
    return m_value;
}


////////////////////////////////////////////////////////////
double Approx::epsilonValue() const noexcept
{
    return m_epsilon;
}


////////////////////////////////////////////////////////////
double Approx::marginValue() const noexcept
{
    return m_margin;
}


////////////////////////////////////////////////////////////
double Approx::scaleValue() const noexcept
{
    return m_scale;
}


////////////////////////////////////////////////////////////
bool operator==(double lhs, const Approx& rhs) noexcept
{
    return compareApprox(lhs, rhs.m_value, rhs.m_epsilon, rhs.m_margin, rhs.m_scale);
}


////////////////////////////////////////////////////////////
bool operator==(const Approx& lhs, double rhs) noexcept
{
    return rhs == lhs;
}


////////////////////////////////////////////////////////////
bool operator!=(double lhs, const Approx& rhs) noexcept
{
    return !(lhs == rhs);
}


////////////////////////////////////////////////////////////
bool operator!=(const Approx& lhs, double rhs) noexcept
{
    return !(lhs == rhs);
}


////////////////////////////////////////////////////////////
bool operator==(float lhs, const Approx& rhs) noexcept
{
    return static_cast<double>(lhs) == rhs;
}


////////////////////////////////////////////////////////////
bool operator==(const Approx& lhs, float rhs) noexcept
{
    return lhs == static_cast<double>(rhs);
}


////////////////////////////////////////////////////////////
bool operator!=(float lhs, const Approx& rhs) noexcept
{
    return !(lhs == rhs);
}


////////////////////////////////////////////////////////////
bool operator!=(const Approx& lhs, float rhs) noexcept
{
    return !(lhs == rhs);
}


////////////////////////////////////////////////////////////
namespace Matchers
{
// Keep matcher arithmetic in double; the float overloads delegate here so
// double test values never silently lose precision through overload ranking.
////////////////////////////////////////////////////////////
bool operator==(float value, const WithinRel& m) noexcept
{
    return static_cast<double>(value) == m;
}


////////////////////////////////////////////////////////////
bool operator==(double value, const WithinRel& m) noexcept
{
    return compareApprox(value, m.target, m.epsilon, defaultMargin, defaultScale);
}


////////////////////////////////////////////////////////////
bool operator==(float value, const WithinAbs& m) noexcept
{
    return static_cast<double>(value) == m;
}


////////////////////////////////////////////////////////////
bool operator==(double value, const WithinAbs& m) noexcept
{
    return sf::base::fabs(value - m.target) <= m.epsilon;
}
} // namespace Matchers

} // namespace tst
