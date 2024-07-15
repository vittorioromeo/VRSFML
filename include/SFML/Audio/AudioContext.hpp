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

#include <SFML/Audio/CaptureDeviceHandle.hpp>
#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <SFML/System/LifetimeDependee.hpp>

#include <SFML/Base/Optional.hpp>
#include <SFML/Base/PassKey.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <vector>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CaptureDevice;
class PlaybackDevice;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API [[nodiscard]] AudioContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new audio context
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API static base::Optional<AudioContext> create();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API ~AudioContext();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API AudioContext(const AudioContext& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API AudioContext& operator=(const AudioContext& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API AudioContext(AudioContext&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API AudioContext& operator=(AudioContext&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get a list of handles to all available audio playback devices
    ///
    /// This function returns a vector of strings containing
    /// handles to all available audio playback devices.
    ///
    /// If no devices are available, this function will return
    /// an empty vector.
    ///
    /// \return A vector containing the device handles or an empty vector if no devices are available
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API std::vector<PlaybackDeviceHandle> getAvailablePlaybackDeviceHandles();

    ////////////////////////////////////////////////////////////
    /// \brief Get a handle to the default audio playback device
    ///
    /// This function returns a handle to the default audio
    /// playback device. If none is available, `base::nullOpt` is
    /// returned instead.
    ///
    /// \return The handle to the default audio playback device
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API base::Optional<PlaybackDeviceHandle> getDefaultPlaybackDeviceHandle();

    ////////////////////////////////////////////////////////////
    /// \brief Get a list of handles to all available audio capture devices
    ///
    /// This function returns a vector of strings containing
    /// handles to all available audio capture devices.
    ///
    /// If no devices are available, this function will return
    /// an empty vector.
    ///
    /// \return A vector containing the device handles or an empty vector if no devices are available
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API std::vector<CaptureDeviceHandle> getAvailableCaptureDeviceHandles();

    ////////////////////////////////////////////////////////////
    /// \brief Get a handle to the default audio capture device
    ///
    /// This function returns a handle to the default audio
    /// capture device. If none is available, `base::nullOpt` is
    /// returned instead.
    ///
    /// \return The handle to the default audio capture device
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API base::Optional<CaptureDeviceHandle> getDefaultCaptureDeviceHandle();

private:
    friend CaptureDevice;
    friend PlaybackDevice;

    ////////////////////////////////////////////////////////////
    /// Implementation detail, returns a pointer to the miniaudio
    /// context. This pointer is used in the playback and capture
    /// device implementations to initialize the miniaudio devices.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API void* getMAContext() const;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API explicit AudioContext(base::PassKey<AudioContext>&&);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details (needs address stability)

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(AudioContext, CaptureDevice);
    SFML_DEFINE_LIFETIME_DEPENDEE(AudioContext, PlaybackDevice);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::AudioContext
/// \ingroup audio
///
/// TODO
///
/// \see TODO
///
////////////////////////////////////////////////////////////
