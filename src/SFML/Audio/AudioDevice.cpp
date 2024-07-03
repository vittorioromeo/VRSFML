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
#include <SFML/Audio/AudioDeviceHandle.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

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
////////////////////////////////////////////////////////////
struct AudioDevice::ListenerProperties
{
    float          volume{100.f};
    Vector3f       position{0, 0, 0};
    Vector3f       direction{0, 0, -1};
    Vector3f       velocity{0, 0, 0};
    Listener::Cone cone{degrees(360.f), degrees(360.f), 1};
    Vector3f       upVector{0, 1, 0};
};


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] bool updateMiniaudioEnginePropertiesFromListenerProperties(
    ma_engine&                             engine,
    const AudioDevice::ListenerProperties& listenerProperties)
{
    // Set master volume, position, velocity, cone and world up vector
    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(&engine), listenerProperties.volume * 0.01f);
        result != MA_SUCCESS)
    {
        err() << "Failed to set audio device master volume: " << ma_result_description(result) << errEndl;
        return false;
    }

    ma_engine_listener_set_position(&engine,
                                    0,
                                    listenerProperties.position.x,
                                    listenerProperties.position.y,
                                    listenerProperties.position.z);

    ma_engine_listener_set_velocity(&engine,
                                    0,
                                    listenerProperties.velocity.x,
                                    listenerProperties.velocity.y,
                                    listenerProperties.velocity.z);

    ma_engine_listener_set_cone(&engine,
                                0,
                                listenerProperties.cone.innerAngle.asRadians(),
                                listenerProperties.cone.outerAngle.asRadians(),
                                listenerProperties.cone.outerGain);

    ma_engine_listener_set_world_up(&engine,
                                    0,
                                    listenerProperties.upVector.x,
                                    listenerProperties.upVector.y,
                                    listenerProperties.upVector.z);

    return true;
}

} // namespace


////////////////////////////////////////////////////////////
class MiniaudioPerHWDevice
{
private:
    std::optional<ma_device> m_maDevice; //!< miniaudio playback device (one per device)
    std::optional<ma_engine> m_maEngine; //!< miniaudio engine (one per device, for effects/spatialisation)

    static void maDeviceDataCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount)
    {
        auto& impl = *static_cast<MiniaudioPerHWDevice*>(device->pUserData);

        if (impl.m_maEngine.has_value())
        {
            if (const ma_result result = ma_engine_read_pcm_frames(&*impl.m_maEngine, output, frameCount, nullptr);
                result != MA_SUCCESS)
            {
                err() << "Failed to read PCM frames from audio engine: " << ma_result_description(result) << errEndl;
            }
        }
    }

    [[nodiscard]] static std::optional<ma_device> tryCreateMADevice(ma_context&                       maContext,
                                                                    MiniaudioPerHWDevice&             self,
                                                                    const std::optional<ma_device_id> deviceId)
    {
        ma_device_config maDeviceConfig   = ma_device_config_init(ma_device_type_playback);
        maDeviceConfig.dataCallback       = &maDeviceDataCallback;
        maDeviceConfig.pUserData          = &self;
        maDeviceConfig.playback.format    = ma_format_f32;
        maDeviceConfig.playback.pDeviceID = deviceId ? &*deviceId : nullptr;

        std::optional<ma_device> device; // Use a single local variable for NRVO
        device.emplace();

        if (const ma_result result = ma_device_init(&maContext, &maDeviceConfig, &*device); result != MA_SUCCESS)
        {
            // TODO:
            // currentDeviceHandle = std::nullopt;

            err() << "Failed to initialize the audio playback device: " << ma_result_description(result) << errEndl;

            device.reset();
            return device; // Empty optional
        }

        assert(device.has_value());
        return device;
    }

    [[nodiscard]] static std::optional<ma_engine> tryCreateMAEngine(ma_context& maContext, ma_device& maDevice)
    {
        auto engineConfig          = ma_engine_config_init();
        engineConfig.pContext      = &maContext;
        engineConfig.pDevice       = &maDevice;
        engineConfig.listenerCount = 1;

        std::optional<ma_engine> engine; // Use a single local variable for NRVO
        engine.emplace();

        if (const ma_result result = ma_engine_init(&engineConfig, &*engine); result != MA_SUCCESS)
        {
            err() << "Failed to initialize the audio engine: " << ma_result_description(result) << errEndl;

            engine.reset();
            return engine; // Empty optional
        }

        assert(engine.has_value());
        return engine;
    }

public:
    explicit MiniaudioPerHWDevice(ma_context& maContext, const std::optional<ma_device_id> deviceId) :
    m_maDevice(tryCreateMADevice(maContext, *this, deviceId)),
    m_maEngine(tryCreateMAEngine(maContext, *m_maDevice))
    {
    }

    ~MiniaudioPerHWDevice()
    {
        if (m_maEngine.has_value())
            ma_engine_uninit(&*m_maEngine);

        if (m_maDevice.has_value())
            ma_device_uninit(&*m_maDevice);
    }

    [[nodiscard]] ma_device& getMADevice()
    {
        assert(m_maDevice.has_value());
        return *m_maDevice;
    }

    [[nodiscard]] ma_engine& getMAEngine()
    {
        assert(m_maEngine.has_value());
        return *m_maEngine;
    }
};


