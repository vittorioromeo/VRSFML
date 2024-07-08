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
#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <miniaudio.h>

#include <cstring>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct AudioDeviceHandle::Impl
{
    ma_device_info maDeviceInfo;
};


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(const void* maDeviceInfo) :
m_impl(*static_cast<const ma_device_info*>(maDeviceInfo))
{
}


////////////////////////////////////////////////////////////
const void* AudioDeviceHandle::getMADeviceInfo() const
{
    return &m_impl->maDeviceInfo;
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(priv::PassKey<AudioContext>&&, const void* maDeviceInfo) :
AudioDeviceHandle{maDeviceInfo}
{
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(priv::PassKey<PlaybackDevice>&&, const void* maDeviceInfo) :
AudioDeviceHandle{maDeviceInfo}
{
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(priv::PassKey<CaptureDevice>&&, const void* maDeviceInfo) :
AudioDeviceHandle{maDeviceInfo}
{
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::~AudioDeviceHandle() = default;


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(const AudioDeviceHandle& rhs) = default;


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(AudioDeviceHandle&& rhs) noexcept = default;


////////////////////////////////////////////////////////////
AudioDeviceHandle& AudioDeviceHandle::operator=(const AudioDeviceHandle& rhs) = default;


////////////////////////////////////////////////////////////
AudioDeviceHandle& AudioDeviceHandle::operator=(AudioDeviceHandle&& rhs) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] const char* AudioDeviceHandle::getName() const
{
    return m_impl->maDeviceInfo.name;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool AudioDeviceHandle::isDefault() const
{
    return static_cast<bool>(m_impl->maDeviceInfo.isDefault);
}


////////////////////////////////////////////////////////////
bool operator==(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs)
{
    // The approach below seems fine:
    // https://github.com/mackron/miniaudio/issues/866

    // NOLINTNEXTLINE(bugprone-suspicious-memory-comparison)
    return std::memcmp(&lhs.m_impl->maDeviceInfo.id, &rhs.m_impl->maDeviceInfo.id, sizeof(ma_device_id)) == 0;
}


////////////////////////////////////////////////////////////
bool operator!=(const AudioDeviceHandle& lhs, const AudioDeviceHandle& rhs)
{
    return !(lhs == rhs);
}

} // namespace sf::priv