#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/InputSoundFile.hpp>
#include <SFML/Audio/OutputSoundFile.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/Time.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Optional.hpp>

#include <unordered_set>
#include <vector>


namespace sf
{

////////////////////////////////////////////////////////////
using SoundList = std::unordered_set<Sound*>; //!< Set of unique sound instances


////////////////////////////////////////////////////////////
struct SoundBuffer::Impl
{
    explicit Impl() = default;

    explicit Impl(std::vector<std::int16_t>&& theSamples) : samples(SFML_BASE_MOVE(theSamples))
    {
    }

    std::vector<std::int16_t> samples;                        //!< Samples buffer
    unsigned int              sampleRate{44100};              //!< Number of samples per second
    ChannelMap                channelMap{SoundChannel::Mono}; //!< The map of position in sample frame to sound channel
    Time                      duration;                       //!< Sound duration
    mutable SoundList         sounds;                         //!< List of sounds that are using this buffer
};


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(const SoundBuffer& copy)
{
    // don't copy the attached sounds
    m_impl->samples  = copy.m_impl->samples;
    m_impl->duration = copy.m_impl->duration;

    // Update the internal buffer with the new samples
    if (!update(copy.getChannelCount(), copy.getSampleRate(), copy.getChannelMap()))
        priv::err() << "Failed to update copy-constructed sound buffer";
}


////////////////////////////////////////////////////////////
SoundBuffer::~SoundBuffer()
{
    // To prevent the iterator from becoming invalid, move the entire buffer to another
    // container. Otherwise calling resetBuffer would result in detachSound being
    // called which removes the sound from the internal list.
    SoundList sounds;
    sounds.swap(m_impl->sounds);

    // Detach the buffer from the sounds that use it
    for (Sound* soundPtr : sounds)
        soundPtr->detachBuffer();
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromFile(const Path& filename)
{
    if (auto file = InputSoundFile::openFromFile(filename))
        return initialize(*file);

    priv::err() << "Failed to open sound buffer from file";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromMemory(const void* data, std::size_t sizeInBytes)
{
    if (auto file = InputSoundFile::openFromMemory(data, sizeInBytes))
        return initialize(*file);

    priv::err() << "Failed to open sound buffer from memory";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromStream(InputStream& stream)
{
    if (auto file = InputSoundFile::openFromStream(stream))
        return initialize(*file);

    priv::err() << "Failed to open sound buffer from stream";
    return base::nullOpt;
}


////////////////////////////////////////////////////////////
template <typename TVector>
base::Optional<SoundBuffer> SoundBuffer::loadFromSamplesImpl(
    TVector&&         samples,
    unsigned int      channelCount,
    unsigned int      sampleRate,
    const ChannelMap& channelMap)
{
    base::Optional<SoundBuffer> soundBuffer; // Use a single local variable for NRVO

    if (channelCount == 0 || sampleRate == 0 || channelMap.isEmpty())
    {
        priv::err() << "Failed to load sound buffer from samples ("
                    << "array: " << samples.data() << ", "
                    << "count: " << samples.size() << ", "
                    << "channels: " << channelCount << ", "
                    << "samplerate: " << sampleRate << ")";

        return soundBuffer; // Empty optional
    }

    // Take ownership of the audio samples
    soundBuffer.emplace(base::PassKey<SoundBuffer>{}, &samples);

    // Update the internal buffer with the new samples
    if (!soundBuffer->update(channelCount, sampleRate, channelMap))
        soundBuffer.reset();

    return soundBuffer;
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::loadFromSamples(
    const std::int16_t* samples,
    std::uint64_t       sampleCount,
    unsigned int        channelCount,
    unsigned int        sampleRate,
    const ChannelMap&   channelMap)
{
    return loadFromSamplesImpl(std::vector<std::int16_t>(samples, samples + sampleCount), channelCount, sampleRate, channelMap);
}


////////////////////////////////////////////////////////////
bool SoundBuffer::saveToFile(const Path& filename) const
{
    // Create the sound file in write mode
    if (auto file = OutputSoundFile::openFromFile(filename, getSampleRate(), getChannelCount(), getChannelMap()))
    {
        // Write the samples to the opened file
        file->write(m_impl->samples.data(), m_impl->samples.size());

        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////
const std::int16_t* SoundBuffer::getSamples() const
{
    return m_impl->samples.empty() ? nullptr : m_impl->samples.data();
}


////////////////////////////////////////////////////////////
std::uint64_t SoundBuffer::getSampleCount() const
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
SoundBuffer& SoundBuffer::operator=(const SoundBuffer& right)
{
    SoundBuffer temp(right);

    std::swap(m_impl->samples, temp.m_impl->samples);
    std::swap(m_impl->sampleRate, temp.m_impl->sampleRate);
    std::swap(m_impl->channelMap, temp.m_impl->channelMap);
    std::swap(m_impl->duration, temp.m_impl->duration);
    std::swap(m_impl->sounds, temp.m_impl->sounds); // swap sounds too, so that they are detached when temp is destroyed

    return *this;
}


////////////////////////////////////////////////////////////
SoundBuffer::SoundBuffer(base::PassKey<SoundBuffer>&&, void* samplesVectorPtr) :
m_impl(SFML_BASE_MOVE(*static_cast<std::vector<std::int16_t>*>(samplesVectorPtr)))
{
}


////////////////////////////////////////////////////////////
base::Optional<SoundBuffer> SoundBuffer::initialize(InputSoundFile& file)
{
    // Read the samples from the provided file
    const std::uint64_t       sampleCount = file.getSampleCount();
    std::vector<std::int16_t> samples(static_cast<std::size_t>(sampleCount));

    if (file.read(samples.data(), sampleCount) != sampleCount)
    {
        return base::nullOpt;
    }

    return loadFromSamplesImpl(SFML_BASE_MOVE(samples), file.getChannelCount(), file.getSampleRate(), file.getChannelMap());
}


////////////////////////////////////////////////////////////
bool SoundBuffer::update(unsigned int channelCount, unsigned int sampleRate, const ChannelMap& channelMap)
{
    // Check parameters
    if (!channelCount || !sampleRate || (channelMap.getSize() != channelCount))
        return false;

    m_impl->sampleRate = sampleRate;
    m_impl->channelMap = channelMap;

    // First make a copy of the list of sounds so we can reattach later
    const SoundList sounds(m_impl->sounds);

    // Detach the buffer from the sounds that use it
    for (Sound* soundPtr : sounds)
        soundPtr->detachBuffer();

    // Compute the duration
    m_impl->duration = seconds(
        static_cast<float>(m_impl->samples.size()) / static_cast<float>(sampleRate) / static_cast<float>(channelCount));

    // Now reattach the buffer to the sounds that use it
    for (Sound* soundPtr : sounds)
        soundPtr->setBuffer(*this);

    return true;
}


////////////////////////////////////////////////////////////
void SoundBuffer::attachSound(Sound* sound) const
{
    m_impl->sounds.insert(sound);
}


////////////////////////////////////////////////////////////
void SoundBuffer::detachSound(Sound* sound) const
{
    m_impl->sounds.erase(sound);
}

} // namespace sf
