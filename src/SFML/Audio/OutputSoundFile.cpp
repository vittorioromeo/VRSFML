#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/OutputSoundFile.hpp>
#include <SFML/Audio/SoundFileFactory.hpp>
#include <SFML/Audio/SoundFileWriter.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Path.hpp>

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
OutputSoundFile::~OutputSoundFile() = default;


////////////////////////////////////////////////////////////
OutputSoundFile::OutputSoundFile(OutputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
OutputSoundFile& OutputSoundFile::operator=(OutputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<OutputSoundFile> OutputSoundFile::openFromFile(
    const Path&       filename,
    unsigned int      sampleRate,
    unsigned int      channelCount,
    const ChannelMap& channelMap)
{
    // Find a suitable writer for the file type
    auto writer = SoundFileFactory::createWriterFromFilename(filename);
    if (!writer)
    {
        // Error message generated in called function.
        return base::nullOpt;
    }

    // Pass the stream to the reader
    if (!writer->open(filename, sampleRate, channelCount, channelMap))
    {
        priv::err() << "Failed to open output sound file from file (writer open failure)";
        return base::nullOpt;
    }

    return sf::base::makeOptional<OutputSoundFile>(base::PassKey<OutputSoundFile>{}, SFML_BASE_MOVE(writer));
}


////////////////////////////////////////////////////////////
void OutputSoundFile::write(const std::int16_t* samples, std::uint64_t count)
{
    SFML_BASE_ASSERT(m_writer != nullptr);

    if (samples && count)
        m_writer->write(samples, count);
}


////////////////////////////////////////////////////////////
OutputSoundFile::OutputSoundFile(base::PassKey<OutputSoundFile>&&, base::UniquePtr<SoundFileWriter>&& writer) :
m_writer(SFML_BASE_MOVE(writer))
{
}

} // namespace sf
