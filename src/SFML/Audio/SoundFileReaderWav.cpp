#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/SoundFileReaderWav.hpp"

#include "SFML/System/InputStream.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"

#include <miniaudio.h>

#include <cstddef>


namespace
{
ma_result onRead(ma_decoder* decoder, void* buffer, size_t bytesToRead, size_t* bytesRead)
{
    auto*                    stream = static_cast<sf::InputStream*>(decoder->pUserData);
    const sf::base::Optional count  = stream->read(buffer, bytesToRead);

    if (!count.hasValue())
        return MA_ERROR;

    *bytesRead = static_cast<std::size_t>(*count);
    return MA_SUCCESS;
}

ma_result onSeek(ma_decoder* decoder, ma_int64 byteOffset, ma_seek_origin origin)
{
    auto* stream = static_cast<sf::InputStream*>(decoder->pUserData);

    switch (origin)
    {
        case ma_seek_origin_start:
        {
            if (!stream->seek(static_cast<std::size_t>(byteOffset)).hasValue())
                return MA_ERROR;

            return MA_SUCCESS;
        }
        case ma_seek_origin_current:
        {
            if (!stream->tell().hasValue())
                return MA_ERROR;

            if (!stream->seek(stream->tell().value() + static_cast<std::size_t>(byteOffset)).hasValue())
                return MA_ERROR;

            return MA_SUCCESS;
        }
        // According to miniaudio comments, ma_seek_origin_end is not used by decoders
        default:
            return MA_ERROR;
    }
}
} // namespace

namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SoundFileReaderWav::Impl
{
    base::Optional<ma_decoder> decoder;        //!< wav decoder
    ma_uint32                  channelCount{}; //!< Number of channels
};


////////////////////////////////////////////////////////////
bool SoundFileReaderWav::check(InputStream& stream)
{
    auto config           = ma_decoder_config_init_default();
    config.encodingFormat = ma_encoding_format_wav;
    config.format         = ma_format_s16;
    ma_decoder decoder{};

    if (ma_decoder_init(&onRead, &onSeek, &stream, &config, &decoder) == MA_SUCCESS)
    {
        ma_decoder_uninit(&decoder);
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
SoundFileReaderWav::SoundFileReaderWav() = default;


////////////////////////////////////////////////////////////
SoundFileReaderWav::~SoundFileReaderWav()
{
    if (m_impl->decoder.hasValue())
    {
        if (const ma_result result = ma_decoder_uninit(m_impl->decoder.asPtr()); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("uninitialize wav decoder", result);
    }
}


////////////////////////////////////////////////////////////
base::Optional<SoundFileReader::Info> SoundFileReaderWav::open(InputStream& stream)
{
    if (m_impl->decoder.hasValue())
    {
        if (const ma_result result = ma_decoder_uninit(m_impl->decoder.asPtr()); result != MA_SUCCESS)
        {
            priv::MiniaudioUtils::fail("uninitialize wav decoder", result);
            return base::nullOpt;
        }
    }
    else
    {
        m_impl->decoder.emplace();
    }

    auto config           = ma_decoder_config_init_default();
    config.encodingFormat = ma_encoding_format_wav;
    config.format         = ma_format_s16;

    if (const ma_result result = ma_decoder_init(&onRead, &onSeek, &stream, &config, m_impl->decoder.asPtr());
        result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("initialize wav decoder", result);
        m_impl->decoder = base::nullOpt;
        return base::nullOpt;
    }

    ma_uint64 frameCount{};
    if (const ma_result result = ma_decoder_get_available_frames(m_impl->decoder.asPtr(), &frameCount); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("get available frames from wav decoder", result);
        return base::nullOpt;
    }

    auto       format = ma_format_unknown;
    ma_uint32  sampleRate{};
    ma_channel channelMap[20]{};

    if (const ma_result result = ma_decoder_get_data_format(m_impl->decoder.asPtr(),
                                                            &format,
                                                            &m_impl->channelCount,
                                                            &sampleRate,
                                                            channelMap,
                                                            base::getArraySize(channelMap));
        result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("get data format from wav decoder", result);
        return base::nullOpt;
    }

    ChannelMap soundChannels;
    soundChannels.reserve(m_impl->channelCount);

    for (auto i = 0u; i < m_impl->channelCount; ++i)
        soundChannels.append(priv::MiniaudioUtils::miniaudioChannelToSoundChannel(std::uint8_t{channelMap[i]}));

    return base::makeOptional<Info>(
        {frameCount * m_impl->channelCount, m_impl->channelCount, sampleRate, SFML_BASE_MOVE(soundChannels)});
}


////////////////////////////////////////////////////////////
void SoundFileReaderWav::seek(std::uint64_t sampleOffset)
{
    SFML_BASE_ASSERT(m_impl->decoder.hasValue() &&
                     "wav decoder not initialized. Call SoundFileReaderWav::open() to initialize it.");

    if (const ma_result result = ma_decoder_seek_to_pcm_frame(m_impl->decoder.asPtr(), sampleOffset / m_impl->channelCount);
        result != MA_SUCCESS)
        priv::MiniaudioUtils::fail("seek wav sound stream", result);
}


////////////////////////////////////////////////////////////
std::uint64_t SoundFileReaderWav::read(std::int16_t* samples, std::uint64_t maxCount)
{
    SFML_BASE_ASSERT(m_impl->decoder.hasValue() &&
                     "wav decoder not initialized. Call SoundFileReaderWav::open() to initialize it.");

    ma_uint64 framesRead{};

    if (const ma_result result = ma_decoder_read_pcm_frames(m_impl->decoder.asPtr(),
                                                            samples,
                                                            maxCount / m_impl->channelCount,
                                                            &framesRead);
        result != MA_SUCCESS)
        priv::MiniaudioUtils::fail("read from wav sound stream", result);

    return framesRead * m_impl->channelCount;
}

} // namespace sf::priv
