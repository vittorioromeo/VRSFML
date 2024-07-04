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
#include <SFML/Audio/Listener.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/LifetimeDependant.hpp>
#include <SFML/System/UniquePtr.hpp>
#include <SFML/System/Vector3.hpp>

#include <miniaudio.h>

#include <mutex>
#include <optional>
#include <vector>

#include <cassert>


namespace sf
{
////////////////////////////////////////////////////////////
struct PlaybackDevice::Impl
{
    static void maDeviceDataCallback(ma_device* maDevice, void* output, const void*, ma_uint32 frameCount)
    {
        ma_engine& maEngine = *static_cast<ma_engine*>(maDevice->pUserData);

        if (const ma_result result = ma_engine_read_pcm_frames(&maEngine, output, frameCount, nullptr); result != MA_SUCCESS)
            priv::err() << "Failed to read PCM frames from audio engine: " << ma_result_description(result) << priv::errEndl;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Impl(AudioContext& theAudioContext,
                  // NOLINTNEXTLINE(modernize-pass-by-value)
                  const PlaybackDeviceHandle& playbackDeviceHandle) :
    audioContext(&theAudioContext),
    playbackDeviceHandle(playbackDeviceHandle)
    {
    }

    ~Impl()
    {
        ma_engine_uninit(&maEngine);
        ma_device_uninit(&maDevice);
    }

    [[nodiscard]] bool initialize()
    {
        auto& maContext = *static_cast<ma_context*>(audioContext->getMAContext());

        // Initialize miniaudio device
        {
            ma_device_config maDeviceConfig = ma_device_config_init(ma_device_type_playback);

            maDeviceConfig.dataCallback    = &maDeviceDataCallback;
            maDeviceConfig.pUserData       = &maEngine;
            maDeviceConfig.playback.format = ma_format_f32;
            maDeviceConfig.playback
                .pDeviceID = &static_cast<const ma_device_info*>(playbackDeviceHandle.getMADeviceInfo())->id;

            if (const ma_result result = ma_device_init(&maContext, &maDeviceConfig, &maDevice); result != MA_SUCCESS)
            {
                priv::err() << "Failed to initialize the audio device: " << ma_result_description(result) << priv::errEndl;
                return false;
            }
        }

        // Initialize miniaudio engine
        {
            ma_engine_config engineConfig = ma_engine_config_init();

            engineConfig.pContext      = &maContext;
            engineConfig.pDevice       = &maDevice;
            engineConfig.listenerCount = 1;

            if (const ma_result result = ma_engine_init(&engineConfig, &maEngine); result != MA_SUCCESS)
            {
                priv::err() << "Failed to initialize the audio engine: " << ma_result_description(result) << priv::errEndl;
                return false;
            }
        }

        return true;
    }

    AudioContext*        audioContext;         //!< TODO
    PlaybackDeviceHandle playbackDeviceHandle; //!< TODO

    std::vector<ResourceEntry> resources;      //!< Registered resources
    std::mutex                 resourcesMutex; //!< The mutex guarding the registered resources

    ma_device maDevice; //!< miniaudio playback device (one per hardware device)
    ma_engine maEngine; //!< miniaudio engine          (one per hardware device, for effects/spatialisation)
};


////////////////////////////////////////////////////////////
std::optional<PlaybackDevice> PlaybackDevice::createDefault(AudioContext& audioContext)
{
    std::optional defaultPlaybackDeviceHandle = audioContext.getDefaultPlaybackDeviceHandle();

    if (!defaultPlaybackDeviceHandle.has_value())
        return std::nullopt;

    return std::make_optional<PlaybackDevice>(audioContext, *defaultPlaybackDeviceHandle);
}


////////////////////////////////////////////////////////////
PlaybackDevice::PlaybackDevice(AudioContext& audioContext, const PlaybackDeviceHandle& playbackDeviceHandle) :
m_impl(priv::makeUnique<Impl>(audioContext, playbackDeviceHandle))
{
    if (!m_impl->initialize())
        priv::err() << "Failed to initialize the playback device" << priv::errEndl;

    SFML_UPDATE_LIFETIME_DEPENDANT(AudioContext, PlaybackDevice, m_impl->audioContext);
}


////////////////////////////////////////////////////////////
PlaybackDevice::~PlaybackDevice() = default;


////////////////////////////////////////////////////////////
PlaybackDevice::PlaybackDevice(PlaybackDevice&& rhs) noexcept = default;


////////////////////////////////////////////////////////////
PlaybackDevice& PlaybackDevice::operator=(PlaybackDevice&&) noexcept = default;


////////////////////////////////////////////////////////////
void PlaybackDevice::transferResourcesTo(PlaybackDevice& other)
{
    if (&other == this)
        return;

    const std::lock_guard lock(m_impl->resourcesMutex);

    // Deinitialize all audio resources from self
    for (ResourceEntry& entry : m_impl->resources)
    {
        // Skip inactive resources
        if (entry.resource == nullptr)
            continue;

        entry.deinitializeFunc(entry.resource);

        const ResourceEntryIndex otherEntryIndex = other.registerResource(entry.resource,
                                                                          entry.deinitializeFunc,
                                                                          entry.reinitializeFunc,
                                                                          entry.transferFunc);

        entry.transferFunc(entry.resource, other, otherEntryIndex);
        entry.reinitializeFunc(entry.resource);

        // Mark resource as inactive (can be recycled)
        entry.resource = nullptr;
    }
}


////////////////////////////////////////////////////////////
[[nodiscard]] const PlaybackDeviceHandle& PlaybackDevice::getDeviceHandle() const
{
    return m_impl->playbackDeviceHandle;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool PlaybackDevice::updateListener(const Listener& listener)
{
    ma_engine* engine = &m_impl->maEngine;

    // Set master volume, position, velocity, cone and world up vector
    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(engine), listener.getVolume() * 0.01f);
        result != MA_SUCCESS)
    {
        priv::err() << "Failed to set audio device master volume: " << ma_result_description(result) << priv::errEndl;
        return false;
    }

