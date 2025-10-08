#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Angle.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/ClampMacro.hpp"
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
struct Angle;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Class template for manipulating
///        2-dimensional vectors
///
////////////////////////////////////////////////////////////
template <typename T>
class [[nodiscard]] Vec2
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the vec2 from polar coordinates <i><b>(floating-point)</b></i>
    ///
    /// \param r   Length of vec2 (can be negative)
    /// \param phi Angle from X axis
    ///
    /// Note that this constructor is lossy: calling `length()` and `angle()`
    /// may return values different to those provided in this constructor.
    ///
    /// In particular, these transforms can be applied:
    /// * `Vec2(r, phi) == Vec2(-r, phi + 180_deg)`
    /// * `Vec2(r, phi) == Vec2(r, phi + n * 360_deg)`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] static inline constexpr Vec2 fromAngle(const T r, const Angle phi)
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::Vec2(T, Angle) is only supported for floating point types");

        return {r * static_cast<T>(base::cos(phi.asRadians())), r * static_cast<T>(base::sin(phi.asRadians()))};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Length of the vec2 <i><b>(floating-point)</b></i>.
    ///
    /// If you are not interested in the actual length, but only in comparisons, consider using `lengthSquared()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T length() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::length() is only supported for floating point types");

        // don't use `std::hypot` because of slow performance
        return base::sqrt(x * x + y * y);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Square of vec2's length.
    ///
    /// Suitable for comparisons, more efficient than `length()`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T lengthSquared() const
    {
        return dot(*this);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Vec2 with same direction but length 1 <i><b>(floating-point)</b></i>.
    ///
    /// \pre `*this` is no zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 normalized() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::normalized() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vec2<T>() && "Vec2::normalized() cannot normalize a zero vec2");

        return (*this) / length();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Signed angle from `*this` to `rhs` <i><b>(floating-point)</b></i>.
    ///
    /// \return The smallest angle which rotates `*this` in positive
    /// or negative direction, until it has the same direction as `rhs`.
    /// The result has a sign and lies in the range [-180, 180) degrees.
    /// \pre Neither `*this` nor `rhs` is a zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle angleTo(const Vec2 rhs) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::angleTo() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vec2<T>() && "Vec2::angleTo() cannot calculate angle from a zero vec2");
        SFML_BASE_ASSERT(rhs != Vec2<T>() && "Vec2::angleTo() cannot calculate angle to a zero vec2");

        return radians(static_cast<float>(base::atan2(cross(rhs), dot(rhs))));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Signed angle from +X or (1,0) vec2 <i><b>(floating-point)</b></i>.
    ///
    /// For example, the vec2 (1,0) corresponds to 0 degrees, (0,1) corresponds to 90 degrees.
    ///
    /// \return Angle in the range [-180, 180) degrees.
    /// \pre This vec2 is no zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Angle angle() const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::angle() is only supported for floating point types");

        SFML_BASE_ASSERT(*this != Vec2<T>() && "Vec2::angle() cannot calculate angle from a zero vec2");

        return radians(static_cast<float>(base::atan2(y, x)));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Rotate by angle \c phi <i><b>(floating-point)</b></i>.
    ///
    /// Returns a vec2 with same length but different direction.
    ///
    /// In SFML's default coordinate system with +X right and +Y down,
    /// this amounts to a clockwise rotation by `phi`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 rotatedBy(const Angle phi) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::rotatedBy() is only supported for floating point types");

        // No zero vec2 assert, because rotating a zero vec2 is well-defined (yields always itself)
        const T cos = base::cos(static_cast<T>(phi.asRadians()));
        const T sin = base::sin(static_cast<T>(phi.asRadians()));

        // Don't manipulate x and y separately, otherwise they're overwritten too early
        return Vec2<T>(cos * x - sin * y, sin * x + cos * y);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move towards angle \c phi <i><b>(floating-point)</b></i> by \c r.
    ///
    /// Returns a vec2 starting at the position of the original, but moved by \c r
    /// units in the direction of \c phi.
    ///
    /// In SFML's default coordinate system with +X rhs and +Y down,
    /// this amounts to a clockwise rotation by \c phi.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 movedTowards(const T r, const Angle phi) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::movedTowards() is only supported for floating point types");

        return *this + Vec2<T>::fromAngle(r, phi);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Projection of this vec2 onto `axis` <i><b>(floating-point)</b></i>.
    ///
    /// \param axis Vec2 being projected onto. Need not be normalized.
    /// \pre `axis` must not have length zero.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 projectedOnto(const Vec2 axis) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vec2::projectedOnto() is only supported for floating point types");

        SFML_BASE_ASSERT(axis != Vec2<T>() && "Vec2::projectedOnto() cannot project onto a zero vec2");
        return dot(axis) / axis.lengthSquared() * axis;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamp the vec2's length to a maximum value <i><b>(floating-point)</b></i>.
    ///
    /// If the vec2's length exceeds `maxLength`, returns a vec2 with the same direction
    /// but with length equal to `maxLength`. Otherwise, returns a copy of the original vec2.
    ///
    /// \param maxLength The maximum length allowed. Must be non-negative.
    ///
    /// \return Vec2 with length clamped to `maxLength`.
    ///
    /// \pre `maxLength >= 0`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampMaxLength(const T maxLength) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vec2::clampMaxLength() is only supported for floating point types");
        SFML_BASE_ASSERT(maxLength >= T{0} && "Vec2::clampMaxLength() requires non-negative maxLength");

        const T currentLengthSquared = lengthSquared();

        if (currentLengthSquared <= maxLength * maxLength)
            return *this;

        return *this * (maxLength / base::sqrt(currentLengthSquared));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamp the vec2's squared length to a maximum value <i><b>(floating-point)</b></i>.
    ///
    /// If the vec2's squared length exceeds `maxLengthSquared`, returns a vec2 with the same direction
    /// but with squared length equal to `maxLengthSquared`. Otherwise, returns a copy of the original vec2.
    /// This version is more efficient than `clampMaxLength` if you already have squared lengths.
    ///
    /// \param maxLengthSquared The maximum squared length allowed. Must be non-negative.
    ///
    /// \return Vec2 with squared length clamped to `maxLengthSquared`.
    ///
    /// \pre `maxLengthSquared >= 0`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampMaxLengthSquared(
        const T maxLengthSquared) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vec2::clampMaxLengthSquared() is only supported for floating point types");
        SFML_BASE_ASSERT(maxLengthSquared >= T{0} &&
                         "Vec2::clampMaxLengthSquared() requires non-negative maxLengthSquared");

        const T currentLengthSquared = lengthSquared();

        if (currentLengthSquared <= maxLengthSquared)
            return *this;

        return *this * base::sqrt(maxLengthSquared / currentLengthSquared);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamp the vec2's length to a minimum value <i><b>(floating-point)</b></i>.
    ///
    /// If the vec2's length is less than `minLength`, returns a vec2 with the same direction
    /// but with length equal to `minLength`. Otherwise, returns a copy of the original vec2.
    ///
    /// \param minLength The minimum length allowed. Must be non-negative.
    ///
    /// \return Vec2 with length clamped to `minLength`.
    ///
    /// \pre `minLength >= 0`.
    /// \pre If `minLength > 0`, `*this` must not be the zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampMinLength(const T minLength) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vec2::clampMinLength() is only supported for floating point types");
        SFML_BASE_ASSERT(minLength >= T{0} && "Vec2::clampMinLength() requires non-negative minLength");

        const T currentLengthSquared = lengthSquared();

        SFML_BASE_ASSERT((currentLengthSquared != T{0} || minLength == T{0}) &&
                         "Vec2::clampMinLength() cannot clamp zero vec2 to a "
                         "positive minimum length");

        if (currentLengthSquared >= minLength * minLength)
            return *this;

        return *this * (minLength / base::sqrt(currentLengthSquared));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamp the vec2's squared length to a minimum value <i><b>(floating-point)</b></i>.
    ///
    /// If the vec2's squared length is less than `minLengthSquared`, returns a vec2 with the same direction
    /// but with squared length equal to `minLengthSquared`. Otherwise, returns a copy of the original vec2.
    /// This version is more efficient than `clampMinLength` if you already have squared lengths.
    ///
    /// \param minLengthSquared The minimum squared length allowed. Must be non-negative.
    ///
    /// \return Vec2 with squared length clamped to `minLengthSquared`.
    ///
    /// \pre `minLengthSquared >= 0`.
    /// \pre If `minLengthSquared > 0`, `*this` must not be the zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampMinLengthSquared(
        const T minLengthSquared) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vec2::clampMinLengthSquared() is only supported for floating point types");
        SFML_BASE_ASSERT(minLengthSquared >= T{0} &&
                         "Vec2::clampMinLengthSquared() requires non-negative minLengthSquared");

        const T currentLengthSquared = lengthSquared();

        SFML_BASE_ASSERT((currentLengthSquared != T{0} || minLengthSquared == T{0}) &&
                         "Vec2::clampMinLengthSquared() cannot clamp zero "
                         "vec2 to a positive minimum squared length");

        if (currentLengthSquared >= minLengthSquared)
            return *this;

        return *this * base::sqrt(minLengthSquared / currentLengthSquared);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamp the vec2's length to be within a specific range <i><b>(floating-point)</b></i>.
    ///
    /// If the vec2's length is less than `minLength`, returns a vec2 with the same direction
    /// but with length equal to `minLength`. If the vec2's length is greater than `maxLength`,
    /// returns a vec2 with the same direction but with length equal to `maxLength`.
    /// Otherwise, returns a copy of the original vec2.
    ///
    /// \param minLength The minimum length allowed. Must be non-negative.
    /// \param maxLength The maximum length allowed. Must be >= `minLength`.
    ///
    /// \return Vec2 with length clamped between `minLength` and `maxLength`.
    ///
    /// \pre `0 <= minLength <= maxLength`.
    /// \pre If `minLength > 0`, `*this` must not be the zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampLength(const T minLength,
                                                                                                 const T maxLength) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T), "Vec2::clampLength() is only supported for floating point types");
        SFML_BASE_ASSERT(minLength >= T{0} && minLength <= maxLength &&
                         "Vec2::clampLength() requires 0 <= minLength <= maxLength");

        const T currentLengthSquared = lengthSquared();

        SFML_BASE_ASSERT((currentLengthSquared != T{0} || minLength == T{0}) &&
                         "Vec2::clampLength() cannot clamp zero vec2 to a "
                         "positive minimum length");

        const T minLengthSquared = minLength * minLength;
        const T maxLengthSquared = maxLength * maxLength;

        if (currentLengthSquared >= minLengthSquared && currentLengthSquared <= maxLengthSquared)
            return *this;

        // If length is zero we cannot scale, but previous check/assert handles this unless minLength is 0.
        // If minLength is 0 and currentLength is 0, we fall through. Clamping 0 to [0, max] should result in 0.
        if (currentLengthSquared == T{0})
            return *this;

        const T currentLength = base::sqrt(currentLengthSquared);

        if (currentLength < minLength) // We know currentLength > 0 here
            return *this * (minLength / currentLength);

        // Must be currentLength > maxLength
        return *this * (maxLength / currentLength);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamp the vec2's squared length to be within a specific range <i><b>(floating-point)</b></i>.
    ///
    /// If the vec2's squared length is less than `minLengthSquared`, returns a vec2 with the same direction
    /// but with squared length equal to `minLengthSquared`. If the vec2's squared length is greater than
    /// `maxLengthSquared`, returns a vec2 with the same direction but with squared length equal to
    /// `maxLengthSquared`. Otherwise, returns a copy of the original vec2.
    /// This version is more efficient than `clampLength` if you already have squared lengths.
    ///
    /// \param minLengthSquared The minimum squared length allowed. Must be non-negative.
    /// \param maxLengthSquared The maximum squared length allowed. Must be >= `minLengthSquared`.
    ///
    /// \return Vec2 with squared length clamped between `minLengthSquared` and `maxLengthSquared`.
    ///
    /// \pre `0 <= minLengthSquared <= maxLengthSquared`.
    /// \pre If `minLengthSquared > 0`, `*this` must not be the zero vec2.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampLengthSquared(
        const T minLengthSquared,
        const T maxLengthSquared) const
    {
        static_assert(SFML_BASE_IS_FLOATING_POINT(T),
                      "Vec2::clampLengthSquared() is only supported for floating point types");
        SFML_BASE_ASSERT(minLengthSquared >= T{0} && minLengthSquared <= maxLengthSquared &&
                         "Vec2::clampLengthSquared() requires 0 <= minLengthSquared <= maxLengthSquared");

        const T currentLengthSquared = lengthSquared();

        SFML_BASE_ASSERT((currentLengthSquared != T{0} || minLengthSquared == T{0}) &&
                         "Vec2::clampLengthSquared() cannot clamp zero vec2 "
                         "to a positive minimum squared length");

        if (currentLengthSquared >= minLengthSquared && currentLengthSquared <= maxLengthSquared)
            return *this;

        // If length is zero we cannot scale, but previous check/assert handles this unless minLengthSquared is 0.
        // If minLengthSquared is 0 and currentLengthSquared is 0, we fall through. Clamping 0 to [0, max^2] results in 0.
        if (currentLengthSquared == T{0})
            return *this;

        if (currentLengthSquared < minLengthSquared) // We know currentLengthSquared > 0 here
            return *this * base::sqrt(minLengthSquared / currentLengthSquared);

        // Must be currentLengthSquared > maxLengthSquared
        return *this * base::sqrt(maxLengthSquared / currentLengthSquared);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Returns a perpendicular vec2.
    ///
    /// Returns `*this` rotated by +90 degrees; (x,y) becomes (-y,x).
    /// For example, the vec2 (1,0) is transformed to (0,1).
    ///
    /// In SFML's default coordinate system with +X rhs and +Y down,
    /// this amounts to a clockwise rotation.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 perpendicular() const
    {
        return Vec2<T>(-y, x);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Dot product of two vec2s.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T dot(const Vec2 rhs) const
    {
        return x * rhs.x + y * rhs.y;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Z component of the cross product of two vec2s.
    ///
    /// Treats the operands as 3D vectors, computes their cross product
    /// and returns the result's Z component (X and Y components are always zero).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr T cross(const Vec2 rhs) const
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 componentWiseMul(const Vec2 rhs) const
    {
        return Vec2<T>(x * rhs.x, y * rhs.y);
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 componentWiseDiv(const Vec2 rhs) const
    {
        SFML_BASE_ASSERT(rhs.x != 0 && "Vec2::componentWiseDiv() cannot divide by 0 (x coordinate)");
        SFML_BASE_ASSERT(rhs.y != 0 && "Vec2::componentWiseDiv() cannot divide by 0 (y coordinate)");

        return Vec2<T>(x / rhs.x, y / rhs.y);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamps the components of the `*this` to the given range
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 componentWiseClamp(const Vec2 mins,
                                                                                                        const Vec2 maxs) const
    {
        return {SFML_BASE_CLAMP(x, mins.x, maxs.x), SFML_BASE_CLAMP(y, mins.y, maxs.y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamps the X component of the `*this` between `[minX, maxX]`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampX(const T minX, const T maxX) const
    {
        return {SFML_BASE_CLAMP(x, minX, maxX), y};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Clamps the Y component of the `*this` between `[minY, maxY]`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 clampY(const T minY, const T maxY) const
    {
        return {x, SFML_BASE_CLAMP(y, minY, maxY)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Returns a new vec2 with `addedX` added to the X component
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 addX(const T addedX) const
    {
        return {x + addedX, y};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Returns a new vec2 with `addedY` added to the Y component
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 addY(const T addedY) const
    {
        return {x, y + addedY};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Returns a new vec2 where components have the absolute values of `*this
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2 abs() const
    {
        return {x < T{0} ? -x : x, y < T{0} ? -y : y};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert to another `Vec2` of type `OtherVec2`
    ///
    /// `OtherVec2` must be a `Vec2<...>` type.
    ///
    ////////////////////////////////////////////////////////////
    template <typename OtherVec2>
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr OtherVec2 to() const
    {
        using ValueType = decltype(OtherVec2{}.x);
        return Vec2<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `*this` to a `Vec2<int>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2<int> toVec2i() const
    {
        return {static_cast<int>(x), static_cast<int>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `*this` to a `Vec2<float>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2<float> toVec2f() const
    {
        return {static_cast<float>(x), static_cast<float>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Convert `*this` to a `Vec2<unsigned int>`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vec2<unsigned int> toVec2u() const
    {
        return {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Overload of binary `operator==`
    ///
    /// This operator compares strict equality between two vec2s.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if `lhs` is equal to `rhs`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr bool operator==(const Vec2& rhs) const = default;


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T x{}; //!< X coordinate of the vec2
    T y{}; //!< Y coordinate of the vec2
};

// Define the most common types
using Vec2i = Vec2<int>;
using Vec2u = Vec2<unsigned int>;
using Vec2f = Vec2<float>;

////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of unary `operator-`
///
/// \param rhs Vec2 to negate
///
/// \return Member-wise opposite of the vec2
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator-(const Vec2<T> rhs)
{
    return Vec2<T>(-rhs.x, -rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator+=`
///
/// This operator performs a member-wise addition of both vec2s,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a vec2)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec2<T>& operator+=(Vec2<T>& lhs, const Vec2<T> rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator-=`
///
/// This operator performs a member-wise subtraction of both vec2s,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a vec2)
///
/// \return Reference to \c lhs
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec2<T>& operator-=(Vec2<T>& lhs, const Vec2<T> rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator+`
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a vec2)
///
/// \return Member-wise addition of both vec2s
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator+(const Vec2<T> lhs, const Vec2<T> rhs)
{
    return Vec2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator-`
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a vec2)
///
/// \return Member-wise subtraction of both vec2s
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator-(const Vec2<T> lhs, const Vec2<T> rhs)
{
    return Vec2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise multiplication by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator*(const Vec2<T> lhs, const T rhs)
{
    return Vec2<T>(lhs.x * rhs, lhs.y * rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator*`
///
/// \param lhs  Left operand (a scalar value)
/// \param rhs Right operand (a vec2)
///
/// \return Member-wise multiplication by `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator*(const T lhs, const Vec2<T> rhs)
{
    return Vec2<T>(lhs * rhs.x, lhs * rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator/`
///
/// \param lhs  Left operand (a scalar value)
/// \param rhs Right operand (a vec2)
///
/// \return Member-wise division by `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator/(const T lhs, const Vec2<T> rhs)
{
    return Vec2<T>(lhs / rhs.x, lhs / rhs.y);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator*=`
///
/// This operator performs a member-wise multiplication by `rhs`,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec2<T>& operator*=(Vec2<T>& lhs, const T rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;

    return lhs;
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator/`
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a scalar value)
///
/// \return Member-wise division by `rhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr Vec2<T> operator/(const Vec2<T> lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vec2::operator/ cannot divide by 0");

    return Vec2<T>(lhs.x / rhs, lhs.y / rhs);
}


////////////////////////////////////////////////////////////
/// \relates Vec2
/// \brief Overload of binary `operator/=`
///
/// This operator performs a member-wise division by `rhs`,
/// and assigns the result to `lhs`.
///
/// \param lhs  Left operand (a vec2)
/// \param rhs Right operand (a scalar value)
///
/// \return Reference to `lhs`
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] constexpr Vec2<T>& operator/=(Vec2<T>& lhs, const T rhs)
{
    SFML_BASE_ASSERT(rhs != 0 && "Vec2::operator/= cannot divide by 0");

    lhs.x /= rhs;
    lhs.y /= rhs;

    return lhs;
}

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class sf::Vec2<float>;
extern template class sf::Vec2<double>;
extern template class sf::Vec2<long double>;
extern template class sf::Vec2<bool>;
extern template class sf::Vec2<int>;
extern template class sf::Vec2<unsigned int>;


////////////////////////////////////////////////////////////
/// \class sf::Vec2
/// \ingroup system
///
/// `sf::Vec2` is a simple class that defines a mathematical
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
/// Note that some operations are only meaningful for vec2s where T is
/// a floating point type (e.g. float or double), often because
/// results cannot be represented accurately with integers.
/// The method documentation mentions "(floating-point)" in those cases.
///
/// You generally don't have to care about the templated form (`sf::Vec2<T>`),
/// the most common specializations have special type aliases:
/// \li `sf::Vec2<float>` is `sf::Vec2f`
/// \li `sf::Vec2<int>` is `sf::Vec2i`
/// \li `sf::Vec2<unsigned int>` is `sf::Vec2u`
///
/// The `sf::Vec2` class has a simple interface, its x and y members
/// can be accessed directly (there are no accessors like setX(), getX()).
///
/// Usage example:
/// \code
/// sf::Vec2f v(16.5f, 24.f);
/// v.x = 18.2f;
/// float y = v.y;
///
/// sf::Vec2f w = v * 5.f;
/// sf::Vec2f u;
/// u = v + w;
///
/// float s = v.dot(w);
///
/// bool different = (v != u);
/// \endcode
///
/// Note: for 3-dimensional vectors, see `sf::Vec3`.
///
////////////////////////////////////////////////////////////
