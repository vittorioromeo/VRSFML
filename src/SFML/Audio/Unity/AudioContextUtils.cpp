#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/AudioContextUtils.hpp"
#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/PassKey.hpp"
#include <miniaudio.h>

#include <vector>


namespace
{
////////////////////////////////////////////////////////////
template <typename THandle, typename F>
std::vector<THandle> getAvailableDeviceHandles(sf::base::PassKey<sf::AudioContextUtils>&& passKey,
                                               ma_context&                                maContext,
                                               const char*                                type,
                                               F&&                                        fMAContextGetDevices)
{
    std::vector<THandle> deviceHandles; // Use a single local variable for NRVO

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
        deviceHandles.emplace_back(SFML_BASE_MOVE(passKey), &maDeviceInfosPtr[i]);

    return deviceHandles;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
std::vector<PlaybackDeviceHandle> AudioContextUtils::getAvailablePlaybackDeviceHandles(AudioContext& audioContext)
{
    return getAvailableDeviceHandles<PlaybackDeviceHandle> //
        (base::PassKey<AudioContextUtils>{},
         *static_cast<ma_context*>(audioContext.getMAContext()),
         "playback",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
    { return ma_context_get_devices(maContext, maDeviceInfosPtr, maDeviceInfoCount, nullptr, nullptr); });
}


////////////////////////////////////////////////////////////
base::Optional<PlaybackDeviceHandle> AudioContextUtils::getDefaultPlaybackDeviceHandle(AudioContext& audioContext)
{
    for (const PlaybackDeviceHandle& deviceHandle : getAvailablePlaybackDeviceHandles(audioContext))
        if (deviceHandle.isDefault())
            return base::makeOptional(deviceHandle);

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
std::vector<CaptureDeviceHandle> AudioContextUtils::getAvailableCaptureDeviceHandles(AudioContext& audioContext)
{
    return getAvailableDeviceHandles<CaptureDeviceHandle> //
        (base::PassKey<AudioContextUtils>{},
         *static_cast<ma_context*>(audioContext.getMAContext()),
         "capture",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
    { return ma_context_get_devices(maContext, nullptr, nullptr, maDeviceInfosPtr, maDeviceInfoCount); });
}


////////////////////////////////////////////////////////////
base::Optional<CaptureDeviceHandle> AudioContextUtils::getDefaultCaptureDeviceHandle(AudioContext& audioContext)
{
    for (const CaptureDeviceHandle& deviceHandle : getAvailableCaptureDeviceHandles(audioContext))
        if (deviceHandle.isDefault())
            return base::makeOptional(deviceHandle);

    return base::nullOpt;
}

} // namespace sf
