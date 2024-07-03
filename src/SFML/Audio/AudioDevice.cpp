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
    if (const ma_result result = ma_context_get_devices(&context, &deviceInfos, &deviceCount, nullptr, nullptr);
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
[[nodiscard]] std::optional<ma_device_id> getSelectedDeviceId(ma_context& maContext, const std::optional<std::string>& deviceName)
{
    const auto devices = getDevices(maContext);

    auto iter = priv::findIf(devices.begin(),
                             devices.end(),
                             [&](const auto& device) { return device.name == deviceName; });

    if (iter != devices.end())
        return iter->id;

    return std::nullopt;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool updateMiniaudioEnginePropertiesFromListenerProperties(
    ma_engine&                             engine,
    const AudioDevice::ListenerProperties& listenerProperties)
{
    // Set master volume, position, velocity, cone and world up vector
    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(&engine), listenerProperties.volume * 0.01f);
        result != MA_SUCCESS)
    {
        priv::err() << "Failed to set audio device master volume: " << ma_result_description(result) << priv::errEndl;
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
class MiniaudioGlobals
{
private:
    std::optional<ma_log>     m_maLog;     //!< miniaudio log (one per program)
    std::optional<ma_context> m_maContext; //!< miniaudio context (one per program)

    static void maLogCallback(void*, ma_uint32 level, const char* message)
    {
        if (level <= MA_LOG_LEVEL_WARNING)
            priv::err() << "miniaudio " << ma_log_level_to_string(level) << ": " << message << priv::errFlush;
    }

    [[nodiscard]] static std::optional<ma_log> tryCreateMALog()
    {
        std::optional<ma_log> log; // Use a single local variable for NRVO
        log.emplace();

        // Create the log
        if (const ma_result result = ma_log_init(nullptr, &*log); result != MA_SUCCESS)
        {
            priv::err() << "Failed to initialize the audio log: " << ma_result_description(result) << priv::errEndl;

            log.reset();
            return log; // Empty optional
        }

        // Register our logging callback to output any warning/error messages
        if (const ma_result result = ma_log_register_callback(&*log, ma_log_callback_init(&maLogCallback, nullptr));
            result != MA_SUCCESS)
        {
            priv::err() << "Failed to register audio log callback: " << ma_result_description(result) << priv::errEndl;

            log.reset();
            return log; // Empty optional
        }

        assert(log.has_value());
        return log;
    }

    [[nodiscard]] static std::optional<ma_context> tryCreateMAContext(ma_log& maLog)
    {
        std::optional<ma_context> context; // Use a single local variable for NRVO
        context.emplace();

        // Create the context
        auto contextConfig = ma_context_config_init();
        contextConfig.pLog = &maLog;

        ma_uint32 deviceCount = 0;

        const ma_backend  nullBackend = ma_backend_null;
        const ma_backend* backendLists[2]{nullptr, &nullBackend};

        for (const auto* backendList : backendLists)
        {
            // We can set backendCount to 1 since it is ignored when backends is set to nullptr
            if (const ma_result result = ma_context_init(backendList, 1, &contextConfig, &*context); result != MA_SUCCESS)
            {
                priv::err() << "Failed to initialize the audio playback context: " << ma_result_description(result)
                            << priv::errEndl;

                context.reset();
                return context; // Empty optional
            }

            // Count the playback devices
            if (const ma_result result = ma_context_get_devices(&*context, nullptr, &deviceCount, nullptr, nullptr);
                result != MA_SUCCESS)
            {
                priv::err() << "Failed to get audio playback devices: " << ma_result_description(result) << priv::errEndl;

                context.reset();
                return context; // Empty optional
            }

            // Check if there are audio playback devices available on the system
            if (deviceCount > 0)
                break;

            // Warn if no devices were found using the default backend list
            if (backendList == nullptr)
                priv::err() << "No audio playback devices available on the system" << priv::errEndl;

            // Clean up the context if we didn't find any devices (TODO: why?)
            ma_context_uninit(&*context);
        }

        // If the NULL audio backend also doesn't provide a device we give up
        if (deviceCount == 0)
        {
            context.reset();
            return context; // Empty optional
        }

        assert(context.has_value());
        return context;
    }

public:
    explicit MiniaudioGlobals() : m_maLog(tryCreateMALog()), m_maContext(tryCreateMAContext(*m_maLog))
    {
        if (m_maContext->backend == ma_backend_null)
            priv::err() << "Using NULL audio backend for playback" << priv::errEndl;
    }

    ~MiniaudioGlobals()
    {
        if (m_maContext.has_value())
            ma_context_uninit(&*m_maContext);

        if (m_maLog.has_value())
            ma_log_uninit(&*m_maLog);
    }

    [[nodiscard]] ma_context& getMAContext()
    {
        assert(m_maContext.has_value());
        return *m_maContext;
    }
};


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
                priv::err() << "Failed to read PCM frames from audio engine: " << ma_result_description(result)
                            << priv::errEndl;
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
            // currentDevice = std::nullopt;

            priv::err() << "Failed to initialize the audio playback device: " << ma_result_description(result)
                        << priv::errEndl;

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
            priv::err() << "Failed to initialize the audio engine: " << ma_result_description(result) << priv::errEndl;

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
    MiniaudioGlobals                    maGlobals;
    std::optional<MiniaudioPerHWDevice> maPerHWDevice;

    std::vector<std::optional<ResourceEntry>> resources;      //!< Registered resources
    std::mutex                                resourcesMutex; //!< The mutex guarding the registered resources

    ListenerProperties         listenerProperties; // !< TODO
    std::optional<std::string> currentDevice;      // !< TODO

    // Update the current device string from the the device we just initialized
    void updateDeviceName(ma_device& maDevice)
    {
        char        deviceName[MA_MAX_DEVICE_NAME_LENGTH + 1]{};
        std::size_t deviceNameLength{};

        if (const ma_result result = ma_device_get_name(&maDevice,
                                                        ma_device_type_playback,
                                                        deviceName,
                                                        priv::getArraySize(deviceName),
                                                        &deviceNameLength);
            result != MA_SUCCESS)
        {
            priv::err() << "Failed to get name of audio playback device: " << ma_result_description(result) << priv::errEndl;

            currentDevice = std::nullopt;
        }
        else
        {
            currentDevice = std::string(deviceName, deviceNameLength);
        }
    }
};


////////////////////////////////////////////////////////////
AudioDevice::AudioDevice()
{
    if (!initialize(m_impl->maGlobals.getMAContext())) // TODO: pass to ctor
        priv::err() << "Failed to initialize audio device or engine" << priv::errEndl;
}


////////////////////////////////////////////////////////////
AudioDevice::~AudioDevice() = default;


////////////////////////////////////////////////////////////
ma_engine& AudioDevice::getEngine()
{
    return m_impl->maPerHWDevice->getMAEngine();
}


////////////////////////////////////////////////////////////
bool AudioDevice::reinitialize(ma_context& context)
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    // Deinitialize all audio resources
    for (const auto& entry : m_impl->resources)
        if (entry.has_value())
            entry->deinitializeFunc(entry->resource);

    // Destroy the old device and engine
    m_impl->maPerHWDevice.reset();

    // Create the new objects
    const bool result = initialize(context);

    // Reinitialize all audio resources
    for (const auto& entry : m_impl->resources)
        if (entry.has_value())
            entry->reinitializeFunc(entry->resource);

    return result;
}


