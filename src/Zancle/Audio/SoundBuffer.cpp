// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/SoundBuffer.hpp"

#include "Zancle/Audio/ChannelMap.hpp"
#include "Zancle/Audio/OutputSoundFile.hpp"
#include "Zancle/Audio/SoundChannel.hpp"
#include "Zancle/Audio/SoundFileFactory.hpp"
#include "Zancle/Audio/SoundFileReader.hpp"

#include "Zancle/Err/Err.hpp"
#include "Zancle/IO/IO.hpp"
#include "Zancle/IO/InputStream.hpp"
#include "Zancle/IO/MemoryInputStream.hpp"
#include "Zancle/IO/Path.hpp"
#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/Memcpy.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"
#include "Zancle/Container/Vector.hpp"


namespace za
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
    explicit Impl(za::SizeT theSampleCount, const ChannelMap& theChannelMap, const unsigned int theSampleRate) :
        channelMap(theChannelMap),
        sampleRate(theSampleRate)
    {
        ZA_ASSERT(channelMap.getSize() > 0u);
        ZA_ASSERT(sampleRate > 0u);

        samples.reserve(theSampleCount);
        samples.unsafeSetSize(theSampleCount);

        duration = seconds(static_cast<float>(theSampleCount) / static_cast<float>(sampleRate) /
                           static_cast<float>(channelMap.getSize()));
    }

    ////////////////////////////////////////////////////////////
    za::Vector<za::I16> samples;                        //!< Samples buffer
    ChannelMap          channelMap{SoundChannel::Mono}; //!< The map of position in sample frame to sound channel
    unsigned int        sampleRate{44'100};             //!< Number of samples per second
    Time                duration;                       //!< Sound duration
};


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(const SoundBuffer& rhs)                = default;
SoundBuffer& SoundBuffer::operator=(const SoundBuffer& rhs)     = default;
SoundBuffer::SoundBuffer(SoundBuffer&& rhs) noexcept            = default;
SoundBuffer& SoundBuffer::operator=(SoundBuffer&& rhs) noexcept = default;
SoundBuffer::~SoundBuffer()                                     = default;


////////////////////////////////////////////////////////////
za::Optional<SoundBuffer> SoundBuffer::loadFromFile(const Path& filename)
{
    // `SoundBuffer` decodes the entire file into PCM up front, so we don't need
    // `InputSoundFile`'s streaming machinery (which exists for `MusicReader`).
    // Read the encoded bytes through the native fast path, then delegate to the
    // memory-based loader which picks a codec and decodes in-place.
    za::Vector<char>& scratch = getThreadLocalScratchCharBuffer();

    if (!readFromFile(filename, scratch))
    {
        priv::errMsg("Failed to open sound buffer from file");
        return za::nullOpt;
    }

    return loadFromMemory(scratch.data(), scratch.size());
}


////////////////////////////////////////////////////////////
za::Optional<SoundBuffer> SoundBuffer::loadFromMemory(const void* data, za::SizeT sizeInBytes)
{
    MemoryInputStream stream{data, sizeInBytes};
    return loadFromStream(stream);
}


////////////////////////////////////////////////////////////
za::Optional<SoundBuffer> SoundBuffer::loadFromStream(InputStream& stream)
{
    // Single named return so NRVO can apply -- failure paths return the
    // already-empty `buf`, success path emplaces into it.
    za::Optional<SoundBuffer> buf;

    auto reader = SoundFileFactory::createReaderFromStream(stream);
    if (reader == nullptr)
    {
        priv::errMsg("Failed to open sound buffer (no codec for the data's format)");
        return buf;
    }

    // `createReaderFromStream` advances the read position while sniffing the codec; rewind before handing the stream to `open`.
    if (const za::Optional seekResult = stream.seek(0); !seekResult.hasValue() || *seekResult != 0)
    {
        priv::errMsg("Failed to open sound buffer (rewind after codec detection failed)");
        return buf;
    }

    const za::Optional info = reader->open(stream);
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
    if (info->sampleCount > static_cast<za::U64>(static_cast<za::SizeT>(-1)))
    {
        priv::errMsg("Failed to load sound buffer (sample count exceeds addressable range on this platform)");
        return buf;
    }

    const auto sampleCount = static_cast<za::SizeT>(info->sampleCount);

    // Construct the `SoundBuffer` with `m_impl->samples` already pre-sized and read directly into it.
    buf.emplace(za::PassKey<SoundBuffer>{}, sampleCount, info->channelMap, info->sampleRate);

    if (reader->read(buf->m_impl->samples.data(), sampleCount) != info->sampleCount)
        buf.reset();

    return buf;
}


////////////////////////////////////////////////////////////
za::Optional<SoundBuffer> SoundBuffer::loadFromSamples(const za::I16*     samples,
                                                       const za::SizeT    sampleCount,
                                                       const ChannelMap&  channelMap,
                                                       const unsigned int sampleRate)
{
    za::Optional<SoundBuffer> buf;

    if (channelMap.isEmpty() || sampleRate == 0u)
    {
        priv::errMsg("Failed to load sound buffer from samples (count: {}, channels: {}, sample rate: {})",
                     sampleCount,
                     channelMap.getSize(),
                     sampleRate);

        return buf;
    }

    buf.emplace(za::PassKey<SoundBuffer>{}, sampleCount, channelMap, sampleRate);
    ZA_MEMCPY(buf->m_impl->samples.data(), samples, sampleCount * sizeof(za::I16));
    return buf;
}


////////////////////////////////////////////////////////////
bool SoundBuffer::saveToFile(const Path& filename) const
{
    // Create the sound file in write mode
    if (za::Optional file = OutputSoundFile::openFromFile(filename, getSampleRate(), getChannelCount(), getChannelMap()))
    {
        // Write the samples to the opened file
        file->write(m_impl->samples.data(), m_impl->samples.size());
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
const za::I16* SoundBuffer::getSamples() const
{
    return m_impl->samples.empty() ? nullptr : m_impl->samples.data();
}


////////////////////////////////////////////////////////////
za::U64 SoundBuffer::getSampleCount() const
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
SoundBuffer::SoundBuffer(za::PassKey<SoundBuffer>&&, za::SizeT sampleCount, const ChannelMap& channelMap, unsigned int sampleRate) :
    m_impl(sampleCount, channelMap, sampleRate)
{
}


} // namespace za
