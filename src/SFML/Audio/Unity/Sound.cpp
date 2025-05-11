#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

// TODO P0: consider passing buffer in on .play and not storing it

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundSource.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct Sound::Impl
{
    ////////////////////////////////////////////////////////////
    void initialize()
    {
        SFML_BASE_ASSERT(soundBase.hasValue());

        if (!soundBase->initialize(&onEnd))
            priv::err() << "Failed to initialize Sound::Impl";

        // Because we are providing a custom data source, we have to provide the channel map ourselves
        if (buffer == nullptr || buffer->getChannelMap().isEmpty())
        {
            soundBase->getSound().engineNode.spatializer.pChannelMapIn = nullptr;
            return;
        }

        soundBase->clearSoundChannelMap();

        for (const SoundChannel channel : buffer->getChannelMap())
            soundBase->addToSoundChannelMap(priv::MiniaudioUtils::soundChannelToMiniaudioChannel(channel));

        soundBase->refreshSoundChannelMap();
    }

    ////////////////////////////////////////////////////////////
    static void onEnd(void* const userData, ma_sound* const soundPtr)
    {
        auto& impl  = *static_cast<Impl*>(userData);
        impl.status = Status::Stopped;

        // Seek back to the start of the sound when it finishes playing
        if (const ma_result result = ma_sound_seek_to_pcm_frame(soundPtr, 0); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("seek sound to frame 0", result);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result read(ma_data_source* const dataSource,
                                        void* const           framesOut,
                                        const ma_uint64       frameCount,
                                        ma_uint64* const      framesRead)
    {
        auto&       impl   = *static_cast<Impl*>(dataSource);
        const auto* buffer = impl.buffer;

        *framesRead = 0u;

        if (buffer == nullptr)
            return MA_NO_DATA_AVAILABLE;

        const ma_uint32 channelCount = buffer->getChannelCount();
        SFML_BASE_ASSERT(channelCount > 0u);

        const ma_uint64 totalBufferSamples = buffer->getSampleCount();
        const ma_uint64 totalBufferFrames  = totalBufferSamples / channelCount;

        // If cursor is already at or beyond the end of the buffer, either loop or exit
        if (impl.cursor >= totalBufferFrames)
        {
            if (impl.mustLoop)
                impl.cursor = 0u;
            else
                return MA_SUCCESS;
        }

        // Determine how many frames we can read
        *framesRead = base::min(frameCount, static_cast<ma_uint64>(totalBufferFrames - impl.cursor));

        // Copy the samples to the output
        const auto sampleCount = *framesRead * buffer->getChannelCount();

        SFML_BASE_MEMCPY(framesOut,
                         buffer->getSamples() + impl.cursor * buffer->getChannelCount(),
                         static_cast<base::SizeT>(sampleCount) * sizeof(buffer->getSamples()[0]));

        impl.cursor += *framesRead;

        // If we are looping and at the end of the sound, set the cursor back to the start
        if (impl.mustLoop && impl.cursor >= totalBufferFrames)
            impl.cursor = 0u;

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result seek(ma_data_source* const dataSource, const ma_uint64 frameIndex)
    {
        auto&       impl   = *static_cast<Impl*>(dataSource);
        const auto* buffer = impl.buffer;

        if (buffer == nullptr)
            return MA_NO_DATA_AVAILABLE;

        impl.cursor = static_cast<base::SizeT>(frameIndex);

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
        const auto& impl   = *static_cast<const Impl*>(dataSource);
        const auto* buffer = impl.buffer;

        // If we don't have valid values yet, initialize with defaults so sound creation doesn't fail
        *format     = ma_format_s16;
        *channels   = buffer && buffer->getChannelCount() ? buffer->getChannelCount() : 1;
        *sampleRate = buffer && buffer->getSampleRate() ? buffer->getSampleRate() : 44'100;

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getCursor(ma_data_source* const dataSource, ma_uint64* const cursor)
    {
        const auto& impl   = *static_cast<const Impl*>(dataSource);
        const auto* buffer = impl.buffer;

        if (buffer == nullptr)
            return MA_NO_DATA_AVAILABLE;

        *cursor = impl.cursor;

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getLength(ma_data_source* const dataSource, ma_uint64* const length)
    {
        const auto& impl   = *static_cast<const Impl*>(dataSource);
        const auto* buffer = impl.buffer;

        if (buffer == nullptr)
            return MA_NO_DATA_AVAILABLE;

        *length = buffer->getSampleCount() / buffer->getChannelCount();

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
    static inline constexpr ma_data_source_vtable vtable{read, seek, getFormat, getCursor, getLength, setLooping, 0};

    base::Optional<priv::MiniaudioUtils::SoundBase> soundBase; //!< Sound base, needs to be first member
    base::SizeT                                     cursor{};  //!< The current playing position (in frames)
    const SoundBuffer*                              buffer{};  //!< Sound buffer bound to the source
    SoundSource::Status                             status{SoundSource::Status::Stopped}; //!< The status
    bool                                            mustLoop{false}; //!< Whether the sound must loop or not
};


////////////////////////////////////////////////////////////
Sound::Sound(const SoundBuffer& buffer)
{
    setBuffer(buffer);

    SFML_UPDATE_LIFETIME_DEPENDANT(SoundBuffer, Sound, this, m_impl->buffer);
}


////////////////////////////////////////////////////////////
Sound::~Sound()
{
    stop();

    if (m_impl->buffer != nullptr)
        m_impl->buffer->detachSound(this);
}


////////////////////////////////////////////////////////////
void Sound::play(PlaybackDevice& playbackDevice)
{
    m_impl->mustLoop = isLooping();

    if (!m_impl->soundBase.hasValue())
    {
        m_impl->soundBase.emplace(playbackDevice, &Impl::vtable, [](void* ptr) { static_cast<Impl*>(ptr)->initialize(); });
        m_impl->initialize();

        SFML_BASE_ASSERT(m_impl->soundBase.hasValue());
        applyStoredSettings(m_impl->soundBase->getSound());
        setEffectProcessor(getEffectProcessor());
        setPlayingOffset(getPlayingOffset());
    }

    if (m_impl->status == Status::Playing)
        setPlayingOffset(Time{});

    if (const ma_result result = ma_sound_start(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("start playing sound", result);
        return;
    }

    m_impl->status = Status::Playing;
}


////////////////////////////////////////////////////////////
void Sound::pause()
{
    if (!m_impl->soundBase.hasValue())
        return;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("pause playing sound", result);
        return;
    }

    if (m_impl->status == Status::Playing)
        m_impl->status = Status::Paused;
}


////////////////////////////////////////////////////////////
void Sound::stop()
{
    if (!m_impl->soundBase.hasValue())
        return;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("stop playing sound", result);
        return;
    }

    setPlayingOffset(Time{});
    m_impl->status = Status::Stopped;
}


////////////////////////////////////////////////////////////
void Sound::setBuffer(const SoundBuffer& buffer)
{
    if (m_impl->buffer == &buffer)
        return;

    // First detach from the previous buffer
    if (m_impl->buffer != nullptr)
    {
        stop();

        // Reset cursor
        m_impl->cursor = 0u;
        m_impl->buffer->detachSound(this);
    }

    // Assign and use the new buffer
    m_impl->buffer = &buffer;
    m_impl->buffer->attachSound(this);

    if (m_impl->soundBase.hasValue())
    {
        m_impl->soundBase->deinitialize();
        m_impl->initialize();

        SFML_BASE_ASSERT(m_impl->soundBase.hasValue());
        applyStoredSettings(m_impl->soundBase->getSound());
        setEffectProcessor(getEffectProcessor());
        setPlayingOffset(getPlayingOffset());
    }

    SFML_UPDATE_LIFETIME_DEPENDANT(SoundBuffer, Sound, this, m_impl->buffer);
}


////////////////////////////////////////////////////////////
void Sound::copySettings(const Sound& rhs)
{
    if (this == &rhs)
        return;

    // Delegate to base class, which copies all the sound attributes
    SoundSource::operator=(rhs);
}


////////////////////////////////////////////////////////////
void Sound::setPlayingOffset(const Time playingOffset)
{
    SoundSource::setPlayingOffset(playingOffset);

    if (!m_impl->soundBase.hasValue())
        return;

    if (m_impl->soundBase->getSound().pDataSource == nullptr || m_impl->soundBase->getSound().engineNode.pEngine == nullptr)
        return;

    const auto frameIndex = ma_uint64{priv::MiniaudioUtils::getFrameIndex(m_impl->soundBase->getSound(), playingOffset)};

    m_impl->cursor = static_cast<base::SizeT>(frameIndex);
}


////////////////////////////////////////////////////////////
void Sound::setEffectProcessor(EffectProcessor effectProcessor)
{
    SoundSource::setEffectProcessor(effectProcessor);

    if (!m_impl->soundBase.hasValue())
        return;

    m_impl->soundBase->setAndConnectEffectProcessor(SFML_BASE_MOVE(effectProcessor));
}


////////////////////////////////////////////////////////////
const SoundBuffer& Sound::getBuffer() const
{
    return *m_impl->buffer;
}


////////////////////////////////////////////////////////////
Time Sound::getPlayingOffset() const
{
    if (m_impl->buffer == nullptr || m_impl->buffer->getChannelCount() == 0 || m_impl->buffer->getSampleRate() == 0)
        return Time{};

    return SoundSource::getPlayingOffset();
}


////////////////////////////////////////////////////////////
Sound::Status Sound::getStatus() const
{
    return m_impl->status;
}


////////////////////////////////////////////////////////////
void Sound::setLooping(const bool loop)
{
    m_impl->mustLoop = loop;
    SoundSource::setLooping(loop);
}


////////////////////////////////////////////////////////////
void Sound::detachBuffer()
{
    // First stop the sound in case it is playing
    stop();

    // Detach the buffer
    if (m_impl->buffer != nullptr)
    {
        m_impl->buffer->detachSound(this);
        m_impl->buffer = nullptr;
    }
}


////////////////////////////////////////////////////////////
void Sound::detachBufferWithoutSignalling()
{
    // First stop the sound in case it is playing
    stop();

    // Detach the buffer without signalling the sound buffer
    m_impl->buffer = nullptr;
}


////////////////////////////////////////////////////////////
void* Sound::getSound() const
{
    if (!m_impl->soundBase.hasValue())
        return nullptr;

    // TODO P0: const bs
    return &const_cast<Sound*>(this)->m_impl->soundBase->getSound();
}

} // namespace sf
