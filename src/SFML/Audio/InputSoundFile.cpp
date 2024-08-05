#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/ChannelMap.hpp>
#include <SFML/Audio/InputSoundFile.hpp>
#include <SFML/Audio/SoundFileFactory.hpp>
#include <SFML/Audio/SoundFileReader.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/MemoryInputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/PathUtils.hpp>
#include <SFML/System/Time.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>

#include <cstdint>


namespace sf
{
////////////////////////////////////////////////////////////
InputSoundFile::~InputSoundFile() = default;


////////////////////////////////////////////////////////////
InputSoundFile::InputSoundFile(InputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
InputSoundFile& InputSoundFile::operator=(InputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
InputSoundFile::StreamDeleter::StreamDeleter(bool theOwned) : owned(theOwned)
{
}


////////////////////////////////////////////////////////////
InputSoundFile::StreamDeleter::StreamDeleter(const base::UniquePtrDefaultDeleter&)
{
}


////////////////////////////////////////////////////////////
void InputSoundFile::StreamDeleter::operator()(InputStream* ptr) const
{
    if (owned)
        delete ptr;
}


////////////////////////////////////////////////////////////
base::Optional<InputSoundFile> InputSoundFile::openFromFile(const Path& filename)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromFilename(filename);
    if (!reader)
    {
        // Error message generated in called function.
        return base::nullOpt;
    }

    // Open the file
    auto fileInputStream = FileInputStream::open(filename);
    if (!fileInputStream.hasValue())
    {
        priv::err() << "Failed to open input sound file from file (couldn't open file input stream)\n"
                    << priv::PathDebugFormatter{filename};

        return base::nullOpt;
    }

    // Wrap the file into a stream
    auto file = base::makeUnique<FileInputStream>(SFML_BASE_MOVE(*fileInputStream));

    // Pass the stream to the reader
    auto info = reader->open(*file);
    if (!info.hasValue())
    {
        priv::err() << "Failed to open input sound file from file (reader open failure)\n"
                    << priv::PathDebugFormatter{filename};

        return base::nullOpt;
    }

    return base::makeOptional<InputSoundFile>(base::PassKey<InputSoundFile>{},
                                              SFML_BASE_MOVE(reader),
                                              SFML_BASE_MOVE(file),
                                              info->sampleCount,
                                              info->sampleRate,
                                              SFML_BASE_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
base::Optional<InputSoundFile> InputSoundFile::openFromMemory(const void* data, std::size_t sizeInBytes)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromMemory(data, sizeInBytes);
    if (!reader)
    {
        // Error message generated in called function.
        return base::nullOpt;
    }

    // Wrap the memory file into a stream
    auto memory = base::makeUnique<MemoryInputStream>(data, sizeInBytes);

    // Pass the stream to the reader
    base::Optional info = reader->open(*memory);
    if (!info.hasValue())
    {
        priv::err() << "Failed to open input sound file from memory (reader open failure)";
        return base::nullOpt;
    }

    return base::makeOptional<InputSoundFile>(base::PassKey<InputSoundFile>{},
                                              SFML_BASE_MOVE(reader),
                                              SFML_BASE_MOVE(memory),
                                              info->sampleCount,
                                              info->sampleRate,
                                              SFML_BASE_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
base::Optional<InputSoundFile> InputSoundFile::openFromStream(InputStream& stream)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromStream(stream);
    if (!reader)
    {
        // Error message generated in called function.
        return base::nullOpt;
    }

    // Don't forget to reset the stream to its beginning before re-opening it
    if (const base::Optional seekResult = stream.seek(0); !seekResult.hasValue() || *seekResult != 0)
    {
        priv::err() << "Failed to open sound file from stream (cannot restart stream)";
        return base::nullOpt;
    }

    // Pass the stream to the reader
    base::Optional info = reader->open(stream);
    if (!info.hasValue())
    {
        priv::err() << "Failed to open input sound file from stream (reader open failure)";
        return base::nullOpt;
    }

    return base::makeOptional<InputSoundFile>(base::PassKey<InputSoundFile>{},
                                              SFML_BASE_MOVE(reader),
                                              base::UniquePtr<InputStream, StreamDeleter>{&stream, false},
                                              info->sampleCount,
                                              info->sampleRate,
                                              SFML_BASE_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
std::uint64_t InputSoundFile::getSampleCount() const
{
    return m_sampleCount;
}


////////////////////////////////////////////////////////////
unsigned int InputSoundFile::getChannelCount() const
{
    return static_cast<unsigned int>(m_channelMap.getSize());
}


////////////////////////////////////////////////////////////
unsigned int InputSoundFile::getSampleRate() const
{
    return m_sampleRate;
}


////////////////////////////////////////////////////////////
const ChannelMap& InputSoundFile::getChannelMap() const
{
    return m_channelMap;
}


////////////////////////////////////////////////////////////
Time InputSoundFile::getDuration() const
{
    // Make sure we don't divide by 0
    if (m_channelMap.isEmpty() || m_sampleRate == 0)
        return Time::Zero;

    return seconds(static_cast<float>(m_sampleCount) / static_cast<float>(m_channelMap.getSize()) /
                   static_cast<float>(m_sampleRate));
}


////////////////////////////////////////////////////////////
Time InputSoundFile::getTimeOffset() const
{
    // Make sure we don't divide by 0
    if (m_channelMap.isEmpty() || m_sampleRate == 0)
        return Time::Zero;

    return seconds(static_cast<float>(m_sampleOffset) / static_cast<float>(m_channelMap.getSize()) /
                   static_cast<float>(m_sampleRate));
}


////////////////////////////////////////////////////////////
std::uint64_t InputSoundFile::getSampleOffset() const
{
    return m_sampleOffset;
}


////////////////////////////////////////////////////////////
void InputSoundFile::seek(std::uint64_t sampleOffset)
{
    SFML_BASE_ASSERT(m_reader != nullptr);

    if (!m_channelMap.isEmpty())
    {
        // The reader handles an overrun gracefully, but we
        // pre-check to keep our known position consistent
        m_sampleOffset = base::min(sampleOffset / m_channelMap.getSize() * m_channelMap.getSize(), m_sampleCount);
        m_reader->seek(m_sampleOffset);
    }
}


////////////////////////////////////////////////////////////
void InputSoundFile::seek(Time timeOffset)
{
    seek(static_cast<std::size_t>(timeOffset.asSeconds() * static_cast<float>(m_sampleRate)) * m_channelMap.getSize());
}


////////////////////////////////////////////////////////////
std::uint64_t InputSoundFile::read(std::int16_t* samples, std::uint64_t maxCount)
{
    SFML_BASE_ASSERT(m_reader != nullptr);

    std::uint64_t readSamples = 0;
    if (samples && maxCount)
        readSamples = m_reader->read(samples, maxCount);
    m_sampleOffset += readSamples;
    return readSamples;
}


////////////////////////////////////////////////////////////
InputSoundFile::InputSoundFile(base::PassKey<InputSoundFile>&&,
                               base::UniquePtr<SoundFileReader>&&            reader,
                               base::UniquePtr<InputStream, StreamDeleter>&& stream,
                               std::uint64_t                                 sampleCount,
                               unsigned int                                  sampleRate,
                               ChannelMap&&                                  channelMap) :
m_reader(SFML_BASE_MOVE(reader)),
m_stream(SFML_BASE_MOVE(stream)),
m_sampleCount(sampleCount),
m_sampleRate(sampleRate),
m_channelMap(SFML_BASE_MOVE(channelMap))
{
}

} // namespace sf