////////////////////////////////////////////////////////////
struct AudioDevice::Impl
{
    PlaybackDevice*                     playbackDevice;
    AudioContext*                       audioContext;
    std::optional<MiniaudioPerHWDevice> maPerHWDevice;

    std::vector<std::optional<ResourceEntry>> resources;      //!< Registered resources
    std::mutex                                resourcesMutex; //!< The mutex guarding the registered resources

    ListenerProperties               listenerProperties; // !< TODO
    std::optional<AudioDeviceHandle> deviceHandle;       // !< TODO

    explicit Impl(PlaybackDevice& thePlaybackDevice, AudioContext& theAudioContext, AudioDeviceHandle theDeviceHandle) :
    playbackDevice(&thePlaybackDevice),
    audioContext(&theAudioContext),
    deviceHandle(theDeviceHandle)
    {
    }
};


////////////////////////////////////////////////////////////
AudioDevice::AudioDevice(PlaybackDevice& playbackDevice, AudioContext& audioContext, const AudioDeviceHandle& deviceHandle) :
m_impl(playbackDevice, audioContext, deviceHandle)
{
    if (!initialize(m_impl->audioContext->getMAContext(), deviceHandle))
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
AudioDevice::ResourceEntryIndex AudioDevice::registerResource(
    void*                       resource,
    ResourceEntry::InitFunc     deinitializeFunc,
    ResourceEntry::InitFunc     reinitializeFunc,
    ResourceEntry::TransferFunc transferFunc)
{
    // There should always be an AudioDevice instance when registerResource is called
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
    // There should always be an AudioDevice instance when unregisterResource is called
    const std::lock_guard lock(m_impl->resourcesMutex);

    auto& resources = m_impl->resources;

    auto it = resources.begin() + static_cast<std::vector<ResourceEntry>::difference_type>(resourceEntryIndex);
    assert(it->has_value() && "Attempted to unregister previously erased audio resource");

    resources.erase(it);
}


////////////////////////////////////////////////////////////
void AudioDevice::setGlobalVolume(float volume)
{
    // Store the volume in case no audio device exists yet (TODO)
    m_impl->listenerProperties.volume = volume;

    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(&m_impl->maPerHWDevice->getMAEngine()),
                                                             volume * 0.01f);
        result != MA_SUCCESS)
        err() << "Failed to set audio device master volume: " << ma_result_description(result) << errEndl;
}


////////////////////////////////////////////////////////////
float AudioDevice::getGlobalVolume() const
{
    return m_impl->listenerProperties.volume;
}


////////////////////////////////////////////////////////////
void AudioDevice::setPosition(const Vector3f& position)
{
    // Store the position in case no audio device exists yet (TODO)
    m_impl->listenerProperties.position = position;

    ma_engine_listener_set_position(&m_impl->maPerHWDevice->getMAEngine(), 0, position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getPosition() const
{
    return m_impl->listenerProperties.position;
}


////////////////////////////////////////////////////////////
void AudioDevice::setDirection(const Vector3f& direction)
{
    // Store the direction in case no audio device exists yet (TODO)
    m_impl->listenerProperties.direction = direction;

    ma_engine_listener_set_direction(&m_impl->maPerHWDevice->getMAEngine(), 0, direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getDirection() const
{
    return m_impl->listenerProperties.direction;
}


////////////////////////////////////////////////////////////
void AudioDevice::setVelocity(const Vector3f& velocity)
{
    // Store the velocity in case no audio device exists yet (TODO)
    m_impl->listenerProperties.velocity = velocity;

    ma_engine_listener_set_velocity(&m_impl->maPerHWDevice->getMAEngine(), 0, velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getVelocity() const
{
    return m_impl->listenerProperties.velocity;
}


////////////////////////////////////////////////////////////
void AudioDevice::setCone(const Listener::Cone& cone)
{
    // Store the cone in case no audio device exists yet (TODO)
    m_impl->listenerProperties.cone = cone;

    ma_engine_listener_set_cone(&m_impl->maPerHWDevice->getMAEngine(),
                                0,
                                clamp(cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                clamp(cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                cone.outerGain);
}


////////////////////////////////////////////////////////////
Listener::Cone AudioDevice::getCone() const
{
    return m_impl->listenerProperties.cone;
}


////////////////////////////////////////////////////////////
void AudioDevice::setUpVector(const Vector3f& upVector)
{
    // Store the up vector in case no audio device exists yet (TODO)
    m_impl->listenerProperties.upVector = upVector;

    ma_engine_listener_set_world_up(&m_impl->maPerHWDevice->getMAEngine(), 0, upVector.x, upVector.y, upVector.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getUpVector() const
{
    return m_impl->listenerProperties.upVector;
}


////////////////////////////////////////////////////////////
bool AudioDevice::initialize(ma_context& maContext, const AudioDeviceHandle& selectedDeviceHandle)
{
    ma_device_info deviceInfo;
    selectedDeviceHandle.copyMADeviceInfoInto(&deviceInfo);

    // Create the device and engine
    m_impl->maPerHWDevice.emplace(maContext, deviceInfo.id);
    if (!m_impl->maPerHWDevice.has_value())
        return false;

    //  m_impl->updateCurrentDeviceHandle(m_impl->maPerHWDevice->getMADevice());

    if (!updateMiniaudioEnginePropertiesFromListenerProperties(m_impl->maPerHWDevice->getMAEngine(), m_impl->listenerProperties))
    {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        return false;
    }

    return true;
}

} // namespace sf::priv
