#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundFileWriterWav.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/StringUtils.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/SizeT.hpp"

#include <algorithm> // std::sort, std::adjacent_find
#include <fstream>
#include <vector>


namespace
{
// The following functions takes integers in host byte order
// and writes them to a stream as little endian

void encode(std::ostream& stream, sf::base::I16 value)
{
    const char bytes[]{static_cast<char>(value & 0xFF), static_cast<char>(value >> 8)};
    stream.write(bytes, static_cast<std::streamsize>(sf::base::getArraySize(bytes)));
}

void encode(std::ostream& stream, sf::base::U16 value)
{
    const char bytes[]{static_cast<char>(value & 0xFF), static_cast<char>(value >> 8)};
    stream.write(bytes, static_cast<std::streamsize>(sf::base::getArraySize(bytes)));
}

void encode(std::ostream& stream, sf::base::U32 value)
{
    const char bytes[]{
        static_cast<char>(value & 0x000000FF),
        static_cast<char>((value & 0x0000FF00) >> 8),
        static_cast<char>((value & 0x00FF0000) >> 16),
        static_cast<char>((value & 0xFF000000) >> 24),
    };
    stream.write(bytes, static_cast<std::streamsize>(sf::base::getArraySize(bytes)));
}
} // namespace

namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SoundFileWriterWav::Impl
{
    std::ofstream file;             //!< File stream to write to
    unsigned int  channelCount{};   //!< Channel count of the sound being written
    base::SizeT   remapTable[18]{}; //!< Table we use to remap source to target channel order
};


////////////////////////////////////////////////////////////
bool SoundFileWriterWav::check(const Path& filename)
{
    return priv::toLower(filename.extension().to<std::string>()) == ".wav";
}


////////////////////////////////////////////////////////////
SoundFileWriterWav::SoundFileWriterWav() = default;


////////////////////////////////////////////////////////////
SoundFileWriterWav::~SoundFileWriterWav()
{
    if (!m_impl->file.is_open())
        return;

    // If the file is open, finalize the header and close it
    m_impl->file.flush();

    // Update the main chunk size and data sub-chunk size
    const base::U32 fileSize = static_cast<base::U32>(m_impl->file.tellp());
    m_impl->file.seekp(4);
    encode(m_impl->file, fileSize - 8); // 8 bytes RIFF header
    m_impl->file.seekp(40);
    encode(m_impl->file, fileSize - 44); // 44 bytes RIFF + WAVE headers

    m_impl->file.close();
}


