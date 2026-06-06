// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/AudioContext.hpp"

#include "Zancle/Audio/CaptureDeviceHandle.hpp"
#include "Zancle/Audio/PlaybackDeviceHandle.hpp"
#include "Zancle/Audio/Priv/MiniaudioUtils.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Concurrency/Atomic.hpp"

#include "Zancle/Container/Vector.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"

#include "Zancle/Diagnostic/Abort.hpp"

#include "Zancle/Base/Macros.hpp"

#include <miniaudio.h>


namespace
{
////////////////////////////////////////////////////////////
void maLogCallback(void*, ma_uint32 level, const char* message)
{
    if (level <= MA_LOG_LEVEL_WARNING)
        za::priv::errMsg("miniaudio {}: {}", ma_log_level_to_string(level), message);
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool tryCreateMALog(ma_log& log)
{
    // Create the log
    if (const ma_result result = ma_log_init(nullptr, &log); result != MA_SUCCESS)
        return za::priv::MiniaudioUtils::fail("initialize the audio log", result);

    // Register our logging callback to output any warning/error messages
    if (const ma_result result = ma_log_register_callback(&log, ma_log_callback_init(&maLogCallback, nullptr));
        result != MA_SUCCESS)
    {
        ma_log_uninit(&log);
        return za::priv::MiniaudioUtils::fail("register audio log callback", result);
    }

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
            return za::priv::MiniaudioUtils::fail("initialize the audio playback", result);

        // Count the playback devices
        if (const ma_result result = ma_context_get_devices(&maContext, nullptr, &deviceCount, nullptr, nullptr);
            result != MA_SUCCESS)
        {
            ma_context_uninit(&maContext);
            return za::priv::MiniaudioUtils::fail("get audio playback devices", result);
        }

        // Check if there are audio playback devices available on the system
        if (deviceCount > 0)
            break;

        // Warn if no devices were found using the default backend list
        if (backendList == nullptr)
            za::priv::errMsg("No audio playback devices available on the system");

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
za::Vector<THandle> getAvailableDeviceHandles(za::PassKey<za::AudioContext>&& passKey,
                                              ma_context&                     maContext,
                                              const char*                     type,
                                              F&&                             fMAContextGetDevices)
{
    za::Vector<THandle> deviceHandles; // Use a single local variable for NRVO

    ma_device_info* maDeviceInfosPtr{};
    ma_uint32       maDeviceInfoCount{};

    // Get the Capture devices
    if (const ma_result result = fMAContextGetDevices(&maContext, &maDeviceInfosPtr, &maDeviceInfoCount);
        result != MA_SUCCESS)
    {
        za::priv::errMsg("Failed to get audio {} devices: {}", type, ma_result_description(result));

        return deviceHandles; // Empty device handle vector
    }

    deviceHandles.reserve(maDeviceInfoCount);

    for (ma_uint32 i = 0u; i < maDeviceInfoCount; ++i)
        deviceHandles.emplaceBack(ZA_MOVE(passKey), &maDeviceInfosPtr[i]);

    return deviceHandles;
}


////////////////////////////////////////////////////////////
struct AudioContextImpl
{
    ma_log     maLog{};     //!< miniaudio log (one per program)
    ma_context maContext{}; //!< miniaudio context (one per program)
    bool       maLogInitialized{};
    bool       maContextInitialized{};

    ~AudioContextImpl()
    {
        if (maContextInitialized)
            ma_context_uninit(&maContext);

        if (maLogInitialized)
            ma_log_uninit(&maLog);
    }
};


////////////////////////////////////////////////////////////
constinit za::Optional<AudioContextImpl> installedAudioContext;
constinit za::Atomic<unsigned int>       audioContextRC{0u};


////////////////////////////////////////////////////////////
AudioContextImpl& ensureInstalled()
{
    if (!installedAudioContext.hasValue()) [[unlikely]]
    {
        za::priv::errMsg("`za::AudioContext` not installed -- did you forget to create one in `main`?");
        za::abort();
    }

    return *installedAudioContext;
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
za::Optional<AudioContext> AudioContext::create()
{
    const auto fail = [](const char* what)
    {
        priv::errMsg("Error creating `za::AudioContext`: {}", what);
        return za::nullOpt;
    };

    //
    // Ensure audio context is not already installed
    if (installedAudioContext.hasValue())
        return fail("an `za::AudioContext` object already exists");

    auto& ac = installedAudioContext.emplace();

    if (!tryCreateMALog(ac.maLog))
    {
        installedAudioContext.reset();
        return za::nullOpt; // Error message generated in called function.
    }

    ac.maLogInitialized = true;

    if (!tryCreateMAContext(ac.maLog, ac.maContext))
    {
        installedAudioContext.reset();
        return za::nullOpt; // Error message generated in called function.
    }

    ac.maContextInitialized = true;

    return za::makeOptional<AudioContext>(za::PassKey<AudioContext>{});
}


////////////////////////////////////////////////////////////
void* AudioContext::getMAContext()
{
    return &ensureInstalled().maContext;
}


////////////////////////////////////////////////////////////
AudioContext::AudioContext(za::PassKey<AudioContext>&&)
{
    audioContextRC.fetchAddRelaxed(1u);
}


////////////////////////////////////////////////////////////
AudioContext::AudioContext(AudioContext&&) noexcept : AudioContext(za::PassKey<AudioContext>{})
{
}


////////////////////////////////////////////////////////////
AudioContext::~AudioContext()
{
    if (audioContextRC.fetchSubRelaxed(1u) > 1u)
        return;

    installedAudioContext.reset();
}


////////////////////////////////////////////////////////////
bool AudioContext::isInstalled()
{
    return installedAudioContext.hasValue();
}


////////////////////////////////////////////////////////////
za::Vector<PlaybackDeviceHandle> AudioContext::getAvailablePlaybackDeviceHandles()
{
    ensureInstalled();

    return getAvailableDeviceHandles<PlaybackDeviceHandle> //
        (za::PassKey<AudioContext>{},
         *static_cast<ma_context*>(AudioContext::getMAContext()),
         "playback",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
    { return ma_context_get_devices(maContext, maDeviceInfosPtr, maDeviceInfoCount, nullptr, nullptr); });
}


////////////////////////////////////////////////////////////
za::Optional<PlaybackDeviceHandle> AudioContext::getDefaultPlaybackDeviceHandle()
{
    ensureInstalled();

    for (const PlaybackDeviceHandle& deviceHandle : getAvailablePlaybackDeviceHandles())
        if (deviceHandle.isDefault())
            return za::makeOptional(deviceHandle);

    return za::nullOpt;
}


////////////////////////////////////////////////////////////
za::Vector<CaptureDeviceHandle> AudioContext::getAvailableCaptureDeviceHandles()
{
    ensureInstalled();

    return getAvailableDeviceHandles<CaptureDeviceHandle> //
        (za::PassKey<AudioContext>{},
         *static_cast<ma_context*>(AudioContext::getMAContext()),
         "capture",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
    { return ma_context_get_devices(maContext, nullptr, nullptr, maDeviceInfosPtr, maDeviceInfoCount); });
}


////////////////////////////////////////////////////////////
za::Optional<CaptureDeviceHandle> AudioContext::getDefaultCaptureDeviceHandle()
{
    ensureInstalled();

    for (const CaptureDeviceHandle& deviceHandle : getAvailableCaptureDeviceHandles())
        if (deviceHandle.isDefault())
            return za::makeOptional(deviceHandle);

    return za::nullOpt;
}

} // namespace za
