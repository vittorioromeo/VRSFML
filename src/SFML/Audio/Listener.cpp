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
#include <SFML/Audio/AudioDevice.hpp>
#include <SFML/Audio/Listener.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
void Listener::setGlobalVolume(float volume)
{
    getAudioDevice().setGlobalVolume(volume);
}


////////////////////////////////////////////////////////////
float Listener::getGlobalVolume()
{
    return getAudioDevice().getGlobalVolume();
}


////////////////////////////////////////////////////////////
void Listener::setPosition(const Vector3f& position)
{
    getAudioDevice().setPosition(position);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getPosition()
{
    return getAudioDevice().getPosition();
}


////////////////////////////////////////////////////////////
void Listener::setDirection(const Vector3f& direction)
{
    getAudioDevice().setDirection(direction);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getDirection()
{
    return getAudioDevice().getDirection();
}


////////////////////////////////////////////////////////////
void Listener::setVelocity(const Vector3f& velocity)
{
    getAudioDevice().setVelocity(velocity);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getVelocity()
{
    return getAudioDevice().getVelocity();
}


////////////////////////////////////////////////////////////
void Listener::setCone(const Listener::Cone& cone)
{
    getAudioDevice().setCone(cone);
}


////////////////////////////////////////////////////////////
Listener::Cone Listener::getCone()
{
    return getAudioDevice().getCone();
}


////////////////////////////////////////////////////////////
void Listener::setUpVector(const Vector3f& upVector)
{
    getAudioDevice().setUpVector(upVector);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getUpVector()
{
    return getAudioDevice().getUpVector();
}

} // namespace sf
