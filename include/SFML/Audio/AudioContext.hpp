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

#include <SFML/Audio/PlaybackDeviceHandle.hpp>

#include <SFML/System/PassKey.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <optional>
#include <vector>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////

struct ma_context;

namespace sf::priv
{
class AudioDevice;
} // namespace sf::priv


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
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    SFML_AUDIO_API static std::optional<AudioContext> create();

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
    /// \brief Get a list of the names of all available audio playback devices
    ///
    /// This function returns a vector of strings containing
    /// the names of all available audio playback devices.
    ///
    /// If the operating system reports multiple devices with
    /// the same name, a number will be appended to the name
    /// of all subsequent devices to distinguish them from each
    /// other. This guarantees that every entry returned by this
    /// function will represent a unique device.
    ///
    /// For example, if the operating system reports multiple
    /// devices with the name "Sound Card", the entries returned
    /// would be:
    ///   - Sound Card
    ///   - Sound Card 2
    ///   - Sound Card 3
    ///   - ...
    ///
    /// The default device, if one is marked as such, will be
    /// placed at the beginning of the vector.
    ///
    /// If no devices are available, this function will return
    /// an empty vector.
    ///
    /// \return A vector containing the device handles or an empty vector if no devices are available
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API std::vector<PlaybackDeviceHandle> getAvailablePlaybackDeviceHandles();

    ////////////////////////////////////////////////////////////
    /// \brief Get the name of the default audio playback device
    ///
    /// This function returns the name of the default audio
    /// playback device. If none is available, an empty string
    /// is returned.
    ///
    /// \return The handle to the default audio playback device
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API std::optional<PlaybackDeviceHandle> getDefaultPlaybackDeviceHandle();

private:
    friend priv::AudioDevice;

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API ma_context& getMAContext() const;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SFML_AUDIO_API explicit AudioContext(priv::PassKey<AudioContext>&&);

private:
    struct Impl;
    priv::UniquePtr<Impl> m_impl; // Needs address stability
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
