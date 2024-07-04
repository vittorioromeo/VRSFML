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
#include <SFML/Audio/Listener.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
Listener::Listener() = default;


////////////////////////////////////////////////////////////
void Listener::setVolume(float volume)
{
    m_volume = volume;
}


////////////////////////////////////////////////////////////
float Listener::getVolume() const
{
    return m_volume;
}


////////////////////////////////////////////////////////////
void Listener::setPosition(const Vector3f& position)
{
    m_position = position;
}


////////////////////////////////////////////////////////////
Vector3f Listener::getPosition() const
{
    return m_position;
}


////////////////////////////////////////////////////////////
void Listener::setDirection(const Vector3f& direction)
{
    m_direction = direction;
}


////////////////////////////////////////////////////////////
Vector3f Listener::getDirection() const
{
    return m_direction;
}


////////////////////////////////////////////////////////////
void Listener::setVelocity(const Vector3f& velocity)
{
    m_velocity = velocity;
}


////////////////////////////////////////////////////////////
Vector3f Listener::getVelocity() const
{
    return m_velocity;
}


////////////////////////////////////////////////////////////
void Listener::setCone(const Listener::Cone& cone)
{
    m_cone = cone;
}


////////////////////////////////////////////////////////////
Listener::Cone Listener::getCone() const
{
    return m_cone;
}


////////////////////////////////////////////////////////////
void Listener::setUpVector(const Vector3f& upVector)
{
    m_upVector = upVector;
}


////////////////////////////////////////////////////////////
Vector3f Listener::getUpVector() const
{
    return m_upVector;
}

} // namespace sf
