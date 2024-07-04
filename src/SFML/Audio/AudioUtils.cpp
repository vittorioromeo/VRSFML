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
#include <SFML/Audio/Export.hpp>

#include <SFML/Audio/AudioUtils.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
AudioUtils::ContextAndDefaultPlaybackDevice::ContextAndDefaultPlaybackDevice(
    AudioContext&&              theAudioContext,
    const PlaybackDeviceHandle& defaultPlaybackDeviceHandle) :
audioContext(SFML_MOVE(theAudioContext)),
defaultPlaybackDevice(audioContext, defaultPlaybackDeviceHandle)
{
}

////////////////////////////////////////////////////////////
AudioUtils::ContextAndDefaultDevices::ContextAndDefaultDevices(AudioContext&&              theAudioContext,
                                                               const PlaybackDeviceHandle& defaultPlaybackDeviceHandle,
                                                               const CaptureDeviceHandle&  defaultCaptureDeviceHandle) :
audioContext(SFML_MOVE(theAudioContext)),
defaultPlaybackDevice(audioContext, defaultPlaybackDeviceHandle),
defaultCaptureDevice(audioContext, defaultCaptureDeviceHandle)
{
}

#define SFML_PRIV_FAIL_IF_UNSET(optionalVar, message)  \
    do                                                 \
    {                                                  \
        if (!(optionalVar).has_value())                \
        {                                              \
            priv::err() << (message) << priv::errEndl; \
            return std::nullopt;                       \
        }                                              \
    } while (false)

////////////////////////////////////////////////////////////
std::optional<AudioUtils::ContextAndDefaultPlaybackDevice> AudioUtils::createContextAndDefaultPlaybackDevice()
{
    std::optional audioContext = sf::AudioContext::create();
    SFML_PRIV_FAIL_IF_UNSET(audioContext, "Failed to create audio context for default playback device");

    std::optional defaultPlaybackDeviceHandle = audioContext->getDefaultPlaybackDeviceHandle();
    SFML_PRIV_FAIL_IF_UNSET(defaultPlaybackDeviceHandle, "Failed to retrieve default playback device handle");

    return std::make_optional<ContextAndDefaultPlaybackDevice>(SFML_MOVE(*audioContext), *defaultPlaybackDeviceHandle);
}

////////////////////////////////////////////////////////////
std::optional<AudioUtils::ContextAndDefaultDevices> AudioUtils::createContextAndDefaultDevices()
{
    std::optional audioContext = sf::AudioContext::create();
    SFML_PRIV_FAIL_IF_UNSET(audioContext, "Failed to create audio context for default playback device");

    std::optional defaultPlaybackDeviceHandle = audioContext->getDefaultPlaybackDeviceHandle();
    SFML_PRIV_FAIL_IF_UNSET(defaultPlaybackDeviceHandle, "Failed to retrieve default playback device handle");

    std::optional defaultCaptureDeviceHandle = audioContext->getDefaultCaptureDeviceHandle();
    SFML_PRIV_FAIL_IF_UNSET(defaultCaptureDeviceHandle, "Failed to retrieve default capture device handle");

    return std::make_optional<ContextAndDefaultDevices>(SFML_MOVE(*audioContext),
                                                        *defaultPlaybackDeviceHandle,
                                                        *defaultCaptureDeviceHandle);
}

#undef SFML_PRIV_FAIL_IF_UNSET

} // namespace sf
