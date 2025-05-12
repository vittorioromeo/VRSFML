#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

// TODO P0: consider passing buffer in on .play and not storing it

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Priv/SoundImplUtils.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/SoundSource.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/MinMax.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct Sound::Impl
{
    ////////////////////////////////////////////////////////////
    explicit Impl(Sound& theOwner) : owner(theOwner)
    {
    }

    ////////////////////////////////////////////////////////////
    void initialize()
    {
        SFML_BASE_ASSERT(buffer != nullptr);
        priv::SoundImplUtils::implInitializeImpl(*this, buffer->getChannelMap());
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
            if (impl.owner.isLooping())
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
        if (impl.owner.isLooping() && impl.cursor >= totalBufferFrames)
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

    Sound&              owner;                                //!< Owning `Sound` object
    base::SizeT         cursor{};                             //!< The current playing position (in frames)
    const SoundBuffer*  buffer{};                             //!< Sound buffer bound to the source
    SoundSource::Status status{SoundSource::Status::Stopped}; //!< The status
    void*               lastUsedPlaybackDevice{nullptr};      //!< Last used playback device
};


////////////////////////////////////////////////////////////
Sound::Sound(const SoundBuffer& buffer) : m_impl(*this)
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
    priv::SoundImplUtils::playImpl(*this, playbackDevice);
}


////////////////////////////////////////////////////////////
void Sound::resumeOnLastPlaybackDevice()
{
    priv::SoundImplUtils::resumeOnLastPlaybackDeviceImpl(*this);
}


////////////////////////////////////////////////////////////
void Sound::pause()
{
    priv::SoundImplUtils::pauseImpl(*this);
}


////////////////////////////////////////////////////////////
void Sound::stop()
{
    priv::SoundImplUtils::stopImpl(*this);
}


////////////////////////////////////////////////////////////
void Sound::setPlayingOffset(const Time playingOffset)
{
    const auto frameIndex = priv::SoundImplUtils::setPlayingOffsetImpl(*this, playingOffset);
    if (!frameIndex.hasValue())
        return;

    m_impl->cursor = static_cast<base::SizeT>(*frameIndex);
}


////////////////////////////////////////////////////////////
Time Sound::getPlayingOffset() const
{
    if (m_impl->buffer == nullptr)
        return Time{};

    return priv::SoundImplUtils::getPlayingOffsetImpl(*this, m_impl->buffer->getChannelCount(), m_impl->buffer->getSampleRate());
}


////////////////////////////////////////////////////////////
Sound::Status Sound::getStatus() const
{
    return m_impl->status;
}


////////////////////////////////////////////////////////////
void* Sound::getSound() const
{
    // TODO P0: const bs
    return m_impl->soundBase.hasValue() ? &const_cast<Sound*>(this)->m_impl->soundBase->getSound() : nullptr;
}


////////////////////////////////////////////////////////////
void Sound::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    SoundSource::setEffectProcessor(effectProcessor);

    if (m_impl->soundBase.hasValue())
        m_impl->soundBase->setAndConnectEffectProcessor(effectProcessor);
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

        applySavedSettings(m_impl->soundBase->getSound());
        setEffectProcessor(getEffectProcessor());
        setPlayingOffset(getPlayingOffset());
    }

    SFML_UPDATE_LIFETIME_DEPENDANT(SoundBuffer, Sound, this, m_impl->buffer);
}


////////////////////////////////////////////////////////////
const SoundBuffer& Sound::getBuffer() const
{
    return *m_impl->buffer;
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

} // namespace sf
