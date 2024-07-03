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
#include <SFML/Audio/PlaybackDevice.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/UniquePtr.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
PlaybackDevice::PlaybackDevice() : m_audioDevice(priv::makeUnique<priv::AudioDevice>())
{
}


////////////////////////////////////////////////////////////
PlaybackDevice::~PlaybackDevice() = default;


////////////////////////////////////////////////////////////
std::vector<std::string> PlaybackDevice::getAvailableDevices()
{
    const auto devices = m_audioDevice->getAvailableDevices();

    std::vector<std::string> deviceNameList;
    deviceNameList.reserve(devices.size());

    for (const auto& device : devices)
        deviceNameList.emplace_back(device.name);

    return deviceNameList;
}


////////////////////////////////////////////////////////////
std::optional<std::string> PlaybackDevice::getDefaultDevice()
{
    return m_audioDevice->getDefaultDevice();
}


////////////////////////////////////////////////////////////
bool PlaybackDevice::setDevice(const std::string& name)
{
    // Perform a sanity check to make sure the user isn't passing us a non-existant device name
    if (const auto& availableDevices = m_audioDevice->getAvailableDevices();
        !priv::anyOf(availableDevices.begin(),
                     availableDevices.end(),
                     [&](const priv::AudioDevice::DeviceEntry& deviceEntry) { return deviceEntry.name == name; }))
    {
        return false;
    }

    return m_audioDevice->setDevice(name);
}


////////////////////////////////////////////////////////////
std::optional<std::string> PlaybackDevice::getDevice()
{
    return m_audioDevice->getDevice();
}


// TODO
[[nodiscard]] priv::AudioDevice& PlaybackDevice::asAudioDevice() noexcept
{
    assert(m_audioDevice != nullptr);
    return *m_audioDevice;
}

[[nodiscard]] const priv::AudioDevice& PlaybackDevice::asAudioDevice() const noexcept
{
    assert(m_audioDevice != nullptr);
    return *m_audioDevice;
}

} // namespace sf
