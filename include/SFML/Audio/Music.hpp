#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/SoundStream.hpp"

#include "SFML/System/LifetimeDependant.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
struct AudioSettings;
class ChannelMap;
class Time;
class MusicReader;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
class SFML_AUDIO_API Music : public SoundStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct the music from a music source
    ///
    /// \param musicReader Music source to stream data from
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Music(PlaybackDevice& playbackDevice, MusicReader& musicReader, const AudioSettings& audioSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Music() override;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Music(const Music&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Music& operator=(const Music&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Music(Music&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Music& operator=(Music&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Structure template defining a time range
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    struct [[nodiscard]] Span
    {
        T offset{}; //!< The beginning offset of the time range
        T length{}; //!< The length of the time range
    };

    // Define the relevant `Span` types
    using TimeSpan = Span<Time>;

    ////////////////////////////////////////////////////////////
    /// \brief Get the positions of the of the music's looping sequence
    ///
    /// \return `TimeSpan` containing looping sequence positions
    ///
    /// \warning Since `setLoopPoints()` performs some adjustments on the
    /// provided values and rounds them to internal samples, a call to
    /// `getLoopPoints()` is not guaranteed to return the same times passed
    /// into a previous call to `setLoopPoints()`. However, it is guaranteed
    /// to return times that will map to the valid internal samples of
    /// this music stream if they are later passed to `setLoopPoints()`.
    ///
    /// \see `setLoopPoints`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] TimeSpan getLoopPoints() const;

    ////////////////////////////////////////////////////////////
    /// \brief Sets the beginning and duration of the sound's looping sequence using `TimeSpan`
    ///
    /// `setLoopPoints()` allows for specifying the beginning offset and the duration of the loop such that, when the music
    /// is enabled for looping, it will seamlessly seek to the beginning whenever it
    /// encounters the end of the duration. Valid ranges for `timePoints.offset` and `timePoints.length` are
    /// `[0, Dur)` and `(0, Dur-offset]` respectively, where Dur is the value returned by `getDuration()`.
    /// Note that the EOF "loop point" from the end to the beginning of the stream is still honored,
    /// in case the caller seeks to a point after the end of the loop range. This function can be
    /// safely called at any point after a stream is opened, and will be applied to a playing sound
    /// without affecting the current playing offset.
    ///
    /// \warning Setting the loop points while the stream's status is `Paused`
    /// will set its status to Stopped. The playing offset will be unaffected.
    ///
    /// \param timePoints The definition of the loop. Can be any time points within the sound's length
    ///
    /// \see `getLoopPoints`
    ///
    ////////////////////////////////////////////////////////////
    void setLoopPoints(TimeSpan timePoints);

    ////////////////////////////////////////////////////////////
    /// \brief Return the number of channels of the music source
    ///
    /// 1 channel means a mono sound, 2 means stereo, etc.
    ///
    /// \return Number of channels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getChannelCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the stream sample rate of the music source
    ///
    /// The sample rate is the number of audio samples played per
    /// second. The higher, the better the quality.
    ///
    /// \return Sample rate, in number of samples per second
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getSampleRate() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the map of position in sample frame to sound channel
    ///
    /// This is used to map a sample in the sample stream to a
    /// position during spatialization.
    ///
    /// \return Map of position in sample frame to sound channel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] ChannelMap getChannelMap() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Request a new chunk of audio samples from the stream source
    ///
    /// This function fills the chunk from the next samples
    /// to read from the audio file.
    ///
    /// \param data Chunk of data to fill
    ///
    /// \return `true` to continue playback, `false` to stop
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool onGetData(Chunk& data) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position in the stream source
    ///
    /// \param timeOffset New playing position, from the beginning of the music
    ///
    ////////////////////////////////////////////////////////////
    void onSeek(Time timeOffset) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current playing position in the stream source to the loop offset
    ///
    /// This is called by the underlying `SoundStream` whenever it needs us to reset
    /// the seek position for a loop. We then determine whether we are looping on a
    /// loop point or the end-of-file, perform the seek, and return the new position.
    ///
    /// \return The seek position after looping (or `base::nullOpt` if there's no loop)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<base::U64> onLoop() override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::Vector<base::I16> m_samples;      //!< Temporary buffer of samples
    Span<base::U64>         m_loopSpan;     //!< Loop range Specifier
    MusicReader*            m_musicSource;  //!< The music source
    base::U64               m_sampleOffset; //!< Current offset in the stream

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(MusicReader);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Music
/// \ingroup audio
///
/// Musics are sounds that are streamed rather than completely
/// loaded in memory. This is especially useful for compressed
/// musics that usually take hundreds of MB when they are
/// uncompressed: by streaming it instead of loading it entirely,
/// you avoid saturating the memory and have almost no loading delay.
/// This implies that the underlying resource (file, stream or
/// memory buffer) must remain valid for the lifetime of the
/// `sf::Music` object.
///
/// Apart from that, a `sf::Music` has almost the same features as
/// the `sf::SoundBuffer` / `sf::Sound` pair: you can play/pause/stop
/// it, request its parameters (channels, sample rate), change
/// the way it is played (pitch, volume, 3D position, ...), etc.
///
/// As a sound stream, a music is played in its own thread in order
/// not to block the rest of the program. This means that you can
/// leave the music alone after calling `play()`, it will manage itself
/// very well.
///
/// Usage example:
/// \code
/// // Create an audio context and get the default playback device
/// auto audioContext = sf::AudioContext::create().value();
/// sf::PlaybackDevice playbackDevice{sf::AudioContext::getDefaultPlaybackDeviceHandle().value()};
///
/// // Open a music source from an audio file
/// auto musicReader = sf::MusicReader::openFromFile("music.ogg").value();
///
/// // Create a music stream from the music source
/// sf::Music music(musicReader);
///
/// // Change some parameters
/// music.setPosition({0, 1, 10}); // change its 3D position
/// music.setPitch(2);             // increase the pitch
/// music.setVolume(0.5f);         // reduce the volume (50%)
/// music.setLooping(true);        // make it loop
///
/// // Play it
/// music.play(playbackDevice);
/// \endcode
///
/// \see `sf::Sound`, `sf::SoundStream`
///
////////////////////////////////////////////////////////////
