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
#include <SFML/Audio/CaptureDeviceHandle.hpp>
#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/PassKey.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <miniaudio.h>

#include <utility>


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
    {
        sf::priv::err() << "Failed to initialize the audio log: " << ma_result_description(result) << sf::priv::errEndl;
        return false;
    }

    // Register our logging callback to output any warning/error messages
    if (const ma_result result = ma_log_register_callback(&log, ma_log_callback_init(&maLogCallback, nullptr));
        result != MA_SUCCESS)
    {
        sf::priv::err() << "Failed to register audio log callback: " << ma_result_description(result) << sf::priv::errEndl;
        return false;
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
        {
            sf::priv::err() << "Failed to initialize the audio playback context: " << ma_result_description(result)
                            << sf::priv::errEndl;

            return false;
        }

        // Count the playback devices
        if (const ma_result result = ma_context_get_devices(&maContext, nullptr, &deviceCount, nullptr, nullptr);
            result != MA_SUCCESS)
        {
            sf::priv::err() << "Failed to get audio playback devices: " << ma_result_description(result)
                            << sf::priv::errEndl;

            return false;
        }

        // Check if there are audio playback devices available on the system
        if (deviceCount > 0)
            break;

        // Warn if no devices were found using the default backend list
        if (backendList == nullptr)
            sf::priv::err() << "No audio playback devices available on the system" << sf::priv::errEndl;

        // Clean up the context if we didn't find any devices (TODO: why?)
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
template <typename F>
[[nodiscard]] bool forAllMADeviceInfos(ma_context& maContext, F&& func)
{
    std::vector<ma_device_info> maDeviceInfoVector; // Use a single local variable for NRVO

    ma_device_info* maDeviceInfosPtr{};
    ma_uint32       maDeviceInfoCount{};

    // Get the playback devices
    if (const ma_result result = ma_context_get_devices(&maContext, &maDeviceInfosPtr, &maDeviceInfoCount, nullptr, nullptr);
        result != MA_SUCCESS)
    {
        sf::priv::err() << "Failed to get audio playback devices: " << ma_result_description(result) << sf::priv::errEndl;
        return false; // Empty device entry impl vector
    }

    maDeviceInfoVector.reserve(maDeviceInfoCount);

    for (ma_uint32 i = 0u; i < maDeviceInfoCount; ++i)
        func(maDeviceInfosPtr[i]);

    return true;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct AudioContext::Impl
{
    ma_log     maLog;     //!< miniaudio log (one per program)
    ma_context maContext; //!< miniaudio context (one per program)

    ~Impl()
    {
        ma_context_uninit(&maContext);
        ma_log_uninit(&maLog);
    }
};


////////////////////////////////////////////////////////////
std::optional<AudioContext> AudioContext::create()
{
    std::optional<AudioContext> result(std::in_place, priv::PassKey<AudioContext>{}); // Use a single local variable for NRVO

    if (!tryCreateMALog(result->m_impl->maLog))
    {
        // Error message generated in called function.
        result.reset();
        return result;
    }

    if (!tryCreateMAContext(result->m_impl->maLog, result->m_impl->maContext))
    {
        // Error message generated in called function.
        result.reset();
        return result;
    }

    return result;
}


////////////////////////////////////////////////////////////
template <typename THandle, typename F>
std::vector<THandle> AudioContext::getAvailableDeviceHandles(const char* type, F&& fMAContextGetDevices)
{
    std::vector<THandle> deviceHandles; // Use a single local variable for NRVO

    ma_device_info* maDeviceInfosPtr{};
    ma_uint32       maDeviceInfoCount{};

    // Get the Capture devices
    if (const ma_result result = fMAContextGetDevices(&m_impl->maContext, &maDeviceInfosPtr, &maDeviceInfoCount);
        result != MA_SUCCESS)
    {
        priv::err() << "Failed to get audio " << type << " devices: " << ma_result_description(result) << priv::errEndl;
        return deviceHandles; // Empty device handle vector
    }

    deviceHandles.reserve(maDeviceInfoCount);

    for (ma_uint32 i = 0u; i < maDeviceInfoCount; ++i)
        deviceHandles.emplace_back(priv::PassKey<AudioContext>{}, &maDeviceInfosPtr[i]);

    return deviceHandles;
}


////////////////////////////////////////////////////////////
std::vector<PlaybackDeviceHandle> AudioContext::getAvailablePlaybackDeviceHandles()
{
    return getAvailableDeviceHandles<PlaybackDeviceHandle> //
        ("playback",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
         { return ma_context_get_devices(maContext, maDeviceInfosPtr, maDeviceInfoCount, nullptr, nullptr); });
}


////////////////////////////////////////////////////////////
std::optional<PlaybackDeviceHandle> AudioContext::getDefaultPlaybackDeviceHandle()
{
    for (const PlaybackDeviceHandle& deviceHandle : getAvailablePlaybackDeviceHandles())
        if (deviceHandle.isDefault())
            return std::make_optional(deviceHandle);

    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::vector<CaptureDeviceHandle> AudioContext::getAvailableCaptureDeviceHandles()
{
    return getAvailableDeviceHandles<CaptureDeviceHandle> //
        ("capture",
         [](ma_context* maContext, ma_device_info** maDeviceInfosPtr, ma_uint32* maDeviceInfoCount)
         { return ma_context_get_devices(maContext, nullptr, nullptr, maDeviceInfosPtr, maDeviceInfoCount); });
}


////////////////////////////////////////////////////////////
std::optional<CaptureDeviceHandle> AudioContext::getDefaultCaptureDeviceHandle()
{
    for (const CaptureDeviceHandle& deviceHandle : getAvailableCaptureDeviceHandles())
        if (deviceHandle.isDefault())
            return std::make_optional(deviceHandle);

    return std::nullopt;
}


////////////////////////////////////////////////////////////
void* AudioContext::getMAContext() const
{
    return &m_impl->maContext;
}


////////////////////////////////////////////////////////////
AudioContext::AudioContext(priv::PassKey<AudioContext>&&) : m_impl(priv::makeUnique<Impl>())
{
}


////////////////////////////////////////////////////////////
AudioContext::~AudioContext() = default;


////////////////////////////////////////////////////////////
AudioContext::AudioContext(AudioContext&& rhs) noexcept = default;


////////////////////////////////////////////////////////////
AudioContext& AudioContext::operator=(AudioContext&& rhs) noexcept = default;

} // namespace sf
