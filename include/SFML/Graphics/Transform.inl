#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
constexpr Transform::Transform(float a00, float a01, float a02, float a10, float a11, float a12) :
m_a00{a00},
m_a10{a10},
m_a01{a01},
m_a11{a11},
m_a02{a02},
m_a12{a12}
{
}


////////////////////////////////////////////////////////////
constexpr void Transform::getMatrix(float (&target)[16]) const
{
    target[0]  = m_a00;
    target[1]  = m_a10;
    target[4]  = m_a01;
    target[5]  = m_a11;
    target[12] = m_a02;
    target[13] = m_a12;
}


////////////////////////////////////////////////////////////
constexpr Transform Transform::getInverse() const
{
    // clang-format off
    // Compute the determinant
    const float det = m_a00 * m_a11 - m_a10 * m_a01;
    // clang-format on

    // Compute the inverse if the determinant is not zero
    // (don't use an epsilon because the determinant may *really* be tiny)
    if (det != 0.f)
    {
        // clang-format off
        return {(                m_a11        ) / det,
               -(                m_a01        ) / det,
                (m_a12 * m_a01 - m_a11 * m_a02) / det,
               -(                m_a10        ) / det,
                (                m_a00        ) / det,
               -(m_a12 * m_a00 - m_a10 * m_a02) / det};
        // clang-format on
    }

    return Identity;
}


////////////////////////////////////////////////////////////
constexpr Vector2f Transform::transformPoint(Vector2f point) const
{
    return {m_a00 * point.x + m_a01 * point.y + m_a02, m_a10 * point.x + m_a11 * point.y + m_a12};
}


////////////////////////////////////////////////////////////
constexpr FloatRect Transform::transformRect(const FloatRect& rectangle) const
{
    // Transform the 4 corners of the rectangle
    const Vector2f points[]{transformPoint(rectangle.position),
                            transformPoint(rectangle.position + Vector2f(0.f, rectangle.size.y)),
                            transformPoint(rectangle.position + Vector2f(rectangle.size.x, 0.f)),
                            transformPoint(rectangle.position + rectangle.size)};

    // Compute the bounding rectangle of the transformed points
    Vector2f pmin = points[0];
    Vector2f pmax = points[0];

    for (base::SizeT i = 1; i < 4; ++i)
    {
        // clang-format off
        if      (points[i].x < pmin.x) pmin.x = points[i].x;
        else if (points[i].x > pmax.x) pmax.x = points[i].x;

        if      (points[i].y < pmin.y) pmin.y = points[i].y;
        else if (points[i].y > pmax.y) pmax.y = points[i].y;
        // clang-format on
    }

    return {pmin, pmax - pmin};
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::combine(const Transform& transform)
{
    *this = sf::operator*(*this, transform);
    return *this;
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::translate(Vector2f offset)
{
    // clang-format off
    const Transform translation(1, 0, offset.x,
                                0, 1, offset.y);
    // clang-format on

    return combine(translation);
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::scaleBy(Vector2f factors)
{
    // clang-format off
    const Transform scaling(factors.x, 0,         0,
                            0,         factors.y, 0);
    // clang-format on

    return combine(scaling);
}


////////////////////////////////////////////////////////////
constexpr Transform& Transform::scaleBy(Vector2f factors, Vector2f center)
{
    // clang-format off
    const Transform scaling(factors.x, 0,         center.x * (1 - factors.x),
                            0,         factors.y, center.y * (1 - factors.y));
    // clang-format on

    return combine(scaling);
}


////////////////////////////////////////////////////////////
constexpr Transform operator*(const Transform& left, const Transform& right)
{
    return {left.m_a00 * right.m_a00 + left.m_a01 * right.m_a10,
            left.m_a00 * right.m_a01 + left.m_a01 * right.m_a11,
            left.m_a00 * right.m_a02 + left.m_a01 * right.m_a12 + left.m_a02,
            left.m_a10 * right.m_a00 + left.m_a11 * right.m_a10,
            left.m_a10 * right.m_a01 + left.m_a11 * right.m_a11,
            left.m_a10 * right.m_a02 + left.m_a11 * right.m_a12 + left.m_a12};
}


////////////////////////////////////////////////////////////
constexpr Transform& operator*=(Transform& left, const Transform& right)
{
    return left.combine(right);
}


////////////////////////////////////////////////////////////
constexpr Vector2f operator*(const Transform& left, Vector2f right)
{
    return left.transformPoint(right);
}


////////////////////////////////////////////////////////////
constexpr bool operator==(const Transform& left, const Transform& right)
{
    return left.m_a00 == right.m_a00 && left.m_a10 == right.m_a10 && left.m_a01 == right.m_a01 &&
           left.m_a11 == right.m_a11 && left.m_a02 == right.m_a02 && left.m_a12 == right.m_a12;
}


////////////////////////////////////////////////////////////
constexpr bool operator!=(const Transform& left, const Transform& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

// Note: the 'inline' keyword here is technically not required, but VS2019 fails
// to compile with a bogus "multiple definition" error if not explicitly used.
inline constexpr Transform Transform::Identity{};

} // namespace sf
