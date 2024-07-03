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
#include <SFML/System/Err.hpp>
#include <SFML/System/Vector3.hpp>

#include <miniaudio.h>

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <cassert>

namespace sf::priv
{
namespace
{
////////////////////////////////////////////////////////////
struct DeviceEntryImpl
{
    std::string  name;
    ma_device_id id{};
    bool         isDefault{};
};


////////////////////////////////////////////////////////////
std::vector<DeviceEntryImpl> getDevices(ma_context& context)
{
    std::vector<DeviceEntryImpl> deviceList; // Use a single local variable for NRVO

    ma_device_info* deviceInfos{};
    ma_uint32       deviceCount{};

    // Get the playback devices
    if (const auto result = ma_context_get_devices(&context, &deviceInfos, &deviceCount, nullptr, nullptr);
        result != MA_SUCCESS)
    {
        priv::err() << "Failed to get audio playback devices: " << ma_result_description(result) << priv::errEndl;
        return deviceList; // Empty device list
    }

    deviceList.reserve(deviceCount);

    // In order to report devices with identical names and still allow
    // the user to differentiate between them when selecting, we append
    // an index (number) to their name starting from the second entry
    std::unordered_map<std::string, int> deviceIndices;
    deviceIndices.reserve(deviceCount);

    for (auto i = 0u; i < deviceCount; ++i)
    {
        auto  name  = std::string(deviceInfos[i].name);
        auto& index = deviceIndices[name];

        ++index;

        if (index > 1)
            name += ' ' + std::to_string(index);

        // Make sure the default device is always placed at the front
        deviceList.emplace(deviceInfos[i].isDefault ? deviceList.begin() : deviceList.end(),
                           DeviceEntryImpl{name, deviceInfos[i].id, deviceInfos[i].isDefault == MA_TRUE});
    }

    return deviceList;
}


////////////////////////////////////////////////////////////
[[nodiscard]] std::vector<DeviceEntryImpl> getAvailableDevicesImpl(ma_context* instanceContext)
{
    std::vector<DeviceEntryImpl> deviceList; // Use a single local variable for NRVO

    // Use an existing instance's context if one exists
    if (instanceContext != nullptr)
    {
        deviceList = getDevices(*instanceContext);
        return deviceList;
    }

    // Otherwise, construct a temporary context
    ma_context context{};

    if (const auto result = ma_context_init(nullptr, 0, nullptr, &context); result != MA_SUCCESS)
    {
        priv::err() << "Failed to initialize the audio playback context: " << ma_result_description(result) << priv::errEndl;
        return deviceList; // Empty vector
    }

    deviceList = getDevices(context);
    ma_context_uninit(&context);

    return deviceList;
}


////////////////////////////////////////////////////////////
[[nodiscard]] std::optional<ma_device_id> getSelectedDeviceId(ma_context*                       instanceContext,
                                                              const std::optional<std::string>& deviceName)
{
    const auto devices = getAvailableDevicesImpl(instanceContext);

    auto iter = priv::findIf(devices.begin(),
                             devices.end(),
                             [&](const auto& device) { return device.name == deviceName; });

    if (iter != devices.end())
        return iter->id;

    return std::nullopt;
}

} // namespace


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


////////////////////////////////////////////////////////////
struct AudioDevice::Impl
{
    std::optional<ma_log>                     log;            //!< The miniaudio log
    std::optional<ma_context>                 context;        //!< The miniaudio context
    std::optional<ma_device>                  playbackDevice; //!< The miniaudio playback device
    std::optional<ma_engine>                  engine;    //!< The miniaudio engine (used for effects and spatialisation)
    std::vector<std::optional<ResourceEntry>> resources; //!< Registered resources
    std::mutex                                resourcesMutex;     //!< The mutex guarding the registered resources
    ListenerProperties                        listenerProperties; // !< TODO
    std::optional<std::string>                currentDevice;      // !< TODO
};


////////////////////////////////////////////////////////////
AudioDevice::AudioDevice()
{
    // Create the log
    m_impl->log.emplace();

    if (const auto result = ma_log_init(nullptr, &*m_impl->log); result != MA_SUCCESS)
    {
        m_impl->log.reset();
        priv::err() << "Failed to initialize the audio log: " << ma_result_description(result) << priv::errEndl;
        return;
    }

    // Register our logging callback to output any warning/error messages
    if (const auto result = ma_log_register_callback(&*m_impl->log,
                                                     ma_log_callback_init(
                                                         [](void*, ma_uint32 level, const char* message)
                                                         {
                                                             if (level <= MA_LOG_LEVEL_WARNING)
                                                                 priv::err()
                                                                     << "miniaudio " << ma_log_level_to_string(level)
                                                                     << ": " << message << priv::errFlush;
                                                         },
                                                         nullptr));
        result != MA_SUCCESS)
        priv::err() << "Failed to register audio log callback: " << ma_result_description(result) << priv::errEndl;

    // Create the context
    m_impl->context.emplace();

    auto contextConfig            = ma_context_config_init();
    contextConfig.pLog            = &*m_impl->log;
    ma_uint32         deviceCount = 0;
    const auto        nullBackend = ma_backend_null;
    const ma_backend* backendLists[2]{nullptr, &nullBackend};

    for (const auto* backendList : backendLists)
    {
        // We can set backendCount to 1 since it is ignored when backends is set to nullptr
        if (const auto result = ma_context_init(backendList, 1, &contextConfig, &*m_impl->context); result != MA_SUCCESS)
        {
            m_impl->context.reset();
            priv::err() << "Failed to initialize the audio playback context: " << ma_result_description(result)
                        << priv::errEndl;
            return;
        }

        // Count the playback devices
        if (const auto result = ma_context_get_devices(&*m_impl->context, nullptr, &deviceCount, nullptr, nullptr);
            result != MA_SUCCESS)
        {
            priv::err() << "Failed to get audio playback devices: " << ma_result_description(result) << priv::errEndl;
            return;
        }

        // Check if there are audio playback devices available on the system
        if (deviceCount > 0)
            break;

        // Warn if no devices were found using the default backend list
        if (backendList == nullptr)
            priv::err() << "No audio playback devices available on the system" << priv::errEndl;

        // Clean up the context if we didn't find any devices
        ma_context_uninit(&*m_impl->context);
    }

    // If the NULL audio backend also doesn't provide a device we give up
    if (deviceCount == 0)
    {
        m_impl->context.reset();
        return;
    }

    if (m_impl->context->backend == ma_backend_null)
        priv::err() << "Using NULL audio backend for playback" << priv::errEndl;

    if (!initialize())
        priv::err() << "Failed to initialize audio device or engine" << priv::errEndl;
}


////////////////////////////////////////////////////////////
AudioDevice::~AudioDevice()
{
    // Destroy the engine
    if (m_impl->engine)
        ma_engine_uninit(&*m_impl->engine);

    // Destroy the playback device
    if (m_impl->playbackDevice)
        ma_device_uninit(&*m_impl->playbackDevice);

    // Destroy the context
    if (m_impl->context)
        ma_context_uninit(&*m_impl->context);

    // Destroy the log
    if (m_impl->log)
        ma_log_uninit(&*m_impl->log);
}


////////////////////////////////////////////////////////////
ma_engine* AudioDevice::getEngine()
{
    if (m_impl->engine.has_value())
        return &*m_impl->engine;

    return nullptr;
}


////////////////////////////////////////////////////////////
bool AudioDevice::reinitialize()
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    // Deinitialize all audio resources
    for (const auto& entry : m_impl->resources)
        if (entry.has_value())
            entry->deinitializeFunc(entry->resource);