////////////////////////////////////////////////////////////
std::vector<AudioDevice::DeviceEntry> AudioDevice::getAvailableDevices()
{
    std::vector<AudioDevice::DeviceEntry> result;

    for (const auto& [name, id, isDefault] : getDevices(m_impl->maGlobals.getMAContext()))
        result.push_back({name, isDefault});

    return result;
}


////////////////////////////////////////////////////////////
bool AudioDevice::setDevice(const std::string& name)
{
    m_impl->currentDevice = name;
    return reinitialize(m_impl->maGlobals.getMAContext());
}


////////////////////////////////////////////////////////////
const std::optional<std::string>& AudioDevice::getDevice() const
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
    // Store the volume in case no audio device exists yet (TODO)
    m_impl->listenerProperties.volume = volume;

    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(&m_impl->maPerHWDevice->getMAEngine()),
                                                             volume * 0.01f);
        result != MA_SUCCESS)
        priv::err() << "Failed to set audio device master volume: " << ma_result_description(result) << priv::errEndl;
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
                                priv::clamp(cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                priv::clamp(cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
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
bool AudioDevice::initialize(ma_context& context)
{
    std::optional<std::string> selectedDeviceName = m_impl->currentDevice;

    if (!selectedDeviceName.has_value())
        selectedDeviceName = getDefaultDevice();

    const std::optional<ma_device_id> deviceId = getSelectedDeviceId(context, selectedDeviceName);

    // Create the device and engine
    m_impl->maPerHWDevice.emplace(context, deviceId);
    if (!m_impl->maPerHWDevice.has_value())
        return false;

    m_impl->updateDeviceName(m_impl->maPerHWDevice->getMADevice());

    if (!updateMiniaudioEnginePropertiesFromListenerProperties(m_impl->maPerHWDevice->getMAEngine(), m_impl->listenerProperties))
    {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        return false;
    }

    return true;
}

} // namespace sf::priv
