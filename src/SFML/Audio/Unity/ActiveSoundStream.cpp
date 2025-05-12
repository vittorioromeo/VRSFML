#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ActiveSoundSource.hpp"
#include "SFML/Audio/ActiveSoundStream.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct ActiveSoundStream::Impl
{
    ////////////////////////////////////////////////////////////
    explicit Impl(PlaybackDevice&    playbackDevice,
                  ActiveSoundStream& theOwner,
                  const ChannelMap&  channelMap, // NOLINT(modernize-pass-by-value)
                  const unsigned int sampleRate) :
    soundBase(playbackDevice, &Impl::vtable),
    owner(theOwner),
    channelMap(channelMap),
    sampleRate(sampleRate)
    {
        if (!soundBase.initialize(&onEnd))
        {
            priv::err() << "Failed to initialize audio sample::Impl";
            return;
        }

        // TODO P0: setchannelmap?
        for (const SoundChannel channel : channelMap)
            soundBase.addToSoundChannelMap(priv::MiniaudioUtils::soundChannelToMiniaudioChannel(channel));
    }

    ////////////////////////////////////////////////////////////
    static void onEnd(void* userData, ma_sound* soundPtr)
    {
        auto& impl     = *static_cast<Impl*>(userData);
        impl.streaming = true;
        impl.playing   = false;

        // Seek back to the start of the sound when it finishes playing
        if (const ma_result result = ma_sound_seek_to_pcm_frame(soundPtr, 0); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("seek sound to frame 0", result);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result read(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        // Try to fill our buffer with new samples if the source is still willing to stream data
        if (impl.sampleBuffer.empty() && impl.streaming)
        {
            Chunk chunk;

            impl.streaming = impl.owner.onGetData(chunk);

            if (chunk.samples && chunk.sampleCount)
            {
                impl.sampleBuffer.clear();
                impl.sampleBuffer.reserve(chunk.sampleCount);
                impl.sampleBuffer.unsafeEmplaceRange(chunk.samples, chunk.sampleCount);

                impl.sampleBufferCursor = 0;
            }
        }

        if (impl.sampleBuffer.empty())
        {
            *framesRead = 0;
            return MA_SUCCESS;
        }

        // Push the samples to miniaudio
        const auto channelCount = impl.channelMap.getSize();

        // Determine how many frames we can read
        *framesRead = base::min(frameCount,
                                static_cast<ma_uint64>((impl.sampleBuffer.size() - impl.sampleBufferCursor) / channelCount));

        const auto sampleCount = *framesRead * channelCount;

        // Copy the samples to the output
        SFML_BASE_MEMCPY(framesOut,
                         impl.sampleBuffer.data() + impl.sampleBufferCursor,
                         static_cast<base::SizeT>(sampleCount) * sizeof(impl.sampleBuffer[0]));

        impl.sampleBufferCursor += static_cast<base::SizeT>(sampleCount);
        impl.samplesProcessed += sampleCount;

        if (impl.sampleBufferCursor >= impl.sampleBuffer.size())
        {
            impl.sampleBuffer.clear();
            impl.sampleBufferCursor = 0;

            // If we are looping and at the end of the loop, set the cursor back to the beginning of the loop
            if (!impl.streaming && impl.owner.isLooping())
            {
                if (const base::Optional seekPositionAfterLoop = impl.owner.onLoop())
                {
                    impl.streaming        = true;
                    impl.samplesProcessed = *seekPositionAfterLoop;
                }
            }
        }

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result seek(ma_data_source* dataSource, ma_uint64 frameIndex)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        const auto channelCount = impl.channelMap.getSize();
        const auto sampleRate   = impl.sampleRate;

        SFML_BASE_ASSERT(channelCount > 0u);
        SFML_BASE_ASSERT(sampleRate > 0u);

        impl.streaming = true;
        impl.sampleBuffer.clear();
        impl.sampleBufferCursor = 0;
        impl.samplesProcessed   = frameIndex * channelCount;

        const Time offset = seconds(static_cast<float>(frameIndex) / static_cast<float>(sampleRate));

        impl.owner.onSeek(offset);
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getFormat(
        ma_data_source* const dataSource,
        ma_format* const      outFormat,
        ma_uint32* const      outChannels,
        ma_uint32* const      outSampleRate,
        ma_channel* const,
        const base::SizeT)
    {
        const auto& impl = *static_cast<const Impl*>(dataSource);

        const auto channelCount = impl.channelMap.getSize();
        const auto sampleRate   = impl.sampleRate;

        SFML_BASE_ASSERT(channelCount > 0u);
        SFML_BASE_ASSERT(sampleRate > 0u);

        // If we don't have valid values yet, initialize with defaults so sound creation doesn't fail
        *outFormat     = ma_format_s16;
        *outChannels   = static_cast<ma_uint32>(channelCount);
        *outSampleRate = static_cast<ma_uint32>(sampleRate);

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getCursor(ma_data_source* dataSource, ma_uint64* cursor)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        const auto channelCount = impl.channelMap.getSize();
        SFML_BASE_ASSERT(channelCount > 0u);

        *cursor = impl.samplesProcessed / channelCount;

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getLength(ma_data_source*, ma_uint64* length)
    {
        *length = 0;
        return MA_NOT_IMPLEMENTED;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result setLooping(ma_data_source* /* dataSource */, ma_bool32 /* looping */)
    {
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    static inline constexpr ma_data_source_vtable vtable{read, seek, getFormat, getCursor, getLength, setLooping, /* flags */ 0};

    priv::MiniaudioUtils::SoundBase soundBase; //!< Sound base, needs to be first member

    ActiveSoundStream&      owner;                //!< Owning `ActiveSoundStream` object
    ChannelMap              channelMap;           //!< The channel map of the sound
    base::U64               sampleRate{};         //!< Number of samples per second
    base::Vector<base::I16> sampleBuffer;         //!< Our temporary sample buffer
    base::SizeT             sampleBufferCursor{}; //!< The current read position in the temporary sample buffer
    base::U64               samplesProcessed{};   //!< Number of samples processed since beginning of the stream
    bool                    streaming{true};      //!< `true` if we are still streaming samples from the source
    bool                    playing{false};       //!< Whether the sound is playing
};


////////////////////////////////////////////////////////////
ActiveSoundStream::ActiveSoundStream(PlaybackDevice& playbackDevice, const ChannelMap& channelMap, const unsigned int sampleRate) :
m_impl(playbackDevice, *this, channelMap, sampleRate)
{
    SFML_BASE_ASSERT(channelMap.getSize() > 0u);
    SFML_BASE_ASSERT(sampleRate > 0u);

    if (const ma_result result = ma_sound_start(&m_impl->soundBase.getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("start playing audio stream", result);
        return;
    }

    m_impl->playing = true;
}


////////////////////////////////////////////////////////////
ActiveSoundStream::~ActiveSoundStream() = default;


////////////////////////////////////////////////////////////
bool ActiveSoundStream::resume()
{
    if (m_impl->playing)
        return true;

    if (const ma_result result = ma_sound_start(&m_impl->soundBase.getSound()); result != MA_SUCCESS)
        return priv::MiniaudioUtils::fail("start playing audio stream", result);

    m_impl->playing = true;
    return true;
}


////////////////////////////////////////////////////////////
bool ActiveSoundStream::pause()
{
    if (!m_impl->playing)
        return true;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase.getSound()); result != MA_SUCCESS)
        return priv::MiniaudioUtils::fail("stop playing audio stream", result);

    m_impl->playing = false;
    return true;
}


////////////////////////////////////////////////////////////
void ActiveSoundStream::setPlayingOffset(Time playingOffset)
{
    SFML_BASE_ASSERT(m_impl->sampleRate > 0u);

    auto& sound = m_impl->soundBase.getSound();

    SFML_BASE_ASSERT(sound.pDataSource != nullptr);
    SFML_BASE_ASSERT(sound.engineNode.pEngine != nullptr);

    const auto frameIndex = priv::MiniaudioUtils::getFrameIndex(m_impl->soundBase.getSound(), playingOffset);

    m_impl->streaming = true;
    m_impl->sampleBuffer.clear();
    m_impl->sampleBufferCursor = 0;
    m_impl->samplesProcessed   = frameIndex * m_impl->channelMap.getSize();

    onSeek(seconds(static_cast<float>(frameIndex) / static_cast<float>(m_impl->sampleRate)));
}


////////////////////////////////////////////////////////////
Time ActiveSoundStream::getPlayingOffset() const
{
    return priv::MiniaudioUtils::getPlayingOffset(getSound());
}


////////////////////////////////////////////////////////////
bool ActiveSoundStream::isPlaying() const
{
    return m_impl->playing;
}


////////////////////////////////////////////////////////////
ma_sound& ActiveSoundStream::getSound() const
{
    // TODO P0: const bs
    return const_cast<ActiveSoundStream*>(this)->m_impl->soundBase.getSound();
}


////////////////////////////////////////////////////////////
void ActiveSoundStream::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    ActiveSoundSource::setEffectProcessor(effectProcessor);
    m_impl->soundBase.setAndConnectEffectProcessor(effectProcessor);
}


////////////////////////////////////////////////////////////
void ActiveSoundStream::applySettings(const AudioSettings& s)
{
    ma_sound* sound = &m_impl->soundBase.getSound();

    ma_sound_set_cone(sound, s.cone.innerAngle.asRadians(), s.cone.outerAngle.asRadians(), s.cone.outerGain);
    ma_sound_set_position(sound, s.position.x, s.position.y, s.position.z);
    ma_sound_set_direction(sound, s.direction.x, s.direction.y, s.direction.z);
    ma_sound_set_velocity(sound, s.velocity.x, s.velocity.y, s.velocity.z);
    ma_sound_set_pitch(sound, s.pitch);
    ma_sound_set_pan(sound, s.pan);
    ma_sound_set_volume(sound, s.volume);
    ma_sound_set_directional_attenuation_factor(sound, s.directionalAttenuationFactor);
    ma_sound_set_doppler_factor(sound, s.dopplerFactor);
    ma_sound_set_min_distance(sound, s.minDistance);
    ma_sound_set_max_distance(sound, s.maxDistance);
    ma_sound_set_min_gain(sound, s.minGain);
    ma_sound_set_max_gain(sound, s.maxGain);
    ma_sound_set_rolloff(sound, s.rollOff);
    ma_sound_set_positioning(sound, static_cast<ma_positioning>(s.positioning));
    ma_sound_set_looping(sound, s.looping);
    ma_sound_set_spatialization_enabled(sound, s.spatializationEnabled);
}


////////////////////////////////////////////////////////////
base::Optional<base::U64> ActiveSoundStream::onLoop()
{
    onSeek(Time{});
    return base::makeOptional(base::U64{0});
}

} // namespace sf