    // Destroy the old engine
    if (m_impl->engine.has_value())
        ma_engine_uninit(&*m_impl->engine);

    // Destroy the old playback device
    if (m_impl->playbackDevice.has_value())
        ma_device_uninit(&*m_impl->playbackDevice);

    // Create the new objects
    const auto result = initialize();

    // Reinitialize all audio resources
    for (const auto& entry : m_impl->resources)
        if (entry.has_value())
            entry->reinitializeFunc(entry->resource);

    return result;
}


////////////////////////////////////////////////////////////
std::vector<AudioDevice::DeviceEntry> AudioDevice::getAvailableDevices()
{
    // Use an existing instance's context if one exists
    auto* context = m_impl->context.has_value() ? &*m_impl->context : nullptr;

    std::vector<AudioDevice::DeviceEntry> result;

    for (const DeviceEntryImpl& deviceEntryImpl : getAvailableDevicesImpl(context))
        result.push_back({deviceEntryImpl.name, deviceEntryImpl.isDefault});

    return result;
}


////////////////////////////////////////////////////////////
bool AudioDevice::setDevice(const std::string& name)
{
    m_impl->currentDevice = name;
    return reinitialize();
}


////////////////////////////////////////////////////////////
std::optional<std::string> AudioDevice::getDevice()
{
    return m_impl->currentDevice;
}


