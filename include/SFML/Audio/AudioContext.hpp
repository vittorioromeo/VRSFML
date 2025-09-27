#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
/// \brief Provides access to audio functionality
///
/// `AudioContext` is a central manager for audio resources
/// and devices. It ensures that the underlying audio system
/// is initialized and provides access to connected audio devices.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_AUDIO_API AudioContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create and initialize a new audio context
    ///
    /// This function performs the necessary initialization of the
    /// audio system. It must be called before any other audio
    /// functions are used. If the audio system cannot be
    /// initialized, this function returns `base::nullOpt`.
    ///
    /// It is recommended to create an AudioContext early in the
    /// program's execution, for example in `main()`.
    ///
    /// \return A new audio context on success, or `base::nullOpt` on failure
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
    /// \brief Check if an audio context has been successfully initialized
    ///
    /// This function returns true if an `AudioContext` instance
    /// has been successfully created (and not yet destroyed),
    /// and false otherwise. It can be used to check if the audio
    /// system is ready to be used.
    ///
    /// \return `true` if an audio context is currently installed, `false` otherwise
    ///
    /// \see create
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isInstalled();

    ////////////////////////////////////////////////////////////
    /// \brief Get a list of handles to all available audio playback devices
    ///
    /// This function returns a vector of handles for all audio
    /// playback devices currently available on the system, such as
    /// connected speakers or headphones.
    ///
    /// These handles can be used to create `sf::PlaybackDevice` instances.
    ///
    /// If no playback devices are available, this function returns
    /// an empty vector.
    ///
    /// \return A vector of available audio playback device handles
    ///
    /// \see PlaybackDevice, getDefaultPlaybackDeviceHandle
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Vector<PlaybackDeviceHandle> getAvailablePlaybackDeviceHandles();

    ////////////////////////////////////////////////////////////
    /// \brief Get a handle to the default audio playback device
    ///
    /// This function returns a handle for the default audio playback
    /// device. The system typically designates one playback device
    /// as the default.
    ///
    /// This handle can be used to create an `sf::PlaybackDevice` instance.
    ///
    /// If no default playback device is available (e.g., no audio
    /// hardware is present), this function returns `base::nullOpt`.
    ///
    /// \return An optional containing a handle to the default audio playback device, or `base::nullOpt`
    ///
    /// \see PlaybackDevice, getAvailablePlaybackDeviceHandles
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<PlaybackDeviceHandle> getDefaultPlaybackDeviceHandle();

    ////////////////////////////////////////////////////////////
    /// \brief Get a list of handles to all available audio capture devices
    ///
    /// This function returns a vector of handles for all audio
    /// capture devices currently available on the system. such as
    /// microphones or other audio inputs.
    ///
    /// These handles can be used to create `sf::CaptureDevice` instances.
    ///
    /// If no capture devices are available, this function returns
    /// an empty vector.
    ///
    /// \return A vector of available audio capture device handles
    ///
    /// \see CaptureDevice, getDefaultCaptureDeviceHandle
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Vector<CaptureDeviceHandle> getAvailableCaptureDeviceHandles();

    ////////////////////////////////////////////////////////////
    /// \brief Get a handle to the default audio capture device
    ///
    /// This function returns a handle for the default audio capture
    /// device. The system typically designates one capture device
    /// as the default.
    ///
    /// This handle can be used to create an `sf::CaptureDevice` instance.
    ///
    /// If no default capture device is available (e.g., no microphone
    /// is connected), this function returns `base::nullOpt`.
    ///
    /// \return An optional containing a handle to the default audio capture device, or `base::nullOpt`
    ///
    /// \see CaptureDevice, getAvailableCaptureDeviceHandles
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
    /// \brief Get a pointer to the underlying miniaudio context
    /// \warning This is an internal function and should not be used by client code.
    ///
    /// This function provides access to the raw miniaudio context
    /// (`ma_context*`). It is used internally by SFML's audio
    /// module, particularly by `sf::PlaybackDevice` and
    /// `sf::CaptureDevice`, to interact with the miniaudio backend.
    ///
    /// Accessing or manipulating the miniaudio context directly
    /// can lead to undefined behavior if not handled correctly
    /// with SFML's own audio management.
    ///
    /// \return A pointer to the miniaudio context, or `nullptr` if not initialized.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static void* getMAContext();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::AudioContext
/// \ingroup audio
///
/// `sf::AudioContext` provides access to audio functionality.
/// It is a central place for managing audio resources and devices.
///
/// An `AudioContext` must be created before most other audio
/// classes can be used. It ensures that the underlying audio
/// system is initialized and provides access to connected
/// audio devices.
///
/// Usage example:
/// \code
/// #include <SFML/Audio/AudioContext.hpp>
///
/// int main()
/// {
///     auto audioContext = sf::AudioContext::create();
///     if (!audioContext.hasValue())
///     {
///         // Handle audio system initialization failure
///         return -1;
///     }
///
///     // ... proceed with using other SFML audio features ...
///
///     // The audio context will be automatically cleaned up when it goes out of scope.
///     return 0;
/// }
/// \endcode
///
/// \see sf::Sound, sf::Music, sf::SoundBufferRecorder, sf::Listener
///
////////////////////////////////////////////////////////////
