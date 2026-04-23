// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/Listener.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"
#include "SFML/Audio/Priv/MiniaudioUtils.hpp"

#include "SFML/System/Angle.hpp"
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
        if (engineInitialized)
            ma_engine_uninit(&maEngine);

        if (deviceInitialized)
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

            // Set the period size to 64 frames worth of data
            // This value serves as a buffer size hint to the device driver but
            // will be clamped to stay within valid minimum and maximum values
            // The value also determines the rate at which data is pulled through
            // the audio node graph
            // Leaving this value at the default of 0 would instruct miniaudio to
            // automatically determine how much data it should attempt to pull
            // through the node graph every time new data is required
            // When playing very short audio clips with a low frame count the total
            // number of frames might not be enough to fill the allocated buffer
            // This is more likely the higher the device sample rate is due to a
            // bigger frame buffer being allocated to accomodate higher sample rate data
            // Testing shows that if the buffer cannot be entirely filled with data,
            // playing the short audio clip will lead to no audio being output until
            // enough data has been pulled through the node graph to fill an entire
            // buffer worth of data, subsequent playing of the audio clip after the
            // buffer has already been filled the first time plays without issues
            // In order to support playing short audio clips we therefore have to
            // explicitly set periodSizeInFrames to a low value to ensure that initial
            // data does not get stuck in the buffer which would lead to no output
            // Decreasing periodSizeInFrames technically does increase CPU load since
            // data is pulled through the node graph more frequently but empirical
            // testing shows that this additional overhead is not significant
            // If this does cause issues we would have to expose setting this value
            // through the public API so the developer can set it to an appropriate
            // value depending on the audio data they intend to play
            maDeviceConfig.periodSizeInFrames = 64;

            if (const ma_result result = ma_device_init(&maContext, &maDeviceConfig, &maDevice); result != MA_SUCCESS)
                return priv::MiniaudioUtils::fail("initialize the audio device", result);

            deviceInitialized = true;
        }

        // Initialize miniaudio engine
        {
            ma_engine_config engineConfig = ma_engine_config_init();

            engineConfig.pContext      = &maContext;
            engineConfig.pDevice       = &maDevice;
            engineConfig.listenerCount = 1;

            if (const ma_result result = ma_engine_init(&engineConfig, &maEngine); result != MA_SUCCESS)
            {
                ma_device_uninit(&maDevice);
                deviceInitialized = false;

                return priv::MiniaudioUtils::fail("initialize the audio engine", result);
            }

            engineInitialized = true;
        }

        return true;
    }

    PlaybackDeviceHandle playbackDeviceHandle; //!< Playback device handle, can be retieved from the playback device

    ma_device maDevice; //!< miniaudio playback device (one per hardware device)
    ma_engine maEngine; //!< miniaudio engine (one per hardware device, for effects/spatialization)

    bool deviceInitialized{false}; //!< Whether maDevice has been successfully initialized
    bool engineInitialized{false}; //!< Whether maEngine has been successfully initialized
};


////////////////////////////////////////////////////////////
// TODO P1: change to a factory returning `base::Optional<PlaybackDevice>` so a
//          failed device can never be observed by the caller.
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
[[nodiscard]] bool PlaybackDevice::applyListener(const Listener& listener)
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
unsigned int PlaybackDevice::getSampleRate() const
{
    return m_impl->maDevice.sampleRate;
}


////////////////////////////////////////////////////////////
void* PlaybackDevice::getMAEngine()
{
    return &m_impl->maEngine;
}

} // namespace sf
