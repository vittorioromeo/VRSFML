#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/SoundFileReaderOgg.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/InputStream.hpp>

#include <SFML/Base/Assert.hpp>

#include <vorbis/vorbisfile.h>

#include <cstdio>


namespace
{
std::size_t read(void* ptr, std::size_t size, std::size_t nmemb, void* data)
{
    auto* stream = static_cast<sf::InputStream*>(data);
    return stream->read(ptr, size * nmemb).valueOr(static_cast<std::size_t>(-1));
}

int seek(void* data, ogg_int64_t signedOffset, int whence)
{
    auto* stream = static_cast<sf::InputStream*>(data);
    auto  offset = static_cast<std::size_t>(signedOffset);
    switch (whence)
    {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset += stream->tell().value();
            break;
        case SEEK_END:
            offset = stream->getSize().value() - offset;
    }
    const sf::base::Optional position = stream->seek(offset);
    return position ? static_cast<int>(*position) : -1;
}

long tell(void* data)
{
    auto*                    stream   = static_cast<sf::InputStream*>(data);
    const sf::base::Optional position = stream->tell();
    return position ? static_cast<long>(*position) : -1;
}

ov_callbacks callbacks = {&read, &seek, nullptr, &tell};
} // namespace

namespace sf::priv
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
    close();
}


////////////////////////////////////////////////////////////
base::Optional<SoundFileReader::Info> SoundFileReaderOgg::open(InputStream& stream)
{
    base::Optional<Info> result; // Use a single local variable for NRVO

    // Open the Vorbis stream
    const int status = ov_open_callbacks(&stream, &m_impl->vorbis, nullptr, 0, callbacks);
    if (status < 0)
    {
        priv::err() << "Failed to open Vorbis file for reading";
        return result; // Empty optional
    }

    // Retrieve the music attributes
    vorbis_info* vorbisInfo = ov_info(&m_impl->vorbis, -1);

    Info& info        = result.emplace();
    info.channelCount = static_cast<unsigned int>(vorbisInfo->channels);
    info.sampleRate   = static_cast<unsigned int>(vorbisInfo->rate);
    info.sampleCount  = static_cast<std::size_t>(ov_pcm_total(&m_impl->vorbis, -1) * vorbisInfo->channels);

    // For Vorbis channel mapping refer to: https://xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-810004.3.9
    switch (info.channelCount)
    {
        case 0:
            priv::err() << "No channels in Vorbis file";
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
            priv::err() << "Vorbis files with more than 8 channels not supported";
            SFML_BASE_ASSERT(false);
            break;
    }

    // We must keep the channel count for the seek function
    m_impl->channelCount = info.channelCount;

    return result;
}


////////////////////////////////////////////////////////////
void SoundFileReaderOgg::seek(std::uint64_t sampleOffset)
{
    SFML_BASE_ASSERT(m_impl->vorbis.datasource != nullptr &&
                     "Vorbis datasource is missing. Call SoundFileReaderOgg::open() to initialize it.");

    ov_pcm_seek(&m_impl->vorbis, static_cast<ogg_int64_t>(sampleOffset / m_impl->channelCount));
}


////////////////////////////////////////////////////////////
std::uint64_t SoundFileReaderOgg::read(std::int16_t* samples, std::uint64_t maxCount)
{
    SFML_BASE_ASSERT(m_impl->vorbis.datasource != nullptr &&
                     "Vorbis datasource is missing. Call SoundFileReaderOgg::open() to initialize it.");

    // Try to read the requested number of samples, stop only on error or end of file
    std::uint64_t count = 0;
    while (count < maxCount)
    {
        const int bytesToRead = static_cast<int>(maxCount - count) * static_cast<int>(sizeof(std::int16_t));
        const long bytesRead = ov_read(&m_impl->vorbis, reinterpret_cast<char*>(samples), bytesToRead, 0, 2, 1, nullptr);
        if (bytesRead > 0)
        {
            const long samplesRead = bytesRead / static_cast<long>(sizeof(std::int16_t));
            count += static_cast<std::uint64_t>(samplesRead);
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


////////////////////////////////////////////////////////////
void SoundFileReaderOgg::close()
{
    if (m_impl->vorbis.datasource)
    {
        ov_clear(&m_impl->vorbis);
        m_impl->vorbis.datasource = nullptr;
        m_impl->channelCount      = 0;
    }
}

} // namespace sf::priv
