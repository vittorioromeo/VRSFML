#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/SoundFileWriterFlac.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/FileUtils.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/StringUtils.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/TrivialVector.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <FLAC/stream_encoder.h>
#include <algorithm> // std::is_permutation
#include <string>

#include <cstdio>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SoundFileWriterFlac::Impl
{
    struct FlacStreamEncoderDeleter
    {
        void operator()(FLAC__StreamEncoder* theEncoder) const
        {
            if (theEncoder == nullptr)
                return;

            FLAC__stream_encoder_finish(theEncoder);
            FLAC__stream_encoder_delete(theEncoder);
        }
    };

    std::FILE*                                                     file{};
    base::UniquePtr<FLAC__StreamEncoder, FlacStreamEncoderDeleter> encoder;        //!< FLAC stream encoder
    unsigned int                                                   channelCount{}; //!< Number of channels
    base::SizeT                    remapTable[8]{}; //!< Table we use to remap source to target channel order
    base::TrivialVector<base::I32> samples32;       //!< Conversion buffer
};


////////////////////////////////////////////////////////////
SoundFileWriterFlac::SoundFileWriterFlac() = default;


////////////////////////////////////////////////////////////
SoundFileWriterFlac::~SoundFileWriterFlac() = default;


////////////////////////////////////////////////////////////
bool SoundFileWriterFlac::check(const Path& filename)
{
    return priv::toLower(filename.extension().to<std::string>()) == ".flac";
}


////////////////////////////////////////////////////////////
bool SoundFileWriterFlac::open(const Path& filename, unsigned int sampleRate, unsigned int channelCount, const ChannelMap& channelMap)
{
    ChannelMap targetChannelMap;

    // For FLAC channel mapping refer to: https://xiph.org/flac/format.html#frame_header
    switch (channelCount)
    {
        case 0:
            priv::err() << "No channels to write to FLAC file";
            return false;
        case 1:
            targetChannelMap = {SoundChannel::Mono};
            break;
        case 2:
            targetChannelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight};
            break;
        case 3:
            targetChannelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight, SoundChannel::FrontCenter};
            break;
        case 4:
            targetChannelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight, SoundChannel::BackLeft, SoundChannel::BackRight};
            break;
        case 5:
            targetChannelMap = {SoundChannel::FrontLeft,
                                SoundChannel::FrontRight,
                                SoundChannel::FrontCenter,
                                SoundChannel::BackLeft,
                                SoundChannel::BackRight};
            break;
        case 6:
            targetChannelMap = {SoundChannel::FrontLeft,
                                SoundChannel::FrontRight,
                                SoundChannel::FrontCenter,
                                SoundChannel::LowFrequencyEffects,
                                SoundChannel::BackLeft,
                                SoundChannel::BackRight};
            break;
        case 7:
            targetChannelMap = {SoundChannel::FrontLeft,
                                SoundChannel::FrontRight,
                                SoundChannel::FrontCenter,
                                SoundChannel::LowFrequencyEffects,
                                SoundChannel::BackCenter,
                                SoundChannel::SideLeft,
                                SoundChannel::SideRight};
            break;
        case 8:
            targetChannelMap = {SoundChannel::FrontLeft,
                                SoundChannel::FrontRight,
                                SoundChannel::FrontCenter,
                                SoundChannel::LowFrequencyEffects,
                                SoundChannel::BackLeft,
                                SoundChannel::BackRight,
                                SoundChannel::SideLeft,
                                SoundChannel::SideRight};
            break;
        default:
            priv::err() << "FLAC files with more than 8 channels not supported";
            return false;
    }

    // Check if the channel map contains channels that we cannot remap to a mapping supported by FLAC
    if (!std::is_permutation(channelMap.begin(), channelMap.end(), targetChannelMap.begin()))
    {
        priv::err() << "Provided channel map cannot be reordered to a channel map supported by FLAC";
        return false;
    }

    // Build the remap rable
    for (auto i = 0u; i < channelCount; ++i)
        m_impl->remapTable[i] = static_cast<base::SizeT>(
            base::find(channelMap.begin(), channelMap.end(), targetChannelMap[i]) - channelMap.begin());

    // Create the encoder
    m_impl->encoder.reset(FLAC__stream_encoder_new());
    if (!m_impl->encoder)
    {
        priv::err() << "Failed to write flac file (failed to allocate encoder)\n" << priv::PathDebugFormatter{filename};
        return false;
    }

    // Open file
    m_impl->file = openFile(filename, "w+b");

    // Setup the encoder
    FLAC__stream_encoder_set_channels(m_impl->encoder.get(), channelCount);
    FLAC__stream_encoder_set_bits_per_sample(m_impl->encoder.get(), 16);
    FLAC__stream_encoder_set_sample_rate(m_impl->encoder.get(), sampleRate);

    // Initialize the output stream
    if (FLAC__stream_encoder_init_FILE(m_impl->encoder.get(), m_impl->file, nullptr, nullptr) !=
        FLAC__STREAM_ENCODER_INIT_STATUS_OK)
    {
        priv::err() << "Failed to write flac file (failed to open the file)\n" << priv::PathDebugFormatter{filename};
        m_impl->encoder.reset();
        return false;
    }

    // Store the channel count
    m_impl->channelCount = channelCount;

    return true;
}


////////////////////////////////////////////////////////////
void SoundFileWriterFlac::write(const base::I16* samples, base::U64 count)
{
    while (count > 0)
    {
        // Make sure that we don't process too many samples at once
        const unsigned int frames = base::min(static_cast<unsigned int>(count / m_impl->channelCount), 10000u);

        // Convert the samples to 32-bits and remap the channels
        m_impl->samples32.clear();
        m_impl->samples32.reserve(frames * m_impl->channelCount);

        for (auto frame = 0u; frame < frames; ++frame)
        {
            for (auto channel = 0u; channel < m_impl->channelCount; ++channel)
                m_impl->samples32.unsafeEmplaceBack(samples[frame * m_impl->channelCount + m_impl->remapTable[channel]]);
        }

        // Write them to the FLAC stream
        FLAC__stream_encoder_process_interleaved(m_impl->encoder.get(), m_impl->samples32.data(), frames);

        // Next chunk
        count -= m_impl->samples32.size();
        samples += m_impl->samples32.size();
    }
}

} // namespace sf::priv
