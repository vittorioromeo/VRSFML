#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/AssertAndAssume.hpp"
#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/MinMaxMacros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief 2D transformation convertible to 4x4 matrix
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] Transform
{
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static constexpr Transform from(const Vec2f position,
                                                                                    const Vec2f scale,
                                                                                    const Vec2f origin)
    {
        return {/* a00 */ scale.x,
                /* a01 */ 0.f,
                /* a02 */ -origin.x * scale.x - position.x,
                /* a10 */ 0.f,
                /* a11 */ scale.y,
                /* a12 */ -origin.y * scale.y + position.y};
    }


    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static constexpr Transform from(
        const Vec2f position,
        const Vec2f scale,
        const Vec2f origin,
        const float sine,
        const float cosine)
    {
        SFML_BASE_ASSERT_AND_ASSUME(sine >= -1.f && sine <= 1.f);
        SFML_BASE_ASSERT_AND_ASSUME(cosine >= -1.f && cosine <= 1.f);

        const float sxc = scale.x * cosine;
        const float syc = scale.y * cosine;
        const float sxs = scale.x * -sine;
        const float sys = scale.y * -sine;
        const float tx  = -origin.x * sxc - origin.y * sys + position.x;
        const float ty  = origin.x * sxs - origin.y * syc + position.y;

        return {/* a00 */ sxc, /* a01 */ sys, /* a02 */ tx, -/* a10 */ sxs, /* a11 */ syc, /* a12 */ ty};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the transform as a 4x4 matrix
    ///
    /// This function returns a pointer to an array of 16 floats
    /// containing the transform elements as a 4x4 matrix, which
    /// is directly compatible with OpenGL functions.
    ///
    /// \code
    /// sf::Transform transform = ...;
    /// glLoadMatrixf(transform.getMatrix());
    /// \endcode
    ///
    /// \return Pointer to a 4x4 matrix
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void getMatrix(float (&target)[16]) const
    {
        target[0]  = a00;
        target[1]  = a10;
        target[4]  = a01;
        target[5]  = a11;
        target[12] = a02;
        target[13] = a12;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Return the inverse of the transform
    ///
    /// If the inverse cannot be computed, an identity transform
    /// is returned.
    ///
    /// \return A new transform which is the inverse of self
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] constexpr Transform getInverse() const
    {
        // clang-format off
        // Compute the determinant
        const float det = a00 * a11 - a10 * a01;
        // clang-format on

        // Compute the inverse if the determinant is not zero
        // (don't use an epsilon because the determinant may *really* be tiny)
        if (det == 0.f)
            return Identity;

        return {a11 / det, -a01 / det, (a12 * a01 - a11 * a02) / det, -a10 / det, a00 / det, -(a12 * a00 - a10 * a02) / det};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Transform a 2D point
    ///
    /// These two statements are equivalent:
    /// \code
    /// sf::Vec2f transformedPoint = matrix.transformPoint(point);
    /// sf::Vec2f transformedPoint = matrix * point;
    /// \endcode
    ///
    /// \param point Point to transform
    ///
    /// \return Transformed point
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vec2f transformPoint(const Vec2f point) const
    {
        return {a00 * point.x + a01 * point.y + a02, a10 * point.x + a11 * point.y + a12};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Transform a rectangle
    ///
    /// Since SFML doesn't provide support for oriented rectangles,
    /// the result of this function is always an axis-aligned
    /// rectangle. Which means that if the transform contains a
    /// rotation, the bounding rectangle of the transformed rectangle
    /// is returned.
    ///
    /// \param rectangle Rectangle to transform
    ///
    /// \return Transformed rectangle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr FloatRect transformRect(const FloatRect& rectangle) const
    {
        const Vec2f p0 = transformPoint(rectangle.position);

        // Transformed offset vec2 for the X-direction side
        const Vec2f dx = {a00 * rectangle.size.x, a10 * rectangle.size.x};

        // Transformed offset vec2 for the Y-direction side
        const Vec2f dy = {a01 * rectangle.size.y, a11 * rectangle.size.y};

        // Calculate other corners relative to `p0`
        const Vec2f p1 = p0 + dy;
        const Vec2f p2 = p0 + dx;
        const Vec2f p3 = p2 + dy; // Or `p1 + dx`

        // Compute the bounding rectangle of the transformed points
        const float minX = SFML_BASE_MIN(SFML_BASE_MIN(p0.x, p1.x), SFML_BASE_MIN(p2.x, p3.x));
        const float maxX = SFML_BASE_MAX(SFML_BASE_MAX(p0.x, p1.x), SFML_BASE_MAX(p2.x, p3.x));
        const float minY = SFML_BASE_MIN(SFML_BASE_MIN(p0.y, p1.y), SFML_BASE_MIN(p2.y, p3.y));
        const float maxY = SFML_BASE_MAX(SFML_BASE_MAX(p0.y, p1.y), SFML_BASE_MAX(p2.y, p3.y));

        return FloatRect{{minX, minY}, {maxX - minX, maxY - minY}};
    }


    ////////////////////////////////////////////////////////////
    /// \relates sf::Transform
    /// \brief Overload of binary `operator*` to combine two transforms
    ///
    /// This call is equivalent to calling `Transform(lhs).combine(rhs)`.
    ///
    /// \param lhs Left operand (the first transform)
    /// \param rhs Right operand (the second transform)
    ///
    /// \return New combined transform
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Transform operator*(const Transform& lhs, const Transform& rhs)
    {
        return {lhs.a00 * rhs.a00 + lhs.a01 * rhs.a10,
                lhs.a00 * rhs.a01 + lhs.a01 * rhs.a11,
                lhs.a00 * rhs.a02 + lhs.a01 * rhs.a12 + lhs.a02,
                lhs.a10 * rhs.a00 + lhs.a11 * rhs.a10,
                lhs.a10 * rhs.a01 + lhs.a11 * rhs.a11,
                lhs.a10 * rhs.a02 + lhs.a11 * rhs.a12 + lhs.a12};
    }


    ////////////////////////////////////////////////////////////
    /// \relates sf::Transform
    /// \brief Overload of binary `operator*=` to combine two transforms
    ///
    /// This call is equivalent to calling `lhs.combine(rhs)`.
    ///
    /// \param lhs Left operand (the first transform)
    /// \param rhs Right operand (the second transform)
    ///
    /// \return The combined transform
    ///
    ////////////////////////////////////////////////////////////
    friend constexpr Transform& operator*=(Transform& lhs, const Transform& rhs)
    {
        return lhs.combine(rhs);
    }


    ////////////////////////////////////////////////////////////
    /// \relates sf::Transform
    /// \brief Overload of binary `operator*` to transform a point
    ///
    /// This call is equivalent to calling `lhs.transformPoint(rhs)`.
    ///
    /// \param lhs Left operand (the transform)
    /// \param rhs Right operand (the point to transform)
    ///
    /// \return New transformed point
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] friend constexpr Vec2f operator*(const Transform& lhs, Vec2f rhs)
    {
        return lhs.transformPoint(rhs);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with another one
    ///
    /// The result is a transform that is equivalent to applying
    /// `transform` followed by `*this`. Mathematically, it is
    /// equivalent to a matrix multiplication `(*this) * transform`.
    ///
    /// These two statements are equivalent:
    /// \code
    /// lhs.combine(rhs);
    /// lhs *= rhs;
    /// \endcode
    ///
    /// \param transform Transform to combine with this transform
    ///
    /// \return Reference to `*this`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& combine(const Transform& transform)
    {
        return *this = operator*(*this, transform);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a translation
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.translate(sf::Vec2f{100, 200}).rotate(sf::degrees(45));
    /// \endcode
    ///
    /// \param offset Translation offset to apply
    ///
    /// \return Reference to `*this`
    ///
    /// \see `rotate`, `scale`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& translate(const Vec2f offset)
    {
        // clang-format off
        const Transform translation(1.f, 0.f, offset.x,
                                    0.f, 1.f, offset.y);
        // clang-format on

        return combine(translation);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a rotation
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.rotate(sf::degrees(90)).translate(50, 20);
    /// \endcode
    ///
    /// \param angle Rotation angle
    ///
    /// \return Reference to `*this`
    ///
    /// \see `translate`, `scale`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& rotate(const Angle angle)
    {
        const auto [sine, cosine] = base::fastSinCos(angle.wrapUnsigned().asRadians());

        // clang-format off
    const Transform rotation(cosine, -sine, 0.f,
                             sine,  cosine, 0.f);
        // clang-format on

        return combine(rotation);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a rotation
    ///
    /// The center of rotation is provided for convenience as a second
    /// argument, so that you can build rotations around arbitrary points
    /// more easily (and efficiently) than the usual
    /// `translate(-center).rotate(angle).translate(center)`.
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.rotate(sf::degrees(90), sf::Vec2f{8, 3}).translate(sf::Vec2f{50, 20});
    /// \endcode
    ///
    /// \param angle Rotation angle
    /// \param center Center of rotation
    ///
    /// \return Reference to `*this`
    ///
    /// \see `translate`, `scale`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& rotate(const Angle angle, const Vec2f center)
    {
        const auto [sine, cosine] = base::fastSinCos(angle.wrapUnsigned().asRadians());

        // clang-format off
            const Transform rotation(cosine, -sine, center.x * (1.f - cosine) + center.y * sine,
                                     sine,  cosine, center.y * (1.f - cosine) - center.x * sine);
        // clang-format on

        return combine(rotation);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a scaling
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.scaleBy(sf::Vec2f{2, 1}).rotate(sf::degrees(45));
    /// \endcode
    ///
    /// \param factors Scaling factors
    ///
    /// \return Reference to `*this`
    ///
    /// \see `translate`, `rotate`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& scaleBy(const Vec2f factors)
    {
        // clang-format off
        const Transform scaling(factors.x, 0.f,       0.f,
                                0.f,       factors.y, 0.f);
        // clang-format on

        return combine(scaling);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a scaling
    ///
    /// The center of scaling is provided for convenience as a second
    /// argument, so that you can build scaling around arbitrary points
    /// more easily (and efficiently) than the usual
    /// `translate(-center).scaleBy(factors).translate(center)`.
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.scaleBy(sf::Vec2f{2, 1}, sf::Vec2f{8, 3}).rotate(45);
    /// \endcode
    ///
    /// \param factors Scaling factors
    /// \param center Center of scaling
    ///
    /// \return Reference to `*this`
    ///
    /// \see `translate`, `rotate`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& scaleBy(const Vec2f factors, const Vec2f center)
    {
        // clang-format off
        const Transform scaling(factors.x, 0.f,       center.x * (1.f - factors.x),
                                0.f,       factors.y, center.y * (1.f - factors.y));
        // clang-format on

        return combine(scaling);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Overload of binary `operator==` to compare two transforms
    ///
    /// Performs an element-wise comparison of the elements of the
    /// lhs transform with the elements of the rhs transform.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if the transforms are equal, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator==(const Transform& rhs) const = default;

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(readability-identifier-naming)
    static const Transform Identity; //!< The identity transform (does nothing)

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    float a00{1.f}, a01{0.f}, a02{0.f}, a10{0.f}, a11{1.f}, a12{0.f};
};


////////////////////////////////////////////////////////////
inline constexpr Transform Transform::Identity{};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Transform
/// \ingroup graphics
///
/// A `sf::Transform` specifies how to translate, rotate, scale,
/// shear, project, whatever things. In mathematical terms, it defines
/// how to transform a coordinate system into another.
///
/// For example, if you apply a rotation transform to a sprite, the
/// result will be a rotated sprite. And anything that is transformed
/// by this rotation transform will be rotated the same way, according
/// to its initial position.
///
/// Transforms are typically used for drawing. But they can also be
/// used for any computation that requires to transform points between
/// the local and global coordinate systems of an entity (like collision
/// detection).
///
/// Example:
/// \code
/// // define a translation transform
/// sf::Transform translation;
/// translation.translate(20, 50);
///
/// // define a rotation transform
/// sf::Transform rotation;
/// rotation.rotate(45);
///
/// // combine them
/// sf::Transform transform = translation * rotation;
///
/// // use the result to transform stuff...
/// sf::Vec2f point = transform.transformPoint({10, 20});
/// sf::FloatRect rect = transform.transformRect(sf::FloatRect({0, 0}, {10, 100}));
/// \endcode
///
/// \see `sf::Transformable`, `sf::RenderStates`
///
////////////////////////////////////////////////////////////
