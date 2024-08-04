#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>

#include <SFML/Audio/SoundSource.hpp>

#include <SFML/System/LifetimeDependant.hpp>

#include <SFML/Base/UniquePtr.hpp>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class EffectProcessor;
class PlaybackDevice;
class SoundBuffer;
class Time;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Regular sound that can be played in the audio environment
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API Sound : public SoundSource
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the sound with a buffer
    ///
    /// \param buffer Sound buffer containing the audio data to play with the sound
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Sound(const SoundBuffer& buffer);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow construction from a temporary sound buffer
    ///
    ////////////////////////////////////////////////////////////
    explicit Sound(const SoundBuffer&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Sound(const Sound& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Sound& operator=(const Sound& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Sound(Sound&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Sound& operator=(Sound&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Sound() override;

    ////////////////////////////////////////////////////////////
    /// \brief Start or resume playing the sound
    ///
    /// This function starts the stream if it was stopped, resumes
    /// it if it was paused, and restarts it from beginning if it
    /// was it already playing.
    /// This function uses its own thread so that it doesn't block
    /// the rest of the program while the sound is played.
    ///
    /// \see pause, stop
    ///
    ////////////////////////////////////////////////////////////
    void play(PlaybackDevice&) override;

    ////////////////////////////////////////////////////////////
    /// \brief Pause the sound
    ///
    /// This function pauses the sound if it was playing,
    /// otherwise (sound already paused or stopped) it has no effect.
    ///
    /// \see play, stop
    ///
    ////////////////////////////////////////////////////////////
    void pause() override;

    ////////////////////////////////////////////////////////////
    /// \brief stop playing the sound
    ///
    /// This function stops the sound if it was playing or paused,
    /// and does nothing if it was already stopped.
    /// It also resets the playing position (unlike pause()).
    ///
    /// \see play, pause
    ///
    ////////////////////////////////////////////////////////////
    void stop() override;

    ////////////////////////////////////////////////////////////
    /// \brief Set the source buffer containing the audio data to play
    ///
    /// It is important to note that the sound buffer is not copied,
    /// thus the sf::SoundBuffer instance must remain alive as long
    /// as it is attached to the sound.
    ///
    /// \param buffer Sound buffer to attach to the sound
    ///
    /// \see getBuffer
    ///
    ////////////////////////////////////////////////////////////
    void setBuffer(const SoundBuffer& buffer);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow setting from a temporary sound buffer
    ///
    ////////////////////////////////////////////////////////////
    void setBuffer(const SoundBuffer&& buffer) = delete;

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
    /// \see getPlayingOffset
    ///
    ////////////////////////////////////////////////////////////
    void setPlayingOffset(Time playingOffset) override;

    ////////////////////////////////////////////////////////////
    /// \brief Set the effect processor to be applied to the sound
    ///
    /// The effect processor is a callable that will be called
    /// with sound data to be processed.
    ///
    /// \param effectProcessor The effect processor to attach to this sound, attach an empty processor to disable processing
    ///
    ////////////////////////////////////////////////////////////
    void setEffectProcessor(EffectProcessor effectProcessor) override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the audio buffer attached to the sound
    ///
    /// \return Sound buffer attached to the sound
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SoundBuffer& getBuffer() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current playing position of the sound
    ///
    /// \return Current playing position, from the beginning of the sound
    ///
    /// \see setPlayingOffset
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Time getPlayingOffset() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current status of the sound (stopped, paused, playing)
    ///
    /// \return Current status of the sound
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Status getStatus() const override;

private:
    friend class SoundBuffer;

    ////////////////////////////////////////////////////////////
    /// \brief Detach sound from its internal buffer
    ///
    /// This allows the sound buffer to temporarily detach the
    /// sounds that use it when the sound buffer gets updated.
    ///
    ////////////////////////////////////////////////////////////
    void detachBuffer();

    ////////////////////////////////////////////////////////////
    /// \brief Get the sound object
    ///
    /// \return The sound object
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] void* getSound() const override;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details

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
/// sf::Sound is the class to use to play sounds.
/// It provides:
/// \li Control (play, pause, stop)
/// \li Ability to modify output parameters in real-time (pitch, volume, ...)
/// \li 3D spatial features (position, attenuation, ...).
///
/// sf::Sound is perfect for playing short sounds that can
/// fit in memory and require no latency, like foot steps or
/// gun shots. For longer sounds, like background musics
/// or long speeches, rather see sf::Music (which is based
/// on streaming).
///
/// In order to work, a sound must be given a buffer of audio
/// data to play. Audio data (samples) is stored in sf::SoundBuffer,
/// and attached to a sound when it is created or with the setBuffer() function.
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
/// \see sf::SoundBuffer, sf::Music
///
////////////////////////////////////////////////////////////
