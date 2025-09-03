#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/Priv/MiniaudioSoundSource.hpp"

#include "SFML/System/LifetimeDependant.hpp"

#include "SFML/Base/InPlacePImpl.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class PlaybackDevice;
class SoundBuffer;
class Time;
struct AudioSettings;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Regular sound that can be played in the audio environment
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API Sound : public priv::MiniaudioSoundSource
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the sound with a buffer
    ///
    /// \param buffer Sound buffer containing the audio data to play with the sound
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Sound(PlaybackDevice& playbackDevice, const SoundBuffer& buffer, const AudioSettings& audioSettings);
    [[nodiscard]] explicit Sound(PlaybackDevice& playbackDevice, const SoundBuffer& buffer);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow construction from a temporary sound buffer
    ///
    ////////////////////////////////////////////////////////////
    Sound(PlaybackDevice&, const SoundBuffer&& buffer, const AudioSettings& audioSettings) = delete;
    Sound(PlaybackDevice&, const SoundBuffer&& buffer)                                     = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Sound() override;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Sound(const Sound& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Sound& operator=(const Sound& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    Sound(Sound&& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment
    ///
    ////////////////////////////////////////////////////////////
    Sound& operator=(Sound&& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position of the sound
    ///
    /// The playing position can be changed when the sound is
    /// either paused or playing. Changing the playing position
    /// when the sound is stopped has no effect, since playing
    /// the sound will reset its position.
    ///
    /// \param playingOffset New playing position, from the beginning of the sound
    ///
    /// \see `getPlayingOffset`
    ///
    ////////////////////////////////////////////////////////////
    void setPlayingOffset(Time playingOffset) override;

    ////////////////////////////////////////////////////////////
    /// \brief Set the sound buffer to play
    ///
    /// This function replaces the current sound buffer with
    /// the one provided in parameter. The sound buffer must
    /// remain valid as long as the sound is using it.
    ///
    /// \param buffer New sound buffer to use
    ///
    /// \see `getBuffer`
    ///
    ////////////////////////////////////////////////////////////
    void setBuffer(const SoundBuffer& buffer);

    ////////////////////////////////////////////////////////////
    /// \brief Get the audio buffer attached to the sound
    ///
    /// \return Sound buffer attached to the sound
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundBuffer& getBuffer() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the playback device
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] PlaybackDevice& getPlaybackDevice() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void resumeOnLastPlaybackDevice();

private:
    friend class SoundBuffer;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sound object
    ///
    /// \return The sound object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] priv::MiniaudioUtils::SoundBase& getSoundBase() const override;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 1896> m_impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(SoundBuffer);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Sound
/// \ingroup audio
///
/// `sf::Sound` is the class to use to play sounds.
/// It provides:
/// \li Control (play, pause, stop)
/// \li Ability to modify output parameters in real-time (pitch, volume, etc...)
/// \li 3D spatial features (position, attenuation, etc...).
///
/// `sf::Sound` is perfect for playing short sounds that can
/// fit in memory and require no latency, like foot steps or
/// gun shots. For longer sounds, like background musics
/// or long speeches, rather see `sf::Music` (which is based
/// on streaming).
///
/// In order to work, a sound must be given a buffer of audio
/// data to play. Audio data (samples) is stored in `sf::SoundBuffer,`
/// and attached to a sound when it is created.
/// The buffer object attached to a sound must remain alive
/// as long as the sound uses it. Note that multiple sounds
/// can use the same sound buffer at the same time.
///
/// Usage example:
/// \code
/// const auto buffer = sf::SoundBuffer::loadFromFile("sound.wav").value();
/// sf::Sound sound(buffer);
/// sound.play();
/// \endcode
///
/// \see `sf::SoundBuffer`, `sf::Music`
///
////////////////////////////////////////////////////////////
