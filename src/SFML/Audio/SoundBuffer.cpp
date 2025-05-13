#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/InputSoundFile.hpp"
#include "SFML/Audio/OutputSoundFile.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundBuffer::Impl
{
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(modernize-pass-by-value)
    explicit Impl(base::Vector<base::I16>&& theSamples, const ChannelMap& theChannelMap, const unsigned int theSampleRate) :
    samples(SFML_BASE_MOVE(theSamples)),
    channelMap(theChannelMap),
    sampleRate(theSampleRate)
    {
        SFML_BASE_ASSERT(channelMap.getSize() > 0u);
        SFML_BASE_ASSERT(sampleRate > 0u);

        duration = seconds(static_cast<float>(samples.size()) / static_cast<float>(sampleRate) /
                           static_cast<float>(channelMap.getSize()));
    }

    ////////////////////////////////////////////////////////////
    base::Vector<base::I16> samples;                        //!< Samples buffer
    ChannelMap              channelMap{SoundChannel::Mono}; //!< The map of position in sample frame to sound channel
    unsigned int            sampleRate{44'100};             //!< Number of samples per second
    Time                    duration;                       //!< Sound duration
};


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(const SoundBuffer& rhs) = default;


////////////////////////////////////////////////////////////
SoundBuffer& SoundBuffer::operator=(const SoundBuffer& rhs) = default;


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(SoundBuffer&& rhs) noexcept = default;


////////////////////////////////////////////////////////////
SoundBuffer& SoundBuffer::operator=(SoundBuffer&& rhs) noexcept = default;


////////////////////////////////////////////////////////////
SoundBuffer::~SoundBuffer() = default;


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromFile(const Path& filename)
{
    if (base::Optional file = InputSoundFile::openFromFile(filename))
        return initialize(*file);

    priv::err() << "Failed to open sound buffer from file";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromMemory(const void* data, base::SizeT sizeInBytes)
{
    if (base::Optional file = InputSoundFile::openFromMemory(data, sizeInBytes))
        return initialize(*file);

    priv::err() << "Failed to open sound buffer from memory";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromStream(InputStream& stream)
{
    if (base::Optional file = InputSoundFile::openFromStream(stream))
        return initialize(*file);

    priv::err() << "Failed to open sound buffer from stream";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
template <typename TVector>
base::Optional<SoundBuffer> SoundBuffer::loadFromSamplesImpl(TVector&&          samples,
                                                             const ChannelMap&  channelMap,
                                                             const unsigned int sampleRate)
{
    if (channelMap.isEmpty() || sampleRate == 0u)
    {
        priv::err() << "Failed to load sound buffer from samples ("
                    << "array: " << samples.data() << ", "
                    << "count: " << samples.size() << ", "
                    << "channels: " << channelMap.getSize() << ", "
                    << "samplerate: " << sampleRate << ")";

        return base::nullOpt; // Empty optional
    }

    // Take ownership of the audio samples
    return base::makeOptional<SoundBuffer>(base::PassKey<SoundBuffer>{}, &samples, channelMap, sampleRate);
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromSamples(const base::I16*   samples,
                                                         const base::U64    sampleCount,
                                                         const ChannelMap&  channelMap,
                                                         const unsigned int sampleRate)
{
    return loadFromSamplesImpl(base::Vector<base::I16>(samples, samples + sampleCount), channelMap, sampleRate);
}


////////////////////////////////////////////////////////////
bool SoundBuffer::saveToFile(const Path& filename) const
{
    // Create the sound file in write mode
    if (base::Optional file = OutputSoundFile::openFromFile(filename, getSampleRate(), getChannelCount(), getChannelMap()))
    {
        // Write the samples to the opened file
        file->write(m_impl->samples.data(), m_impl->samples.size());
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
const base::I16* SoundBuffer::getSamples() const
{
    return m_impl->samples.empty() ? nullptr : m_impl->samples.data();
}


////////////////////////////////////////////////////////////
base::U64 SoundBuffer::getSampleCount() const
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
ChannelMap SoundBuffer::getChannelMap() const
{
    return m_impl->channelMap;
}


////////////////////////////////////////////////////////////
Time SoundBuffer::getDuration() const
{
    return m_impl->duration;
}


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(base::PassKey<SoundBuffer>&&, void* samplesVectorPtr, const ChannelMap& channelMap, unsigned int sampleRate) :
m_impl(SFML_BASE_MOVE(*static_cast<base::Vector<base::I16>*>(samplesVectorPtr)), channelMap, sampleRate)
{
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::initialize(InputSoundFile& file)
{
    // Read the samples from the provided file
    const base::U64         sampleCount = file.getSampleCount();
    base::Vector<base::I16> samples(static_cast<base::SizeT>(sampleCount));

    if (file.read(samples.data(), sampleCount) != sampleCount)
        return base::nullOpt;

    return loadFromSamplesImpl(SFML_BASE_MOVE(samples), file.getChannelMap(), file.getSampleRate());
}

} // namespace sf
