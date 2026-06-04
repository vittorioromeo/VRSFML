#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Export.hpp"

#include "Zancle/Audio/ChannelMap.hpp"

#include "ZancleBase/IntTypes.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Path;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for sound file encoding
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API SoundFileWriter
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Virtual destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~SoundFileWriter() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for writing
    ///
    /// \param filename     Path of the file to open
    /// \param sampleRate   Sample rate of the sound
    /// \param channelCount Number of channels of the sound
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return `true` if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool open(const Path&       filename,
                                    unsigned int      sampleRate,
                                    unsigned int      channelCount,
                                    const ChannelMap& channelMap) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the open file
    ///
    /// \param samples Pointer to the sample array to write
    /// \param count   Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    virtual void write(const base::I16* samples, base::U64 count) = 0;
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::SoundFileWriter
/// \ingroup audio
///
/// This class allows users to write audio file formats not natively
/// supported by SFML, and thus extend the set of supported writable
/// audio formats.
///
/// A valid sound file writer must override the `open` and `write`
/// virtual functions, as well as providing a static `check`
/// function; the latter is used by SFML to find a suitable writer
/// for a given filename (typically by extension).
///
/// All sample data exchanged through this interface is 16-bit
/// signed PCM. Writers that encode to formats with different
/// internal sample formats (e.g. floats) are responsible for
/// converting on the fly inside `write`.
///
/// To register a new writer, use the `za::SoundFileFactory::registerWriter`
/// template function.
///
/// Usage example:
/// \code
/// class MySoundFileWriter : public za::SoundFileWriter
/// {
/// public:
///
///     [[nodiscard]] static bool check(const Path& filename)
///     {
///         // typically, check the extension
///         // return true if the writer can handle the format
///     }
///
///     [[nodiscard]] bool open(const Path& filename, unsigned int sampleRate, unsigned int channelCount, const
///     ChannelMap& channelMap) override
///     {
///         // open the file 'filename' for writing,
///         // write the given sample rate and channel count to the file header
///         // return true on success
///     }
///
///     void write(const base::I16* samples, base::U64 count) override
///     {
///         // write 'count' samples stored at address 'samples',
///         // convert them (for example to normalized float) if the format requires it
///     }
/// };
///
/// za::SoundFileFactory::registerWriter<MySoundFileWriter>();
/// \endcode
///
/// \see `za::OutputSoundFile`, `za::SoundFileFactory`, `za::SoundFileReader`
///
////////////////////////////////////////////////////////////
