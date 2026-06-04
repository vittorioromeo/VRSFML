// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/SoundFileReaderOgg.hpp"

#include "Zancle/Audio/SoundChannel.hpp"
#include "Zancle/Audio/SoundFileReader.hpp"

#include "Zancle/System/Err.hpp"
#include "Zancle/System/InputStream.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <ogg/config_types.h>

#include <cstdio>


namespace
{
////////////////////////////////////////////////////////////
zb::SizeT read(void* ptr, zb::SizeT size, zb::SizeT nmemb, void* data)
{
    auto* stream = static_cast<za::InputStream*>(data);
    return stream->read(ptr, size * nmemb).valueOr(static_cast<zb::SizeT>(-1));
}


////////////////////////////////////////////////////////////
int seek(void* data, ogg_int64_t signedOffset, int whence)
{
    auto*           stream = static_cast<za::InputStream*>(data);
    zb::SizeT offset{};

    switch (whence)
    {
        case SEEK_SET:
            offset = static_cast<zb::SizeT>(signedOffset);
            break;
        case SEEK_CUR:
            offset = static_cast<zb::SizeT>(static_cast<ogg_int64_t>(stream->tell().value()) + signedOffset);
            break;
        case SEEK_END:
            offset = static_cast<zb::SizeT>(static_cast<ogg_int64_t>(stream->getSize().value()) + signedOffset);
            break;
    }

    const zb::Optional position = stream->seek(offset);
    return position.hasValue() ? static_cast<int>(*position) : -1;
}


////////////////////////////////////////////////////////////
long tell(void* data)
{
    auto*                    stream   = static_cast<za::InputStream*>(data);
    const zb::Optional position = stream->tell();
    return position.hasValue() ? static_cast<long>(*position) : -1;
}


////////////////////////////////////////////////////////////
ov_callbacks callbacks = {&read, &seek, nullptr, &tell};

} // namespace


