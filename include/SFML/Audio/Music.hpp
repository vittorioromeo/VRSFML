#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/SoundStream.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class MusicReader;
class PlaybackDevice;
class Time;
struct AudioSettings;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Streamed music source built on top of `sf::SoundStream`
///
/// `Music` is the high-level streamed counterpart to
/// `sf::Sound`: it pulls samples on demand from a
/// `sf::MusicReader` instead of holding the entire decoded
/// audio in memory. It is the right choice for long tracks
/// (background music, voice-overs, etc.) where loading the
/// whole file at once would be wasteful.
///
/// A `Music` instance is bound to a `sf::PlaybackDevice` and a
/// `sf::MusicReader`; both must outlive the music object.
///
/// `Music` also adds support for custom loop sub-ranges via
/// `setLoopPoints` / `getLoopPoints`.
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API Music : public SoundStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a music stream from a music reader and a settings snapshot
    ///
    /// The music is bound to `playbackDevice` (which must
    /// outlive it) and pulls samples from `musicReader` (which
    /// must also outlive it). Every property in `audioSettings`
    /// is applied immediately.
    ///
    /// \param playbackDevice Playback device to render through
    /// \param musicReader    Music source to stream data from
    /// \param audioSettings  Initial audio settings to apply
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Music(PlaybackDevice& playbackDevice, MusicReader& musicReader, const AudioSettings& audioSettings);

    ////////////////////////////////////////////////////////////
    /// \brief Construct a music stream from a music reader with default settings
    ///
    /// Equivalent to passing a default-constructed
    /// `AudioSettings` to the other constructor.
    ///
    /// \param playbackDevice Playback device to render through
    /// \param musicReader    Music source to stream data from
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Music(PlaybackDevice& playbackDevice, MusicReader& musicReader);

    ////////////////////////////////////////////////////////////
    /// \brief Disallow construction from a temporary music reader
    ///
    /// `Music` only stores a reference to the reader, so
    /// constructing one from a temporary would immediately
    /// dangle.
    ///
    ////////////////////////////////////////////////////////////
    Music(PlaybackDevice&, const MusicReader&& buffer, const AudioSettings& audioSettings) = delete;
    Music(PlaybackDevice&, const MusicReader&& buffer)                                     = delete;

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
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    Music(Music&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment
    ///
    ////////////////////////////////////////////////////////////
    Music& operator=(Music&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Generic `[offset, offset + length)` range
    ///
    /// Used to express loop ranges (see `setLoopPoints`) in
    /// either time units or sample units.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    struct [[nodiscard]] Span
    {
        T offset{}; //!< Beginning of the range
        T length{}; //!< Length of the range
    };

    /// Time-valued specialization of `Span`, used by `setLoopPoints` / `getLoopPoints`.
    using TimeSpan = Span<Time>;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current loop range of the music
    ///
    /// \return `TimeSpan` describing the active loop sub-range
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
    /// \brief Get the music reader this stream is pulling samples from
    ///
    /// \return Reference to the music reader this music was constructed with
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const MusicReader& getMusicReader() const;

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
    struct Impl;
    base::InPlacePImpl<Impl, 196> m_impl; //!< Implementation details
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
/// the way it is played (pitch, volume, 3D position, etc...), etc.
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
/// sf::Music music(playbackDevice, musicReader);
///
/// // Change some parameters
/// music.setPosition({0, 1, 10}); // change its 3D position
/// music.setPitch(2);             // increase the pitch
/// music.setVolume(0.5f);         // reduce the volume (50%)
/// music.setLooping(true);        // make it loop
///
/// // Play it
/// music.play();
/// \endcode
///
/// \see `sf::Sound`, `sf::SoundStream`
///
////////////////////////////////////////////////////////////
