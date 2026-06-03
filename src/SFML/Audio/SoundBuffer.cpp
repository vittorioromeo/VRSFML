// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/OutputSoundFile.hpp"
#include "SFML/Audio/SoundChannel.hpp"
#include "SFML/Audio/SoundFileFactory.hpp"
#include "SFML/Audio/SoundFileReader.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/MemoryInputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundBuffer::Impl
{
    ////////////////////////////////////////////////////////////
    /// Allocate `samples` to hold exactly `theSampleCount` elements without
    /// zero-initializing them; the caller is expected to fill the buffer via
    /// `samples.data()` immediately after construction -- either by reading
    /// from a `SoundFileReader` or by `memcpy`'ing caller-provided samples in.
    /// `duration` is computed from the requested count.
    // NOLINTNEXTLINE(modernize-pass-by-value)
    explicit Impl(base::SizeT theSampleCount, const ChannelMap& theChannelMap, const unsigned int theSampleRate) :
        channelMap(theChannelMap),
        sampleRate(theSampleRate)
    {
        SFML_BASE_ASSERT(channelMap.getSize() > 0u);
        SFML_BASE_ASSERT(sampleRate > 0u);

        samples.reserve(theSampleCount);
        samples.unsafeSetSize(theSampleCount);

        duration = seconds(static_cast<float>(theSampleCount) / static_cast<float>(sampleRate) /
                           static_cast<float>(channelMap.getSize()));
    }

    ////////////////////////////////////////////////////////////
    base::Vector<base::I16> samples;                        //!< Samples buffer
    ChannelMap              channelMap{SoundChannel::Mono}; //!< The map of position in sample frame to sound channel
    unsigned int            sampleRate{44'100};             //!< Number of samples per second
    Time                    duration;                       //!< Sound duration
};


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(const SoundBuffer& rhs)                = default;
SoundBuffer& SoundBuffer::operator=(const SoundBuffer& rhs)     = default;
SoundBuffer::SoundBuffer(SoundBuffer&& rhs) noexcept            = default;
SoundBuffer& SoundBuffer::operator=(SoundBuffer&& rhs) noexcept = default;
SoundBuffer::~SoundBuffer()                                     = default;


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromFile(const Path& filename)
{
    // `SoundBuffer` decodes the entire file into PCM up front, so we don't need
    // `InputSoundFile`'s streaming machinery (which exists for `MusicReader`).
    // Read the encoded bytes through the native fast path, then delegate to the
    // memory-based loader which picks a codec and decodes in-place.
    base::Vector<char>& scratch = getThreadLocalScratchCharBuffer();

    if (!readFromFile(filename, scratch))
    {
        priv::errMsg("Failed to open sound buffer from file");
        return base::nullOpt;
    }

    return loadFromMemory(scratch.data(), scratch.size());
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromMemory(const void* data, base::SizeT sizeInBytes)
{
    MemoryInputStream stream{data, sizeInBytes};
    return loadFromStream(stream);
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromStream(InputStream& stream)
{
    // Single named return so NRVO can apply -- failure paths return the
    // already-empty `buf`, success path emplaces into it.
    base::Optional<SoundBuffer> buf;

    auto reader = SoundFileFactory::createReaderFromStream(stream);
    if (reader == nullptr)
    {
        priv::errMsg("Failed to open sound buffer (no codec for the data's format)");
        return buf;
    }

    // `createReaderFromStream` advances the read position while sniffing the codec; rewind before handing the stream to `open`.
    if (const base::Optional seekResult = stream.seek(0); !seekResult.hasValue() || *seekResult != 0)
    {
        priv::errMsg("Failed to open sound buffer (rewind after codec detection failed)");
        return buf;
    }

    const base::Optional info = reader->open(stream);
    if (!info.hasValue())
    {
        priv::errMsg("Failed to open sound buffer (codec rejected the data)");
        return buf;
    }

    if (info->channelMap.isEmpty() || info->sampleRate == 0u)
    {
        priv::errMsg("Failed to load sound buffer (codec returned invalid metadata)");
        return buf;
    }

    // On 32-bit targets (e.g. Emscripten) `SizeT` is narrower than `U64`; reject files we can't address.
    if (info->sampleCount > static_cast<base::U64>(static_cast<base::SizeT>(-1)))
    {
        priv::errMsg("Failed to load sound buffer (sample count exceeds addressable range on this platform)");
        return buf;
    }

    const auto sampleCount = static_cast<base::SizeT>(info->sampleCount);

    // Construct the `SoundBuffer` with `m_impl->samples` already pre-sized and read directly into it.
    buf.emplace(base::PassKey<SoundBuffer>{}, sampleCount, info->channelMap, info->sampleRate);

    if (reader->read(buf->m_impl->samples.data(), sampleCount) != info->sampleCount)
        buf.reset();

    return buf;
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromSamples(const base::I16*   samples,
                                                         const base::SizeT  sampleCount,
                                                         const ChannelMap&  channelMap,
                                                         const unsigned int sampleRate)
{
    base::Optional<SoundBuffer> buf;

    if (channelMap.isEmpty() || sampleRate == 0u)
    {
        priv::errMsg("Failed to load sound buffer from samples (count: {}, channels: {}, sample rate: {})",
                     sampleCount,
                     channelMap.getSize(),
                     sampleRate);

        return buf;
    }

    buf.emplace(base::PassKey<SoundBuffer>{}, sampleCount, channelMap, sampleRate);
    SFML_BASE_MEMCPY(buf->m_impl->samples.data(), samples, sampleCount * sizeof(base::I16));
    return buf;
}


////////////////////////////////////////////////////////////
bool SoundBuffer::saveToFile(const Path& filename) const
{
    // Create the sound file in write mode
    if (base::Optional file = OutputSoundFile::openFromFile(filename, getSampleRate(), getChannelCount(), getChannelMap()))
    {
        // Write the samples to the opened file
        file->write(m_impl->samples.data(), m_impl->samples.size());
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
const base::I16* SoundBuffer::getSamples() const
{
    return m_impl->samples.empty() ? nullptr : m_impl->samples.data();
}


////////////////////////////////////////////////////////////
base::U64 SoundBuffer::getSampleCount() const
{
    return m_impl->samples.size();
}


////////////////////////////////////////////////////////////
unsigned int SoundBuffer::getSampleRate() const
{
    return m_impl->sampleRate;
}


////////////////////////////////////////////////////////////
unsigned int SoundBuffer::getChannelCount() const
{
    return static_cast<unsigned int>(m_impl->channelMap.getSize());
}


////////////////////////////////////////////////////////////
const ChannelMap& SoundBuffer::getChannelMap() const
{
    return m_impl->channelMap;
}


////////////////////////////////////////////////////////////
Time SoundBuffer::getDuration() const
{
    return m_impl->duration;
}


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(base::PassKey<SoundBuffer>&&,
                         base::SizeT       sampleCount,
                         const ChannelMap& channelMap,
                         unsigned int      sampleRate) :
    m_impl(sampleCount, channelMap, sampleRate)
{
}


} // namespace sf
