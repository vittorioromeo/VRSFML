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

#include <SFML/System/InPlacePImpl.hpp>
#include <SFML/System/LifetimeDependee.hpp>
#include <SFML/System/PassKey.hpp>

#include <filesystem>
#include <optional>
#include <vector>

#include <cstddef>
#include <cstdint>


namespace sf
{
class Time;
class PlaybackDevice;
class InputStream;
class InputSoundFile;
class MusicStream;

////////////////////////////////////////////////////////////
/// \brief Audio file used as a source for `sf::MusicStream`
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API MusicSource
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~MusicSource();

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    MusicSource(MusicSource&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    MusicSource& operator=(MusicSource&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Open a music source from an audio file
    ///
    /// This function doesn't start playing the music (create a
    /// `sf::MusicStream` via `createStream` to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather
    /// streamed continuously, the file must remain accessible until
    /// the `sf::MusicSource` object loads a new music or is destroyed
    /// and until all active `sf::MusicStream` objects linked to this
    /// `sf::MusicSource` instance are destroyed.
    ///
    /// \param filename Path of the music file to open
    ///
    /// \return Music source if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see openFromMemory, openFromStream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<MusicSource> openFromFile(const std::filesystem::path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file in memory
    ///
    /// This function doesn't start playing the music (create a
    /// `sf::MusicStream` via `createStream` to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather streamed
    /// continuously, the \a data buffer must remain accessible until
    /// the `sf::MusicSource` object loads a new music or is destroyed
    /// and until all active `sf::MusicStream` objects linked to this
    /// `sf::MusicSource` instance are destroyed. You can't deallocate the buffer
    /// right after calling this function.
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return Music source if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see openFromFile, openFromStream
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<MusicSource> openFromMemory(const void* data, std::size_t sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music from an audio file in a custom stream
    ///
    /// This function doesn't start playing the music (create a
    /// `sf::MusicStream` via `createStream` to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather
    /// streamed continuously, the \a stream must remain accessible
    /// until the `sf::MusicSource` object loads a new music or is destroyed
    /// and until all active `sf::MusicStream` objects linked to this
    /// `sf::MusicSource` instance are destroyed.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Music source if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see openFromFile, openFromMemory
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<MusicSource> openFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the music
    ///
    /// \return MusicSource duration
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

    ////////////////////////////////////////////////////////////
    /// \brief TODO
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] MusicStream createStream();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Try opening the music file from an optional input sound file
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<MusicSource> tryOpenFromInputSoundFile(std::optional<InputSoundFile>&& optFile,
                                                                              const char* errorContext);

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Initialize the internal state after loading a new music
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit MusicSource(priv::PassKey<MusicSource>&&, InputSoundFile&& file);

private:
    friend MusicStream;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    priv::InPlacePImpl<InputSoundFile, 128> m_file; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(MusicSource, MusicStream);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::MusicSource
/// \ingroup audio
///
/// Musics are sounds that are streamed rather than completely
/// loaded in memory. This is especially useful for compressed
/// musics that usually take hundreds of MB when they are
/// uncompressed: by streaming it instead of loading it entirely,
/// you avoid saturating the memory and have almost no loading delay.
/// This implies that the underlying resource (file, stream or
/// memory buffer) must remain valid for the lifetime of the
/// sf::MusicSource object.
///
/// Apart from that, a `sf::MusicSource` has almost the same features as
/// the `sf::SoundBuffer` / `sf::Sound` pair: you can play/pause/stop
/// it (via `sf::MusicStream`), request its parameters (channels, sample
/// rate), change the way it is played (pitch, volume, 3D position, ...),
/// etc.
///
/// A `sf::MusicStream` created via `sf::MusicSource` is played in its own thread
/// in order not to block the rest of the program. This means that you can
/// leave the music stream alone after calling play(), it will manage itself.
///
/// Usage example:
/// \code
/// // Open a music from an audio file
/// auto music = sf::MusicSource::openFromFile("music.ogg").value();
///
/// // Create a music stream TODO
/// sf::PlaybackDevice playbackDevice;
/// auto musicStream = music.createStream();
///
/// // Change some parameters
/// musicStream.setPosition({0, 1, 10}); // change its 3D position
/// musicStream.setPitch(2);             // increase the pitch
/// musicStream.setVolume(50);           // reduce the volume
/// musicStream.setLoop(true);           // make it loop
///
/// // Play it
/// musicStream.play(playbackDevice);
/// \endcode
///
/// \see sf::Sound, sf::MusicStream
///
////////////////////////////////////////////////////////////