    ma_engine_listener_set_position(engine,
                                    0,
                                    listener.getPosition().x,
                                    listener.getPosition().y,
                                    listener.getPosition().z);

    ma_engine_listener_set_velocity(engine,
                                    0,
                                    listener.getVelocity().x,
                                    listener.getVelocity().y,
                                    listener.getVelocity().z);

    const auto& cone = listener.getCone();
    ma_engine_listener_set_cone(engine,
                                0,
                                priv::clamp(cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                priv::clamp(cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                                cone.outerGain);

    ma_engine_listener_set_world_up(engine,
                                    0,
                                    listener.getUpVector().x,
                                    listener.getUpVector().y,
                                    listener.getUpVector().z);

    return true;
}


////////////////////////////////////////////////////////////
PlaybackDevice::ResourceEntryIndex PlaybackDevice::registerResource(
    void*                       resource,
    ResourceEntry::InitFunc     deinitializeFunc,
    ResourceEntry::InitFunc     reinitializeFunc,
    ResourceEntry::TransferFunc transferFunc)
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    for (ResourceEntryIndex i = 0; i < m_impl->resources.size(); ++i)
    {
        // Skip active resources
        if (m_impl->resources[i].resource != nullptr)
            continue;

        // Reuse first inactive recyclable resource
        m_impl->resources[i] = {resource, deinitializeFunc, reinitializeFunc, transferFunc};
        return i;
    }

    // Add a new resource slot
    m_impl->resources.emplace_back(resource, deinitializeFunc, reinitializeFunc, transferFunc);
    return m_impl->resources.size() - 1;
}


////////////////////////////////////////////////////////////
void PlaybackDevice::unregisterResource(ResourceEntryIndex resourceEntryIndex)
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    assert(m_impl->resources.size() > resourceEntryIndex && //
           "Attempted to unregister audio resource with invalid index");

    assert(m_impl->resources[resourceEntryIndex].resource != nullptr && //
           "Attempted to unregister previously erased audio resource");

    // Mark resource as inactive (can be recycled)
    m_impl->resources[resourceEntryIndex].resource = nullptr;
}


////////////////////////////////////////////////////////////
void* PlaybackDevice::getMAEngine() const
{
    return &m_impl->maEngine;
}

} // namespace sf
