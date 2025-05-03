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
class [[nodiscard]] Vector3
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Length of the vector <i><b>(floating-point)</b></i>.
    ///
    /// If you are not interested in the actual length, but only in comparisons, consider using `lengthSquared()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_SYSTEM_API constexpr T length() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector3::length() is only supported for floating point types");

        // don't use std::hypot because of slow performance
        return base::sqrt(x * x + y * y + z * z);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Square of vector's length.
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
    /// \pre `*this` is no zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_SYSTEM_API constexpr Vector3 normalized() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vector3::normalized() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vector3<T>() && "Vector3::normalized() cannot normalize a zero vector");
        return (*this) / length();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dot product of two 3D vectors.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr T dot(const Vector3& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Cross product of two 3D vectors.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr Vector3 cross(const Vector3& rhs) const
    {
        return Vector3<T>((y * rhs.z) - (z * rhs.y), (z * rhs.x) - (x * rhs.z), (x * rhs.y) - (y * rhs.x));
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr Vector3 componentWiseMul(
        const Vector3& rhs) const
    {
        return Vector3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] SFML_API_EXPORT constexpr Vector3 componentWiseDiv(
        const Vector3& rhs) const
    {
        SFML_BASE_ASSERT(rhs.x != 0 && "Vector3::componentWiseDiv() cannot divide by 0 (x coordinate)");
        SFML_BASE_ASSERT(rhs.y != 0 && "Vector3::componentWiseDiv() cannot divide by 0 (y coordinate)");
        SFML_BASE_ASSERT(rhs.z != 0 && "Vector3::componentWiseDiv() cannot divide by 0 (z coordinate)");

        return Vector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Vector3` of type `OtherVector3`
    ///
    /// `OtherVector3` must be a `Vector3<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename OtherVector3>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr OtherVector3 to() const
    {
        using ValueType = decltype(OtherVector3{}.x);
        return Vector3<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y), static_cast<ValueType>(z)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Overload of binary `operator==`
    ///
    /// This operator compares strict equality between two vectors.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if `lhs` is equal to `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr bool operator==(const Vector3<T>& rhs) const = default;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T x{}; //!< X coordinate of the vector
    T y{}; //!< Y coordinate of the vector
    T z{}; //!< Z coordinate of the vector
};


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of unary `operator-`
///
/// \param lhs Vector to negate
///
/// \return Member-wise opposite of the vector
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector3<T> operator-(const Vector3<T>& lhs)
{
    return Vector3<T>(-lhs.x, -lhs.y, -lhs.z);
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator+=`
///
/// This operator performs a member-wise addition of both vectors,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector3<T>& operator+=(Vector3<T>& lhs, const Vector3<T>& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator-=`
///
/// This operator performs a member-wise subtraction of both vectors,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector3<T>& operator-=(Vector3<T>& lhs, const Vector3<T>& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator+`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise addition of both vectors
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector3<T> operator+(const Vector3<T>& lhs,
                                                                                          const Vector3<T>& rhs)
{
    return Vector3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator-`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise subtraction of both vectors
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector3<T> operator-(const Vector3<T>& lhs,
                                                                                          const Vector3<T>& rhs)
{
    return Vector3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise multiplication by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector3<T> operator*(const Vector3<T>& lhs, const T rhs)
{
    return Vector3<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a scalar value)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise multiplication by `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector3<T> operator*(const T lhs, const Vector3<T>& rhs)
{
    return Vector3<T>(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs);
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator*=`
///
/// This operator performs a member-wise multiplication by `rhs`,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector3<T>& operator*=(Vector3<T>& lhs, const T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    lhs.z *= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator/`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise division by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector3<T> operator/(const Vector3<T>& lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector3::operator/ cannot divide by 0");

    return Vector3<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vector3
/// \brief Overload of binary `operator/=`
///
/// This operator performs a member-wise division by `rhs`,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector3<T>& operator/=(Vector3<T>& lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector3::operator/= cannot divide by 0");

    lhs.x /= rhs;
    lhs.y /= rhs;
    lhs.z /= rhs;

    return lhs;
}

// Aliases for the most common types
using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class sf::Vector3<float>;
extern template class sf::Vector3<double>;
extern template class sf::Vector3<long double>;
extern template class sf::Vector3<bool>;
extern template class sf::Vector3<int>;
extern template class sf::Vector3<unsigned int>;


////////////////////////////////////////////////////////////
/// \class sf::Vector3
/// \ingroup system
///
/// `sf::Vector3` is a simple class that defines a mathematical
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
/// You generally don't have to care about the templated form (`sf::Vector3<T>`),
/// the most common specializations have special type aliases:
/// \li `sf::Vector3<float>` is `sf::Vector3f`
/// \li `sf::Vector3<int>` is `sf::Vector3i`
///
/// The `sf::Vector3` class has a small and simple interface, its x, y and z members
/// can be accessed directly (there are no accessors like `setX()`, `getX()`).
///
/// Usage example:
/// \code
/// sf::Vector3f v(16.5f, 24.f, -3.2f);
/// v.x = 18.2f;
/// float y = v.y;
///
/// sf::Vector3f w = v * 5.f;
/// sf::Vector3f u;
/// u = v + w;
///
/// float s = v.dot(w);
/// sf::Vector3f t = v.cross(w);
///
/// bool different = (v != u);
/// \endcode
///
/// Note: for 2-dimensional vectors, see `sf::Vector2`.
///
////////////////////////////////////////////////////////////
