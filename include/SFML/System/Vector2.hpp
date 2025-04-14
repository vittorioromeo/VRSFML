#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Math/Atan2.hpp"
#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"
#include "SFML/Base/Math/Sqrt.hpp"
#include "SFML/Base/Traits/IsFloatingPoint.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Angle;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Class template for manipulating
///        2-dimensional vectors
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Vector2
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the vector from polar coordinates <i><b>(floating-point)</b></i>
    ///
    /// \param r   Length of vector (can be negative)
    /// \param phi Angle from X axis
    ///
    /// Note that this constructor is lossy: calling `length()` and `angle()`
    /// may return values different to those provided in this constructor.
    ///
    /// In particular, these transforms can be applied:
    /// * `Vector2(r, phi) == Vector2(-r, phi + 180_deg)`
    /// * `Vector2(r, phi) == Vector2(r, phi + n * 360_deg)`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] static inline constexpr Vector2 fromAngle(const T r, const Angle phi)
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vector2::Vector2(T, Angle) is only supported for floating point types");

        return {r * static_cast<T>(base::cos(phi.asRadians())), r * static_cast<T>(base::sin(phi.asRadians()))};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Length of the vector <i><b>(floating-point)</b></i>.
    ///
    /// If you are not interested in the actual length, but only in comparisons, consider using `lengthSquared()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T length() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::length() is only supported for floating point types");

        // don't use std::hypot because of slow performance
        return base::sqrt(x * x + y * y);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Square of vector's length.
    ///
    /// Suitable for comparisons, more efficient than `length()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T lengthSquared() const
    {
        return dot(*this);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Vector with same direction but length 1 <i><b>(floating-point)</b></i>.
    ///
    /// \pre `*this` is no zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 normalized() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vector2::normalized() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::normalized() cannot normalize a zero vector");

        return (*this) / length();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Signed angle from `*this` to `rhs` <i><b>(floating-point)</b></i>.
    ///
    /// \return The smallest angle which rotates `*this` in positive
    /// or negative direction, until it has the same direction as `rhs`.
    /// The result has a sign and lies in the range [-180, 180) degrees.
    /// \pre Neither `*this` nor `rhs` is a zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle angleTo(const Vector2 rhs) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::angleTo() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::angleTo() cannot calculate angle from a zero vector");
        SFML_BASE_ASSERT(rhs != Vector2<T>() && "Vector2::angleTo() cannot calculate angle to a zero vector");

        return radians(static_cast<float>(base::atan2(cross(rhs), dot(rhs))));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Signed angle from +X or (1,0) vector <i><b>(floating-point)</b></i>.
    ///
    /// For example, the vector (1,0) corresponds to 0 degrees, (0,1) corresponds to 90 degrees.
    ///
    /// \return Angle in the range [-180, 180) degrees.
    /// \pre This vector is no zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle angle() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::angle() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::angle() cannot calculate angle from a zero vector");

        return radians(static_cast<float>(base::atan2(y, x)));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Rotate by angle \c phi <i><b>(floating-point)</b></i>.
    ///
    /// Returns a vector with same length but different direction.
    ///
    /// In SFML's default coordinate system with +X right and +Y down,
    /// this amounts to a clockwise rotation by `phi`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 rotatedBy(const Angle phi) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vector2::rotatedBy() is only supported for floating point types");

        // No zero vector assert, because rotating a zero vector is well-defined (yields always itself)
        const T cos = base::cos(static_cast<T>(phi.asRadians()));
        const T sin = base::sin(static_cast<T>(phi.asRadians()));

        // Don't manipulate x and y separately, otherwise they're overwritten too early
        return Vector2<T>(cos * x - sin * y, sin * x + cos * y);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move towards angle \c phi <i><b>(floating-point)</b></i> by \c r.
    ///
    /// Returns a vector starting at the position of the original, but moved by \c r
    /// units in the direction of \c phi.
    ///
    /// In SFML's default coordinate system with +X rhs and +Y down,
    /// this amounts to a clockwise rotation by \c phi.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 movedTowards(const T r, const Angle phi) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vector2::movedTowards() is only supported for floating point types");

        return *this + Vector2<T>::fromAngle(r, phi);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Projection of this vector onto `axis` <i><b>(floating-point)</b></i>.
    ///
    /// \param axis Vector being projected onto. Need not be normalized.
    /// \pre `axis` must not have length zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 projectedOnto(const Vector2 axis) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vector2::projectedOnto() is only supported for floating point types");

        SFML_BASE_ASSERT(axis != Vector2<T>() && "Vector2::projectedOnto() cannot project onto a zero vector");
        return dot(axis) / axis.lengthSquared() * axis;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Returns a perpendicular vector.
    ///
    /// Returns `*this` rotated by +90 degrees; (x,y) becomes (-y,x).
    /// For example, the vector (1,0) is transformed to (0,1).
    ///
    /// In SFML's default coordinate system with +X rhs and +Y down,
    /// this amounts to a clockwise rotation.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 perpendicular() const
    {
        return Vector2<T>(-y, x);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dot product of two 2D vectors.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T dot(const Vector2 rhs) const
    {
        return x * rhs.x + y * rhs.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Z component of the cross product of two 2D vectors.
    ///
    /// Treats the operands as 3D vectors, computes their cross product
    /// and returns the result's Z component (X and Y components are always zero).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T cross(const Vector2 rhs) const
    {
        return x * rhs.y - y * rhs.x;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Component-wise multiplication of `*this` and `rhs`.
    ///
    /// Computes `(lhs.x*rhs.x, lhs.y*rhs.y)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    /// This operation is also known as the Hadamard or Schur product.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 componentWiseMul(const Vector2 rhs) const
    {
        return Vector2<T>(x * rhs.x, y * rhs.y);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Component-wise division of `*this` and `rhs`.
    ///
    /// Computes `(lhs.x/rhs.x, lhs.y/rhs.y)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    ///
    /// \pre Neither component of `rhs` is zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 componentWiseDiv(const Vector2 rhs) const
    {
        SFML_BASE_ASSERT(rhs.x != 0 && "Vector2::componentWiseDiv() cannot divide by 0 (x coordinate)");
        SFML_BASE_ASSERT(rhs.y != 0 && "Vector2::componentWiseDiv() cannot divide by 0 (y coordinate)");

        return Vector2<T>(x / rhs.x, y / rhs.y);
    }


#define SFML_PRIV_CLAMP_BY_VALUE(value, minValue, maxValue) \
    (((value) < (minValue)) ? (minValue) : (((value) > (maxValue)) ? (maxValue) : (value)))

    ////////////////////////////////////////////////////////////
    /// \brief Clamps the components of the `*this` to the given range
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 componentWiseClamp(
        const Vector2 mins,
        const Vector2 maxs) const
    {
        return {SFML_PRIV_CLAMP_BY_VALUE(x, mins.x, maxs.x), SFML_PRIV_CLAMP_BY_VALUE(y, mins.y, maxs.y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamps the X component of the `*this` between `[minX, maxX]`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 clampX(const T minX, const T maxX) const
    {
        return {SFML_PRIV_CLAMP_BY_VALUE(x, minX, maxX), y};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamps the Y component of the `*this` between `[minY, maxY]`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 clampY(const T minY, const T maxY) const
    {
        return {x, SFML_PRIV_CLAMP_BY_VALUE(y, minY, maxY)};
    }

#undef SFML_PRIV_CLAMP_BY_VALUE


    ////////////////////////////////////////////////////////////
    /// \brief Returns a new vector with `addedX` added to the X component
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 addX(const T addedX) const
    {
        return {x + addedX, y};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Returns a new vector with `addedY` added to the Y component
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 addY(const T addedY) const
    {
        return {x, y + addedY};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Vector2` of type `OtherVector2`
    ///
    /// `OtherVector2` must be a `Vector2<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename OtherVector2>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr OtherVector2 to() const
    {
        using ValueType = decltype(OtherVector2{}.x);
        return Vector2<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `*this` to a `Vector2<int>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2<int> toVector2i() const
    {
        return {static_cast<int>(x), static_cast<int>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `*this` to a `Vector2<float>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2<float> toVector2f() const
    {
        return {static_cast<float>(x), static_cast<float>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `*this` to a `Vector2<unsigned int>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2<unsigned int> toVector2u() const
    {
        return {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr bool operator==(const Vector2& rhs) const = default;


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T x{}; //!< X coordinate of the vector
    T y{}; //!< Y coordinate of the vector
};

// Define the most common types
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;
using Vector2f = Vector2<float>;

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of unary `operator-`
///
/// \param rhs Vector to negate
///
/// \return Member-wise opposite of the vector
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator-(const Vector2<T> rhs)
{
    return Vector2<T>(-rhs.x, -rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vector2
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
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator+=(Vector2<T>& lhs, const Vector2<T> rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator-=`
///
/// This operator performs a member-wise subtraction of both vectors,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Reference to \c lhs
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator-=(Vector2<T>& lhs, const Vector2<T> rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator+`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise addition of both vectors
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator+(const Vector2<T> lhs,
                                                                                           const Vector2<T> rhs)
{
    return Vector2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator-`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise subtraction of both vectors
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator-(const Vector2<T> lhs,
                                                                                           const Vector2<T> rhs)
{
    return Vector2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise multiplication by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator*(const Vector2<T> lhs, const T rhs)
{
    return Vector2<T>(lhs.x * rhs, lhs.y * rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a scalar value)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise multiplication by `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator*(const T lhs, const Vector2<T> rhs)
{
    return Vector2<T>(rhs.x * lhs, rhs.y * lhs);
}


////////////////////////////////////////////////////////////
/// \relates Vector2
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
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator*=(Vector2<T>& lhs, const T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator/`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise division by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator/(const Vector2<T> lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector2::operator/ cannot divide by 0");

    return Vector2<T>(lhs.x / rhs, lhs.y / rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vector2
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
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator/=(Vector2<T>& lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vector2::operator/= cannot divide by 0");

    lhs.x /= rhs;
    lhs.y /= rhs;

    return lhs;
}


} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class sf::Vector2<float>;
extern template class sf::Vector2<double>;
extern template class sf::Vector2<long double>;
extern template class sf::Vector2<bool>;
extern template class sf::Vector2<int>;
extern template class sf::Vector2<unsigned int>;


////////////////////////////////////////////////////////////
/// \class sf::Vector2
/// \ingroup system
///
/// `sf::Vector2` is a simple class that defines a mathematical
/// vector with two coordinates (x and y). It can be used to
/// represent anything that has two dimensions: a size, a point,
/// a velocity, a scale, etc.
///
/// The API provides basic arithmetic (addition, subtraction, scale), as
/// well as more advanced geometric operations, such as dot/cross products,
/// length and angle computations, projections, rotations, etc.
///
/// The template parameter T is the type of the coordinates. It
/// can be any type that supports arithmetic operations (+, -, /, *)
/// and comparisons (==, !=), for example int or float.
/// Note that some operations are only meaningful for vectors where T is
/// a floating point type (e.g. float or double), often because
/// results cannot be represented accurately with integers.
/// The method documentation mentions "(floating-point)" in those cases.
///
/// You generally don't have to care about the templated form (`sf::Vector2<T>`),
/// the most common specializations have special type aliases:
/// \li `sf::Vector2<float>` is `sf::Vector2f`
/// \li `sf::Vector2<int>` is `sf::Vector2i`
/// \li `sf::Vector2<unsigned int>` is `sf::Vector2u`
///
/// The `sf::Vector2` class has a simple interface, its x and y members
/// can be accessed directly (there are no accessors like setX(), getX()).
///
/// Usage example:
/// \code
/// sf::Vector2f v(16.5f, 24.f);
/// v.x = 18.2f;
/// float y = v.y;
///
/// sf::Vector2f w = v * 5.f;
/// sf::Vector2f u;
/// u = v + w;
///
/// float s = v.dot(w);
///
/// bool different = (v != u);
/// \endcode
///
/// Note: for 3-dimensional vectors, see `sf::Vector3`.
///
////////////////////////////////////////////////////////////
