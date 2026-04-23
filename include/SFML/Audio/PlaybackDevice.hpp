#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/System/LifetimeDependee.hpp"

#include "SFML/Base/InPlacePImpl.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv::MiniaudioUtils
{
struct SoundBase;
} // namespace sf::priv::MiniaudioUtils

namespace sf
{
class PlaybackDeviceHandle;
class Sound;
class SoundStream;
struct Listener;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Opened audio playback device used to play sound sources
///
/// A `PlaybackDevice` represents a live, opened audio output
/// (speakers, headphones, virtual sink, etc.). It owns the
/// underlying miniaudio engine and is the entity that
/// `sf::Sound` and `sf::Music` instances render through.
///
/// A playback device is created from a `PlaybackDeviceHandle`
/// obtained via `sf::AudioContext`:
/// \code
/// auto audioContext   = sf::AudioContext::create().value();
/// auto handle         = sf::AudioContext::getDefaultPlaybackDeviceHandle().value();
/// sf::PlaybackDevice  playbackDevice{handle};
/// \endcode
///
/// `PlaybackDevice` is non-copyable and non-movable: any
/// `sf::Sound` or `sf::SoundStream` constructed from it holds
/// a non-owning reference, so the device must outlive every
/// sound source bound to it.
///
/// \see `sf::AudioContext`, `sf::PlaybackDeviceHandle`, `sf::Sound`, `sf::SoundStream`, `sf::Music`
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API PlaybackDevice
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Open a playback device from a device handle
    ///
    /// Initializes the underlying miniaudio engine bound to the
    /// device described by `deviceHandle`. The handle is
    /// typically obtained from `sf::AudioContext`.
    ///
    /// \param deviceHandle Handle identifying the device to open
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit PlaybackDevice(const PlaybackDeviceHandle& deviceHandle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Closes the underlying device. All `sf::Sound`,
    /// `sf::SoundStream`, and `sf::Music` instances bound to
    /// this device must already have been destroyed.
    ///
    ////////////////////////////////////////////////////////////
    ~PlaybackDevice();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    PlaybackDevice(const PlaybackDevice&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    PlaybackDevice& operator=(const PlaybackDevice&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    PlaybackDevice(PlaybackDevice&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment
    ///
    ////////////////////////////////////////////////////////////
    PlaybackDevice& operator=(PlaybackDevice&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the handle of the device that was opened
    ///
    /// \return Reference to the device handle this device was
    ///         constructed from
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const PlaybackDeviceHandle& getDeviceHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Push the listener parameters to the underlying engine
    ///
    /// `sf::Listener` is a plain data structure: changes made to
    /// a `Listener` value have no effect until they are committed
    /// to a playback device through this function. Each playback
    /// device keeps its own listener state.
    ///
    /// \param listener Listener configuration to apply
    ///
    /// \return `true` on success, `false` on failure
    ///
    /// \see `sf::Listener`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool applyListener(const Listener& listener);

    ////////////////////////////////////////////////////////////
    /// \brief Get the human-readable name of the opened device
    ///
    /// \return Null-terminated UTF-8 device name
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getName() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the opened device is the system default
    ///
    /// \return `true` if this device is the system's default
    ///         playback device, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDefault() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sample rate of the underlying audio engine
    ///
    /// \return Sample rate (number of audio samples to play per second)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getSampleRate() const;

private:
    // Friends
    using SoundBase = priv::MiniaudioUtils::SoundBase;
    friend SoundBase;
    friend Sound;
    friend SoundStream;

    ////////////////////////////////////////////////////////////
    /// \brief Get the internal miniaudio engine pointer
    ///
    /// Internal accessor used by `sf::Sound` / `sf::SoundStream`
    /// to attach themselves to the engine. Not part of the
    /// public API.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] void* getMAEngine();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 7680> m_impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(PlaybackDevice, SoundBase);
};

} // namespace sf
