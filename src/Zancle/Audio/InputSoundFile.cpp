// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/InputSoundFile.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/SoundFileFactory.hpp"
#include "Zancle/Audio/SoundFileReader.hpp"

#include "Zancle/System/Err.hpp"
#include "Zancle/System/FileInputStream.hpp"
#include "Zancle/System/InputStream.hpp"
#include "Zancle/System/MemoryInputStream.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/PathUtils.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/MinMax.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/UniquePtr.hpp"


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
InputSoundFile::StreamDeleter::StreamDeleter(const zb::UniquePtrDefaultDeleter&)
{
}


////////////////////////////////////////////////////////////
void InputSoundFile::StreamDeleter::operator()(InputStream* ptr) const
{
    if (owned)
        delete ptr;
}


////////////////////////////////////////////////////////////
zb::Optional<InputSoundFile> InputSoundFile::openFromFile(const Path& filename)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromFilename(filename);
    if (!reader)
    {
        // Error message generated in called function.
        return zb::nullOpt;
    }

    // Open the file
    auto fileInputStream = FileInputStream::open(filename);
    if (!fileInputStream.hasValue())
    {
        priv::errMsg("Failed to open input sound file from file (couldn't open file input stream)\n{}",
                     priv::PathDebugFormatter{filename});

        return zb::nullOpt;
    }

    // Wrap the file into a stream
    auto file = zb::makeUnique<FileInputStream>(ZB_MOVE(*fileInputStream));

    // Pass the stream to the reader
    auto info = reader->open(*file);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open input sound file from file (reader open failure)\n{}",
                     priv::PathDebugFormatter{filename});

        return zb::nullOpt;
    }

    return zb::makeOptional<InputSoundFile>(zb::PassKey<InputSoundFile>{},
                                            ZB_MOVE(reader),
                                            ZB_MOVE(file),
                                            info->sampleCount,
                                            info->sampleRate,
                                            ZB_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
zb::Optional<InputSoundFile> InputSoundFile::openFromMemory(const void* data, zb::SizeT sizeInBytes)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromMemory(data, sizeInBytes);
    if (!reader)
    {
        // Error message generated in called function.
        return zb::nullOpt;
    }

    // Wrap the memory file into a stream
    auto memory = zb::makeUnique<MemoryInputStream>(data, sizeInBytes);

    // Pass the stream to the reader
    zb::Optional info = reader->open(*memory);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open input sound file from memory (reader open failure)");
        return zb::nullOpt;
    }

    return zb::makeOptional<InputSoundFile>(zb::PassKey<InputSoundFile>{},
                                            ZB_MOVE(reader),
                                            ZB_MOVE(memory),
                                            info->sampleCount,
                                            info->sampleRate,
                                            ZB_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
zb::Optional<InputSoundFile> InputSoundFile::openFromStream(InputStream& stream)
{
    // Find a suitable reader for the file type
    auto reader = SoundFileFactory::createReaderFromStream(stream);
    if (!reader)
    {
        // Error message generated in called function.
        return zb::nullOpt;
    }

    // Don't forget to reset the stream to its beginning before re-opening it
    if (const zb::Optional seekResult = stream.seek(0); !seekResult.hasValue() || *seekResult != 0)
    {
        priv::errMsg("Failed to open sound file from stream (cannot restart stream)");
        return zb::nullOpt;
    }

    // Pass the stream to the reader
    zb::Optional info = reader->open(stream);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open input sound file from stream (reader open failure)");
        return zb::nullOpt;
    }

    return zb::makeOptional<InputSoundFile>(zb::PassKey<InputSoundFile>{},
                                            ZB_MOVE(reader),
                                            zb::UniquePtr<InputStream, StreamDeleter>{&stream, false},
                                            info->sampleCount,
                                            info->sampleRate,
                                            ZB_MOVE(info->channelMap));
}


////////////////////////////////////////////////////////////
zb::U64 InputSoundFile::getSampleCount() const
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

    ZB_ASSERT(m_sampleCount % m_channelMap.getSize() == 0u);
    const auto samplesPerChannel = m_sampleCount / m_channelMap.getSize();

    return microseconds(static_cast<zb::I64>(samplesPerChannel * 1'000'000 / m_sampleRate));
}


////////////////////////////////////////////////////////////
Time InputSoundFile::getTimeOffset(const zb::U64 sampleOffset) const
{
    // Make sure we don't divide by 0
    if (m_channelMap.isEmpty() || m_sampleRate == 0u)
        return Time{};

    ZB_ASSERT(sampleOffset % m_channelMap.getSize() == 0u);
    const auto sampleOffsetPerChannel = sampleOffset / m_channelMap.getSize();

    return microseconds(static_cast<zb::I64>(sampleOffsetPerChannel * 1'000'000 / m_sampleRate));
}


////////////////////////////////////////////////////////////
zb::U64 InputSoundFile::seek(zb::U64 sampleOffset)
{
    ZB_ASSERT(m_reader != nullptr);

    if (m_channelMap.isEmpty())
        return 0u;

    // The reader handles an overrun gracefully, but we
    // pre-check to keep our known position consistent
    const auto clampedSampleOffset = zb::min(sampleOffset / m_channelMap.getSize() * m_channelMap.getSize(), m_sampleCount);
    m_reader->seek(clampedSampleOffset);
    return clampedSampleOffset;
}


////////////////////////////////////////////////////////////
zb::U64 InputSoundFile::seek(Time timeOffset)
{
    return seek(static_cast<zb::SizeT>(timeOffset.asSeconds() * static_cast<float>(m_sampleRate)) * m_channelMap.getSize());
}


////////////////////////////////////////////////////////////
zb::U64 InputSoundFile::read(zb::I16* samples, zb::U64 maxCount)
{
    ZB_ASSERT(m_reader != nullptr);

    zb::U64 readSamples = 0u;

    if (samples && maxCount)
        readSamples = m_reader->read(samples, maxCount);

    return readSamples;
}


////////////////////////////////////////////////////////////
InputSoundFile::InputSoundFile(zb::PassKey<InputSoundFile>&&,
                               zb::UniquePtr<SoundFileReader>&&            reader,
                               zb::UniquePtr<InputStream, StreamDeleter>&& stream,
                               zb::U64                                     sampleCount,
                               unsigned int                                sampleRate,
                               ChannelMap&&                                channelMap) :
    m_reader(ZB_MOVE(reader)),
    m_stream(ZB_MOVE(stream)),
    m_sampleCount(sampleCount),
    m_sampleRate(sampleRate),
    m_channelMap(ZB_MOVE(channelMap))
{
}

} // namespace za
