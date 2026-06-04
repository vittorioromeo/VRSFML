// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundFileWriterWav.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"

#include "SFML/Base/Algorithm/AdjacentFind.hpp"
#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Algorithm/Sort.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
// The following functions take integers in host byte order and write them
// to a stream as little endian. Each returns `false` on write failure so
// callers can short-circuit further work on the file.

[[nodiscard]] bool encode(sf::OutFile& stream, sf::base::I16 value)
{
    const char bytes[]{static_cast<char>(value & 0xFF), static_cast<char>(value >> 8)};
    return stream.write(bytes, sf::base::getArraySize(bytes));
}

[[nodiscard]] bool encode(sf::OutFile& stream, sf::base::U16 value)
{
    const char bytes[]{static_cast<char>(value & 0xFF), static_cast<char>(value >> 8)};
    return stream.write(bytes, sf::base::getArraySize(bytes));
}

[[nodiscard]] bool encode(sf::OutFile& stream, sf::base::U32 value)
{
    const char bytes[]{
        static_cast<char>((value & 0x00'00'00'FF) >> 0),
        static_cast<char>((value & 0x00'00'FF'00) >> 8),
        static_cast<char>((value & 0x00'FF'00'00) >> 16),
        static_cast<char>((value & 0xFF'00'00'00) >> 24),
    };
    return stream.write(bytes, sf::base::getArraySize(bytes));
}
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SoundFileWriterWav::Impl
{
    base::Optional<sf::OutFile> file;             //!< Output file handle (empty before `open()`)
    unsigned int                channelCount{};   //!< Channel count of the sound being written
    base::SizeT                 remapTable[18]{}; //!< Table we use to remap source to target channel order
};


////////////////////////////////////////////////////////////
bool SoundFileWriterWav::check(const Path& filename)
{
    return filename.extensionIs(".wav");
}


////////////////////////////////////////////////////////////
SoundFileWriterWav::SoundFileWriterWav() = default;


////////////////////////////////////////////////////////////
SoundFileWriterWav::~SoundFileWriterWav()
{
    if (!m_impl->file.hasValue())
        return;

    // Best-effort header finalization. Per-step failures get logged but
    // don't abort the cleanup -- the destructor of `m_impl->file` then
    // closes the OS handle. Destructors can't propagate errors.
    const auto reportFailure = [](const char* op) { priv::errMsg("WAV writer cleanup: {} failed", op); };

    auto& file = *m_impl->file;

    if (!file.flush())
        reportFailure("flush");

    base::PtrDiffT fileSizeRaw = 0;
    if (!file.tellPos(fileSizeRaw))
        reportFailure("tellPos");

    const auto fileSize = static_cast<base::U32>(fileSizeRaw);

    if (!file.seekPos(4) || !encode(file, fileSize - 8)) // 8 bytes RIFF header
        reportFailure("RIFF chunk-size patch");

    if (!file.seekPos(40) || !encode(file, fileSize - 44)) // 44 bytes RIFF + WAVE headers
        reportFailure("data chunk-size patch");
}


////////////////////////////////////////////////////////////
bool SoundFileWriterWav::open(const Path& filename, unsigned int sampleRate, unsigned int channelCount, const ChannelMap& channelMap)
{
    auto channelMask = 0u;

    if (channelCount == 0)
    {
        priv::errMsg("WAV sound file channel count 0");
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
        // For WAVE channel mapping refer to:
        // https://learn.microsoft.com/en-us/previous-versions/windows/hardware/design/dn653308(v=vs.85)#default-channel-ordering
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
            speakerBackCenter         = 0x1'00u,
            speakerSideLeft           = 0x2'00u,
            speakerSideRight          = 0x4'00u,
            speakerTopCenter          = 0x8'00u,
            speakerTopFrontLeft       = 0x10'00u,
            speakerTopFrontCenter     = 0x20'00u,
            speakerTopFrontRight      = 0x40'00u,
            speakerTopBackLeft        = 0x80'00u,
            speakerTopBackCenter      = 0x1'00'00u,
            speakerTopBackRight       = 0x2'00'00u
        };

        struct SupportedChannel
        {
            base::U32    bit;
            SoundChannel channel;
        };

        base::Vector<SupportedChannel>
            targetChannelMap{{speakerFrontLeft, SoundChannel::FrontLeft},
                             SupportedChannel{speakerFrontRight, SoundChannel::FrontRight},
                             SupportedChannel{speakerFrontCenter, SoundChannel::FrontCenter},
                             SupportedChannel{speakerLowFrequency, SoundChannel::LowFrequencyEffects},
                             SupportedChannel{speakerBackLeft, SoundChannel::BackLeft},
                             SupportedChannel{speakerBackRight, SoundChannel::BackRight},
                             SupportedChannel{speakerFrontLeftOfCenter, SoundChannel::FrontLeftOfCenter},
                             SupportedChannel{speakerFrontRightOfCenter, SoundChannel::FrontRightOfCenter},
                             SupportedChannel{speakerBackCenter, SoundChannel::BackCenter},
                             SupportedChannel{speakerSideLeft, SoundChannel::SideLeft},
                             SupportedChannel{speakerSideRight, SoundChannel::SideRight},
                             SupportedChannel{speakerTopCenter, SoundChannel::TopCenter},
                             SupportedChannel{speakerTopFrontLeft, SoundChannel::TopFrontLeft},
                             SupportedChannel{speakerTopFrontCenter, SoundChannel::TopFrontCenter},
                             SupportedChannel{speakerTopFrontRight, SoundChannel::TopFrontRight},
                             SupportedChannel{speakerTopBackLeft, SoundChannel::TopBackLeft},
                             SupportedChannel{speakerTopBackCenter, SoundChannel::TopBackCenter},
                             SupportedChannel{speakerTopBackRight, SoundChannel::TopBackRight}};


        // Check for duplicate channel entries
        {
            auto sortedChannelMap = channelMap;
            sf::base::quickSort(sortedChannelMap.begin(), sortedChannelMap.end());

            if (sf::base::adjacentFind(sortedChannelMap.begin(), sortedChannelMap.end()) != sortedChannelMap.end())
            {
                priv::errMsg("Duplicate channels in channel map");
                return false;
            }
        }

        // Construct the target channel map by removing unused channels
        for (auto* iter = targetChannelMap.begin(); iter != targetChannelMap.end();)
        {
            if (base::find(channelMap.begin(), channelMap.end(), iter->channel) == channelMap.end())
                iter = targetChannelMap.erase(iter);
            else
                ++iter;
        }

        // Verify that all the input channels exist in the target channel map
        for (const SoundChannel channel : channelMap)
        {
            if (base::findIf(targetChannelMap.begin(), targetChannelMap.end(), [channel](const SupportedChannel& c) {
                return c.channel == channel;
            }) == targetChannelMap.end())
            {
                priv::errMsg("Could not map all input channels to a channel supported by WAV");
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
    m_impl->file = sf::OutFile::open(filename.c_str(), FileOpenMode::bin);
    if (!m_impl->file.hasValue())
    {
        priv::errMsg("Failed to open WAV sound file for writing\n{}", priv::PathDebugFormatter{filename});
        return false;
    }

    // Write the header
    writeHeader(sampleRate, channelCount, channelMask);

    return true;
}


////////////////////////////////////////////////////////////
void SoundFileWriterWav::write(const base::I16* samples, base::U64 count)
{
    SFML_BASE_ASSERT(count % m_impl->channelCount == 0);
    SFML_BASE_ASSERT(m_impl->file.hasValue() && "WAV writer: `write` called before `open`");

    if (count % m_impl->channelCount != 0)
        priv::errMsg("Writing samples to WAV sound file requires writing full frames at a time");

    auto& file = *m_impl->file;

    // The base class's `write` is `void`; we can't propagate per-call errors
    // upward. Bail on the first encode failure and log via `errMsg` so the
    // user notices a truncated/corrupt output file.
    while (count >= m_impl->channelCount)
    {
        for (auto i = 0u; i < m_impl->channelCount; ++i)
            if (!encode(file, samples[m_impl->remapTable[i]]))
            {
                priv::errMsg("WAV writer: sample encode failed; output truncated");
                return;
            }

        samples += m_impl->channelCount;
        count -= m_impl->channelCount;
    }
}


////////////////////////////////////////////////////////////
void SoundFileWriterWav::writeHeader(unsigned int sampleRate, unsigned int channelCount, unsigned int channelMask)
{
    SFML_BASE_ASSERT(m_impl->file.hasValue() && "WAV writer: `writeHeader` called before `open`");

    auto& file = *m_impl->file;

    const auto fail = []()
    {
        priv::errMsg("WAV writer: header write failed; output truncated");
        return false;
    };

    constexpr const char mainChunkId[]{'R', 'I', 'F', 'F'};
    if (!file.write(mainChunkId, base::getArraySize(mainChunkId)))
    {
        (void)fail();
        return;
    }

    // Placeholder; main chunk size will be patched in the destructor.
    if (!encode(file, base::U32{0}))
    {
        (void)fail();
        return;
    }

    constexpr const char mainChunkFormat[]{'W', 'A', 'V', 'E'};
    if (!file.write(mainChunkFormat, base::getArraySize(mainChunkFormat)))
    {
        (void)fail();
        return;
    }

    // Sub-chunk 1 ("format") id and size
    constexpr const char fmtChunkId[]{'f', 'm', 't', ' '};
    if (!file.write(fmtChunkId, base::getArraySize(fmtChunkId)))
    {
        (void)fail();
        return;
    }

    if (channelCount > 2)
    {
        if (!encode(file, base::U32{40}))
        {
            (void)fail();
            return;
        }
        if (!encode(file, base::U16{65'534}))
        {
            (void)fail();
            return;
        } // Extensible
    }
    else
    {
        if (!encode(file, base::U32{16}))
        {
            (void)fail();
            return;
        }
        if (!encode(file, base::U16{1}))
        {
            (void)fail();
            return;
        } // PCM
    }

    // Sound attributes
    if (!encode(file, static_cast<base::U16>(channelCount)))
    {
        (void)fail();
        return;
    }
    if (!encode(file, sampleRate))
    {
        (void)fail();
        return;
    }
    if (!encode(file, base::U32{sampleRate * channelCount * 2}))
    {
        (void)fail();
        return;
    } // byteRate
    if (!encode(file, static_cast<base::U16>(channelCount * 2)))
    {
        (void)fail();
        return;
    } // blockAlign

    constexpr base::U16 bitsPerSample = 16;
    if (!encode(file, bitsPerSample))
    {
        (void)fail();
        return;
    }

    if (channelCount > 2)
    {
        if (!encode(file, base::U16{16}))
        {
            (void)fail();
            return;
        } // extensionSize
        if (!encode(file, bitsPerSample))
        {
            (void)fail();
            return;
        }
        if (!encode(file, channelMask))
        {
            (void)fail();
            return;
        }

        // PCM subformat
        constexpr char subformat[] =
            {'\x01', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x00', '\x80', '\x00', '\x00', '\xAA', '\x00', '\x38', '\x9B', '\x71'};
        if (!file.write(subformat, base::getArraySize(subformat)))
        {
            (void)fail();
            return;
        }
    }

    // Sub-chunk 2 ("data") id and placeholder size (patched in the destructor)
    constexpr const char dataChunkId[]{'d', 'a', 't', 'a'};
    if (!file.write(dataChunkId, base::getArraySize(dataChunkId)))
    {
        (void)fail();
        return;
    }
    if (!encode(file, base::U32{0}))
    {
        (void)fail();
        return;
    }
}


} // namespace sf::priv
