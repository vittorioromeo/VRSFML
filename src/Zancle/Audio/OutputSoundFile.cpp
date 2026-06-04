// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/OutputSoundFile.hpp"
#include "Zancle/Audio/SoundFileFactory.hpp"
#include "Zancle/Audio/SoundFileWriter.hpp"
#include "Zancle/System/Err.hpp"
#include "Zancle/System/Path.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/UniquePtr.hpp"


namespace za
{
////////////////////////////////////////////////////////////
OutputSoundFile::~OutputSoundFile()                                     = default;
OutputSoundFile::OutputSoundFile(OutputSoundFile&&) noexcept            = default;
OutputSoundFile& OutputSoundFile::operator=(OutputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
zb::Optional<OutputSoundFile> OutputSoundFile::openFromFile(
    const Path&       filename,
    unsigned int      sampleRate,
    unsigned int      channelCount,
    const ChannelMap& channelMap)
{
    ZB_ASSERT(channelCount == channelMap.getSize() && "channelCount must match channelMap size");

    if (channelCount != channelMap.getSize())
    {
        priv::errMsg("Channel count ({}) does not match channel map size ({})", channelCount, channelMap.getSize());

        return zb::nullOpt;
    }

    // Find a suitable writer for the file type
    auto writer = SoundFileFactory::createWriterFromFilename(filename);
    if (!writer)
    {
        // Error message generated in called function.
        return zb::nullOpt;
    }

    // Pass the stream to the reader
    if (!writer->open(filename, sampleRate, channelCount, channelMap))
    {
        priv::errMsg("Failed to open output sound file from file (writer open failure)");
        return zb::nullOpt;
    }

    return zb::makeOptional<OutputSoundFile>(zb::PassKey<OutputSoundFile>{}, ZB_MOVE(writer));
}


////////////////////////////////////////////////////////////
void OutputSoundFile::write(const zb::I16* samples, zb::U64 count)
{
    ZB_ASSERT(m_writer != nullptr);

    if (samples && count)
        m_writer->write(samples, count);
}


////////////////////////////////////////////////////////////
OutputSoundFile::OutputSoundFile(zb::PassKey<OutputSoundFile>&&, zb::UniquePtr<SoundFileWriter>&& writer) :
    m_writer(ZB_MOVE(writer))
{
}

} // namespace za
