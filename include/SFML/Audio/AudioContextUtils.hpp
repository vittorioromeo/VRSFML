#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>

#include <SFML/Base/Optional.hpp>

#include <vector>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class AudioContext;
class CaptureDevice;
class CaptureDeviceHandle;
class PlaybackDevice;
class PlaybackDeviceHandle;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API [[nodiscard]] AudioContextUtils
{
public:
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
    [[nodiscard]] static SFML_AUDIO_API std::vector<PlaybackDeviceHandle> getAvailablePlaybackDeviceHandles(
        AudioContext& audioContext);

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
    [[nodiscard]] static SFML_AUDIO_API base::Optional<PlaybackDeviceHandle> getDefaultPlaybackDeviceHandle(
        AudioContext& audioContext);

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
    [[nodiscard]] static SFML_AUDIO_API std::vector<CaptureDeviceHandle> getAvailableCaptureDeviceHandles(
        AudioContext& audioContext);

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
    [[nodiscard]] static SFML_AUDIO_API base::Optional<CaptureDeviceHandle> getDefaultCaptureDeviceHandle(
        AudioContext& audioContext);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::AudioContextUtils
/// \ingroup audio
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
