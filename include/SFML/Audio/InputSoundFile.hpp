#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/ChannelMap.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <cstddef>
#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class InputStream;
class Path;
class SoundFileReader;
class Time;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Provide read access to sound files
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API InputSoundFile
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    InputSoundFile(InputSoundFile&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    InputSoundFile& operator=(InputSoundFile&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~InputSoundFile();

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file from the disk for reading
    ///
    /// The supported audio formats are: WAV (PCM only), OGG/Vorbis, FLAC, MP3.
    /// The supported sample sizes for FLAC and WAV are 8, 16, 24 and 32 bit.
    ///
    /// Because of minimp3_ex limitation, for MP3 files with big (>16kb) APEv2 tag,
    /// it may not be properly removed, tag data will be treated as MP3 data
    /// and there is a low chance of garbage decoded at the end of file.
    /// See also: https://github.com/lieff/minimp3
    ///
    /// \param filename Path of the sound file to load
    ///
    /// \return Input sound file if the file was successfully opened, otherwise `base::nullOpt`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<InputSoundFile> openFromFile(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file in memory for reading
    ///
    /// The supported audio formats are: WAV (PCM only), OGG/Vorbis, FLAC.
    /// The supported sample sizes for FLAC and WAV are 8, 16, 24 and 32 bit.
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return Input sound file if the file was successfully opened, otherwise `base::nullOpt`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<InputSoundFile> openFromMemory(const void* data, std::size_t sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file from a custom stream for reading
    ///
    /// The supported audio formats are: WAV (PCM only), OGG/Vorbis, FLAC.
    /// The supported sample sizes for FLAC and WAV are 8, 16, 24 and 32 bit.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Input sound file if the file was successfully opened, otherwise `base::nullOpt`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<InputSoundFile> openFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Get the total number of audio samples in the file
    ///
    /// \return Number of samples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t getSampleCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of channels used by the sound
    ///
    /// \return Number of channels (1 = mono, 2 = stereo)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getChannelCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sample rate of the sound
    ///
    /// \return Sample rate, in samples per second
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
    /// \see getSampleRate, getChannelCount, getDuration
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const ChannelMap& getChannelMap() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the sound file
    ///
    /// This function is provided for convenience, the duration is
    /// deduced from the other sound file attributes.
    ///
    /// \return Duration of the sound file
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Time getDuration() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the read offset of the file in time
    ///
    /// \return Time position
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Time getTimeOffset() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the read offset of the file in samples
    ///
    /// \return Sample position
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t getSampleOffset() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current read position to the given sample offset
    ///
    /// This function takes a sample offset to provide maximum
    /// precision. If you need to jump to a given time, use the
    /// other overload.
    ///
    /// The sample offset takes the channels into account.
    /// If you have a time offset instead, you can easily find
    /// the corresponding sample offset with the following formula:
    /// `timeInSeconds * sampleRate * channelCount`
    /// If the given offset exceeds to total number of samples,
    /// this function jumps to the end of the sound file.
    ///
    /// \param sampleOffset Index of the sample to jump to, relative to the beginning
    ///
    ////////////////////////////////////////////////////////////
    void seek(std::uint64_t sampleOffset);

    ////////////////////////////////////////////////////////////
    /// \brief Change the current read position to the given time offset
    ///
    /// Using a time offset is handy but imprecise. If you need an accurate
    /// result, consider using the overload which takes a sample offset.
    ///
    /// If the given time exceeds to total duration, this function jumps
    /// to the end of the sound file.
    ///
    /// \param timeOffset Time to jump to, relative to the beginning
    ///
    ////////////////////////////////////////////////////////////
    void seek(Time timeOffset);

    ////////////////////////////////////////////////////////////
    /// \brief Read audio samples from the open file
    ///
    /// \param samples  Pointer to the sample array to fill
    /// \param maxCount Maximum number of samples to read
    ///
    /// \return Number of samples actually read (may be less than \a maxCount)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t read(std::int16_t* samples, std::uint64_t maxCount);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Deleter for input streams that only conditionally deletes
    ///
    ////////////////////////////////////////////////////////////
    struct SFML_AUDIO_API StreamDeleter
    {
        StreamDeleter(bool theOwned);

        // To accept ownership transfer from default deleter
        StreamDeleter(const base::UniquePtrDefaultDeleter&);

        void operator()(InputStream* ptr) const;

        bool owned{true};
    };

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Constructor from reader, stream, and attributes
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] InputSoundFile(base::PassKey<InputSoundFile>&&,
                                 base::UniquePtr<SoundFileReader>&&            reader,
                                 base::UniquePtr<InputStream, StreamDeleter>&& stream,
                                 std::uint64_t                                 sampleCount,
                                 unsigned int                                  sampleRate,
                                 ChannelMap&&                                  channelMap);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::UniquePtr<SoundFileReader> m_reader; //!< Reader that handles I/O on the file's format
    base::UniquePtr<InputStream, StreamDeleter> m_stream{nullptr, false}; //!< Input stream used to access the file's data
    std::uint64_t m_sampleOffset{};                                       //!< Sample Read Position
    std::uint64_t m_sampleCount{};                                        //!< Total number of samples in the file
    unsigned int  m_sampleRate{};                                         //!< Number of samples per second
    ChannelMap    m_channelMap; //!< The map of position in sample frame to sound channel
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::InputSoundFile
/// \ingroup audio
///
/// This class decodes audio samples from a sound file. It is
/// used internally by higher-level classes such as sf::SoundBuffer
/// and sf::Music, but can also be useful if you want to process
/// or analyze audio files without playing them, or if you want to
/// implement your own version of sf::Music with more specific
/// features.
///
/// Usage example:
/// \code
/// // Open a sound file
/// auto file = sf::InputSoundFile::openFromFile("music.ogg").value();
///
/// // Print the sound attributes
/// std::cout << "duration: " << file.getDuration().asSeconds() << '\n'
///           << "channels: " << file.getChannelCount() << '\n'
///           << "sample rate: " << file.getSampleRate() << '\n'
///           << "sample count: " << file.getSampleCount() << '\n';
///
/// // Read and process batches of samples until the end of file is reached
/// std::int16_t samples[1024];
/// std::uint64_t count;
/// do
/// {
///     count = file.read(samples, 1024);
///
///     // process, analyze, play, convert, or whatever
///     // you want to do with the samples...
/// }
/// while (count > 0);
/// \endcode
///
/// \see sf::SoundFileReader, sf::OutputSoundFile
///
////////////////////////////////////////////////////////////
