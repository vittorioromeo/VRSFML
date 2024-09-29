#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Angle;
class RenderTarget;
struct Transform;
struct Sprite;
struct Transformable;
struct Vertex;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Define a 3x3 transform matrix TODO P0:
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] Transform
{
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
    [[gnu::always_inline]] constexpr void getMatrix(float (&target)[16]) const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the inverse of the transform
    ///
    /// If the inverse cannot be computed, an identity transform
    /// is returned.
    ///
    /// \return A new transform which is the inverse of self
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] constexpr Transform getInverse() const;

    ////////////////////////////////////////////////////////////
    /// \brief Transform a 2D point
    ///
    /// These two statements are equivalent:
    /// \code
    /// sf::Vector2f transformedPoint = matrix.transformPoint(point);
    /// sf::Vector2f transformedPoint = matrix * point;
    /// \endcode
    ///
    /// \param point Point to transform
    ///
    /// \return Transformed point
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr Vector2f transformPoint(Vector2f point) const;

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
    [[nodiscard, gnu::pure]] constexpr FloatRect transformRect(const FloatRect& rectangle) const;

    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with another one
    ///
    /// The result is a transform that is equivalent to applying
    /// \a `transform` followed by `*this`. Mathematically, it is
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
    [[gnu::always_inline]] constexpr Transform& combine(const Transform& transform);

    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a translation
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.translate(sf::Vector2f{100, 200}).rotate(sf::degrees(45));
    /// \endcode
    ///
    /// \param offset Translation offset to apply
    ///
    /// \return Reference to `*this`
    ///
    /// \see `rotate`, `scale`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& translate(Vector2f offset);

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
    SFML_GRAPHICS_API Transform& rotate(Angle angle);

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
    /// transform.rotate(sf::degrees(90), sf::Vector2f{8, 3}).translate(sf::Vector2f{50, 20});
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
    SFML_GRAPHICS_API Transform& rotate(Angle angle, Vector2f center);

    ////////////////////////////////////////////////////////////
    /// \brief Combine the current transform with a scaling
    ///
    /// This function returns a reference to `*this`, so that calls
    /// can be chained.
    /// \code
    /// sf::Transform transform;
    /// transform.scaleBy(sf::Vector2f{2, 1}).rotate(sf::degrees(45));
    /// \endcode
    ///
    /// \param factors Scaling factors
    ///
    /// \return Reference to `*this`
    ///
    /// \see `translate`, `rotate`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr Transform& scaleBy(Vector2f factors);

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
    /// transform.scaleBy(sf::Vector2f{2, 1}, sf::Vector2f{8, 3}).rotate(45);
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
    [[gnu::always_inline]] constexpr Transform& scaleBy(Vector2f factors, Vector2f center);

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
    /// \brief Overload of binary `operator!=` to compare two transforms
    ///
    /// This call is equivalent to `!(lhs == rhs)`.
    ///
    /// \param rhs Right operand
    ///
    /// \return `true` if the transforms are not equal, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool operator!=(const Transform& rhs) const = default;

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
/// \relates `sf::Transform`
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
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Transform operator*(const Transform& lhs, const Transform& rhs);

////////////////////////////////////////////////////////////
/// \relates `sf::Transform`
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
constexpr Transform& operator*=(Transform& lhs, const Transform& rhs);

////////////////////////////////////////////////////////////
/// \relates `sf::Transform`
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
[[nodiscard, gnu::always_inline, gnu::pure]] constexpr Vector2f operator*(const Transform& lhs, Vector2f rhs);

} // namespace sf

#include "SFML/Graphics/Transform.inl"


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
/// sf::Vector2f point = transform.transformPoint({10, 20});
/// sf::FloatRect rect = transform.transformRect(sf::FloatRect({0, 0}, {10, 100}));
/// \endcode
///
/// \see `sf::Transformable`, `sf::RenderStates`
///
////////////////////////////////////////////////////////////
