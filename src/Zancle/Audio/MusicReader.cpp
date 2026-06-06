// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/MusicReader.hpp"

#include "Zancle/Audio/InputSoundFile.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Concurrency/AtomicMutex.hpp"
#include "Zancle/Concurrency/LockGuard.hpp"

#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"

#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
struct MusicReader::Impl
{
    InputSoundFile file;  //!< Input sound file
    AtomicMutex    mutex; //!< Mutex protecting the data

    explicit Impl(InputSoundFile&& theFile) : file(ZA_MOVE(theFile))
    {
    }
};


////////////////////////////////////////////////////////////
MusicReader::MusicReader(za::PassKey<MusicReader>&&, InputSoundFile&& file) :
    m_impl(za::makeUnique<Impl>(ZA_MOVE(file)))
{
}


////////////////////////////////////////////////////////////
MusicReader::~MusicReader()                                 = default;
MusicReader::MusicReader(MusicReader&&) noexcept            = default;
MusicReader& MusicReader::operator=(MusicReader&&) noexcept = default;


////////////////////////////////////////////////////////////
za::Optional<MusicReader> MusicReader::tryOpenFromInputSoundFile(za::Optional<InputSoundFile>&& optFile,
                                                                 const char* const              errorContext)
{
    if (!optFile.hasValue())
    {
        priv::errMsg("Failed to open music from {}", errorContext);
        return za::nullOpt;
    }

    return za::makeOptional<MusicReader>(za::PassKey<MusicReader>{}, ZA_MOVE(*optFile));
}


////////////////////////////////////////////////////////////
za::Optional<MusicReader> MusicReader::openFromFile(const Path& filename)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromFile(filename), "file");
}


////////////////////////////////////////////////////////////
za::Optional<MusicReader> MusicReader::openFromMemory(const void* const data, const za::SizeT sizeInBytes)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromMemory(data, sizeInBytes), "memory");
}


////////////////////////////////////////////////////////////
za::Optional<MusicReader> MusicReader::openFromStream(InputStream& stream)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromStream(stream), "stream");
}


////////////////////////////////////////////////////////////
Time MusicReader::getDuration() const
{
    return m_impl->file.getDuration();
}


////////////////////////////////////////////////////////////
unsigned int MusicReader::getChannelCount() const
{
    return m_impl->file.getChannelCount();
}


////////////////////////////////////////////////////////////
unsigned int MusicReader::getSampleRate() const
{
    return m_impl->file.getSampleRate();
}


////////////////////////////////////////////////////////////
const ChannelMap& MusicReader::getChannelMap() const
{
    return m_impl->file.getChannelMap();
}


////////////////////////////////////////////////////////////
[[nodiscard]] za::U64 MusicReader::getSampleCount() const
{
    return m_impl->file.getSampleCount();
}


////////////////////////////////////////////////////////////
MusicReader::SeekAndReadResult MusicReader::seekAndRead(const za::U64 sampleOffset, za::I16* const samples, const za::U64 maxCount)
{
    const LockGuard lock(m_impl->mutex);

    const auto clampedSampleOffset = m_impl->file.seek(sampleOffset);
    const auto readSamples         = m_impl->file.read(samples, maxCount);

    return SeekAndReadResult{clampedSampleOffset, readSamples};
}

} // namespace za
