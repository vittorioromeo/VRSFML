#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/Vector.hpp"

#include <miniaudio.h>

#include <atomic>


namespace
{
////////////////////////////////////////////////////////////
void maLogCallback(void*, ma_uint32 level, const char* message)
{
    if (level <= MA_LOG_LEVEL_WARNING)
        sf::priv::err() << "miniaudio " << ma_log_level_to_string(level) << ": " << message << sf::priv::errFlush;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool tryCreateMALog(ma_log& log)
{
    // Create the log
    if (const ma_result result = ma_log_init(nullptr, &log); result != MA_SUCCESS)
        return sf::priv::MiniaudioUtils::fail("initialize the audio log", result);

    // Register our logging callback to output any warning/error messages
    if (const ma_result result = ma_log_register_callback(&log, ma_log_callback_init(&maLogCallback, nullptr));
        result != MA_SUCCESS)
        return sf::priv::MiniaudioUtils::fail("register audio log callback", result);

    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool tryCreateMAContext(ma_log& maLog, ma_context& maContext)
{
    // Create the context
    auto contextConfig = ma_context_config_init();
    contextConfig.pLog = &maLog;

    ma_uint32 deviceCount = 0;

    const ma_backend  nullBackend = ma_backend_null;
    const ma_backend* backendLists[2]{nullptr, &nullBackend};

    for (const auto* backendList : backendLists)
    {
        // We can set backendCount to 1 since it is ignored when backends is set to nullptr
        if (const ma_result result = ma_context_init(backendList, 1, &contextConfig, &maContext); result != MA_SUCCESS)
            return sf::priv::MiniaudioUtils::fail("initialize the audio playback", result);

        // Count the playback devices
        if (const ma_result result = ma_context_get_devices(&maContext, nullptr, &deviceCount, nullptr, nullptr);
            result != MA_SUCCESS)
            return sf::priv::MiniaudioUtils::fail("get audio playback devices", result);

        // Check if there are audio playback devices available on the system
        if (deviceCount > 0)
            break;

        // Warn if no devices were found using the default backend list
        if (backendList == nullptr)
            sf::priv::err() << "No audio playback devices available on the system";

        // Clean up the context if we didn't find any devices (TODO P1: why?)
        ma_context_uninit(&maContext);
    }

    // If the NULL audio backend also doesn't provide a device we give up
    if (deviceCount == 0)
    {
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
template <typename THandle, typename F>
sf::base::Vector<THandle> getAvailableDeviceHandles(sf::base::PassKey<sf::AudioContext>&& passKey,
                                                    ma_context&                           maContext,
                                                    const char*                           type,
                                                    F&&                                   fMAContextGetDevices)
{
    sf::base::Vector<THandle> deviceHandles; // Use a single local variable for NRVO

    ma_device_info* maDeviceInfosPtr{};
    ma_uint32       maDeviceInfoCount{};

    // Get the Capture devices
    if (const ma_result result = fMAContextGetDevices(&maContext, &maDeviceInfosPtr, &maDeviceInfoCount);
        result != MA_SUCCESS)
    {
        sf::priv::err() << "Failed to get audio " << type << " devices: " << ma_result_description(result);

        return deviceHandles; // Empty device handle vector
    }

    deviceHandles.reserve(maDeviceInfoCount);

    for (ma_uint32 i = 0u; i < maDeviceInfoCount; ++i)
        deviceHandles.emplaceBack(SFML_BASE_MOVE(passKey), &maDeviceInfosPtr[i]);

    return deviceHandles;
}


////////////////////////////////////////////////////////////
struct AudioContextImpl
{
    ma_log     maLog;     //!< miniaudio log (one per program)
    ma_context maContext; //!< miniaudio context (one per program)

    ~AudioContextImpl()
    {
        ma_context_uninit(&maContext);
        ma_log_uninit(&maLog);
    }
};


////////////////////////////////////////////////////////////
constinit sf::base::Optional<AudioContextImpl> installedAudioContext;
constinit std::atomic<unsigned int>            audioContextRC{0u};


////////////////////////////////////////////////////////////
AudioContextImpl& ensureInstalled()
{
    if (!installedAudioContext.hasValue()) [[unlikely]]
    {
        sf::priv::err() << "`sf::AudioContext` not installed -- did you forget to create one in `main`?";
        sf::base::abort();
    }

    return *installedAudioContext;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
base::Optional<AudioContext> AudioContext::create()
{
    const auto fail = [](const char* what)
    {
        priv::err() << "Error creating `sf::AudioContext`: " << what;
        return base::nullOpt;
    };

    //
    // Ensure audio context is not already installed
    if (installedAudioContext.hasValue())
        return fail("an `sf::AudioContext` object already exists");

    auto& ac = installedAudioContext.emplace();

    if (!tryCreateMALog(ac.maLog))
        return base::nullOpt; // Error message generated in called function.

    if (!tryCreateMAContext(ac.maLog, ac.maContext))
        return base::nullOpt; // Error message generated in called function.

    return base::makeOptional<AudioContext>(base::PassKey<AudioContext>{});
}


////////////////////////////////////////////////////////////
void* AudioContext::getMAContext()
{
    return &ensureInstalled().maContext;
}


////////////////////////////////////////////////////////////
AudioContext::AudioContext(base::PassKey<AudioContext>&&)
{
    audioContextRC.fetch_add(1u, std::memory_order::relaxed);
}


////////////////////////////////////////////////////////////
AudioContext::AudioContext(AudioContext&&) noexcept : AudioContext(base::PassKey<AudioContext>{})
{
}


////////////////////////////////////////////////////////////
AudioContext::~AudioContext()
{
    if (audioContextRC.fetch_sub(1u, std::memory_order::relaxed) > 1u)
        return;

    installedAudioContext.reset();
}


////////////////////////////////////////////////////////////
base::Vector<PlaybackDeviceHandle> AudioContext::getAvailablePlaybackDeviceHandles()
{
    ensureInstalled();

    return getAvailableDeviceHandles<PlaybackDeviceHandle> //
        (base::PassKey<AudioContext>{},
         *static_cast<ma_context*>(AudioContext::getMAContext()),
         "playback",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
    { return ma_context_get_devices(maContext, maDeviceInfosPtr, maDeviceInfoCount, nullptr, nullptr); });
}


////////////////////////////////////////////////////////////
base::Optional<PlaybackDeviceHandle> AudioContext::getDefaultPlaybackDeviceHandle()
{
    ensureInstalled();

    for (const PlaybackDeviceHandle& deviceHandle : getAvailablePlaybackDeviceHandles())
        if (deviceHandle.isDefault())
            return base::makeOptional(deviceHandle);

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Vector<CaptureDeviceHandle> AudioContext::getAvailableCaptureDeviceHandles()
{
    ensureInstalled();

    return getAvailableDeviceHandles<CaptureDeviceHandle> //
        (base::PassKey<AudioContext>{},
         *static_cast<ma_context*>(AudioContext::getMAContext()),
         "capture",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
    { return ma_context_get_devices(maContext, nullptr, nullptr, maDeviceInfosPtr, maDeviceInfoCount); });
}


////////////////////////////////////////////////////////////
base::Optional<CaptureDeviceHandle> AudioContext::getDefaultCaptureDeviceHandle()
{
    ensureInstalled();

    for (const CaptureDeviceHandle& deviceHandle : getAvailableCaptureDeviceHandles())
        if (deviceHandle.isDefault())
            return base::makeOptional(deviceHandle);

    return base::nullOpt;
}

} // namespace sf
