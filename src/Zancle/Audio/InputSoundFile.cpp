// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/InputSoundFile.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/SoundFileFactory.hpp"
#include "Zancle/Audio/SoundFileReader.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/IO/FileInputStream.hpp"
#include "Zancle/IO/InputStream.hpp"
#include "Zancle/IO/MemoryInputStream.hpp"
#include "Zancle/IO/Path.hpp"
#include "Zancle/IO/PathUtils.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Math/MinMax.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
InputSoundFile::~InputSoundFile()                                    = default;
InputSoundFile::InputSoundFile(InputSoundFile&&) noexcept            = default;
InputSoundFile& InputSoundFile::operator=(InputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
InputSoundFile::StreamDeleter::StreamDeleter(bool theOwned) : owned(theOwned)
{
}


////////////////////////////////////////////////////////////
InputSoundFile::StreamDeleter::StreamDeleter(const za::UniquePtrDefaultDeleter&)
{
}


////////////////////////////////////////////////////////////
void InputSoundFile::StreamDeleter::operator()(InputStream* ptr) const
{
    if (owned)
        delete ptr;
}


////////////////////////////////////////////////////////////
za::Optional<InputSoundFile> InputSoundFile::openFromFile(const Path& filename)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromFilename(filename);
    if (!reader)
    {
        // Error message generated in called function.
        return za::nullOpt;
    }

    // Open the file
    auto fileInputStream = FileInputStream::open(filename);
    if (!fileInputStream.hasValue())
    {
        priv::errMsg("Failed to open input sound file from file (couldn't open file input stream)\n{}",
                     priv::PathDebugFormatter{filename});

        return za::nullOpt;
    }

    // Wrap the file into a stream
    auto file = za::makeUnique<FileInputStream>(ZA_MOVE(*fileInputStream));

    // Pass the stream to the reader
    auto info = reader->open(*file);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open input sound file from file (reader open failure)\n{}",
                     priv::PathDebugFormatter{filename});

        return za::nullOpt;
    }

    return za::makeOptional<InputSoundFile>(za::PassKey<InputSoundFile>{},
                                            ZA_MOVE(reader),
                                            ZA_MOVE(file),
                                            info->sampleCount,
                                            info->sampleRate,
                                            ZA_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
za::Optional<InputSoundFile> InputSoundFile::openFromMemory(const void* data, za::SizeT sizeInBytes)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromMemory(data, sizeInBytes);
    if (!reader)
    {
        // Error message generated in called function.
        return za::nullOpt;
    }

    // Wrap the memory file into a stream
    auto memory = za::makeUnique<MemoryInputStream>(data, sizeInBytes);

    // Pass the stream to the reader
    za::Optional info = reader->open(*memory);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open input sound file from memory (reader open failure)");
        return za::nullOpt;
    }

    return za::makeOptional<InputSoundFile>(za::PassKey<InputSoundFile>{},
                                            ZA_MOVE(reader),
                                            ZA_MOVE(memory),
                                            info->sampleCount,
                                            info->sampleRate,
                                            ZA_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
za::Optional<InputSoundFile> InputSoundFile::openFromStream(InputStream& stream)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromStream(stream);
    if (!reader)
    {
        // Error message generated in called function.
        return za::nullOpt;
    }

    // Don't forget to reset the stream to its beginning before re-opening it
    if (const za::Optional seekResult = stream.seek(0); !seekResult.hasValue() || *seekResult != 0)
    {
        priv::errMsg("Failed to open sound file from stream (cannot restart stream)");
        return za::nullOpt;
    }

    // Pass the stream to the reader
    za::Optional info = reader->open(stream);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open input sound file from stream (reader open failure)");
        return za::nullOpt;
    }

    return za::makeOptional<InputSoundFile>(za::PassKey<InputSoundFile>{},
                                            ZA_MOVE(reader),
                                            za::UniquePtr<InputStream, StreamDeleter>{&stream, false},
                                            info->sampleCount,
                                            info->sampleRate,
                                            ZA_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
za::U64 InputSoundFile::getSampleCount() const
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
    if (m_channelMap.isEmpty() || m_sampleRate == 0u)
        return Time{};

    ZA_ASSERT(m_sampleCount % m_channelMap.getSize() == 0u);
    const auto samplesPerChannel = m_sampleCount / m_channelMap.getSize();

    return microseconds(static_cast<za::I64>(samplesPerChannel * 1'000'000 / m_sampleRate));
}


////////////////////////////////////////////////////////////
Time InputSoundFile::getTimeOffset(const za::U64 sampleOffset) const
{
    // Make sure we don't divide by 0
    if (m_channelMap.isEmpty() || m_sampleRate == 0u)
        return Time{};

    ZA_ASSERT(sampleOffset % m_channelMap.getSize() == 0u);
    const auto sampleOffsetPerChannel = sampleOffset / m_channelMap.getSize();

    return microseconds(static_cast<za::I64>(sampleOffsetPerChannel * 1'000'000 / m_sampleRate));
}


////////////////////////////////////////////////////////////
za::U64 InputSoundFile::seek(za::U64 sampleOffset)
{
    ZA_ASSERT(m_reader != nullptr);

    if (m_channelMap.isEmpty())
        return 0u;

    // The reader handles an overrun gracefully, but we
    // pre-check to keep our known position consistent
    const auto clampedSampleOffset = za::min(sampleOffset / m_channelMap.getSize() * m_channelMap.getSize(), m_sampleCount);
    m_reader->seek(clampedSampleOffset);
    return clampedSampleOffset;
}


////////////////////////////////////////////////////////////
za::U64 InputSoundFile::seek(Time timeOffset)
{
    return seek(static_cast<za::SizeT>(timeOffset.asSeconds() * static_cast<float>(m_sampleRate)) * m_channelMap.getSize());
}


////////////////////////////////////////////////////////////
za::U64 InputSoundFile::read(za::I16* samples, za::U64 maxCount)
{
    ZA_ASSERT(m_reader != nullptr);

    za::U64 readSamples = 0u;

    if (samples && maxCount)
        readSamples = m_reader->read(samples, maxCount);

    return readSamples;
}


////////////////////////////////////////////////////////////
InputSoundFile::InputSoundFile(za::PassKey<InputSoundFile>&&,
                               za::UniquePtr<SoundFileReader>&&            reader,
                               za::UniquePtr<InputStream, StreamDeleter>&& stream,
                               za::U64                                     sampleCount,
                               unsigned int                                sampleRate,
                               ChannelMap&&                                channelMap) :
    m_reader(ZA_MOVE(reader)),
    m_stream(ZA_MOVE(stream)),
    m_sampleCount(sampleCount),
    m_sampleRate(sampleRate),
    m_channelMap(ZA_MOVE(channelMap))
{
}

} // namespace za
