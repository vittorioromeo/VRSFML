// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#define MINIMP3_IMPLEMENTATION // Minimp3 control define, places implementation in this file.
#ifndef NOMINMAX
    #define NOMINMAX // To avoid windows.h and min/max issue
#endif
#define MINIMP3_NO_STDIO // Minimp3 control define, eliminate file manipulation code which is useless here

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4242 4244 4267 4456 4706)
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#include <minimp3_ex.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#else
    #pragma GCC diagnostic pop
#endif

#undef NOMINMAX
#undef MINIMP3_NO_STDIO

#include "Zancle/Audio/SoundFileReaderMp3.hpp"

#include "Zancle/Err/Err.hpp"
#include "Zancle/IO/InputStream.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/Memcmp.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Math/MinMax.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] za::SizeT readCallback(void* ptr, za::SizeT size, void* data)
{
    auto* stream = static_cast<za::InputStream*>(data);
    return stream->read(ptr, size).valueOr(static_cast<za::SizeT>(-1));
}


////////////////////////////////////////////////////////////
[[nodiscard]] int seekCallback(uint64_t offset, void* data) // cannot use base here due to mismatch on unix
{
    auto*              stream   = static_cast<za::InputStream*>(data);
    const za::Optional position = stream->seek(static_cast<za::SizeT>(offset));
    return position.hasValue() ? 0 : -1;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool hasValidId3Tag(const za::U8* header)
{
    return ZA_MEMCMP(header, "ID3", 3) == 0 &&
           !((header[5] & 15) || (header[6] & 0x80) || (header[7] & 0x80) || (header[8] & 0x80) || (header[9] & 0x80));
}

} // namespace


namespace za::priv
{
////////////////////////////////////////////////////////////
struct SoundFileReaderMp3::Impl
{
    mp3dec_io_t io{};
    mp3dec_ex_t decoder{};
    za::U64     numSamples{}; // Decompressed audio storage size
    za::U64     position{};   // Position in decompressed audio buffer
};


////////////////////////////////////////////////////////////
bool SoundFileReaderMp3::check(InputStream& stream)
{
    za::U8 header[10];

    if (za::Optional readResult = stream.read(header, sizeof(header));
        !readResult.hasValue() || *readResult != sizeof(header))
        return false;

    return hasValidId3Tag(header) || hdr_valid(header);
}


////////////////////////////////////////////////////////////
SoundFileReaderMp3::SoundFileReaderMp3()
{
    m_impl->io.read = readCallback;
    m_impl->io.seek = seekCallback;
}


////////////////////////////////////////////////////////////
SoundFileReaderMp3::~SoundFileReaderMp3()
{
    mp3dec_ex_close(&m_impl->decoder);
}


////////////////////////////////////////////////////////////
za::Optional<SoundFileReader::Info> SoundFileReaderMp3::open(InputStream& stream)
{
    // Init IO callbacks
    m_impl->io.read_data = &stream;
    m_impl->io.seek_data = &stream;

    za::Optional<Info> result; // Use a single local variable for NRVO

    // Init mp3 decoder
    mp3dec_ex_open_cb(&m_impl->decoder, &m_impl->io, MP3D_SEEK_TO_SAMPLE);
    if (!m_impl->decoder.samples)
        return result; // Empty optional

    // Retrieve the music attributes
    Info& info       = result.emplace();
    info.sampleRate  = static_cast<unsigned int>(m_impl->decoder.info.hz);
    info.sampleCount = m_impl->decoder.samples;

    // MP3 only supports mono/stereo channels
    switch (static_cast<unsigned int>(m_impl->decoder.info.channels))
    {
        case 0:
            priv::errMsg("No channels in MP3 file");
            break;
        case 1:
            info.channelMap = {SoundChannel::Mono};
            break;
        case 2:
            info.channelMap = {SoundChannel::SideLeft, SoundChannel::SideRight};
            break;
        default:
            priv::errMsg("MP3 files with more than 2 channels not supported");
            ZA_ASSERT(false);
            break;
    }

    m_impl->numSamples = info.sampleCount;
    return result;
}


////////////////////////////////////////////////////////////
void SoundFileReaderMp3::seek(za::U64 sampleOffset)
{
    m_impl->position = za::min(sampleOffset, m_impl->numSamples);
    mp3dec_ex_seek(&m_impl->decoder, m_impl->position);
}


////////////////////////////////////////////////////////////
za::U64 SoundFileReaderMp3::read(za::I16* samples, za::U64 maxCount)
{
    za::U64 toRead = za::min(maxCount, m_impl->numSamples - m_impl->position);
    toRead         = za::U64{mp3dec_ex_read(&m_impl->decoder, samples, static_cast<za::SizeT>(toRead))};
    m_impl->position += toRead;
    return toRead;
}

} // namespace za::priv