////////////////////////////////////////////////////////////
bool SoundFileWriterWav::open(const Path& filename, unsigned int sampleRate, unsigned int channelCount, const ChannelMap& channelMap)
{
    auto channelMask = 0u;

    if (channelCount == 0)
    {
        priv::err() << "WAV sound file channel count 0";
        return false;
    }

    if (channelCount == 1)
    {
        m_impl->remapTable[0] = 0;
    }
    else if (channelCount == 2)
    {
        m_impl->remapTable[0] = 0;
        m_impl->remapTable[1] = 1;
    }
    else
    {
        // For WAVE channel mapping refer to: https://learn.microsoft.com/en-us/previous-versions/windows/hardware/design/dn653308(v=vs.85)#default-channel-ordering
        enum : unsigned int
        {
            speakerFrontLeft          = 0x1u,
            speakerFrontRight         = 0x2u,
            speakerFrontCenter        = 0x4u,
            speakerLowFrequency       = 0x8u,
            speakerBackLeft           = 0x10u,
            speakerBackRight          = 0x20u,
            speakerFrontLeftOfCenter  = 0x40u,
            speakerFrontRightOfCenter = 0x80u,
            speakerBackCenter         = 0x100u,
            speakerSideLeft           = 0x200u,
            speakerSideRight          = 0x400u,
            speakerTopCenter          = 0x800u,
            speakerTopFrontLeft       = 0x1000u,
            speakerTopFrontCenter     = 0x2000u,
            speakerTopFrontRight      = 0x4000u,
            speakerTopBackLeft        = 0x8000u,
            speakerTopBackCenter      = 0x10000u,
            speakerTopBackRight       = 0x20000u
        };

        struct SupportedChannel
        {
            base::U32    bit;
            SoundChannel channel;
        };

        std::vector<SupportedChannel>
            targetChannelMap{{speakerFrontLeft, SoundChannel::FrontLeft},
                             {speakerFrontRight, SoundChannel::FrontRight},
                             {speakerFrontCenter, SoundChannel::FrontCenter},
                             {speakerLowFrequency, SoundChannel::LowFrequencyEffects},
                             {speakerBackLeft, SoundChannel::BackLeft},
                             {speakerBackRight, SoundChannel::BackRight},
                             {speakerFrontLeftOfCenter, SoundChannel::FrontLeftOfCenter},
                             {speakerFrontRightOfCenter, SoundChannel::FrontRightOfCenter},
                             {speakerBackCenter, SoundChannel::BackCenter},
                             {speakerSideLeft, SoundChannel::SideLeft},
                             {speakerSideRight, SoundChannel::SideRight},
                             {speakerTopCenter, SoundChannel::TopCenter},
                             {speakerTopFrontLeft, SoundChannel::TopFrontLeft},
                             {speakerTopFrontCenter, SoundChannel::TopFrontCenter},
                             {speakerTopFrontRight, SoundChannel::TopFrontRight},
                             {speakerTopBackLeft, SoundChannel::TopBackLeft},
                             {speakerTopBackCenter, SoundChannel::TopBackCenter},
                             {speakerTopBackRight, SoundChannel::TopBackRight}};

        // Check for duplicate channel entries
        {
            auto sortedChannelMap = channelMap;
            std::sort(sortedChannelMap.begin(), sortedChannelMap.end());

            if (std::adjacent_find(sortedChannelMap.begin(), sortedChannelMap.end()) != sortedChannelMap.end())
            {
                priv::err() << "Duplicate channels in channel map";
                return false;
            }
        }

        // Construct the target channel map by removing unused channels
        for (auto iter = targetChannelMap.begin(); iter != targetChannelMap.end();)
        {
            if (base::find(channelMap.begin(), channelMap.end(), iter->channel) == channelMap.end())
            {
                iter = targetChannelMap.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        // Verify that all the input channels exist in the target channel map
        for (const SoundChannel channel : channelMap)
        {
            if (base::findIf(targetChannelMap.begin(),
                             targetChannelMap.end(),
                             [channel](const SupportedChannel& c) { return c.channel == channel; }) ==
                targetChannelMap.end())
            {
                priv::err() << "Could not map all input channels to a channel supported by WAV";
                return false;
            }
        }

        // Build the remap table
        for (auto i = 0u; i < channelCount; ++i)
            m_impl->remapTable[i] = static_cast<base::SizeT>(
                base::find(channelMap.begin(), channelMap.end(), targetChannelMap[i].channel) - channelMap.begin());

        // Generate the channel mask
        for (const auto& channel : targetChannelMap)
            channelMask |= channel.bit;
    }

    // Save the channel count
    m_impl->channelCount = channelCount;

    // Open the file
    m_impl->file.open(filename.to<std::string>(), std::ios::binary);
    if (!m_impl->file)
    {
        priv::err() << "Failed to open WAV sound file for writing\n" << priv::PathDebugFormatter{filename};
        return false;
    }

    // Write the header
    writeHeader(sampleRate, channelCount, channelMask);

    return true;
}


////////////////////////////////////////////////////////////
void SoundFileWriterWav::write(const base::I16* samples, base::U64 count)
{
    SFML_BASE_ASSERT(m_impl->file.good() && "Most recent I/O operation failed");
    SFML_BASE_ASSERT(count % m_impl->channelCount == 0);

    if (count % m_impl->channelCount != 0)
        priv::err() << "Writing samples to WAV sound file requires writing full frames at a time";

    while (count >= m_impl->channelCount)
    {
        for (auto i = 0u; i < m_impl->channelCount; ++i)
            encode(m_impl->file, samples[m_impl->remapTable[i]]);

        samples += m_impl->channelCount;
        count -= m_impl->channelCount;
    }
}


////////////////////////////////////////////////////////////
void SoundFileWriterWav::writeHeader(unsigned int sampleRate, unsigned int channelCount, unsigned int channelMask)
{
    SFML_BASE_ASSERT(m_impl->file.good() && "Most recent I/O operation failed");

    // Write the main chunk ID
    char mainChunkId[]{'R', 'I', 'F', 'F'};
    m_impl->file.write(mainChunkId, static_cast<std::streamsize>(base::getArraySize(mainChunkId)));

    // Write the main chunk header
    encode(m_impl->file, base::U32{0}); // 0 is a placeholder, will be written later
    char mainChunkFormat[]{'W', 'A', 'V', 'E'};
    m_impl->file.write(mainChunkFormat, static_cast<std::streamsize>(base::getArraySize(mainChunkFormat)));

    // Write the sub-chunk 1 ("format") id and size
    char fmtChunkId[]{'f', 'm', 't', ' '};
    m_impl->file.write(fmtChunkId, static_cast<std::streamsize>(base::getArraySize(fmtChunkId)));

    if (channelCount > 2)
    {
        const base::U32 fmtChunkSize = 40;
        encode(m_impl->file, fmtChunkSize);

        // Write the format (Extensible)
        const base::U16 format = 65534;
        encode(m_impl->file, format);
    }
    else
    {
        const base::U32 fmtChunkSize = 16;
        encode(m_impl->file, fmtChunkSize);

        // Write the format (PCM)
        const base::U16 format = 1;
        encode(m_impl->file, format);
    }

    // Write the sound attributes
    encode(m_impl->file, static_cast<base::U16>(channelCount));
    encode(m_impl->file, sampleRate);
    const base::U32 byteRate = sampleRate * channelCount * 2;
    encode(m_impl->file, byteRate);
    const auto blockAlign = static_cast<base::U16>(channelCount * 2);
    encode(m_impl->file, blockAlign);
    const base::U16 bitsPerSample = 16;
    encode(m_impl->file, bitsPerSample);

    if (channelCount > 2)
    {
        const base::U16 extensionSize = 16;
        encode(m_impl->file, extensionSize);
        encode(m_impl->file, bitsPerSample);
        encode(m_impl->file, channelMask);
        // Write the subformat (PCM)
        char subformat[] =
            {'\x01', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x00', '\x80', '\x00', '\x00', '\xAA', '\x00', '\x38', '\x9B', '\x71'};
        m_impl->file.write(subformat, static_cast<std::streamsize>(base::getArraySize(subformat)));
    }

    // Write the sub-chunk 2 ("data") id and size
    char dataChunkId[]{'d', 'a', 't', 'a'};
    m_impl->file.write(dataChunkId, static_cast<std::streamsize>(base::getArraySize(dataChunkId)));
    const base::U32 dataChunkSize = 0; // placeholder, will be written later
    encode(m_impl->file, dataChunkSize);
}


} // namespace sf::priv
