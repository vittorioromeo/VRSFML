#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/PlaybackDeviceHandle.hpp"

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
AudioDeviceHandle::AudioDeviceHandle(base::PassKey<AudioContext>&&, const void* maDeviceInfo) :
AudioDeviceHandle{maDeviceInfo}
{
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(base::PassKey<AudioContextUtils>&&, const void* maDeviceInfo) :
AudioDeviceHandle{maDeviceInfo}
{
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(base::PassKey<PlaybackDevice>&&, const void* maDeviceInfo) :
AudioDeviceHandle{maDeviceInfo}
{
}


////////////////////////////////////////////////////////////
AudioDeviceHandle::AudioDeviceHandle(base::PassKey<CaptureDevice>&&, const void* maDeviceInfo) :
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
