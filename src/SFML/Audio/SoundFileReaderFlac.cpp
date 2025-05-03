#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundFileReaderFlac.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/InputStream.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <FLAC/stream_decoder.h>


namespace
{
////////////////////////////////////////////////////////////
/// \brief Hold the state that is passed to the decoder callbacks
///
////////////////////////////////////////////////////////////
struct FlacClientData
{
    sf::InputStream*                stream{};
    sf::SoundFileReader::Info       info;
    sf::base::I16*                  buffer{};
    sf::base::U64                   remaining{};
    sf::base::Vector<sf::base::I16> leftovers;
    bool                            error{};
};


////////////////////////////////////////////////////////////
FLAC__StreamDecoderReadStatus streamRead(const FLAC__StreamDecoder*, FLAC__byte buffer[], sf::base::SizeT* bytes, void* clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    if (const sf::base::Optional count = data->stream->read(buffer, *bytes))
    {
        if (*count > 0)
        {
            *bytes = *count;
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }

        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}


////////////////////////////////////////////////////////////
FLAC__StreamDecoderSeekStatus streamSeek(const FLAC__StreamDecoder*, FLAC__uint64 absoluteByteOffset, void* clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    if (data->stream->seek(static_cast<sf::base::SizeT>(absoluteByteOffset)).hasValue())
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;

    return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}


////////////////////////////////////////////////////////////
FLAC__StreamDecoderTellStatus streamTell(const FLAC__StreamDecoder*, FLAC__uint64* absoluteByteOffset, void* clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    if (const sf::base::Optional position = data->stream->tell())
    {
        *absoluteByteOffset = *position;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }

    return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
}


////////////////////////////////////////////////////////////
FLAC__StreamDecoderLengthStatus streamLength(const FLAC__StreamDecoder*, FLAC__uint64* streamLength, void* clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    if (const sf::base::Optional count = data->stream->getSize())
    {
        *streamLength = *count;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }

    return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
}


////////////////////////////////////////////////////////////
FLAC__bool streamEof(const FLAC__StreamDecoder*, void* clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    return data->stream->tell() == data->stream->getSize();
}


////////////////////////////////////////////////////////////
FLAC__StreamDecoderWriteStatus streamWrite(const FLAC__StreamDecoder*,
                                           const FLAC__Frame*       frame,
                                           const FLAC__int32* const buffer[],
                                           void*                    clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    // Reserve memory if we're going to use the leftovers buffer
    const unsigned int frameSamples = frame->header.blocksize * frame->header.channels;
    if (data->remaining < frameSamples)
        data->leftovers.reserve(static_cast<sf::base::SizeT>(frameSamples - data->remaining));

    // Decode the samples
    for (unsigned i = 0; i < frame->header.blocksize; ++i)
    {
        for (unsigned int j = 0; j < frame->header.channels; ++j)
        {
            // Decode the current sample
            sf::base::I16 sample = 0;
            switch (frame->header.bits_per_sample)
            {
                case 8:
                    sample = static_cast<sf::base::I16>(buffer[j][i] << 8);
                    break;
                case 16:
                    sample = static_cast<sf::base::I16>(buffer[j][i]);
                    break;
                case 24:
                    sample = static_cast<sf::base::I16>(buffer[j][i] >> 8);
                    break;
                case 32:
                    sample = static_cast<sf::base::I16>(buffer[j][i] >> 16);
                    break;
                default:
                    SFML_BASE_ASSERT(false && "Invalid bits per sample. Must be 8, 16, 24, or 32.");
                    break;
            }

            if (data->buffer && data->remaining > 0)
            {
                // If there's room in the output buffer, copy the sample there
                *data->buffer++ = sample;
                --data->remaining;
            }
            else
            {
                // We are either seeking (null buffer) or have decoded all the requested samples during a
                // normal read (0 remaining), so we put the sample in a temporary buffer until next call
                data->leftovers.pushBack(sample);
            }
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


////////////////////////////////////////////////////////////
void streamMetadata(const FLAC__StreamDecoder*, const FLAC__StreamMetadata* meta, void* clientData)
{
    auto* data = static_cast<FlacClientData*>(clientData);

    if (meta->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        data->info.sampleCount  = meta->data.stream_info.total_samples * meta->data.stream_info.channels;
        data->info.sampleRate   = meta->data.stream_info.sample_rate;
        data->info.channelCount = meta->data.stream_info.channels;

        // For FLAC channel mapping refer to: https://xiph.org/flac/format.html#frame_header
        switch (data->info.channelCount)
        {
            case 0:
                sf::priv::err() << "No channels in FLAC file";
                break;
            case 1:
                data->info.channelMap = {sf::SoundChannel::Mono};
                break;
            case 2:
                data->info.channelMap = {sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight};
                break;
            case 3:
                data->info.channelMap = {sf::SoundChannel::FrontLeft,
                                         sf::SoundChannel::FrontRight,
                                         sf::SoundChannel::FrontCenter};
                break;
            case 4:
                data->info.channelMap = {sf::SoundChannel::FrontLeft,
                                         sf::SoundChannel::FrontRight,
                                         sf::SoundChannel::BackLeft,
                                         sf::SoundChannel::BackRight};
                break;
            case 5:
                data->info.channelMap = {sf::SoundChannel::FrontLeft,
                                         sf::SoundChannel::FrontRight,
                                         sf::SoundChannel::FrontCenter,
                                         sf::SoundChannel::BackLeft,
                                         sf::SoundChannel::BackRight};
                break;
            case 6:
                data->info.channelMap = {sf::SoundChannel::FrontLeft,
                                         sf::SoundChannel::FrontRight,
                                         sf::SoundChannel::FrontCenter,
                                         sf::SoundChannel::LowFrequencyEffects,
                                         sf::SoundChannel::BackLeft,
                                         sf::SoundChannel::BackRight};
                break;
            case 7:
                data->info.channelMap = {sf::SoundChannel::FrontLeft,
                                         sf::SoundChannel::FrontRight,
                                         sf::SoundChannel::FrontCenter,
                                         sf::SoundChannel::LowFrequencyEffects,
                                         sf::SoundChannel::BackCenter,
                                         sf::SoundChannel::SideLeft,
                                         sf::SoundChannel::SideRight};
                break;
            case 8:
                data->info.channelMap = {sf::SoundChannel::FrontLeft,
                                         sf::SoundChannel::FrontRight,
                                         sf::SoundChannel::FrontCenter,
                                         sf::SoundChannel::LowFrequencyEffects,
                                         sf::SoundChannel::BackLeft,
                                         sf::SoundChannel::BackRight,
                                         sf::SoundChannel::SideLeft,
                                         sf::SoundChannel::SideRight};
                break;
            default:
                sf::priv::err() << "FLAC files with more than 8 channels not supported";
                SFML_BASE_ASSERT(false);
                break;
        }
    }
}


////////////////////////////////////////////////////////////
void streamError(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void* clientData)
{
    auto* data  = static_cast<FlacClientData*>(clientData);
    data->error = true;
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SoundFileReaderFlac::Impl
{
    struct FlacStreamDecoderDeleter
    {
        void operator()(FLAC__StreamDecoder* theDecoder) const
        {
            if (theDecoder == nullptr)
                return;

            FLAC__stream_decoder_finish(theDecoder);
            FLAC__stream_decoder_delete(theDecoder);
        }
    };

    base::UniquePtr<FLAC__StreamDecoder, FlacStreamDecoderDeleter> decoder; //!< FLAC decoder
    FlacClientData clientData; //!< Structure passed to the decoder callbacks
};


////////////////////////////////////////////////////////////
SoundFileReaderFlac::SoundFileReaderFlac() = default;


////////////////////////////////////////////////////////////
SoundFileReaderFlac::~SoundFileReaderFlac() = default;


////////////////////////////////////////////////////////////
bool SoundFileReaderFlac::check(InputStream& stream)
{
    // Create a decoder
    FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
    if (!decoder)
        return false;

    // Initialize the decoder with our callbacks
    FlacClientData data;
    data.stream = &stream;
    data.error  = false;
    FLAC__stream_decoder_init_stream(decoder,
                                     &streamRead,
                                     &streamSeek,
                                     &streamTell,
                                     &streamLength,
                                     &streamEof,
                                     &streamWrite,
                                     nullptr,
                                     &streamError,
                                     &data);

    // Read the header
    const bool valid = FLAC__stream_decoder_process_until_end_of_metadata(decoder) != 0;

    // Destroy the decoder
    FLAC__stream_decoder_finish(decoder);
    FLAC__stream_decoder_delete(decoder);

    return valid && !data.error;
}


////////////////////////////////////////////////////////////
base::Optional<SoundFileReader::Info> SoundFileReaderFlac::open(InputStream& stream)
{
    // Create the decoder
    m_impl->decoder.reset(FLAC__stream_decoder_new());
    if (!m_impl->decoder)
    {
        priv::err() << "Failed to open FLAC file (failed to allocate the decoder)";
        return base::nullOpt;
    }

    // Initialize the decoder with our callbacks
    m_impl->clientData.stream = &stream;
    FLAC__stream_decoder_init_stream(m_impl->decoder.get(),
                                     &streamRead,
                                     &streamSeek,
                                     &streamTell,
                                     &streamLength,
                                     &streamEof,
                                     &streamWrite,
                                     &streamMetadata,
                                     &streamError,
                                     &m_impl->clientData);

    // Read the header
    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_impl->decoder.get()))
    {
        m_impl->decoder.reset();
        priv::err() << "Failed to open FLAC file (failed to read metadata)";
        return base::nullOpt;
    }

    // Retrieve the sound properties
    return base::makeOptional(m_impl->clientData.info); // was filled in the "metadata" callback
}


////////////////////////////////////////////////////////////
void SoundFileReaderFlac::seek(base::U64 sampleOffset)
{
    SFML_BASE_ASSERT(m_impl->decoder != nullptr &&
                     "No decoder available. Call SoundFileReaderFlac::open() to create a new one.");

    // Reset the callback data (the "write" callback will be called)
    m_impl->clientData.buffer    = nullptr;
    m_impl->clientData.remaining = 0;
    m_impl->clientData.leftovers.clear();

    // FLAC decoder expects absolute sample offset, so we take the channel count out
    if (sampleOffset < m_impl->clientData.info.sampleCount)
    {
        // The "write" callback will populate the leftovers buffer with the first batch of samples from the
        // seek destination, and since we want that data in this typical case, we don't re-clear it afterward
        FLAC__stream_decoder_seek_absolute(m_impl->decoder.get(), sampleOffset / m_impl->clientData.info.channelCount);
    }
    else
    {
        // FLAC decoder can't skip straight to EOF, so we short-seek by one sample and skip the rest
        FLAC__stream_decoder_seek_absolute(m_impl->decoder.get(),
                                           (m_impl->clientData.info.sampleCount / m_impl->clientData.info.channelCount) - 1);
        FLAC__stream_decoder_skip_single_frame(m_impl->decoder.get());

        // This was re-populated during the seek, but we're skipping everything in this, so we need it emptied
        m_impl->clientData.leftovers.clear();
    }
}


////////////////////////////////////////////////////////////
base::U64 SoundFileReaderFlac::read(base::I16* samples, base::U64 maxCount)
{
    SFML_BASE_ASSERT(m_impl->decoder != nullptr &&
                     "No decoder available. Call SoundFileReaderFlac::open() to create a new one.");

    // If there are leftovers from previous call, use it first
    const base::SizeT left = m_impl->clientData.leftovers.size();
    if (left > 0)
    {
        if (left > maxCount)
        {
            // There are more leftovers than needed
            for (base::SizeT i = 0; i < maxCount; ++i)
                samples[i] = m_impl->clientData.leftovers[i];

            m_impl->clientData.leftovers = base::Vector<base::I16>(m_impl->clientData.leftovers.begin(),
                                                                   m_impl->clientData.leftovers.begin() + maxCount);
            return maxCount;
        }

        // We can use all the leftovers and decode new frames
        base::copy(m_impl->clientData.leftovers.begin(), m_impl->clientData.leftovers.end(), samples);
    }

    // Reset the data that will be used in the callback
    m_impl->clientData.buffer    = samples + left;
    m_impl->clientData.remaining = maxCount - left;
    m_impl->clientData.leftovers.clear();

    // Decode frames one by one until we reach the requested sample count, the end of file or an error
    while (m_impl->clientData.remaining > 0)
    {
        // Everything happens in the "write" callback
        // This will break on any fatal error (does not include EOF)
        if (!FLAC__stream_decoder_process_single(m_impl->decoder.get()))
            break;

        // Break on EOF
        if (FLAC__stream_decoder_get_state(m_impl->decoder.get()) == FLAC__STREAM_DECODER_END_OF_STREAM)
            break;
    }

    return maxCount - m_impl->clientData.remaining;
}

} // namespace sf::priv
