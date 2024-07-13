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

#include <SFML/Audio/SoundChannel.hpp>
#include <SFML/Audio/SoundStream.hpp>

#include <SFML/System/Optional.hpp>
#include <SFML/System/PassKey.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <vector>

#include <cstddef>
#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class InputSoundFile;
class InputStream;
class Path;
class Time;
} // namespace sf


namespace sf
{
class Music : public SoundStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Music() override;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Music(Music&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Music& operator=(Music&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Open a music source from an audio file
    ///
    /// This function doesn't start playing the music (use `play`
    /// to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather
    /// streamed continuously, the file must remain accessible until
    /// the `sf::Music` object loads a new music or is destroyed
    /// and until all active `sf::Music` objects linked to this
    /// `sf::Music` instance are destroyed.
    ///
    /// \param filename Path of the music file to open
    ///
    /// \return Music source if loading succeeded, `sf::nullOpt` if it failed
    ///
    /// \see openFromMemory, openFromStream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Optional<Music> openFromFile(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file in memory
    ///
    /// This function doesn't start playing the music (use `play`
    /// to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather streamed
    /// continuously, the \a data buffer must remain accessible until
    /// the `sf::Music` object loads a new music or is destroyed
    /// and until all active `sf::Music` objects linked to this
    /// `sf::Music` instance are destroyed. You can't deallocate the buffer
    /// right after calling this function.
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return Music source if loading succeeded, `sf::nullOpt` if it failed
    ///
    /// \see openFromFile, openFromStream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Optional<Music> openFromMemory(const void* data, std::size_t sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file in a custom stream
    ///
    /// This function doesn't start playing the music (use `play`
    /// to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather
    /// streamed continuously, the \a stream must remain accessible
    /// until the `sf::Music` object loads a new music or is destroyed
    /// and until all active `sf::Music` objects linked to this
    /// `sf::Music` instance are destroyed.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Music source if loading succeeded, `sf::nullOpt` if it failed
    ///
    /// \see openFromFile, openFromMemory
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Optional<Music> openFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the music
    ///
    /// \return Music duration
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Time getDuration() const;

    ////////////////////////////////////////////////////////////
    /// \brief Return the number of channels of the stream
    ///
    /// 1 channel means a mono sound, 2 means stereo, etc.
    ///
    /// \return Number of channels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getChannelCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the stream sample rate of the stream
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
    /// position during spatialisation.
    ///
    /// \return Map of position in sample frame to sound channel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::vector<SoundChannel> getChannelMap() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the total number of audio samples in the source file
    ///
    /// \return Number of samples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t getSampleCount() const;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Try opening the music file from an optional input sound file
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Optional<Music> tryOpenFromInputSoundFile(Optional<InputSoundFile>&& optFile,
                                                                   const char*                errorContext);

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Initialize the internal state after loading a new music
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Music(priv::PassKey<Music>&&, InputSoundFile&& file);

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
    /// \see setLoopPoints
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
    /// \see getLoopPoints
    ///
    ////////////////////////////////////////////////////////////
    void setLoopPoints(TimeSpan timePoints);

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Request a new chunk of audio samples from the stream source
    ///
    /// This function fills the chunk from the next samples
    /// to read from the audio file.
    ///
    /// \param data Chunk of data to fill
    ///
    /// \return True to continue playback, false to stop
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
    /// This is called by the underlying SoundStream whenever it needs us to reset
    /// the seek position for a loop. We then determine whether we are looping on a
    /// loop point or the end-of-file, perform the seek, and return the new position.
    ///
    /// \return The seek position after looping (or sf::nullOpt if there's no loop)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Optional<std::uint64_t> onLoop() override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    priv::UniquePtr<Impl> m_impl; //!< Implementation details
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
/// sf::Music object.
///
/// Apart from that, a sf::Music has almost the same features as
/// the sf::SoundBuffer / sf::Sound pair: you can play/pause/stop
/// it, request its parameters (channels, sample rate), change
/// the way it is played (pitch, volume, 3D position, ...), etc.
///
/// As a sound stream, a music is played in its own thread in order
/// not to block the rest of the program. This means that you can
/// leave the music alone after calling play(), it will manage itself
/// very well.
///
/// Usage example:
/// \code
/// // Open a music from an audio file
/// auto music = sf::Music::openFromFile("music.ogg").value();
///
/// // Change some parameters
/// music.setPosition({0, 1, 10}); // change its 3D position
/// music.setPitch(2);             // increase the pitch
/// music.setVolume(50);           // reduce the volume
/// music.setLoop(true);           // make it loop
///
/// // Create an audio context and get the default playback device
/// auto audioContext = sf::AudioContext::create().value();
/// auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();
///
/// // Play it
/// music.play(playbackDevice);
/// \endcode
///
/// \see sf::Sound, sf::SoundStream
///
////////////////////////////////////////////////////////////
