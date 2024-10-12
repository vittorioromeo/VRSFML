#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] static inline constexpr Vector2 fromAngle(T r, Angle phi);

    ////////////////////////////////////////////////////////////
    /// \brief Length of the vector <i><b>(floating-point)</b></i>.
    ///
    /// If you are not interested in the actual length, but only in comparisons, consider using `lengthSquared()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T length() const;

    ////////////////////////////////////////////////////////////
    /// \brief Square of vector's length.
    ///
    /// Suitable for comparisons, more efficient than `length()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T lengthSquared() const;

    ////////////////////////////////////////////////////////////
    /// \brief Vector with same direction but length 1 <i><b>(floating-point)</b></i>.
    ///
    /// \pre `*this` is no zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 normalized() const;

    ////////////////////////////////////////////////////////////
    /// \brief Signed angle from `*this` to \a `rhs` <i><b>(floating-point)</b></i>.
    ///
    /// \return The smallest angle which rotates `*this` in positive
    /// or negative direction, until it has the same direction as \a `rhs`.
    /// The result has a sign and lies in the range [-180, 180) degrees.
    /// \pre Neither `*this` nor \a `rhs` is a zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle angleTo(Vector2 rhs) const;

    ////////////////////////////////////////////////////////////
    /// \brief Signed angle from +X or (1,0) vector <i><b>(floating-point)</b></i>.
    ///
    /// For example, the vector (1,0) corresponds to 0 degrees, (0,1) corresponds to 90 degrees.
    ///
    /// \return Angle in the range [-180, 180) degrees.
    /// \pre This vector is no zero vector.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle angle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Rotate by angle \c phi <i><b>(floating-point)</b></i>.
    ///
    /// Returns a vector with same length but different direction.
    ///
    /// In SFML's default coordinate system with +X rhs and +Y down,
    /// this amounts to a clockwise rotation by \a `phi`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 rotatedBy(Angle phi) const;

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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 movedTowards(T r, Angle phi) const;

    ////////////////////////////////////////////////////////////
    /// \brief Projection of this vector onto \a `axis` <i><b>(floating-point)</b></i>.
    ///
    /// \param axis Vector being projected onto. Need not be normalized.
    /// \pre \a `axis` must not have length zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 projectedOnto(Vector2 axis) const;

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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 perpendicular() const;

    ////////////////////////////////////////////////////////////
    /// \brief Dot product of two 2D vectors.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T dot(Vector2 rhs) const;

    ////////////////////////////////////////////////////////////
    /// \brief Z component of the cross product of two 2D vectors.
    ///
    /// Treats the operands as 3D vectors, computes their cross product
    /// and returns the result's Z component (X and Y components are always zero).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T cross(Vector2 rhs) const;

    ////////////////////////////////////////////////////////////
    /// \brief Component-wise multiplication of `*this` and \a `rhs`.
    ///
    /// Computes `(lhs.x*rhs.x, lhs.y*rhs.y)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    /// This operation is also known as the Hadamard or Schur product.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 componentWiseMul(Vector2 rhs) const;

    ////////////////////////////////////////////////////////////
    /// \brief Component-wise division of `*this` and \a `rhs`.
    ///
    /// Computes `(lhs.x/rhs.x, lhs.y/rhs.y)`.
    ///
    /// Scaling is the most common use case for component-wise multiplication/division.
    ///
    /// \pre Neither component of \a `rhs` is zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2 componentWiseDiv(Vector2 rhs) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Vector2` of type `OtherVector2`
    ///
    /// `OtherVector2` must be a `Vector2<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename OtherVector2>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr OtherVector2 to() const;

    ////////////////////////////////////////////////////////////
    /// \brief More convenient conversion functions
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2<int>   toVector2i() const;
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2<float> toVector2f() const;
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2<unsigned int> toVector2u() const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of binary `operator==`
    ///
    /// This operator compares strict equality between two vectors.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if \a `lhs` is equal to \a `rhs`
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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator-(Vector2<T> rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator+=`
///
/// This operator performs a member-wise addition of both vectors,
/// and assigns the result to \a `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Reference to \a `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator+=(Vector2<T>& lhs, Vector2<T> rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator-=`
///
/// This operator performs a member-wise subtraction of both vectors,
/// and assigns the result to \a `lhs.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a vector)
///
/// \return Reference to \c lhs
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator-=(Vector2<T>& lhs, Vector2<T> rhs);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator+(Vector2<T> lhs, Vector2<T> rhs);

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
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator-(Vector2<T> lhs, Vector2<T> rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise multiplication by \a `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator*(Vector2<T> lhs, T rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a scalar value)
/// \param rhs Right operand (a vector)
///
/// \return Member-wise multiplication by \a `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator*(T lhs, Vector2<T> rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator*=`
///
/// This operator performs a member-wise multiplication by \a `rhs`,
/// and assigns the result to \a `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to \a `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator*=(Vector2<T>& lhs, T rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator/`
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise division by \a `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vector2<T> operator/(Vector2<T> lhs, T rhs);

////////////////////////////////////////////////////////////
/// \relates Vector2
/// \brief Overload of binary `operator/=`
///
/// This operator performs a member-wise division by \a `rhs`,
/// and assigns the result to \a `lhs`.
///
/// \param lhs  Left operand (a vector)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to \a `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vector2<T>& operator/=(Vector2<T>& lhs, T rhs);

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

#include "SFML/System/Vector2.inl"


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
