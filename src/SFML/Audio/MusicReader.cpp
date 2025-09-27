// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/MusicReader.hpp"

#include "SFML/Audio/InputSoundFile.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <mutex>


namespace sf
{
////////////////////////////////////////////////////////////
struct MusicReader::Impl
{
    InputSoundFile       file;  //!< Input sound file
    std::recursive_mutex mutex; //!< Mutex protecting the data

    explicit Impl(InputSoundFile&& theFile) : file(SFML_BASE_MOVE(theFile))
    {
    }
};


////////////////////////////////////////////////////////////
MusicReader::MusicReader(base::PassKey<MusicReader>&&, InputSoundFile&& file) :
    m_impl(base::makeUnique<Impl>(SFML_BASE_MOVE(file)))
{
}


////////////////////////////////////////////////////////////
MusicReader::~MusicReader()                                 = default;
MusicReader::MusicReader(MusicReader&&) noexcept            = default;
MusicReader& MusicReader::operator=(MusicReader&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<MusicReader> MusicReader::tryOpenFromInputSoundFile(base::Optional<InputSoundFile>&& optFile,
                                                                   const char* const                errorContext)
{
    if (!optFile.hasValue())
    {
        priv::err() << "Failed to open music from " << errorContext;
        return base::nullOpt;
    }

    return base::makeOptional<MusicReader>(base::PassKey<MusicReader>{}, SFML_BASE_MOVE(*optFile));
}


////////////////////////////////////////////////////////////
base::Optional<MusicReader> MusicReader::openFromFile(const Path& filename)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromFile(filename), "file");
}


////////////////////////////////////////////////////////////
base::Optional<MusicReader> MusicReader::openFromMemory(const void* const data, const base::SizeT sizeInBytes)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromMemory(data, sizeInBytes), "memory");
}


////////////////////////////////////////////////////////////
base::Optional<MusicReader> MusicReader::openFromStream(InputStream& stream)
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
[[nodiscard]] base::U64 MusicReader::getSampleCount() const
{
    return m_impl->file.getSampleCount();
}


////////////////////////////////////////////////////////////
MusicReader::SeekAndReadResult MusicReader::seekAndRead(const base::U64  sampleOffset,
                                                        base::I16* const samples,
                                                        const base::U64  maxCount)
{
    const std::lock_guard lock(m_impl->mutex);

    const auto clampedSampleOffset = m_impl->file.seek(sampleOffset);
    const auto readSamples         = m_impl->file.read(samples, maxCount);

    return SeekAndReadResult{clampedSampleOffset, readSamples};
}

} // namespace sf
