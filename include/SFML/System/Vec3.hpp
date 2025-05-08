#pragma once
#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/Traits/IsFloatingPoint.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility template class for manipulating
///        3-dimensional vectors
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Vec3
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Length of the vec3 <i><b>(floating-point)</b></i>.
    ///
    /// If you are not interested in the actual length, but only in comparisons, consider using `lengthSquared()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_SYSTEM_API constexpr T length() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec3::length() is only supported for floating point types");

        // don't use std::hypot because of slow performance
        return base::sqrt(x * x + y * y + z * z);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Square of vec3's length.
    ///
    /// Suitable for comparisons, more efficient than `length()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr T lengthSquared() const
    {
        return dot(*this);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Vector with same direction but length 1 <i><b>(floating-point)</b></i>.
    ///
    /// \pre `*this` is no zero vec3.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_SYSTEM_API constexpr Vec3 normalized() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec3::normalized() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vec3<T>() && "Vec3::normalized() cannot normalize a zero vec3");
        return (*this) / length();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dot product of two vec3s.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr T dot(const Vec3& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Cross product of two vec3s.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr Vec3 cross(const Vec3& rhs) const
    {
        return Vec3<T>((y * rhs.z) - (z * rhs.y), (z * rhs.x) - (x * rhs.z), (x * rhs.y) - (y * rhs.x));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Component-wise multiplication of `*this` and `rhs`.
    ///
    /// Computes `(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    /// This operation is also known as the Hadamard or Schur product.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr Vec3 componentWiseMul(const Vec3& rhs) const
    {
        return Vec3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Component-wise division of `*this` and `rhs`.
    ///
    /// Computes `(lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    ///
    /// \pre Neither component of `rhs` is zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr Vec3 componentWiseDiv(const Vec3& rhs) const
    {
        SFML_BASE_ASSERT(rhs.x != 0 && "Vec3::componentWiseDiv() cannot divide by 0 (x coordinate)");
        SFML_BASE_ASSERT(rhs.y != 0 && "Vec3::componentWiseDiv() cannot divide by 0 (y coordinate)");
        SFML_BASE_ASSERT(rhs.z != 0 && "Vec3::componentWiseDiv() cannot divide by 0 (z coordinate)");

        return Vec3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Vec3` of type `OtherVec3`
    ///
    /// `OtherVec3` must be a `Vec3<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename OtherVec3>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr OtherVec3 to() const
    {
        using ValueType = decltype(OtherVec3{}.x);
        return Vec3<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y), static_cast<ValueType>(z)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Overload of binary `operator==`
    ///
    /// This operator compares strict equality between two vec3s.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if `lhs` is equal to `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr bool operator==(const Vec3<T>& rhs) const = default;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T x{}; //!< X coordinate of the vec3
    T y{}; //!< Y coordinate of the vec3
    T z{}; //!< Z coordinate of the vec3
};


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of unary `operator-`
///
/// \param lhs Vector to negate
///
/// \return Member-wise opposite of the vec3
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec3<T> operator-(const Vec3<T>& lhs)
{
    return Vec3<T>(-lhs.x, -lhs.y, -lhs.z);
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator+=`
///
/// This operator performs a member-wise addition of both vec3s,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a vec3)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec3<T>& operator+=(Vec3<T>& lhs, const Vec3<T>& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator-=`
///
/// This operator performs a member-wise subtraction of both vec3s,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a vec3)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec3<T>& operator-=(Vec3<T>& lhs, const Vec3<T>& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator+`
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a vec3)
///
/// \return Member-wise addition of both vec3s
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec3<T> operator+(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
    return Vec3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator-`
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a vec3)
///
/// \return Member-wise subtraction of both vec3s
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec3<T> operator-(const Vec3<T>& lhs, const Vec3<T>& rhs)
{
    return Vec3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise multiplication by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec3<T> operator*(const Vec3<T>& lhs, const T rhs)
{
    return Vec3<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a scalar value)
/// \param rhs Right operand (a vec3)
///
/// \return Member-wise multiplication by `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec3<T> operator*(const T lhs, const Vec3<T>& rhs)
{
    return Vec3<T>(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs);
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator*=`
///
/// This operator performs a member-wise multiplication by `rhs`,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec3<T>& operator*=(Vec3<T>& lhs, const T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator/`
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise division by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec3<T> operator/(const Vec3<T>& lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vec3::operator/ cannot divide by 0");

    return Vec3<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vec3
/// \brief Overload of binary `operator/=`
///
/// This operator performs a member-wise division by `rhs`,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec3)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec3<T>& operator/=(Vec3<T>& lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vec3::operator/= cannot divide by 0");

    lhs.x /= rhs;
    lhs.y /= rhs;
    lhs.z /= rhs;

    return lhs;
}

// Aliases for the most common types
using Vec3i = Vec3<int>;
using Vec3f = Vec3<float>;

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class sf::Vec3<float>;
extern template class sf::Vec3<double>;
extern template class sf::Vec3<long double>;
extern template class sf::Vec3<bool>;
extern template class sf::Vec3<int>;
extern template class sf::Vec3<unsigned int>;


////////////////////////////////////////////////////////////
/// \class sf::Vec3
/// \ingroup system
///
/// `sf::Vec3` is a simple class that defines a mathematical
/// vector with three coordinates (x, y and z). It can be used to
/// represent anything that has three dimensions: a size, a point,
/// a velocity, etc.
///
/// The template parameter T is the type of the coordinates. It
/// can be any type that supports arithmetic operations (+, -, /, *)
/// and comparisons (==, !=), for example int or float.
/// Note that some operations are only meaningful for vectors where T is
/// a floating point type (e.g. float or double), often because
/// results cannot be represented accurately with integers.
/// The method documentation mentions "(floating-point)" in those cases.
///
/// You generally don't have to care about the templated form (`sf::Vec3<T>`),
/// the most common specializations have special type aliases:
/// \li `sf::Vec3<float>` is `sf::Vec3f`
/// \li `sf::Vec3<int>` is `sf::Vec3i`
///
/// The `sf::Vec3` class has a small and simple interface, its x, y and z members
/// can be accessed directly (there are no accessors like `setX()`, `getX()`).
///
/// Usage example:
/// \code
/// sf::Vec3f v(16.5f, 24.f, -3.2f);
/// v.x = 18.2f;
/// float y = v.y;
///
/// sf::Vec3f w = v * 5.f;
/// sf::Vec3f u;
/// u = v + w;
///
/// float s = v.dot(w);
/// sf::Vec3f t = v.cross(w);
///
/// bool different = (v != u);
/// \endcode
///
/// Note: for 2-dimensional vectors, see `sf::Vec2`.
///
////////////////////////////////////////////////////////////
