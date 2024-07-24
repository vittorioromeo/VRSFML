#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Transformable.hpp>

#include <SFML/Base/Math.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
Transformable::Transformable() = default;


////////////////////////////////////////////////////////////
void Transformable::setPosition(Vector2f position)
{
    m_position                   = position;
    m_transformNeedUpdate        = true;
    m_inverseTransformNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Transformable::setRotation(Angle angle)
{
    m_rotation = angle.wrapUnsigned();

    m_transformNeedUpdate        = true;
    m_inverseTransformNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Transformable::setScale(Vector2f factors)
{
    m_scale                      = factors;
    m_transformNeedUpdate        = true;
    m_inverseTransformNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Transformable::setOrigin(Vector2f origin)
{
    m_origin                     = origin;
    m_transformNeedUpdate        = true;
    m_inverseTransformNeedUpdate = true;
}


////////////////////////////////////////////////////////////
Vector2f Transformable::getPosition() const
{
    return m_position;
}


////////////////////////////////////////////////////////////
Angle Transformable::getRotation() const
{
    return m_rotation;
}


////////////////////////////////////////////////////////////
Vector2f Transformable::getScale() const
{
    return m_scale;
}


////////////////////////////////////////////////////////////
Vector2f Transformable::getOrigin() const
{
    return m_origin;
}


////////////////////////////////////////////////////////////
void Transformable::move(Vector2f offset)
{
    setPosition(m_position + offset);
}


////////////////////////////////////////////////////////////
void Transformable::rotate(Angle angle)
{
    setRotation(m_rotation + angle);
}


////////////////////////////////////////////////////////////
void Transformable::scale(Vector2f factor)
{
    setScale({m_scale.x * factor.x, m_scale.y * factor.y});
}


////////////////////////////////////////////////////////////
const Transform& Transformable::getTransform() const
{
    // Recompute the combined transform if needed
    if (m_transformNeedUpdate)
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

        // clang-format off
        m_transform = Transform( sxc, sys, tx,
                                -sxs, syc, ty,
                                 0.f, 0.f, 1.f);
        // clang-format on
        m_transformNeedUpdate = false;
    }

    return m_transform;
}


////////////////////////////////////////////////////////////
const Transform& Transformable::getInverseTransform() const
{
    // Recompute the inverse transform if needed
    if (m_inverseTransformNeedUpdate)
    {
        m_inverseTransform           = getTransform().getInverse();
        m_inverseTransformNeedUpdate = false;
    }

    return m_inverseTransform;
}

} // namespace sf
