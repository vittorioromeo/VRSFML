#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Export.hpp"

#include "Zancle/Audio/ChannelMap.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/UniquePtr.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Path;
class SoundFileWriter;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Provide write access to sound files
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API OutputSoundFile
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
    /// The output format is selected from the file extension.
    ///
    /// `channelCount` and `channelMap.getSize()` must agree.
    ///
    /// \param filename     Path of the sound file to write
    /// \param sampleRate   Sample rate of the sound, in samples per second
    /// \param channelCount Number of channels in the sound
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return Output sound file on success, `zb::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static zb::Optional<OutputSoundFile> openFromFile(
        const Path&       filename,
        unsigned int      sampleRate,
        unsigned int      channelCount,
        const ChannelMap& channelMap);

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the file
    ///
    /// Samples are interleaved 16-bit signed PCM. The total
    /// number of samples (`count`) must be a multiple of the
    /// channel count.
    ///
    /// \param samples     Pointer to the sample array to write
    /// \param count       Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    void write(const zb::I16* samples, zb::U64 count);

    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Constructor from writer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit OutputSoundFile(zb::PassKey<OutputSoundFile>&&, zb::UniquePtr<SoundFileWriter>&& writer);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    zb::UniquePtr<SoundFileWriter> m_writer; //!< Writer that handles I/O on the file's format
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::OutputSoundFile
/// \ingroup audio
///
/// This class encodes audio samples to a sound file. It is
/// used internally by higher-level classes such as `za::SoundBuffer`,
/// but can also be useful if you want to create audio files from
/// custom data sources, like generated audio samples.
///
/// Samples are always provided as 16-bit signed PCM. The
/// channel map describes how those samples map to physical
/// speaker positions for the encoded file.
///
/// Usage example:
/// \code
/// // Create a sound file, ogg/vorbis format, 44100 Hz, stereo (front-left/front-right)
/// const auto channelMap = za::ChannelMap{za::SoundChannel::FrontLeft, za::SoundChannel::FrontRight};
/// auto file = za::OutputSoundFile::openFromFile("music.ogg", 44100, 2, channelMap).value();
///
/// while (...)
/// {
///     // Read or generate audio samples from your custom source
///     std::vector<zb::I16> samples = ...;
///
///     // Write them to the file
///     file.write(samples.data(), samples.size());
/// }
/// \endcode
///
/// \see `za::SoundFileWriter`, `za::InputSoundFile`
///
////////////////////////////////////////////////////////////
