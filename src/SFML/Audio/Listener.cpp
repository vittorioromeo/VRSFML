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
Listener::Listener(PlaybackDevice& playbackDevice) : m_playbackDevice(&playbackDevice)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(PlaybackDevice, Listener, m_playbackDevice);
}


////////////////////////////////////////////////////////////
void Listener::setGlobalVolume(float volume)
{
    m_playbackDevice->asAudioDevice().setGlobalVolume(volume);
}


////////////////////////////////////////////////////////////
float Listener::getGlobalVolume()
{
    return m_playbackDevice->asAudioDevice().getGlobalVolume();
}


////////////////////////////////////////////////////////////
void Listener::setPosition(const Vector3f& position)
{
    m_playbackDevice->asAudioDevice().setPosition(position);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getPosition()
{
    return m_playbackDevice->asAudioDevice().getPosition();
}


////////////////////////////////////////////////////////////
void Listener::setDirection(const Vector3f& direction)
{
    m_playbackDevice->asAudioDevice().setDirection(direction);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getDirection()
{
    return m_playbackDevice->asAudioDevice().getDirection();
}


////////////////////////////////////////////////////////////
void Listener::setVelocity(const Vector3f& velocity)
{
    m_playbackDevice->asAudioDevice().setVelocity(velocity);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getVelocity()
{
    return m_playbackDevice->asAudioDevice().getVelocity();
}


////////////////////////////////////////////////////////////
void Listener::setCone(const Listener::Cone& cone)
{
    m_playbackDevice->asAudioDevice().setCone(cone);
}


////////////////////////////////////////////////////////////
Listener::Cone Listener::getCone()
{
    return m_playbackDevice->asAudioDevice().getCone();
}


////////////////////////////////////////////////////////////
void Listener::setUpVector(const Vector3f& upVector)
{
    m_playbackDevice->asAudioDevice().setUpVector(upVector);
}


////////////////////////////////////////////////////////////
Vector3f Listener::getUpVector()
{
    return m_playbackDevice->asAudioDevice().getUpVector();
}


////////////////////////////////////////////////////////////
sf::PlaybackDevice& Listener::getPlaybackDevice()
{
    return *m_playbackDevice;
}


////////////////////////////////////////////////////////////
const sf::PlaybackDevice& Listener::getPlaybackDevice() const
{
    return *m_playbackDevice;
}

} // namespace sf
