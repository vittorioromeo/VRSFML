// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/OutputSoundFile.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/SoundFileFactory.hpp"
#include "Zancle/Audio/SoundFileWriter.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/Macros.hpp"


namespace za
{
////////////////////////////////////////////////////////////
OutputSoundFile::~OutputSoundFile()                                     = default;
OutputSoundFile::OutputSoundFile(OutputSoundFile&&) noexcept            = default;
OutputSoundFile& OutputSoundFile::operator=(OutputSoundFile&&) noexcept = default;


////////////////////////////////////////////////////////////
za::Optional<OutputSoundFile> OutputSoundFile::openFromFile(
    const Path&       filename,
    unsigned int      sampleRate,
    unsigned int      channelCount,
    const ChannelMap& channelMap)
{
    ZA_ASSERT(channelCount == channelMap.getSize() && "channelCount must match channelMap size");

    if (channelCount != channelMap.getSize())
    {
        priv::errMsg("Channel count ({}) does not match channel map size ({})", channelCount, channelMap.getSize());

        return za::nullOpt;
    }

    // Find a suitable writer for the file type
    auto writer = SoundFileFactory::createWriterFromFilename(filename);
    if (!writer)
    {
        // Error message generated in called function.
        return za::nullOpt;
    }

    // Pass the stream to the reader
    if (!writer->open(filename, sampleRate, channelCount, channelMap))
    {
        priv::errMsg("Failed to open output sound file from file (writer open failure)");
        return za::nullOpt;
    }

    return za::makeOptional<OutputSoundFile>(za::PassKey<OutputSoundFile>{}, ZA_MOVE(writer));
}


////////////////////////////////////////////////////////////
void OutputSoundFile::write(const za::I16* samples, za::U64 count)
{
    ZA_ASSERT(m_writer != nullptr);

    if (samples && count)
        m_writer->write(samples, count);
}


////////////////////////////////////////////////////////////
OutputSoundFile::OutputSoundFile(za::PassKey<OutputSoundFile>&&, za::UniquePtr<SoundFileWriter>&& writer) :
    m_writer(ZA_MOVE(writer))
{
}

} // namespace za
