// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Priv/SoundStreamStateImpl.hpp"

#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/Priv/MiniaudioUtils.hpp"
#include "SFML/Audio/Priv/SoundBase.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include <miniaudio.h>

#include <atomic>


namespace sf::priv
{
////////////////////////////////////////////////////////////
// The `Internals` struct IS the miniaudio data source: it has
// `soundBase` as its first member (whose first member is
// `ma_data_source_base`). This lets the miniaudio callbacks
// cast the `ma_data_source*` they receive back to `Internals*`.
//
// Its destructor calls `soundBase.uninitSound()` first --
// draining the audio thread -- before `sampleBuffer` and
// other members die. `SoundStreamState<State>`'s own destructor
// additionally calls `detachFromEngineAndDrain()` before
// `State` dies (which lives in `SoundStreamState<State>`, not
// here), so the audio callback never touches freed memory.
////////////////////////////////////////////////////////////
struct SoundStreamStateImpl::Internals
{
    ////////////////////////////////////////////////////////////
    MiniaudioUtils::SoundBase soundBase; //!< MUST be first member

    ////////////////////////////////////////////////////////////
    ChannelMap              channelMap;
    unsigned int            sampleRate{};
    base::Vector<base::I16> sampleBuffer;
    base::SizeT             sampleBufferCursor{};
    base::U64               samplesProcessed{};
    std::atomic<bool>       streaming{true};

    ////////////////////////////////////////////////////////////
    void*                         statePtr; //!< Opaque pointer to the `State` owned by `SoundStreamState<State>`
    SoundStreamStateImplCallbacks callbacks;

    ////////////////////////////////////////////////////////////
    explicit Internals(PlaybackDevice&                      thePlaybackDevice,
                       const ChannelMap&                    theChannelMap, // NOLINT(modernize-pass-by-value)
                       const unsigned int                   theSampleRate,
                       void* const                          theStatePtr,
                       const SoundStreamStateImplCallbacks& theCallbacks) :
        soundBase(thePlaybackDevice, &vtable, theChannelMap),
        channelMap(theChannelMap),
        sampleRate(theSampleRate),
        statePtr(theStatePtr),
        callbacks(theCallbacks)
    {
        if (!soundBase.initialize(&onEnd))
            priv::err() << "Failed to initialize sound base";
    }

    ////////////////////////////////////////////////////////////
    ~Internals()
    {
        // Drain audio thread BEFORE `sampleBuffer`/etc. die. The
        // owning `SoundStreamState<State>` has already drained via
        // `detachFromEngineAndDrain()`, so this is idempotent.
        soundBase.uninitSound();
    }

    Internals(const Internals&)            = delete;
    Internals& operator=(const Internals&) = delete;
    Internals(Internals&&)                 = delete;
    Internals& operator=(Internals&&)      = delete;

