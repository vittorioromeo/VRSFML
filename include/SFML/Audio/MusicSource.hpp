#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/System/LifetimeDependee.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class ActiveMusic;
class ChannelMap;
class InputSoundFile;
class InputStream;
class Music;
class Path;
class Time;
} // namespace sf


namespace sf
{
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
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    MusicSource(const MusicSource&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    MusicSource& operator=(const MusicSource&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    MusicSource(MusicSource&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    MusicSource& operator=(MusicSource&& rhs) noexcept;

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
    /// \return Music source if loading succeeded, `base::nullOpt` if it failed
    ///
    /// \see `openFromMemory`, `openFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<MusicSource> openFromFile(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music source from an audio file in memory
    ///
    /// This function doesn't start playing the music (use `play`
    /// to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather streamed
    /// continuously, the `data` buffer must remain accessible until
    /// the `sf::Music` object loads a new music or is destroyed. That is,
    /// you can't deallocate the buffer right after calling this function.
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return Music source if loading succeeded, `base::nullOpt` if it failed
    ///
    /// \see `openFromFile`, `openFromStream`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<MusicSource> openFromMemory(const void* data, base::SizeT sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Open a music source from an audio file in a custom stream
    ///
    /// This function doesn't start playing the music (use `play`
    /// to do so).
    ///
    /// See the documentation of `sf::InputSoundFile` for the list
    /// of supported formats.
    ///
    /// \warning Since the music is not loaded at once but rather
    /// streamed continuously, the `stream` must remain accessible
    /// until the `sf::Music` object loads a new music or is destroyed.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Music source if loading succeeded, `base::nullOpt` if it failed
    ///
    /// \see `openFromFile`, `openFromMemory`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<MusicSource> openFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the music source
    ///
    /// \return Music duration
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Time getDuration() const;

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

    ////////////////////////////////////////////////////////////
    /// \brief Get the total number of audio samples in the music source
    ///
    /// \return Number of samples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::U64 getSampleCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Stores seek/read information
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] SeekAndReadResult
    {
        base::U64 sampleOffset; //!< Sample offset after seeking
        base::U64 samplesRead;  //!< Number of samples read
    };

    ////////////////////////////////////////////////////////////
    /// \see `InputSoundFile::seek`, `InputSoundFile::read`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] SeekAndReadResult seekAndRead(base::U64 sampleOffset, base::I16* samples, base::U64 maxCount);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Initialize the internal state after loading a new music
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit MusicSource(base::PassKey<MusicSource>&&, InputSoundFile&& file);

private:
    ////////////////////////////////////////////////////////////
    friend Music;

    ////////////////////////////////////////////////////////////
    /// \brief Try opening the music source from an optional input sound file
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<MusicSource> tryOpenFromInputSoundFile(base::Optional<InputSoundFile>&& optFile,
                                                                               const char* errorContext);


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details
    // TODO P0: can the uptr be avoided here? the problem is the mutex

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(MusicSource, Music);
    SFML_DEFINE_LIFETIME_DEPENDEE(MusicSource, ActiveMusic);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::MusicSource
/// \ingroup audio
///
/// \see `sf::Music`
///
////////////////////////////////////////////////////////////
