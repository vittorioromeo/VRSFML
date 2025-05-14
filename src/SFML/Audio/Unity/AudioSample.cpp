#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ActiveSoundSource.hpp"
#include "SFML/Audio/AudioSample.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/ChannelMap.hpp" // used
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundBase.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/MinMax.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct AudioSample::Impl
{
    ////////////////////////////////////////////////////////////
    explicit Impl(PlaybackDevice& thePlaybackDevice, AudioSample& theOwner, const SoundBuffer& theBuffer) :
    soundBase(thePlaybackDevice, &Impl::vtable, theBuffer.getChannelMap()),
    owner(theOwner),
    buffer(theBuffer)
    {
        if (!soundBase.initialize(&onEnd))
        {
            priv::err() << "Failed to initialize sound base";
            return;
        }
    }

    ////////////////////////////////////////////////////////////
    static void onEnd(void* const userData, ma_sound* const soundPtr)
    {
        static_cast<Impl*>(userData)->owner.m_playing = false;

        // Seek back to the start of the sound when it finishes playing
        if (const ma_result result = ma_sound_seek_to_pcm_frame(soundPtr, 0u); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("seek sound to frame 0", result);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result read(ma_data_source* const dataSource,
                                        void* const           framesOut,
                                        const ma_uint64       frameCount,
                                        ma_uint64* const      framesRead)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        *framesRead = 0u;

        const ma_uint32 channelCount = impl.buffer.getChannelCount();
        SFML_BASE_ASSERT(channelCount > 0u);

        const ma_uint64 totalBufferSamples = impl.buffer.getSampleCount();
        const ma_uint64 totalBufferFrames  = totalBufferSamples / channelCount;

        // If cursor is already at or beyond the end of the buffer, either loop or exit
        if (impl.cursor >= totalBufferFrames)
        {
            if (impl.owner.isLooping())
                impl.cursor = 0u;
            else
                return MA_SUCCESS;
        }

        // Determine how many frames we can read
        *framesRead = base::min(frameCount, static_cast<ma_uint64>(totalBufferFrames - impl.cursor));

        // Copy the samples to the output
        const auto sampleCount = *framesRead * impl.buffer.getChannelCount();

        SFML_BASE_MEMCPY(framesOut,
                         impl.buffer.getSamples() + impl.cursor * impl.buffer.getChannelCount(),
                         static_cast<base::SizeT>(sampleCount) * sizeof(impl.buffer.getSamples()[0]));

        impl.cursor += *framesRead;

        // If we are looping and at the end of the sound, set the cursor back to the start
        if (impl.owner.isLooping() && impl.cursor >= totalBufferFrames)
            impl.cursor = 0u;

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result seek(ma_data_source* const dataSource, const ma_uint64 frameIndex)
    {
        static_cast<Impl*>(dataSource)->cursor = static_cast<base::SizeT>(frameIndex);
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getFormat(
        ma_data_source* const dataSource,
        ma_format* const      format,
        ma_uint32* const      channels,
        ma_uint32* const      sampleRate,
        ma_channel* const,
        const base::SizeT)
    {
        const auto& impl = *static_cast<const Impl*>(dataSource);

        // If we don't have valid values yet, initialize with defaults so sound creation doesn't fail
        *format     = ma_format_s16;
        *channels   = impl.buffer.getChannelCount();
        *sampleRate = impl.buffer.getSampleRate();

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getCursor(ma_data_source* const dataSource, ma_uint64* const cursor)
    {
        *cursor = static_cast<const Impl*>(dataSource)->cursor;
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getLength(ma_data_source* const dataSource, ma_uint64* const length)
    {
        const auto& impl = *static_cast<const Impl*>(dataSource);

        *length = impl.buffer.getSampleCount() / impl.buffer.getChannelCount();
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result setLooping(ma_data_source* const /* dataSource */, const ma_bool32 /* looping */)
    {
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    static inline constexpr ma_data_source_vtable vtable{read, seek, getFormat, getCursor, getLength, setLooping, /* flags */ 0u};

    priv::MiniaudioUtils::SoundBase soundBase; //!< Sound base, needs to be first member

    AudioSample&       owner;    //!< Owning `AudioSample` object
    base::U64          cursor{}; //!< The current playing position (in frames)
    const SoundBuffer& buffer;   //!< AudioSample buffer bound to the source
};


////////////////////////////////////////////////////////////
AudioSample::AudioSample(PlaybackDevice& playbackDevice, const SoundBuffer& buffer, const AudioSettings& audioSettings) :
m_impl(playbackDevice, *this, buffer)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(SoundBuffer, AudioSample, this, (&m_impl->buffer));

    // TODO P0: needed???
    applyAudioSettings(audioSettings);
}


////////////////////////////////////////////////////////////
AudioSample::~AudioSample() = default;


////////////////////////////////////////////////////////////
void AudioSample::setPlayingOffset(const Time playingOffset)
{
    auto& sound = m_impl->soundBase.getSound();

    SFML_BASE_ASSERT(sound.pDataSource != nullptr);
    SFML_BASE_ASSERT(sound.engineNode.pEngine != nullptr);

    m_impl->cursor = priv::MiniaudioUtils::getFrameIndex(sound, playingOffset).value();
}


////////////////////////////////////////////////////////////
priv::MiniaudioUtils::SoundBase& AudioSample::getSoundBase() const
{
    // TODO P0: const bs
    return const_cast<AudioSample*>(this)->m_impl->soundBase;
}


////////////////////////////////////////////////////////////
const SoundBuffer& AudioSample::getBuffer() const
{
    return m_impl->buffer;
}


////////////////////////////////////////////////////////////
PlaybackDevice& AudioSample::getPlaybackDevice() const
{
    return *m_impl->soundBase.playbackDevice;
}

} // namespace sf
