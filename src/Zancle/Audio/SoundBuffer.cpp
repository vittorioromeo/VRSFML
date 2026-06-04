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

#include "Zancle/System/Err.hpp"
#include "Zancle/System/IO.hpp"
#include "Zancle/System/InputStream.hpp"
#include "Zancle/System/MemoryInputStream.hpp"
#include "Zancle/System/Path.hpp"
#include "Zancle/System/Time.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Builtin/Memcpy.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/UniquePtr.hpp"
#include "ZancleBase/Vector.hpp"


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
    explicit Impl(zb::SizeT theSampleCount, const ChannelMap& theChannelMap, const unsigned int theSampleRate) :
        channelMap(theChannelMap),
        sampleRate(theSampleRate)
    {
        ZB_ASSERT(channelMap.getSize() > 0u);
        ZB_ASSERT(sampleRate > 0u);

        samples.reserve(theSampleCount);
        samples.unsafeSetSize(theSampleCount);

        duration = seconds(static_cast<float>(theSampleCount) / static_cast<float>(sampleRate) /
                           static_cast<float>(channelMap.getSize()));
    }

    ////////////////////////////////////////////////////////////
    zb::Vector<zb::I16> samples;                        //!< Samples buffer
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
zb::Optional<SoundBuffer> SoundBuffer::loadFromFile(const Path& filename)
{
    // `SoundBuffer` decodes the entire file into PCM up front, so we don't need
    // `InputSoundFile`'s streaming machinery (which exists for `MusicReader`).
    // Read the encoded bytes through the native fast path, then delegate to the
    // memory-based loader which picks a codec and decodes in-place.
    zb::Vector<char>& scratch = getThreadLocalScratchCharBuffer();

    if (!readFromFile(filename, scratch))
    {
        priv::errMsg("Failed to open sound buffer from file");
        return zb::nullOpt;
    }

    return loadFromMemory(scratch.data(), scratch.size());
}


////////////////////////////////////////////////////////////
zb::Optional<SoundBuffer> SoundBuffer::loadFromMemory(const void* data, zb::SizeT sizeInBytes)
{
    MemoryInputStream stream{data, sizeInBytes};
    return loadFromStream(stream);
}


////////////////////////////////////////////////////////////
zb::Optional<SoundBuffer> SoundBuffer::loadFromStream(InputStream& stream)
{
    // Single named return so NRVO can apply -- failure paths return the
    // already-empty `buf`, success path emplaces into it.
    zb::Optional<SoundBuffer> buf;

    auto reader = SoundFileFactory::createReaderFromStream(stream);
    if (reader == nullptr)
    {
        priv::errMsg("Failed to open sound buffer (no codec for the data's format)");
        return buf;
    }

    // `createReaderFromStream` advances the read position while sniffing the codec; rewind before handing the stream to `open`.
    if (const zb::Optional seekResult = stream.seek(0); !seekResult.hasValue() || *seekResult != 0)
    {
        priv::errMsg("Failed to open sound buffer (rewind after codec detection failed)");
        return buf;
    }

    const zb::Optional info = reader->open(stream);
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
    if (info->sampleCount > static_cast<zb::U64>(static_cast<zb::SizeT>(-1)))
    {
        priv::errMsg("Failed to load sound buffer (sample count exceeds addressable range on this platform)");
        return buf;
    }

    const auto sampleCount = static_cast<zb::SizeT>(info->sampleCount);

    // Construct the `SoundBuffer` with `m_impl->samples` already pre-sized and read directly into it.
    buf.emplace(zb::PassKey<SoundBuffer>{}, sampleCount, info->channelMap, info->sampleRate);

    if (reader->read(buf->m_impl->samples.data(), sampleCount) != info->sampleCount)
        buf.reset();

    return buf;
}


////////////////////////////////////////////////////////////
zb::Optional<SoundBuffer> SoundBuffer::loadFromSamples(const zb::I16*     samples,
                                                       const zb::SizeT    sampleCount,
                                                       const ChannelMap&  channelMap,
                                                       const unsigned int sampleRate)
{
    zb::Optional<SoundBuffer> buf;

    if (channelMap.isEmpty() || sampleRate == 0u)
    {
        priv::errMsg("Failed to load sound buffer from samples (count: {}, channels: {}, sample rate: {})",
                     sampleCount,
                     channelMap.getSize(),
                     sampleRate);

        return buf;
    }

    buf.emplace(zb::PassKey<SoundBuffer>{}, sampleCount, channelMap, sampleRate);
    ZB_MEMCPY(buf->m_impl->samples.data(), samples, sampleCount * sizeof(zb::I16));
    return buf;
}


////////////////////////////////////////////////////////////
bool SoundBuffer::saveToFile(const Path& filename) const
{
    // Create the sound file in write mode
    if (zb::Optional file = OutputSoundFile::openFromFile(filename, getSampleRate(), getChannelCount(), getChannelMap()))
    {
        // Write the samples to the opened file
        file->write(m_impl->samples.data(), m_impl->samples.size());
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
const zb::I16* SoundBuffer::getSamples() const
{
    return m_impl->samples.empty() ? nullptr : m_impl->samples.data();
}


////////////////////////////////////////////////////////////
zb::U64 SoundBuffer::getSampleCount() const
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
SoundBuffer::SoundBuffer(zb::PassKey<SoundBuffer>&&, zb::SizeT sampleCount, const ChannelMap& channelMap, unsigned int sampleRate) :
    m_impl(sampleCount, channelMap, sampleRate)
{
}


} // namespace za
