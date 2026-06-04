// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/MusicReader.hpp"

#include "Zancle/Audio/InputSoundFile.hpp"

#include "Zancle/System/AtomicMutex.hpp"
#include "Zancle/System/Err.hpp"
#include "Zancle/System/LockGuard.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/UniquePtr.hpp"


namespace za
{
////////////////////////////////////////////////////////////
struct MusicReader::Impl
{
    InputSoundFile file;  //!< Input sound file
    AtomicMutex    mutex; //!< Mutex protecting the data

    explicit Impl(InputSoundFile&& theFile) : file(ZB_MOVE(theFile))
    {
    }
};


////////////////////////////////////////////////////////////
MusicReader::MusicReader(zb::PassKey<MusicReader>&&, InputSoundFile&& file) :
    m_impl(zb::makeUnique<Impl>(ZB_MOVE(file)))
{
}


////////////////////////////////////////////////////////////
MusicReader::~MusicReader()                                 = default;
MusicReader::MusicReader(MusicReader&&) noexcept            = default;
MusicReader& MusicReader::operator=(MusicReader&&) noexcept = default;


////////////////////////////////////////////////////////////
zb::Optional<MusicReader> MusicReader::tryOpenFromInputSoundFile(zb::Optional<InputSoundFile>&& optFile,
                                                                   const char* const                errorContext)
{
    if (!optFile.hasValue())
    {
        priv::errMsg("Failed to open music from {}", errorContext);
        return zb::nullOpt;
    }

    return zb::makeOptional<MusicReader>(zb::PassKey<MusicReader>{}, ZB_MOVE(*optFile));
}


////////////////////////////////////////////////////////////
zb::Optional<MusicReader> MusicReader::openFromFile(const Path& filename)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromFile(filename), "file");
}


////////////////////////////////////////////////////////////
zb::Optional<MusicReader> MusicReader::openFromMemory(const void* const data, const zb::SizeT sizeInBytes)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromMemory(data, sizeInBytes), "memory");
}


////////////////////////////////////////////////////////////
zb::Optional<MusicReader> MusicReader::openFromStream(InputStream& stream)
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
[[nodiscard]] zb::U64 MusicReader::getSampleCount() const
{
    return m_impl->file.getSampleCount();
}


////////////////////////////////////////////////////////////
MusicReader::SeekAndReadResult MusicReader::seekAndRead(const zb::U64  sampleOffset,
                                                        zb::I16* const samples,
                                                        const zb::U64  maxCount)
{
    const LockGuard lock(m_impl->mutex);

    const auto clampedSampleOffset = m_impl->file.seek(sampleOffset);
    const auto readSamples         = m_impl->file.read(samples, maxCount);

    return SeekAndReadResult{clampedSampleOffset, readSamples};
}

} // namespace za
