#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundStream.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundStream::Impl
{
    ////////////////////////////////////////////////////////////
    explicit Impl(SoundStream* const theOwner) : owner(theOwner)
    {
    }

    ////////////////////////////////////////////////////////////
    void initialize()
    {
        SFML_BASE_ASSERT(soundBase.hasValue());

        if (!soundBase->initialize(&onEnd))
            priv::err() << "Failed to initialize SoundStream::Impl";

        const auto channelMap = owner->getChannelMap();

        // Because we are providing a custom data source, we have to provide the channel map ourselves
        if (channelMap.isEmpty())
        {
            soundBase->getSound().engineNode.spatializer.pChannelMapIn = nullptr;
            return;
        }

        soundBase->clearSoundChannelMap();

        for (const SoundChannel channel : channelMap)
            soundBase->addToSoundChannelMap(priv::MiniaudioUtils::soundChannelToMiniaudioChannel(channel));

        soundBase->refreshSoundChannelMap();
    }

    ////////////////////////////////////////////////////////////
    static void onEnd(void* userData, ma_sound* soundPtr)
    {
        // Seek back to the start of the sound when it finishes playing
        auto& impl     = *static_cast<Impl*>(userData);
        impl.streaming = true;
        impl.status    = Status::Stopped;

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

            impl.streaming = impl.owner->onGetData(chunk);

            if (chunk.samples && chunk.sampleCount)
            {
                impl.sampleBuffer.clear();
                impl.sampleBuffer.reserve(chunk.sampleCount);
                impl.sampleBuffer.unsafeEmplaceRange(chunk.samples, chunk.sampleCount);

                impl.sampleBufferCursor = 0;
            }
        }

        // Push the samples to miniaudio
        if (!impl.sampleBuffer.empty())
        {
            const auto channelCount = impl.owner->getChannelCount();

            // Determine how many frames we can read
            *framesRead = base::min(frameCount,
                                    static_cast<ma_uint64>(
                                        (impl.sampleBuffer.size() - impl.sampleBufferCursor) / channelCount));

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
                if (!impl.streaming && impl.owner->isLooping())
                {
                    if (const base::Optional seekPositionAfterLoop = impl.owner->onLoop())
                    {
                        impl.streaming        = true;
                        impl.samplesProcessed = *seekPositionAfterLoop;
                    }
                }
            }
        }
        else
        {
            *framesRead = 0;
        }

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result seek(ma_data_source* dataSource, ma_uint64 frameIndex)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        const auto channelCount = impl.owner->getChannelCount();
        const auto sampleRate   = impl.owner->getSampleRate();

        impl.streaming = true;
        impl.sampleBuffer.clear();
        impl.sampleBufferCursor = 0;
        impl.samplesProcessed   = frameIndex * channelCount;

        const Time offset = sampleRate == 0 ? Time{}
                                            : seconds(static_cast<float>(frameIndex) / static_cast<float>(sampleRate));

        impl.owner->onSeek(offset);
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getFormat(
        ma_data_source* dataSource,
        ma_format*      outFormat,
        ma_uint32*      outChannels,
        ma_uint32*      outSampleRate,
        ma_channel*,
        base::SizeT)
    {
        const auto& impl = *static_cast<const Impl*>(dataSource);

        const auto channelCount = impl.owner->getChannelCount();
        const auto sampleRate   = impl.owner->getSampleRate();

        // If we don't have valid values yet, initialize with defaults so sound creation doesn't fail
        *outFormat     = ma_format_s16;
        *outChannels   = channelCount ? channelCount : 1;
        *outSampleRate = sampleRate ? sampleRate : 44'100;

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getCursor(ma_data_source* dataSource, ma_uint64* cursor)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        const auto channelCount = impl.owner->getChannelCount();

        *cursor = channelCount ? impl.samplesProcessed / channelCount : 0;

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

    base::Optional<priv::MiniaudioUtils::SoundBase> soundBase; //!< Sound base, needs to be first member

    SoundStream*            owner;                //!< Owning `SoundStream` object
    base::Vector<base::I16> sampleBuffer;         //!< Our temporary sample buffer
    base::SizeT             sampleBufferCursor{}; //!< The current read position in the temporary sample buffer
    base::U64               samplesProcessed{};   //!< Number of samples processed since beginning of the stream
    bool                    streaming{true};      //!< `true` if we are still streaming samples from the source
    SoundSource::Status     status{SoundSource::Status::Stopped}; //!< The status
};


////////////////////////////////////////////////////////////
SoundStream::SoundStream() : m_impl(this)
{
}


////////////////////////////////////////////////////////////
SoundStream::~SoundStream() = default;


////////////////////////////////////////////////////////////
void SoundStream::play(PlaybackDevice& playbackDevice)
{
    m_lastPlaybackDevice = &playbackDevice;

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
        priv::MiniaudioUtils::fail("start playing sound stream", result);
        return;
    }

    m_impl->status = Status::Playing;
}


////////////////////////////////////////////////////////////
void SoundStream::pause()
{
    if (!m_impl->soundBase.hasValue())
        return;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("stop playing sound stream", result);
        return;
    }

    if (m_impl->status == Status::Playing)
        m_impl->status = Status::Paused;
}


////////////////////////////////////////////////////////////
void SoundStream::stop()
{
    if (!m_impl->soundBase.hasValue())
        return;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("stop playing sound stream", result);
        return;
    }

    setPlayingOffset(Time{});
    m_impl->status = Status::Stopped;
}


////////////////////////////////////////////////////////////
SoundStream::Status SoundStream::getStatus() const
{
    return m_impl->status;
}


////////////////////////////////////////////////////////////
void SoundStream::setPlayingOffset(Time playingOffset)
{
    SoundSource::setPlayingOffset(playingOffset);

    if (!m_impl->soundBase.hasValue())
        return;

    if (getSampleRate() == 0)
        return;

    if (m_impl->soundBase->getSound().pDataSource == nullptr || m_impl->soundBase->getSound().engineNode.pEngine == nullptr)
        return;

    const auto frameIndex = ma_uint64{priv::MiniaudioUtils::getFrameIndex(m_impl->soundBase->getSound(), playingOffset)};

    m_impl->streaming = true;
    m_impl->sampleBuffer.clear();
    m_impl->sampleBufferCursor = 0;
    m_impl->samplesProcessed   = frameIndex * getChannelCount();

    onSeek(seconds(static_cast<float>(frameIndex) / static_cast<float>(getSampleRate())));
}


////////////////////////////////////////////////////////////
Time SoundStream::getPlayingOffset() const
{
    if (getChannelCount() == 0 || getSampleRate() == 0)
        return Time{};

    return SoundSource::getPlayingOffset();
}


////////////////////////////////////////////////////////////
void SoundStream::setEffectProcessor(EffectProcessor effectProcessor)
{
    SoundSource::setEffectProcessor(effectProcessor);

    if (!m_impl->soundBase.hasValue())
        return;

    m_impl->soundBase->setAndConnectEffectProcessor(SFML_BASE_MOVE(effectProcessor));
}


////////////////////////////////////////////////////////////
base::Optional<base::U64> SoundStream::onLoop()
{
    onSeek(Time{});
    return base::makeOptional(base::U64{0});
}


////////////////////////////////////////////////////////////
void* SoundStream::getSound() const
{
    if (!m_impl->soundBase.hasValue())
        return nullptr;

    // TODO P0: const bs
    return &const_cast<SoundStream*>(this)->m_impl->soundBase->getSound();
}

} // namespace sf