namespace za::priv
{
////////////////////////////////////////////////////////////
struct SoundFileReaderOgg::Impl
{
    OggVorbis_File vorbis{};       // ogg/vorbis file handle
    unsigned int   channelCount{}; // number of channels of the open sound file
};


////////////////////////////////////////////////////////////
bool SoundFileReaderOgg::check(InputStream& stream)
{
    OggVorbis_File file;
    if (ov_test_callbacks(&stream, &file, nullptr, 0, callbacks) == 0)
    {
        ov_clear(&file);
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
SoundFileReaderOgg::SoundFileReaderOgg() = default;


////////////////////////////////////////////////////////////
SoundFileReaderOgg::~SoundFileReaderOgg()
{
    if (m_impl->vorbis.datasource)
    {
        ov_clear(&m_impl->vorbis);
        m_impl->vorbis.datasource = nullptr;
        m_impl->channelCount      = 0;
    }
}


////////////////////////////////////////////////////////////
zb::Optional<SoundFileReader::Info> SoundFileReaderOgg::open(InputStream& stream)
{
    zb::Optional<Info> result; // Use a single local variable for NRVO

    // Open the Vorbis stream
    const int status = ov_open_callbacks(&stream, &m_impl->vorbis, nullptr, 0, callbacks);
    if (status < 0)
    {
        priv::errMsg("Failed to open Vorbis file for reading");
        return result; // Empty optional
    }

    // Retrieve the music attributes
    vorbis_info* vorbisInfo = ov_info(&m_impl->vorbis, -1);

    // When attempting to retrieve the total PCM sample count using ov_pcm_total() below,
    // make sure to set the read position back to the beginning of the file after calling ov_info().
    // Depending on how big the header and embedded comment data is, not seeking back
    // to the beginning of the file can lead to a wrong PCM sample count being returned.
    // See:
    // https://web.archive.org/web/20250924022223/https://stackoverflow.com/questions/8653670/vorbis-finding-decompressed-size-of-file/72482773#72482773
    // https://github.com/xiph/vorbis/issues/60
    // https://github.com/xiph/vorbis/pull/71
    if (ov_raw_seek(&m_impl->vorbis, 0) < 0)
    {
        priv::errMsg("Failed to seek to start of Vorbis file");
        return result; // Empty optional
    }

    Info& info       = result.emplace();
    info.sampleRate  = static_cast<unsigned int>(vorbisInfo->rate);
    info.sampleCount = static_cast<zb::SizeT>(ov_pcm_total(&m_impl->vorbis, -1) * vorbisInfo->channels);

    // For Vorbis channel mapping refer to: https://xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-810004.3.9
    switch (static_cast<unsigned int>(vorbisInfo->channels))
    {
        case 0:
            priv::errMsg("No channels in Vorbis file");
            break;
        case 1:
            info.channelMap = {SoundChannel::Mono};
            break;
        case 2:
            info.channelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight};
            break;
        case 3:
            info.channelMap = {SoundChannel::FrontLeft, SoundChannel::FrontCenter, SoundChannel::FrontRight};
            break;
        case 4:
            info.channelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight, SoundChannel::BackLeft, SoundChannel::BackRight};
            break;
        case 5:
            info.channelMap = {SoundChannel::FrontLeft,
                               SoundChannel::FrontCenter,
                               SoundChannel::FrontRight,
                               SoundChannel::BackLeft,
                               SoundChannel::BackRight};
            break;
        case 6:
            info.channelMap = {SoundChannel::FrontLeft,
                               SoundChannel::FrontCenter,
                               SoundChannel::FrontRight,
                               SoundChannel::BackLeft,
                               SoundChannel::BackRight,
                               SoundChannel::LowFrequencyEffects};
            break;
        case 7:
            info.channelMap = {SoundChannel::FrontLeft,
                               SoundChannel::FrontCenter,
                               SoundChannel::FrontRight,
                               SoundChannel::SideLeft,
                               SoundChannel::SideRight,
                               SoundChannel::BackCenter,
                               SoundChannel::LowFrequencyEffects};
            break;
        case 8:
            info.channelMap = {SoundChannel::FrontLeft,
                               SoundChannel::FrontCenter,
                               SoundChannel::FrontRight,
                               SoundChannel::SideLeft,
                               SoundChannel::SideRight,
                               SoundChannel::BackLeft,
                               SoundChannel::BackRight,
                               SoundChannel::LowFrequencyEffects};
            break;
        default:
            priv::errMsg("Vorbis files with more than 8 channels not supported");
            ZB_ASSERT(false);
            break;
    }

    // We must keep the channel count for the seek function
    m_impl->channelCount = static_cast<unsigned int>(info.channelMap.getSize());

    return result;
}


////////////////////////////////////////////////////////////
void SoundFileReaderOgg::seek(zb::U64 sampleOffset)
{
    ZB_ASSERT(m_impl->vorbis.datasource != nullptr &&
                     "Vorbis datasource is missing. Call SoundFileReaderOgg::open() to initialize it.");

    ov_pcm_seek(&m_impl->vorbis, static_cast<ogg_int64_t>(sampleOffset / m_impl->channelCount));
}


////////////////////////////////////////////////////////////
zb::U64 SoundFileReaderOgg::read(zb::I16* samples, zb::U64 maxCount)
{
    ZB_ASSERT(m_impl->vorbis.datasource != nullptr &&
                     "Vorbis datasource is missing. Call SoundFileReaderOgg::open() to initialize it.");

    // Try to read the requested number of samples, stop only on error or end of file
    zb::U64 count = 0;
    while (count < maxCount)
    {
        const int bytesToRead = static_cast<int>(maxCount - count) * static_cast<int>(sizeof(zb::I16));
        const long bytesRead = ov_read(&m_impl->vorbis, reinterpret_cast<char*>(samples), bytesToRead, ZA_IS_BIG_ENDIAN, 2, 1, nullptr);
        if (bytesRead > 0)
        {
            const long samplesRead = bytesRead / static_cast<long>(sizeof(zb::I16));
            count += static_cast<zb::U64>(samplesRead);
            samples += samplesRead;
        }
        else
        {
            // error or end of file
            break;
        }
    }

    return count;
}

} // namespace za::priv