    ////////////////////////////////////////////////////////////
    static void onEnd(void* const userData, ma_sound* const soundPtr)
    {
        auto& internals = *static_cast<Internals*>(userData);
        internals.streaming.store(true, std::memory_order::release);

        if (const ma_result result = ma_sound_seek_to_pcm_frame(soundPtr, 0u); result != MA_SUCCESS)
            MiniaudioUtils::fail("seek sound to frame 0", result);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result read(ma_data_source* const dataSource,
                                        void* const           framesOut,
                                        const ma_uint64       frameCount,
                                        ma_uint64* const      framesRead)
    {
        auto& internals = *static_cast<Internals*>(dataSource);

        // Fill the buffer by asking the state for a chunk. The state
        // writes directly into `internals.sampleBuffer` -- no pointer
        // to derived-class memory ever escapes.
        if (internals.sampleBuffer.empty() && internals.streaming.load(std::memory_order::acquire))
        {
            internals.sampleBuffer.clear();

            const bool keepStreaming = internals.callbacks.onGetData(internals.statePtr, internals.sampleBuffer);
            internals.streaming.store(keepStreaming, std::memory_order::release);

            internals.sampleBufferCursor = 0;
        }

        if (internals.sampleBuffer.empty())
        {
            *framesRead = 0;
            return MA_SUCCESS;
        }

        const auto channelCount = internals.channelMap.getSize();

        *framesRead = base::min(frameCount,
                                static_cast<ma_uint64>(
                                    (internals.sampleBuffer.size() - internals.sampleBufferCursor) / channelCount));

        const auto sampleCount = *framesRead * channelCount;

        SFML_BASE_MEMCPY(framesOut,
                         internals.sampleBuffer.data() + internals.sampleBufferCursor,
                         static_cast<base::SizeT>(sampleCount) * sizeof(internals.sampleBuffer[0]));

        internals.sampleBufferCursor += static_cast<base::SizeT>(sampleCount);
        internals.samplesProcessed += sampleCount;

        if (internals.sampleBufferCursor >= internals.sampleBuffer.size())
        {
            internals.sampleBuffer.clear();
            internals.sampleBufferCursor = 0;

            // Streaming has ended -- ask the state whether we should loop.
            // `onLoop` returning nullopt stops playback; a valid sample
            // position resumes from there.
            if (!internals.streaming.load(std::memory_order::acquire))
            {
                if (const base::Optional seekPositionAfterLoop = internals.callbacks.onLoop(internals.statePtr))
                {
                    internals.streaming.store(true, std::memory_order::release);
                    internals.samplesProcessed = *seekPositionAfterLoop;
                }
            }
        }

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result seek(ma_data_source* const dataSource, const ma_uint64 frameIndex)
    {
        auto& internals = *static_cast<Internals*>(dataSource);
        SFML_BASE_ASSERT(internals.sampleRate > 0u);

        const auto channelCount = internals.channelMap.getSize();
        SFML_BASE_ASSERT(channelCount > 0u);

        internals.streaming.store(true, std::memory_order::release);

        internals.sampleBuffer.clear();
        internals.sampleBufferCursor = 0u;

        internals.samplesProcessed = frameIndex * channelCount;

        const Time offset = seconds(static_cast<float>(frameIndex) / static_cast<float>(internals.sampleRate));
        internals.callbacks.onSeek(internals.statePtr, offset);

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
        const auto& internals = *static_cast<const Internals*>(dataSource);
        SFML_BASE_ASSERT(internals.sampleRate > 0u);

        const auto channelCount = internals.channelMap.getSize();
        SFML_BASE_ASSERT(channelCount > 0u);

        *outFormat     = ma_format_s16;
        *outChannels   = static_cast<ma_uint32>(channelCount);
        *outSampleRate = static_cast<ma_uint32>(internals.sampleRate);

        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getCursor(ma_data_source* const dataSource, ma_uint64* const cursor)
    {
        auto& internals = *static_cast<Internals*>(dataSource);

        const auto channelCount = internals.channelMap.getSize();
        SFML_BASE_ASSERT(channelCount > 0u);

        *cursor = internals.samplesProcessed / channelCount;
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result getLength(ma_data_source* const, ma_uint64* const length)
    {
        *length = 0;
        return MA_NOT_IMPLEMENTED;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static ma_result setLooping(ma_data_source* const, const ma_bool32)
    {
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr ma_data_source_vtable vtable{read, seek, getFormat, getCursor, getLength, setLooping, 0u};
};


////////////////////////////////////////////////////////////
SoundStreamStateImpl::SoundStreamStateImpl(PlaybackDevice&                     playbackDevice,
                                           const ChannelMap&                   channelMap,
                                           const unsigned int                  sampleRate,
                                           void* const                         statePtr,
                                           const SoundStreamStateImplCallbacks callbacks) :
    m_internals(playbackDevice, channelMap, sampleRate, statePtr, callbacks)
{
    SFML_BASE_ASSERT(channelMap.getSize() > 0u);
    SFML_BASE_ASSERT(sampleRate > 0u);
}


////////////////////////////////////////////////////////////
SoundStreamStateImpl::~SoundStreamStateImpl() = default; // `Internals::~Internals` drains the audio thread


////////////////////////////////////////////////////////////
void SoundStreamStateImpl::setPlayingOffset(const Time playingOffset)
{
    auto& sound = m_internals->soundBase.getSound();
    SFML_BASE_ASSERT(sound.pDataSource != nullptr);
    SFML_BASE_ASSERT(sound.engineNode.pEngine != nullptr);

    const auto frameIndex = MiniaudioUtils::getFrameIndex(sound, playingOffset).value();

    m_internals->streaming.store(true, std::memory_order::release);

    m_internals->sampleBuffer.clear();
    m_internals->sampleBufferCursor = 0;

    m_internals->samplesProcessed = frameIndex * m_internals->channelMap.getSize();

    const Time offset = seconds(static_cast<float>(frameIndex) / static_cast<float>(m_internals->sampleRate));
    m_internals->callbacks.onSeek(m_internals->statePtr, offset);
}


////////////////////////////////////////////////////////////
void SoundStreamStateImpl::detachFromEngineAndDrain()
{
    m_internals->soundBase.uninitSound();
}


////////////////////////////////////////////////////////////
PlaybackDevice& SoundStreamStateImpl::getPlaybackDevice() const
{
    return *m_internals->soundBase.playbackDevice;
}


////////////////////////////////////////////////////////////
MiniaudioUtils::SoundBase& SoundStreamStateImpl::getSoundBase()
{
    return m_internals->soundBase;
}

} // namespace sf::priv
