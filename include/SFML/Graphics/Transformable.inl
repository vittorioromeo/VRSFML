#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transformable.hpp" // NOLINT(misc-header-include-cycle)

#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
constexpr void Transformable::setPosition(Vector2f position)
{
    m_position = position;
}


////////////////////////////////////////////////////////////
constexpr void Transformable::setRotation(Angle angle)
{
    m_rotation = angle.wrapUnsigned();
}


////////////////////////////////////////////////////////////
constexpr void Transformable::setScale(Vector2f factors)
{
    m_scale = factors;
}


////////////////////////////////////////////////////////////
constexpr void Transformable::setOrigin(Vector2f origin)
{
    m_origin = origin;
}


////////////////////////////////////////////////////////////
constexpr Vector2f Transformable::getPosition() const
{
    return m_position;
}


////////////////////////////////////////////////////////////
constexpr Angle Transformable::getRotation() const
{
    return m_rotation;
}


////////////////////////////////////////////////////////////
constexpr Vector2f Transformable::getScale() const
{
    return m_scale;
}


////////////////////////////////////////////////////////////
constexpr Vector2f Transformable::getOrigin() const
{
    return m_origin;
}


////////////////////////////////////////////////////////////
constexpr void Transformable::move(Vector2f offset)
{
    setPosition(m_position + offset);
}


////////////////////////////////////////////////////////////
constexpr void Transformable::rotate(Angle angle)
{
    setRotation(m_rotation + angle);
}


////////////////////////////////////////////////////////////
constexpr void Transformable::scale(Vector2f factor)
{
    setScale({m_scale.x * factor.x, m_scale.y * factor.y});
}


////////////////////////////////////////////////////////////
constexpr Transform Transformable::getTransform() const
{
    const float angle  = -m_rotation.asRadians();
    const float cosine = base::cos(angle);
    const float sine   = base::sin(angle);
    const float sxc    = m_scale.x * cosine;
    const float syc    = m_scale.y * cosine;
    const float sxs    = m_scale.x * sine;
    const float sys    = m_scale.y * sine;
    const float tx     = -m_origin.x * sxc - m_origin.y * sys + m_position.x;
    const float ty     = m_origin.x * sxs - m_origin.y * syc + m_position.y;

    return {/* a00 */ sxc, /* a01 */ sys, /* a02 */ tx, -/* a10 */ sxs, /* a11 */ syc, /* a12 */ ty};
}


////////////////////////////////////////////////////////////
constexpr Transform Transformable::getInverseTransform() const
{
    return getTransform().getInverse();
}

} // namespace sf
