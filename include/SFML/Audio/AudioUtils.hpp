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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>

#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/CaptureDevice.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

#include <optional>


namespace sf
{
////////////////////////////////////////////////////////////
class AudioUtils
{
private:
    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] ContextAndDefaultPlaybackDevice
    {
        AudioContext   audioContext;
        PlaybackDevice defaultPlaybackDevice;

        [[nodiscard]] explicit ContextAndDefaultPlaybackDevice(AudioContext&&              theAudioContext,
                                                               const PlaybackDeviceHandle& defaultPlaybackDeviceHandle);
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] ContextAndDefaultDevices
    {
        AudioContext   audioContext;
        PlaybackDevice defaultPlaybackDevice;
        CaptureDevice  defaultCaptureDevice;

        [[nodiscard]] explicit ContextAndDefaultDevices(AudioContext&&              theAudioContext,
                                                        const PlaybackDeviceHandle& defaultPlaybackDeviceHandle,
                                                        const CaptureDeviceHandle&  defaultCaptureDeviceHandle);
    };

public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<ContextAndDefaultPlaybackDevice> createContextAndDefaultPlaybackDevice();

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<ContextAndDefaultDevices> createContextAndDefaultDevices();
};

} // namespace sf
