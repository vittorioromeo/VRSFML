#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/ChannelMap.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class InputStream;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Abstract base class for sound file decoding
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundFileReader
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Structure holding the audio properties of a sound file
    ///
    ////////////////////////////////////////////////////////////
    struct Info
    {
        base::U64    sampleCount{};  //!< Total number of samples in the file
        unsigned int channelCount{}; //!< Number of channels of the sound
        unsigned int sampleRate{};   //!< Samples rate of the sound, in samples per second
        ChannelMap   channelMap;     //!< Map of position in sample frame to sound channel
    };

    ////////////////////////////////////////////////////////////
    /// \brief Virtual destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~SoundFileReader() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for reading
    ///
    /// The provided stream reference is valid as long as the
    /// `SoundFileReader` is alive, so it is safe to use/store it
    /// during the whole lifetime of the reader.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Properties of the loaded sound on success, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::Optional<Info> open(InputStream& stream) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current read position to the given sample offset
    ///
    /// The sample offset takes the channels into account.
    /// If you have a time offset instead, you can easily find
    /// the corresponding sample offset with the following formula:
    /// `timeInSeconds * sampleRate * channelCount`
    /// If the given offset exceeds to total number of samples,
    /// this function must jump to the end of the file.
    ///
    /// \param sampleOffset Index of the sample to jump to, relative to the beginning
    ///
    ////////////////////////////////////////////////////////////
    virtual void seek(base::U64 sampleOffset) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Read audio samples from the open file
    ///
    /// \param samples  Pointer to the sample array to fill
    /// \param maxCount Maximum number of samples to read
    ///
    /// \return Number of samples actually read (may be less than \a maxCount)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual base::U64 read(base::I16* samples, base::U64 maxCount) = 0;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundFileReader
/// \ingroup audio
///
/// This class allows users to read audio file formats not natively
/// supported by SFML, and thus extend the set of supported readable
/// audio formats.
///
/// A valid sound file reader must override the open, seek and write functions,
/// as well as providing a static check function; the latter is used by
/// SFML to find a suitable writer for a given input file.
///
/// To register a new reader, use the `sf::SoundFileFactory::registerReader`
/// template function.
///
/// Usage example:
/// \code
/// class MySoundFileReader : public sf::SoundFileReader
/// {
/// public:
///
///     [[nodiscard]] static bool check(sf::InputStream& stream)
///     {
///         // typically, read the first few header bytes and check fields that identify the format
///         // return true if the reader can handle the format
///     }
///
///     [[nodiscard]] sf::base::Optional<sf::SoundFileReader::Info> open(sf::InputStream& stream) override
///     {
///         // read the sound file header and fill the sound attributes
///         // (channel count, sample count and sample rate)
///         // return true on success
///     }
///
///     void seek(base::U64 sampleOffset) override
///     {
///         // advance to the sampleOffset-th sample from the beginning of the
///         sound
///     }
///
///     base::U64 read(base::I16* samples, base::U64 maxCount) override
///     {
///         // read up to 'maxCount' samples into the 'samples' array,
///         // convert them (for example from normalized float) if they are not stored
///         // as 16-bits signed integers in the file
///         // return the actual number of samples read
///     }
/// };
///
/// sf::SoundFileFactory::registerReader<MySoundFileReader>();
/// \endcode
///
/// \see `sf::InputSoundFile`, `sf::SoundFileFactory`, `sf::SoundFileWriter`
///
////////////////////////////////////////////////////////////
