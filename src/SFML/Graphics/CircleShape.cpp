////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/CircleShape.hpp>

#include <SFML/System/Angle.hpp>


namespace
{
////////////////////////////////////////////////////////////
sf::Vector2f computeCirclePoint(std::size_t index, std::size_t pointCount, float radius)
{
    const sf::Angle angle = static_cast<float>(index) / static_cast<float>(pointCount) * sf::degrees(360.f) -
                            sf::degrees(90.f);

    return sf::Vector2f(radius, radius) + sf::Vector2f(radius, angle);
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
CircleShape::CircleShape(float radius, std::size_t pointCount) : m_radius(radius)
{
    update(radius, pointCount);
}


////////////////////////////////////////////////////////////
void CircleShape::setRadius(float radius)
{
    m_radius = radius;
    update(radius, m_points.size());
}


////////////////////////////////////////////////////////////
float CircleShape::getRadius() const
{
    return m_radius;
}


////////////////////////////////////////////////////////////
void CircleShape::setPointCount(std::size_t count)
{
    update(m_radius, count);
}

////////////////////////////////////////////////////////////
std::size_t CircleShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getPoint(std::size_t index) const
{
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f CircleShape::getGeometricCenter() const
{
    return {m_radius, m_radius};
}


////////////////////////////////////////////////////////////
void CircleShape::update(float radius, std::size_t pointCount)
{
    m_points.resize(pointCount);

    for (std::size_t i = 0; i < pointCount; ++i)
        m_points[i] = computeCirclePoint(i, pointCount, radius);

    Shape::update(m_points.data(), m_points.size());
}

} // namespace sf