////////////////////////////////////////////////////////////
std::optional<std::string> AudioDevice::getDefaultDevice()
{
    for (const auto& [name, isDefault] : getAvailableDevices())
    {
        if (isDefault)
            return std::make_optional(name);
    }

    return std::nullopt;
}


////////////////////////////////////////////////////////////
AudioDevice::ResourceEntryIndex AudioDevice::registerResource(void*               resource,
                                                              ResourceEntry::Func deinitializeFunc,
                                                              ResourceEntry::Func reinitializeFunc)
{
    // There should always be an AudioDevice instance when registerResource is called
    const std::lock_guard lock(m_impl->resourcesMutex);

    auto& resources = m_impl->resources;

    for (ResourceEntryIndex i = 0; i < resources.size(); ++i)
        if (!resources[i].has_value())
            return i;

    resources.emplace_back(ResourceEntry{resource, deinitializeFunc, reinitializeFunc});
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
    // Store the volume in case no audio device exists yet
    m_impl->listenerProperties.volume = volume;

    if (!m_impl->engine.has_value())
        return;

    if (const auto result = ma_device_set_master_volume(ma_engine_get_device(&*m_impl->engine), volume * 0.01f);
        result != MA_SUCCESS)
        priv::err() << "Failed to set audio device master volume: " << ma_result_description(result) << priv::errEndl;
}


////////////////////////////////////////////////////////////
float AudioDevice::getGlobalVolume()
{
    return m_impl->listenerProperties.volume;
}


