#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/MinMax.hpp"


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
    // Transform the 4 corners of the rectangle
    const auto p0 = transformPoint(rectangle.position);
    const auto p1 = transformPoint(rectangle.position + Vector2f(0.f, rectangle.size.y));
    const auto p2 = transformPoint(rectangle.position + Vector2f(rectangle.size.x, 0.f));
    const auto p3 = transformPoint(rectangle.position + rectangle.size);

    // Compute the bounding rectangle of the transformed points
    const float minX = base::min(base::min(p0.x, p1.x), base::min(p2.x, p3.x));
    const float maxX = base::max(base::max(p0.x, p1.x), base::max(p2.x, p3.x));
    const float minY = base::min(base::min(p0.y, p1.y), base::min(p2.y, p3.y));
    const float maxY = base::max(base::max(p0.y, p1.y), base::max(p2.y, p3.y));

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
