#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioContextUtils.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/LifetimeDependant.hpp"
#include "SFML/System/Vector3.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/TrivialVector.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <miniaudio.h>

#include <mutex>


namespace sf
{
////////////////////////////////////////////////////////////
struct PlaybackDevice::Impl
{
    static void maDeviceDataCallback(ma_device* maDevice, void* output, const void*, ma_uint32 frameCount)
    {
        ma_engine& maEngine = *static_cast<ma_engine*>(maDevice->pUserData);

        if (const ma_result result = ma_engine_read_pcm_frames(&maEngine, output, frameCount, nullptr); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("read PCM frames from audio engine", result);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Impl(AudioContext& theAudioContext,
                  // NOLINTNEXTLINE(modernize-pass-by-value)
                  const PlaybackDeviceHandle& thePlaybackDeviceHandle) :
    audioContext(&theAudioContext),
    playbackDeviceHandle(thePlaybackDeviceHandle)
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
                return priv::MiniaudioUtils::fail("initialize the audio device", result);
        }

        // Initialize miniaudio engine
        {
            ma_engine_config engineConfig = ma_engine_config_init();

            engineConfig.pContext      = &maContext;
            engineConfig.pDevice       = &maDevice;
            engineConfig.listenerCount = 1;

            if (const ma_result result = ma_engine_init(&engineConfig, &maEngine); result != MA_SUCCESS)
                return priv::MiniaudioUtils::fail("initialize the audio engine", result);
        }

        return true;
    }

    AudioContext*        audioContext; //!< The audio context (used to get the MA context and for lifetime tracking)
    PlaybackDeviceHandle playbackDeviceHandle; //!< Playback device handle, can be retieved from the playback device

    base::TrivialVector<ResourceEntry> resources;      //!< Registered resources
    std::mutex                         resourcesMutex; //!< The mutex guarding the registered resources

    ma_device maDevice; //!< miniaudio playback device (one per hardware device)
    ma_engine maEngine; //!< miniaudio engine (one per hardware device, for effects/spatialization)
};


////////////////////////////////////////////////////////////
base::Optional<PlaybackDevice> PlaybackDevice::createDefault(AudioContext& audioContext)
{
    base::Optional defaultPlaybackDeviceHandle = AudioContextUtils::getDefaultPlaybackDeviceHandle(audioContext);

    if (!defaultPlaybackDeviceHandle.hasValue())
        return base::nullOpt;

    return base::makeOptional<PlaybackDevice>(audioContext, *defaultPlaybackDeviceHandle);
}


////////////////////////////////////////////////////////////
PlaybackDevice::PlaybackDevice(AudioContext& audioContext, const PlaybackDeviceHandle& playbackDeviceHandle) :
m_impl(base::makeUnique<Impl>(audioContext, playbackDeviceHandle))
{
    if (!m_impl->initialize())
        priv::err() << "Failed to initialize the playback device";

    SFML_UPDATE_LIFETIME_DEPENDANT(AudioContext, PlaybackDevice, this, m_impl->audioContext);
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
    if (const ma_result result = ma_device_set_master_volume(ma_engine_get_device(engine), listener.volume);
        result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("set audio device master volume", result);
        return false;
    }

    ma_engine_listener_set_position(engine, 0, listener.position.x, listener.position.y, listener.position.z);
    ma_engine_listener_set_velocity(engine, 0, listener.velocity.x, listener.velocity.y, listener.velocity.z);

    ma_engine_listener_set_cone(engine,
                                0,
                                base::clamp(listener.cone.innerAngle, Angle::Zero, Angle::Full).asRadians(),
                                base::clamp(listener.cone.outerAngle, Angle::Zero, Angle::Full).asRadians(),
                                listener.cone.outerGain);

    ma_engine_listener_set_world_up(engine, 0, listener.upVector.x, listener.upVector.y, listener.upVector.z);

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

    for (ResourceEntryIndex i = 0; i < static_cast<PlaybackDevice::ResourceEntryIndex>(m_impl->resources.size()); ++i)
    {
        // Skip active resources
        if (m_impl->resources[i].resource != nullptr)
            continue;

        // Reuse first inactive recyclable resource
        m_impl->resources[i] = {resource, deinitializeFunc, reinitializeFunc, transferFunc};
        return i;
    }

    // Add a new resource slot
    m_impl->resources.emplaceBack(resource, deinitializeFunc, reinitializeFunc, transferFunc);
    return static_cast<PlaybackDevice::ResourceEntryIndex>(m_impl->resources.size()) - 1;
}


////////////////////////////////////////////////////////////
void PlaybackDevice::unregisterResource(ResourceEntryIndex resourceEntryIndex)
{
    const std::lock_guard lock(m_impl->resourcesMutex);

    SFML_BASE_ASSERT(m_impl->resources.size() > resourceEntryIndex && //
                     "Attempted to unregister audio resource with invalid index");

    SFML_BASE_ASSERT(m_impl->resources[resourceEntryIndex].resource != nullptr && //
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