////////////////////////////////////////////////////////////
void AudioDevice::setPosition(const Vector3f& position)
{
    // Store the position in case no audio device exists yet
    m_impl->listenerProperties.position = position;

    if (!m_impl->engine.has_value())
        return;

    ma_engine_listener_set_position(&*m_impl->engine, 0, position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getPosition()
{
    return m_impl->listenerProperties.position;
}


////////////////////////////////////////////////////////////
void AudioDevice::setDirection(const Vector3f& direction)
{
    // Store the direction in case no audio device exists yet
    m_impl->listenerProperties.direction = direction;

    if (!m_impl->engine.has_value())
        return;

    ma_engine_listener_set_direction(&*m_impl->engine, 0, direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getDirection()
{
    return m_impl->listenerProperties.direction;
}


////////////////////////////////////////////////////////////
void AudioDevice::setVelocity(const Vector3f& velocity)
{
    // Store the velocity in case no audio device exists yet
    m_impl->listenerProperties.velocity = velocity;

    if (!m_impl->engine.has_value())
        return;

    ma_engine_listener_set_velocity(&*m_impl->engine, 0, velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getVelocity()
{
    return m_impl->listenerProperties.velocity;
}


////////////////////////////////////////////////////////////
void AudioDevice::setCone(const Listener::Cone& cone)
{
    // Store the cone in case no audio device exists yet
    m_impl->listenerProperties.cone = cone;

    if (!m_impl->engine.has_value())
        return;

    ma_engine_listener_set_cone(&*m_impl->engine,
                                0,
                                priv::clamp(cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                priv::clamp(cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                cone.outerGain);
}


////////////////////////////////////////////////////////////
Listener::Cone AudioDevice::getCone()
{
    return m_impl->listenerProperties.cone;
}


////////////////////////////////////////////////////////////
void AudioDevice::setUpVector(const Vector3f& upVector)
{
    // Store the up vector in case no audio device exists yet
    m_impl->listenerProperties.upVector = upVector;

    if (!m_impl->engine.has_value())
        return;

    ma_engine_listener_set_world_up(&*m_impl->engine, 0, upVector.x, upVector.y, upVector.z);
}


////////////////////////////////////////////////////////////
Vector3f AudioDevice::getUpVector()
{
    return m_impl->listenerProperties.upVector;
}


////////////////////////////////////////////////////////////
bool AudioDevice::initialize()
{
    std::optional<std::string> selectedDeviceName = m_impl->currentDevice;

    if (!selectedDeviceName.has_value())
        selectedDeviceName = getDefaultDevice();

    const auto deviceId = getSelectedDeviceId(m_impl->context.has_value() ? &*m_impl->context : nullptr, selectedDeviceName);

    // Create the playback device
    m_impl->playbackDevice.emplace();

    auto playbackDeviceConfig         = ma_device_config_init(ma_device_type_playback);
    playbackDeviceConfig.dataCallback = [](ma_device* device, void* output, const void*, ma_uint32 frameCount)
    {
        auto& audioDevice = *static_cast<AudioDevice*>(device->pUserData);

        if (audioDevice.m_impl->engine)
        {
            if (const auto result = ma_engine_read_pcm_frames(&*audioDevice.m_impl->engine, output, frameCount, nullptr);
                result != MA_SUCCESS)
                priv::err() << "Failed to read PCM frames from audio engine: " << ma_result_description(result)
                            << priv::errEndl;
        }
    };
    playbackDeviceConfig.pUserData          = this;
    playbackDeviceConfig.playback.format    = ma_format_f32;
    playbackDeviceConfig.playback.pDeviceID = deviceId ? &*deviceId : nullptr;

    if (const auto result = ma_device_init(&*m_impl->context, &playbackDeviceConfig, &*m_impl->playbackDevice);
        result != MA_SUCCESS)
    {
        m_impl->playbackDevice.reset();
        m_impl->currentDevice = std::nullopt;
        priv::err() << "Failed to initialize the audio playback device: " << ma_result_description(result) << priv::errEndl;
        return false;
    }

    // Update the current device string from the the device we just initialized
    {
        char        deviceName[MA_MAX_DEVICE_NAME_LENGTH + 1]{};
        std::size_t deviceNameLength{};

        if (const auto result = ma_device_get_name(&*m_impl->playbackDevice,
                                                   ma_device_type_playback,
                                                   deviceName,
                                                   priv::getArraySize(deviceName),
                                                   &deviceNameLength);
            result != MA_SUCCESS)
        {
            priv::err() << "Failed to get name of audio playback device: " << ma_result_description(result) << priv::errEndl;
            m_impl->currentDevice = std::nullopt;
        }
        else
        {
            m_impl->currentDevice = std::string(deviceName, deviceNameLength);
        }
    }

    // Create the engine
    auto engineConfig          = ma_engine_config_init();
    engineConfig.pContext      = &*m_impl->context;
    engineConfig.pDevice       = &*m_impl->playbackDevice;
    engineConfig.listenerCount = 1;

    m_impl->engine.emplace();

    if (const auto result = ma_engine_init(&engineConfig, &*m_impl->engine); result != MA_SUCCESS)
    {
        m_impl->engine.reset();
        priv::err() << "Failed to initialize the audio engine: " << ma_result_description(result) << priv::errEndl;
        return false;
    }

    // Set master volume, position, velocity, cone and world up vector
    if (const auto result = ma_device_set_master_volume(ma_engine_get_device(&*m_impl->engine),
                                                        m_impl->listenerProperties.volume * 0.01f);
        result != MA_SUCCESS)
        priv::err() << "Failed to set audio device master volume: " << ma_result_description(result) << priv::errEndl;

    ma_engine_listener_set_position(&*m_impl->engine,
                                    0,
                                    m_impl->listenerProperties.position.x,
                                    m_impl->listenerProperties.position.y,
                                    m_impl->listenerProperties.position.z);

    ma_engine_listener_set_velocity(&*m_impl->engine,
                                    0,
                                    m_impl->listenerProperties.velocity.x,
                                    m_impl->listenerProperties.velocity.y,
                                    m_impl->listenerProperties.velocity.z);

    ma_engine_listener_set_cone(&*m_impl->engine,
                                0,
                                m_impl->listenerProperties.cone.innerAngle.asRadians(),
                                m_impl->listenerProperties.cone.outerAngle.asRadians(),
                                m_impl->listenerProperties.cone.outerGain);

    ma_engine_listener_set_world_up(&*m_impl->engine,
                                    0,
                                    m_impl->listenerProperties.upVector.x,
                                    m_impl->listenerProperties.upVector.y,
                                    m_impl->listenerProperties.upVector.z);

    return true;
}

} // namespace sf::priv
