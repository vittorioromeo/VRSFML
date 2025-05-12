#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/Clamp.hpp"

#include <miniaudio.h>


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
    explicit Impl(
        // NOLINTNEXTLINE(modernize-pass-by-value)
        const PlaybackDeviceHandle& thePlaybackDeviceHandle) :
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
        auto& maContext = *static_cast<ma_context*>(sf::AudioContext::getMAContext());

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

    PlaybackDeviceHandle playbackDeviceHandle; //!< Playback device handle, can be retieved from the playback device

    ma_device maDevice; //!< miniaudio playback device (one per hardware device)
    ma_engine maEngine; //!< miniaudio engine (one per hardware device, for effects/spatialization)
};


////////////////////////////////////////////////////////////
PlaybackDevice::PlaybackDevice(const PlaybackDeviceHandle& playbackDeviceHandle) : m_impl(playbackDeviceHandle)
{
    if (!m_impl->initialize())
        priv::err() << "Failed to initialize the playback device";
}


////////////////////////////////////////////////////////////
PlaybackDevice::~PlaybackDevice() = default;


////////////////////////////////////////////////////////////
[[nodiscard]] const PlaybackDeviceHandle& PlaybackDevice::getDeviceHandle() const
{
    return m_impl->playbackDeviceHandle;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool PlaybackDevice::updateListener(const Listener& listener)
{
    ma_engine* engine = &m_impl->maEngine;

    // Set master volume, position, velocity, cone and world up vec
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

    ma_engine_listener_set_world_up(engine, 0, listener.upVec.x, listener.upVec.y, listener.upVec.z);

    return true;
}


////////////////////////////////////////////////////////////
const char* PlaybackDevice::getName() const
{
    return m_impl->playbackDeviceHandle.getName();
}


////////////////////////////////////////////////////////////
bool PlaybackDevice::isDefault() const
{
    return m_impl->playbackDeviceHandle.isDefault();
}


////////////////////////////////////////////////////////////
void* PlaybackDevice::getMAEngine()
{
    return &m_impl->maEngine;
}

} // namespace sf
