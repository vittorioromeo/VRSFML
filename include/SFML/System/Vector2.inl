////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp> // NOLINT(misc-header-include-cycle)

#include <cassert>
#include <cmath>

namespace sf::priv
{

// Named differently from the one in 'Vector3.cpp' to support unity builds.

// clang-format off
template <typename> inline constexpr bool isVec2FloatingPoint              = false;
template <>         inline constexpr bool isVec2FloatingPoint<float>       = true;
template <>         inline constexpr bool isVec2FloatingPoint<double>      = true;
template <>         inline constexpr bool isVec2FloatingPoint<long double> = true;
// clang-format on

} // namespace sf::priv

namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>::Vector2() = default;


////////////////////////////////////////////////////////////
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
template <typename T>
constexpr Vector2<T>::Vector2(T x, T y) : x(x), y(y)
{
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


////////////////////////////////////////////////////////////
template <typename T>
template <typename U>
constexpr Vector2<T>::Vector2(const Vector2<U>& vector) : x(static_cast<T>(vector.x)), y(static_cast<T>(vector.y))
{
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::lengthSq() const
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
constexpr T Vector2<T>::dot(const Vector2<T>& rhs) const
{
    return x * rhs.x + y * rhs.y;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::cross(const Vector2<T>& rhs) const
{
    return x * rhs.y - y * rhs.x;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::cwiseMul(const Vector2<T>& rhs) const
{
    return Vector2<T>(x * rhs.x, y * rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::cwiseDiv(const Vector2<T>& rhs) const
{
    assert(rhs.x != 0 && "Vector2::cwiseDiv() cannot divide by 0 (x coordinate)");
    assert(rhs.y != 0 && "Vector2::cwiseDiv() cannot divide by 0 (y coordinate)");

    return Vector2<T>(x / rhs.x, y / rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator-(const Vector2<T>& right)
{
    return Vector2<T>(-right.x, -right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator+=(Vector2<T>& left, const Vector2<T>& right)
{
    left.x += right.x;
    left.y += right.y;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator-=(Vector2<T>& left, const Vector2<T>& right)
{
    left.x -= right.x;
    left.y -= right.y;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right)
{
    return Vector2<T>(left.x + right.x, left.y + right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right)
{
    return Vector2<T>(left.x - right.x, left.y - right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator*(const Vector2<T>& left, T right)
{
    return Vector2<T>(left.x * right, left.y * right);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator*(T left, const Vector2<T>& right)
{
    return Vector2<T>(right.x * left, right.y * left);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator*=(Vector2<T>& left, T right)
{
    left.x *= right;
    left.y *= right;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator/(const Vector2<T>& left, T right)
{
    assert(right != 0 && "Vector2::operator/ cannot divide by 0");

    return Vector2<T>(left.x / right, left.y / right);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator/=(Vector2<T>& left, T right)
{
    assert(right != 0 && "Vector2::operator/= cannot divide by 0");

    left.x /= right;
    left.y /= right;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator==(const Vector2<T>& left, const Vector2<T>& right)
{
    return (left.x == right.x) && (left.y == right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator!=(const Vector2<T>& left, const Vector2<T>& right)
{
    return (left.x != right.x) || (left.y != right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::normalized() const
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::normalized() is only supported for floating point types");

    assert(*this != Vector2<T>() && "Vector2::normalized() cannot normalize a zero vector");

    return (*this) / length();
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Angle Vector2<T>::angleTo(const Vector2<T>& rhs) const
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::angleTo() is only supported for floating point types");

    assert(*this != Vector2<T>() && "Vector2::angleTo() cannot calculate angle from a zero vector");
    assert(rhs != Vector2<T>() && "Vector2::angleTo() cannot calculate angle to a zero vector");

    return radians(static_cast<float>(std::atan2(cross(rhs), dot(rhs))));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Angle Vector2<T>::angle() const
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::angle() is only supported for floating point types");

    assert(*this != Vector2<T>() && "Vector2::angle() cannot calculate angle from a zero vector");

    return radians(static_cast<float>(std::atan2(y, x)));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::rotatedBy(Angle phi) const
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::rotatedBy() is only supported for floating point types");

    // No zero vector assert, because rotating a zero vector is well-defined (yields always itself)
    const T cos = std::cos(static_cast<T>(phi.asRadians()));
    const T sin = std::sin(static_cast<T>(phi.asRadians()));

    // Don't manipulate x and y separately, otherwise they're overwritten too early
    return Vector2<T>(cos * x - sin * y, sin * x + cos * y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::movedAndRotatedBy(T r, Angle phi) const
{
    static_assert(priv::isVec2FloatingPoint<T>,
                  "Vector2::movedAndRotatedBy() is only supported for floating point types");

    return *this + Vector2<T>(r, phi);
}

////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::projectedOnto(const Vector2<T>& axis) const
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::projectedOnto() is only supported for floating point types");

    assert(axis != Vector2<T>() && "Vector2::projectedOnto() cannot project onto a zero vector");
    return dot(axis) / axis.lengthSq() * axis;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>::Vector2(T r, Angle phi) :
x(r * static_cast<T>(std::cos(phi.asRadians()))),
y(r * static_cast<T>(std::sin(phi.asRadians())))
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::Vector2(T, Angle) is only supported for floating point types");
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::length() const
{
    static_assert(priv::isVec2FloatingPoint<T>, "Vector2::length() is only supported for floating point types");

    // don't use std::hypot because of slow performance
    return std::sqrt(x * x + y * y);
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

template <typename T>
inline constexpr Vector2<T> Vector2<T>::UnitX(static_cast<T>(1), static_cast<T>(0));

template <typename T>
inline constexpr Vector2<T> Vector2<T>::UnitY(static_cast<T>(0), static_cast<T>(1));

template <typename T>
inline constexpr Vector2<T> Vector2<T>::Zero(static_cast<T>(0), static_cast<T>(0));

} // namespace sf
