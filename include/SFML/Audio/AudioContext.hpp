#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/CaptureDeviceHandle.hpp"
#include "SFML/Audio/PlaybackDeviceHandle.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
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
class [[nodiscard]] SFML_AUDIO_API AudioContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new audio context
    ///
    ////////////////////////////////////////////////////////////
    static base::Optional<AudioContext> create();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~AudioContext();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioContext(const AudioContext& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    AudioContext& operator=(const AudioContext& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioContext(AudioContext&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    AudioContext& operator=(AudioContext&& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Check if the audio context is installed
    ///
    /// Installation can be done by calling `create` early in
    /// your program (e.g. in `main`).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isInstalled();

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
    [[nodiscard]] static base::Vector<PlaybackDeviceHandle> getAvailablePlaybackDeviceHandles();

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
    [[nodiscard]] static base::Optional<PlaybackDeviceHandle> getDefaultPlaybackDeviceHandle();

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
    [[nodiscard]] static base::Vector<CaptureDeviceHandle> getAvailableCaptureDeviceHandles();

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
    [[nodiscard]] static base::Optional<CaptureDeviceHandle> getDefaultCaptureDeviceHandle();

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit AudioContext(base::PassKey<AudioContext>&&);

private:
    friend CaptureDevice;
    friend PlaybackDevice;

    ////////////////////////////////////////////////////////////
    /// Implementation detail, returns a pointer to the miniaudio
    /// context. This pointer is used in the playback and capture
    /// device implementations to initialize the miniaudio devices.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static void* getMAContext();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::AudioContext
/// \ingroup audio
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
