#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/MinMaxMacros.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
constexpr void Transform::getMatrix(float (&target)[16]) const
{
    target[0]  = a00;
    target[1]  = a10;
    target[4]  = a01;
    target[5]  = a11;
    target[12] = a02;
    target[13] = a12;
}


////////////////////////////////////////////////////////////
constexpr Transform Transform::getInverse() const
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
constexpr Vector2f Transform::transformPoint(Vector2f point) const
{
    return {a00 * point.x + a01 * point.y + a02, a10 * point.x + a11 * point.y + a12};
}


////////////////////////////////////////////////////////////
constexpr FloatRect Transform::transformRect(const FloatRect& rectangle) const
{
    const Vector2f p0 = transformPoint(rectangle.position);

    // Transformed offset vector for the X-direction side
    const Vector2f dx = {a00 * rectangle.size.x, a10 * rectangle.size.x};

    // Transformed offset vector for the Y-direction side
    const Vector2f dy = {a01 * rectangle.size.y, a11 * rectangle.size.y};

    // Calculate other corners relative to `p0`
    const Vector2f p1 = p0 + dy;
    const Vector2f p2 = p0 + dx;
    const Vector2f p3 = p2 + dy; // Or `p1 + dx`

    // Compute the bounding rectangle of the transformed points
    const float minX = SFML_BASE_MIN(SFML_BASE_MIN(p0.x, p1.x), SFML_BASE_MIN(p2.x, p3.x));
    const float maxX = SFML_BASE_MAX(SFML_BASE_MAX(p0.x, p1.x), SFML_BASE_MAX(p2.x, p3.x));
    const float minY = SFML_BASE_MIN(SFML_BASE_MIN(p0.y, p1.y), SFML_BASE_MIN(p2.y, p3.y));
    const float maxY = SFML_BASE_MAX(SFML_BASE_MAX(p0.y, p1.y), SFML_BASE_MAX(p2.y, p3.y));

    return FloatRect{{minX, minY}, {maxX - minX, maxY - minY}};
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::combine(const Transform& transform)
{
    return *this = sf::operator*(*this, transform);
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::translate(Vector2f offset)
{
    // clang-format off
    const Transform translation(1.f, 0.f, offset.x,
                                0.f, 1.f, offset.y);
    // clang-format on

    return combine(translation);
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::scaleBy(Vector2f factors)
{
    // clang-format off
    const Transform scaling(factors.x, 0.f,       0.f,
                            0.f,       factors.y, 0.f);
    // clang-format on

    return combine(scaling);
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::scaleBy(Vector2f factors, Vector2f center)
{
    // clang-format off
    const Transform scaling(factors.x, 0.f,       center.x * (1.f - factors.x),
                            0.f,       factors.y, center.y * (1.f - factors.y));
    // clang-format on

    return combine(scaling);
}


////////////////////////////////////////////////////////////
constexpr Transform operator*(const Transform& lhs, const Transform& rhs)
{
    return {lhs.a00 * rhs.a00 + lhs.a01 * rhs.a10,
            lhs.a00 * rhs.a01 + lhs.a01 * rhs.a11,
            lhs.a00 * rhs.a02 + lhs.a01 * rhs.a12 + lhs.a02,
            lhs.a10 * rhs.a00 + lhs.a11 * rhs.a10,
            lhs.a10 * rhs.a01 + lhs.a11 * rhs.a11,
            lhs.a10 * rhs.a02 + lhs.a11 * rhs.a12 + lhs.a12};
}


////////////////////////////////////////////////////////////
constexpr Transform& operator*=(Transform& lhs, const Transform& rhs)
{
    return lhs.combine(rhs);
}


////////////////////////////////////////////////////////////
constexpr Vector2f operator*(const Transform& lhs, Vector2f rhs)
{
    return lhs.transformPoint(rhs);
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Transform Transform::Identity{};

} // namespace sf
