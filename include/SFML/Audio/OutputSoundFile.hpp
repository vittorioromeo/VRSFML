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

#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Path;
class SoundFileWriter;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Provide write access to sound files
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API OutputSoundFile
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    OutputSoundFile(OutputSoundFile&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    OutputSoundFile& operator=(OutputSoundFile&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~OutputSoundFile();

    ////////////////////////////////////////////////////////////
    /// \brief Open the sound file from the disk for writing
    ///
    /// The supported audio formats are: WAV, OGG/Vorbis, FLAC.
    ///
    /// \param filename     Path of the sound file to write
    /// \param sampleRate   Sample rate of the sound
    /// \param channelCount Number of channels in the sound
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return Output sound file if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<OutputSoundFile> openFromFile(
        const Path&       filename,
        unsigned int      sampleRate,
        unsigned int      channelCount,
        const ChannelMap& channelMap);

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the file
    ///
    /// \param samples     Pointer to the sample array to write
    /// \param count       Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    void write(const std::int16_t* samples, std::uint64_t count);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Constructor from writer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit OutputSoundFile(base::PassKey<OutputSoundFile>&&, base::UniquePtr<SoundFileWriter>&& writer);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::UniquePtr<SoundFileWriter> m_writer; //!< Writer that handles I/O on the file's format
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::OutputSoundFile
/// \ingroup audio
///
/// This class encodes audio samples to a sound file. It is
/// used internally by higher-level classes such as sf::SoundBuffer,
/// but can also be useful if you want to create audio files from
/// custom data sources, like generated audio samples.
///
/// Usage example:
/// \code
/// // Create a sound file, ogg/vorbis format, 44100 Hz, stereo
/// auto file = sf::OutputSoundFile::openFromFile("music.ogg", 44100, 2).value();
///
/// while (...)
/// {
///     // Read or generate audio samples from your custom source
///     std::vector<std::int16_t> samples = ...;
///
///     // Write them to the file
///     file.write(samples.data(), samples.size());
/// }
/// \endcode
///
/// \see sf::SoundFileWriter, sf::InputSoundFile
///
////////////////////////////////////////////////////////////
