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
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/AudioDevice.hpp>
#include <SFML/Audio/Listener.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Vector3.hpp>

#include <miniaudio.h>

#include <mutex>
#include <optional>
#include <vector>

#include <cassert>


namespace sf::priv
{
namespace
{
////////////////////////////////////////////////////////////
void maDeviceDataCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount)
{
    ma_engine& maEngine = *static_cast<ma_engine*>(device->pUserData);

    if (const ma_result result = ma_engine_read_pcm_frames(&maEngine, output, frameCount, nullptr); result != MA_SUCCESS)
        err() << "Failed to read PCM frames from audio engine: " << ma_result_description(result) << errEndl;
}

////////////////////////////////////////////////////////////
[[nodiscard]] bool tryCreateMADevice(ma_device& maDevice, ma_context& maContext, ma_engine& maEngine, const ma_device_id& deviceId)
{
    ma_device_config maDeviceConfig   = ma_device_config_init(ma_device_type_playback);
    maDeviceConfig.dataCallback       = &maDeviceDataCallback;
    maDeviceConfig.pUserData          = &maEngine;
    maDeviceConfig.playback.format    = ma_format_f32;
    maDeviceConfig.playback.pDeviceID = &deviceId;

    if (const ma_result result = ma_device_init(&maContext, &maDeviceConfig, &maDevice); result != MA_SUCCESS)
    {
        err() << "Failed to initialize the audio playback device: " << ma_result_description(result) << errEndl;
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////
[[nodiscard]] bool tryCreateMAEngine(ma_engine& maEngine, ma_context& maContext, ma_device& maDevice)
{
    auto engineConfig          = ma_engine_config_init();
    engineConfig.pContext      = &maContext;
    engineConfig.pDevice       = &maDevice;
    engineConfig.listenerCount = 1;

    if (const ma_result result = ma_engine_init(&engineConfig, &maEngine); result != MA_SUCCESS)
    {
        err() << "Failed to initialize the audio engine: " << ma_result_description(result) << errEndl;
        return false;
    }

    return true;
}
} // namespace

////////////////////////////////////////////////////////////
class MiniaudioPerHWDevice
{
private:
    ma_device m_maDevice; //!< miniaudio playback device (one per device)
    ma_engine m_maEngine; //!< miniaudio engine (one per device, for effects/spatialisation)

public:
    ~MiniaudioPerHWDevice()
    {
        ma_engine_uninit(&m_maEngine);
        ma_device_uninit(&m_maDevice);
    }

    [[nodiscard]] ma_device& getMADevice()
    {
        return m_maDevice;
    }

    [[nodiscard]] ma_engine& getMAEngine()
    {
        return m_maEngine;
    }
};


////////////////////////////////////////////////////////////
struct AudioDevice::Impl
{
    PlaybackDevice*                           playbackDevice; // !< TODO
    ma_context&                               maContext;      // !< TODO
    std::optional<PlaybackDeviceHandle>       deviceHandle;   // !< TODO
    std::optional<MiniaudioPerHWDevice>       maPerHWDevice;
    std::vector<std::optional<ResourceEntry>> resources;      //!< Registered resources
    std::mutex                                resourcesMutex; //!< The mutex guarding the registered resources


    explicit Impl(PlaybackDevice&      thePlaybackDevice,
                  AudioContext&        theAudioContext,
                  PlaybackDeviceHandle theDeviceHandle,
                  const ma_device_id&  maDeviceId) :
    playbackDevice(&thePlaybackDevice),
    maContext(theAudioContext.getMAContext()),
    deviceHandle(theDeviceHandle)
    {
        maPerHWDevice.emplace();

        if (!tryCreateMADevice(maPerHWDevice->getMADevice(), maContext, maPerHWDevice->getMAEngine(), maDeviceId))
        {
            err() << "Failed to initialize audio device " << errEndl;
            maPerHWDevice.reset();
            return;
        }

        if (!tryCreateMAEngine(maPerHWDevice->getMAEngine(), maContext, maPerHWDevice->getMADevice()))
        {
            err() << "Failed to initialize audio engine " << errEndl;
            maPerHWDevice.reset();
            return;
        }
    }
};


////////////////////////////////////////////////////////////
AudioDevice::AudioDevice(PlaybackDevice& playbackDevice, AudioContext& audioContext, const PlaybackDeviceHandle& deviceHandle) :
m_impl(playbackDevice,
       audioContext,
       deviceHandle,
       [&]() -> ma_device_id
       {
           ma_device_info deviceInfo;
           deviceHandle.copyMADeviceInfoInto(&deviceInfo);
           return deviceInfo.id;
       }())
{
    // Create the device and engine
    if (!m_impl->maPerHWDevice.has_value())
        err() << "Failed to initialize audio device or engine" << errEndl;
}


////////////////////////////////////////////////////////////
AudioDevice::~AudioDevice() = default;


////////////////////////////////////////////////////////////
ma_engine& AudioDevice::getEngine()
{
    return m_impl->maPerHWDevice->getMAEngine();
}


////////////////////////////////////////////////////////////
void AudioDevice::transferResourcesTo(AudioDevice& other)
{
    std::vector<ResourceEntryIndex> toUnregister;

    {
        const std::lock_guard lock(m_impl->resourcesMutex);

        // Deinitialize all audio resources from self
        for (ResourceEntryIndex i = 0; i < m_impl->resources.size(); ++i)
        {
            std::optional<ResourceEntry>& entry = m_impl->resources[i];
            if (!entry.has_value())
                continue;

            toUnregister.push_back(i);
            entry->deinitializeFunc(entry->resource);

            const ResourceEntryIndex otherEntryIndex = other.registerResource(entry->resource,
                                                                              entry->deinitializeFunc,
                                                                              entry->reinitializeFunc,
                                                                              entry->transferFunc);

            entry->transferFunc(entry->resource, *other.m_impl->playbackDevice, otherEntryIndex);
            entry->reinitializeFunc(entry->resource);
        }
    }

    for (ResourceEntryIndex i : toUnregister)
        unregisterResource(i);
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool AudioDevice::updateFromListener(const Listener& listener)
{
    ma_engine& engine = m_impl->maPerHWDevice->getMAEngine();

    // Set master volume, position, velocity, cone and world up vector
    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(&engine), listener.getVolume() * 0.01f);
        result != MA_SUCCESS)
    {
        err() << "Failed to set audio device master volume: " << ma_result_description(result) << errEndl;
        return false;
    }

    ma_engine_listener_set_position(&engine,
                                    0,
                                    listener.getPosition().x,
                                    listener.getPosition().y,
                                    listener.getPosition().z);

    ma_engine_listener_set_velocity(&engine,
                                    0,
                                    listener.getVelocity().x,
                                    listener.getVelocity().y,
                                    listener.getVelocity().z);

    const auto& cone = listener.getCone();
    ma_engine_listener_set_cone(&engine,
                                0,
                                clamp(cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                clamp(cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                cone.outerGain);

    ma_engine_listener_set_world_up(&engine,
                                    0,
                                    listener.getUpVector().x,
                                    listener.getUpVector().y,
                                    listener.getUpVector().z);

    return true;
}


////////////////////////////////////////////////////////////
AudioDevice::ResourceEntryIndex AudioDevice::registerResource(
    void*                       resource,
    ResourceEntry::InitFunc     deinitializeFunc,
    ResourceEntry::InitFunc     reinitializeFunc,
    ResourceEntry::TransferFunc transferFunc)
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    auto& resources = m_impl->resources;

    for (ResourceEntryIndex i = 0; i < resources.size(); ++i)
        if (!resources[i].has_value())
            return i;

    resources.emplace_back(std::in_place, resource, deinitializeFunc, reinitializeFunc, transferFunc);
    return resources.size() - 1;
}


////////////////////////////////////////////////////////////
void AudioDevice::unregisterResource(AudioDevice::ResourceEntryIndex resourceEntryIndex)
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    auto& resources = m_impl->resources;

    auto it = resources.begin() + static_cast<std::vector<ResourceEntry>::difference_type>(resourceEntryIndex);
    assert(it->has_value() && "Attempted to unregister previously erased audio resource");

    resources.erase(it);
}

} // namespace sf::priv
