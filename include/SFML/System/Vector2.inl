#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp"
#include "SFML/System/Vector2.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Math/Atan2.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/Traits/IsFloatingPoint.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::lengthSquared() const
{
    return dot(*this);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::perpendicular() const
{
    return Vector2<T>(-y, x);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::dot(Vector2<T> rhs) const
{
    return x * rhs.x + y * rhs.y;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::cross(Vector2<T> rhs) const
{
    return x * rhs.y - y * rhs.x;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::componentWiseMul(Vector2<T> rhs) const
{
    return Vector2<T>(x * rhs.x, y * rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::componentWiseDiv(Vector2<T> rhs) const
{
    SFML_BASE_ASSERT(rhs.x != 0 && "Vector2::componentWiseDiv() cannot divide by 0 (x coordinate)");
    SFML_BASE_ASSERT(rhs.y != 0 && "Vector2::componentWiseDiv() cannot divide by 0 (y coordinate)");

    return Vector2<T>(x / rhs.x, y / rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator-(Vector2<T> rhs)
{
    return Vector2<T>(-rhs.x, -rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator+=(Vector2<T>& lhs, Vector2<T> rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator-=(Vector2<T>& lhs, Vector2<T> rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator+(Vector2<T> lhs, Vector2<T> rhs)
{
    return Vector2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator-(Vector2<T> lhs, Vector2<T> rhs)
{
    return Vector2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator*(Vector2<T> lhs, T rhs)
{
    return Vector2<T>(lhs.x * rhs, lhs.y * rhs);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator*(T lhs, Vector2<T> rhs)
{
    return Vector2<T>(rhs.x * lhs, rhs.y * lhs);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator*=(Vector2<T>& lhs, T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator/(Vector2<T> lhs, T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector2::operator/ cannot divide by 0");

    return Vector2<T>(lhs.x / rhs, lhs.y / rhs);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator/=(Vector2<T>& lhs, T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector2::operator/= cannot divide by 0");

    lhs.x /= rhs;
    lhs.y /= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::normalized() const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::normalized() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::normalized() cannot normalize a zero vector");

    return (*this) / length();
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Angle Vector2<T>::angleTo(Vector2<T> rhs) const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::angleTo() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::angleTo() cannot calculate angle from a zero vector");
    SFML_BASE_ASSERT(rhs != Vector2<T>() && "Vector2::angleTo() cannot calculate angle to a zero vector");

    return radians(static_cast<float>(base::atan2(cross(rhs), dot(rhs))));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Angle Vector2<T>::angle() const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::angle() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::angle() cannot calculate angle from a zero vector");

    return radians(static_cast<float>(base::atan2(y, x)));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::rotatedBy(Angle phi) const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::rotatedBy() is only supported for floating point types");

    // No zero vector assert, because rotating a zero vector is well-defined (yields always itself)
    const T cos = base::cos(static_cast<T>(phi.asRadians()));
    const T sin = base::sin(static_cast<T>(phi.asRadians()));

    // Don't manipulate x and y separately, otherwise they're overwritten too early
    return Vector2<T>(cos * x - sin * y, sin * x + cos * y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::movedTowards(T r, Angle phi) const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::movedTowards() is only supported for floating point types");

    return *this + Vector2<T>::fromAngle(r, phi);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::projectedOnto(Vector2<T> axis) const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::projectedOnto() is only supported for floating point types");

    SFML_BASE_ASSERT(axis != Vector2<T>() && "Vector2::projectedOnto() cannot project onto a zero vector");
    return dot(axis) / axis.lengthSquared() * axis;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::fromAngle(T r, Angle phi)
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                  "Vector2::Vector2(T, Angle) is only supported for floating point types");
    return {r * static_cast<T>(base::cos(phi.asRadians())), r * static_cast<T>(base::sin(phi.asRadians()))};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::length() const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::length() is only supported for floating point types");

    // don't use std::hypot because of slow performance
    return base::sqrt(x * x + y * y);
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename OtherVector2>
constexpr OtherVector2 Vector2<T>::to() const
{
    using ValueType = decltype(OtherVector2{}.x);
    return Vector2<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y)};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<int> Vector2<T>::toVector2i() const
{
    return {static_cast<int>(x), static_cast<int>(y)};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<float> Vector2<T>::toVector2f() const
{
    return {static_cast<float>(x), static_cast<float>(y)};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<unsigned int> Vector2<T>::toVector2u() const
{
    return {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
}

} // namespace sf
