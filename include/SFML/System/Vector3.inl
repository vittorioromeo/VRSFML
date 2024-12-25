#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector3.hpp" // NOLINTNEXTLINE(misc-header-include-cycle)

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/Traits/IsFloatingPoint.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::normalized() const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector3::normalized() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector3<T>() && "Vector3::normalized() cannot normalize a zero vector");
    return (*this) / length();
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector3<T>::length() const
{
    static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector3::length() is only supported for floating point types");

    // don't use std::hypot because of slow performance
    return base::sqrt(x * x + y * y + z * z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector3<T>::lengthSquared() const
{
    return dot(*this);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector3<T>::dot(const Vector3<T>& rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::cross(const Vector3<T>& rhs) const
{
    return Vector3<T>((y * rhs.z) - (z * rhs.y), (z * rhs.x) - (x * rhs.z), (x * rhs.y) - (y * rhs.x));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::componentWiseMul(const Vector3<T>& rhs) const
{
    return Vector3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::componentWiseDiv(const Vector3<T>& rhs) const
{
    SFML_BASE_ASSERT(rhs.x != 0 && "Vector3::componentWiseDiv() cannot divide by 0 (x coordinate)");
    SFML_BASE_ASSERT(rhs.y != 0 && "Vector3::componentWiseDiv() cannot divide by 0 (y coordinate)");
    SFML_BASE_ASSERT(rhs.z != 0 && "Vector3::componentWiseDiv() cannot divide by 0 (z coordinate)");

    return Vector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename OtherVector3>
constexpr OtherVector3 Vector3<T>::to() const
{
    using ValueType = decltype(OtherVector3{}.x);
    return Vector3<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y), static_cast<ValueType>(z)};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator-(const Vector3<T>& lhs)
{
    return Vector3<T>(-lhs.x, -lhs.y, -lhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator+=(Vector3<T>& lhs, const Vector3<T>& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator-=(Vector3<T>& lhs, const Vector3<T>& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator+(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
    return Vector3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator-(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
    return Vector3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator*(const Vector3<T>& lhs, T rhs)
{
    return Vector3<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator*(T lhs, const Vector3<T>& rhs)
{
    return Vector3<T>(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator*=(Vector3<T>& lhs, T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator/(const Vector3<T>& lhs, T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector3::operator/ cannot divide by 0");

    return Vector3<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator/=(Vector3<T>& lhs, T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector3::operator/= cannot divide by 0");

    lhs.x /= rhs;
    lhs.y /= rhs;
    lhs.z /= rhs;

    return lhs;
}

} // namespace sf
