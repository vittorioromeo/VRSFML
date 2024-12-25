#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/ChannelMap.hpp"

#include "SFML/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Path;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for sound file encoding
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundFileWriter
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

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundFileWriter
/// \ingroup audio
///
/// This class allows users to write audio file formats not natively
/// supported by SFML, and thus extend the set of supported writable
/// audio formats.
///
/// A valid sound file writer must override the open and write functions,
/// as well as providing a static check function; the latter is used by
/// SFML to find a suitable writer for a given filename.
///
/// To register a new writer, use the `sf::SoundFileFactory::registerWriter`
/// template function.
///
/// Usage example:
/// \code
/// class MySoundFileWriter : public sf::SoundFileWriter
/// {
/// public:
///
///     [[nodiscard]] static bool check(const Path& filename)
///     {
///         // typically, check the extension
///         // return true if the writer can handle the format
///     }
///
///     [[nodiscard]] bool open(const Path& filename, unsigned int sampleRate, unsigned int channelCount, const ChannelMap& channelMap) override
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
/// sf::SoundFileFactory::registerWriter<MySoundFileWriter>();
/// \endcode
///
/// \see `sf::OutputSoundFile`, `sf::SoundFileFactory`, `sf::SoundFileReader`
///
////////////////////////////////////////////////////////////
