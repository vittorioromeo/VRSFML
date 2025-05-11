#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/InputSoundFile.hpp"
#include "SFML/Audio/MusicSource.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <mutex>


namespace sf
{
////////////////////////////////////////////////////////////
struct MusicSource::Impl
{
    InputSoundFile       file;  //!< Input sound file
    std::recursive_mutex mutex; //!< Mutex protecting the data

    explicit Impl(InputSoundFile&& theFile) : file(SFML_BASE_MOVE(theFile))
    {
    }
};


////////////////////////////////////////////////////////////
MusicSource::MusicSource(base::PassKey<MusicSource>&&, InputSoundFile&& file) :
m_impl(base::makeUnique<Impl>(SFML_BASE_MOVE(file)))
{
}


////////////////////////////////////////////////////////////
MusicSource::~MusicSource() = default;


////////////////////////////////////////////////////////////
MusicSource::MusicSource(MusicSource&&) noexcept = default;


////////////////////////////////////////////////////////////
MusicSource& MusicSource::operator=(MusicSource&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<MusicSource> MusicSource::tryOpenFromInputSoundFile(base::Optional<InputSoundFile>&& optFile,
                                                                   const char* const                errorContext)
{
    if (!optFile.hasValue())
    {
        priv::err() << "Failed to open music from " << errorContext;
        return base::nullOpt;
    }

    return base::makeOptional<MusicSource>(base::PassKey<MusicSource>{}, SFML_BASE_MOVE(*optFile));
}


////////////////////////////////////////////////////////////
base::Optional<MusicSource> MusicSource::openFromFile(const Path& filename)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromFile(filename), "file");
}


////////////////////////////////////////////////////////////
base::Optional<MusicSource> MusicSource::openFromMemory(const void* const data, const base::SizeT sizeInBytes)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromMemory(data, sizeInBytes), "memory");
}


////////////////////////////////////////////////////////////
base::Optional<MusicSource> MusicSource::openFromStream(InputStream& stream)
{
    return tryOpenFromInputSoundFile(InputSoundFile::openFromStream(stream), "stream");
}


////////////////////////////////////////////////////////////
Time MusicSource::getDuration() const
{
    return m_impl->file.getDuration();
}


////////////////////////////////////////////////////////////
unsigned int MusicSource::getChannelCount() const
{
    return m_impl->file.getChannelCount();
}


////////////////////////////////////////////////////////////
unsigned int MusicSource::getSampleRate() const
{
    return m_impl->file.getSampleRate();
}


////////////////////////////////////////////////////////////
ChannelMap MusicSource::getChannelMap() const
{
    return m_impl->file.getChannelMap();
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::U64 MusicSource::getSampleCount() const
{
    return m_impl->file.getSampleCount();
}


////////////////////////////////////////////////////////////
MusicSource::SeekAndReadResult MusicSource::seekAndRead(const base::U64  sampleOffset,
                                                        base::I16* const samples,
                                                        const base::U64  maxCount)
{
    const std::lock_guard lock(m_impl->mutex);

    const auto clampedSampleOffset = m_impl->file.seek(sampleOffset);
    const auto readSamples         = m_impl->file.read(samples, maxCount);

    return SeekAndReadResult{clampedSampleOffset, readSamples};
}

} // namespace sf
