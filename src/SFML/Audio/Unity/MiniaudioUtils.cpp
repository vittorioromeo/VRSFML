#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"
#include "SFML/Audio/SoundChannel.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Builtins/OffsetOf.hpp"
#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/MinMax.hpp"

#include <miniaudio.h>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct MiniaudioUtils::SoundBase::Impl
{
    ////////////////////////////////////////////////////////////
    struct EffectNode
    {
        ma_node_base base{};
        ma_uint32    channelCount{};
    };

    ////////////////////////////////////////////////////////////
    explicit Impl(PlaybackDevice& thePlaybackDevice) :
    dataSourceBase{}, // must be first member!
    playbackDevice(&thePlaybackDevice)
    {
    }

    ////////////////////////////////////////////////////////////
    static void nodeOnProcess(ma_node* const      node,
                              const float** const framesIn,
                              ma_uint32* const    frameCountIn,
                              float** const       framesOut,
                              ma_uint32* const    frameCountOut)
    {
        Impl& impl = *(reinterpret_cast<Impl*>(static_cast<char*>(node) - SFML_BASE_OFFSETOF(Impl, effectNode)));

        // Assuming that `onProcess` is never called after the destructor of `SoundBase` is finished
        SFML_BASE_ASSERT(!impl.effectNodeUninitialized);
        impl.processEffect(framesIn, *frameCountIn, framesOut, *frameCountOut);
    }

    ////////////////////////////////////////////////////////////
    void processEffect(const float** const framesIn, base::U32& frameCountIn, float** const framesOut, base::U32& frameCountOut) const
    {
        // If a processor is set, call it
        if (effectProcessor)
        {
            if (!framesIn)
                frameCountIn = 0;

            effectProcessor(framesIn ? framesIn[0] : nullptr, frameCountIn, framesOut[0], frameCountOut, effectNode.channelCount);
            return;
        }

        // Otherwise just pass the data through 1:1
        if (framesIn == nullptr)
        {
            frameCountIn  = 0;
            frameCountOut = 0;
            return;
        }

        const auto toProcess = base::min(frameCountIn, frameCountOut);
        SFML_BASE_MEMCPY(framesOut[0], framesIn[0], toProcess * effectNode.channelCount * sizeof(float));
        frameCountIn  = toProcess;
        frameCountOut = toProcess;
    }

    ////////////////////////////////////////////////////////////
    static inline constexpr ma_node_vtable effectNodeVTable{
        .onProcess                    = &nodeOnProcess,
        .onGetRequiredInputFrameCount = nullptr,
        .inputBusCount                = 1,
        .outputBusCount               = 1,
        .flags                        = MA_NODE_FLAG_CONTINUOUS_PROCESSING | MA_NODE_FLAG_ALLOW_NULL_INPUT,
    };

    ////////////////////////////////////////////////////////////
    ma_data_source_base dataSourceBase{}; //!< The struct that makes this object a miniaudio data source (must be first member)

    PlaybackDevice* playbackDevice;

    EffectNode effectNode; //!< The engine node that performs effect processing

    base::InPlaceVector<ma_channel, MA_CHANNEL_POSITION_COUNT>
        soundChannelMap; //!< The map of position in sample frame to sound channel (miniaudio channels)

    ma_sound        sound{};         //!< The sound
    EffectProcessor effectProcessor; //!< The effect processor

    [[maybe_unused]] bool effectNodeUninitialized{}; //!< Failsafe debug boolean to check if `onProcess` is called after destruction
};


////////////////////////////////////////////////////////////
MiniaudioUtils::SoundBase::SoundBase(PlaybackDevice& thePlaybackDevice, const void* const dataSourceVTable) :
impl(thePlaybackDevice)
{
    // Set this object up as a miniaudio data source
    ma_data_source_config config = ma_data_source_config_init();
    config.vtable                = static_cast<const ma_data_source_vtable*>(dataSourceVTable);

    if (const ma_result result = ma_data_source_init(&config, &impl->dataSourceBase); result != MA_SUCCESS)
        fail("initialize audio data source", result);

    SFML_UPDATE_LIFETIME_DEPENDANT(PlaybackDevice, SoundBase, this, impl->playbackDevice);
}


////////////////////////////////////////////////////////////
MiniaudioUtils::SoundBase::~SoundBase()
{
    ma_sound_uninit(&impl->sound);

    ma_node_uninit(&impl->effectNode, nullptr);
    impl->effectNodeUninitialized = true; // Only for debugging

    ma_data_source_uninit(&impl->dataSourceBase);
}


////////////////////////////////////////////////////////////
bool MiniaudioUtils::SoundBase::initialize(ma_sound_end_proc endCallback)
{
    // Get the engine
    auto* engine = static_cast<ma_engine*>(impl->playbackDevice->getMAEngine());

    // Initialize the sound
    ma_sound_config soundConfig      = ma_sound_config_init();
    soundConfig.pDataSource          = this;
    soundConfig.pEndCallbackUserData = this;
    soundConfig.endCallback          = endCallback;

    if (const ma_result result = ma_sound_init_ex(engine, &soundConfig, &impl->sound); result != MA_SUCCESS)
        return fail("initialize sound", result);

    // Initialize the custom effect node
    const auto nodeChannelCount = ma_engine_get_channels(engine);

    ma_node_config nodeConfig  = ma_node_config_init();
    nodeConfig.vtable          = &Impl::effectNodeVTable;
    nodeConfig.pInputChannels  = &nodeChannelCount;
    nodeConfig.pOutputChannels = &nodeChannelCount;

    if (const ma_result result = ma_node_init(ma_engine_get_node_graph(engine), &nodeConfig, nullptr, &impl->effectNode);
        result != MA_SUCCESS)
        return fail("initialize effect node", result);

    impl->effectNode.channelCount = nodeChannelCount;

    // Route the sound through the effect node depending on whether an effect processor is set
    connectEffect(bool{impl->effectProcessor});

    return true;
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::deinitialize()
{
    ma_sound_uninit(&impl->sound);
    ma_node_uninit(&impl->effectNode, nullptr);
}


////////////////////////////////////////////////////////////
bool MiniaudioUtils::SoundBase::connectEffect(bool connect)
{
    auto* engine = static_cast<ma_engine*>(impl->playbackDevice->getMAEngine());

    if (connect)
    {
        // Attach the custom effect node output to our engine endpoint
        if (const ma_result result = ma_node_attach_output_bus(&impl->effectNode, 0, ma_engine_get_endpoint(engine), 0);
            result != MA_SUCCESS)
            return fail("attach effect node output to endpoint", result);
    }
    else
    {
        // Detach the custom effect node output from our engine endpoint
        if (const ma_result result = ma_node_detach_output_bus(&impl->effectNode, 0); result != MA_SUCCESS)
            return fail("detach effect node output from endpoint", result);
    }

    // Attach the sound output to the custom effect node or the engine endpoint
    if (const ma_result result = ma_node_attach_output_bus(&impl->sound,
                                                           0,
                                                           connect ? &impl->effectNode : ma_engine_get_endpoint(engine),
                                                           0);
        result != MA_SUCCESS)
        return fail("attach sound node output to effect node", result);

    return true;
}


////////////////////////////////////////////////////////////
ma_sound& MiniaudioUtils::SoundBase::getSound()
{
    return impl->sound;
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::clearSoundChannelMap()
{
    impl->soundChannelMap.clear();
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::addToSoundChannelMap(const base::U8 maChannel)
{
    impl->soundChannelMap.pushBack(maChannel);
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::refreshSoundChannelMap()
{
    impl->sound.engineNode.spatializer.pChannelMapIn = impl->soundChannelMap.data();
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::setAndConnectEffectProcessor(const EffectProcessor& effectProcessor)
{
    impl->effectProcessor = effectProcessor;
    connectEffect(bool{impl->effectProcessor});
}


////////////////////////////////////////////////////////////
base::U8 MiniaudioUtils::soundChannelToMiniaudioChannel(const SoundChannel soundChannel)
{
    switch (soundChannel)
    {
            // clang-format off
        case SoundChannel::Unspecified:         return base::U8{MA_CHANNEL_NONE};
        case SoundChannel::Mono:                return base::U8{MA_CHANNEL_MONO};
        case SoundChannel::FrontLeft:           return base::U8{MA_CHANNEL_FRONT_LEFT};
        case SoundChannel::FrontRight:          return base::U8{MA_CHANNEL_FRONT_RIGHT};
        case SoundChannel::FrontCenter:         return base::U8{MA_CHANNEL_FRONT_CENTER};
        case SoundChannel::FrontLeftOfCenter:   return base::U8{MA_CHANNEL_FRONT_LEFT_CENTER};
        case SoundChannel::FrontRightOfCenter:  return base::U8{MA_CHANNEL_FRONT_RIGHT_CENTER};
        case SoundChannel::LowFrequencyEffects: return base::U8{MA_CHANNEL_LFE};
        case SoundChannel::BackLeft:            return base::U8{MA_CHANNEL_BACK_LEFT};
        case SoundChannel::BackRight:           return base::U8{MA_CHANNEL_BACK_RIGHT};
        case SoundChannel::BackCenter:          return base::U8{MA_CHANNEL_BACK_CENTER};
        case SoundChannel::SideLeft:            return base::U8{MA_CHANNEL_SIDE_LEFT};
        case SoundChannel::SideRight:           return base::U8{MA_CHANNEL_SIDE_RIGHT};
        case SoundChannel::TopCenter:           return base::U8{MA_CHANNEL_TOP_CENTER};
        case SoundChannel::TopFrontLeft:        return base::U8{MA_CHANNEL_TOP_FRONT_LEFT};
        case SoundChannel::TopFrontRight:       return base::U8{MA_CHANNEL_TOP_FRONT_RIGHT};
        case SoundChannel::TopFrontCenter:      return base::U8{MA_CHANNEL_TOP_FRONT_CENTER};
        case SoundChannel::TopBackLeft:         return base::U8{MA_CHANNEL_TOP_BACK_LEFT};
        case SoundChannel::TopBackRight:        return base::U8{MA_CHANNEL_TOP_BACK_RIGHT};
            // clang-format on

        default:
            SFML_BASE_ASSERT(soundChannel == SoundChannel::TopBackCenter);
            return base::U8{MA_CHANNEL_TOP_BACK_CENTER};
    }
}


////////////////////////////////////////////////////////////
SoundChannel MiniaudioUtils::miniaudioChannelToSoundChannel(const base::U8 soundChannel)
{
    const ma_channel maChannel{soundChannel};

    switch (maChannel)
    {
            // clang-format off
        case MA_CHANNEL_NONE:               return SoundChannel::Unspecified;
        case MA_CHANNEL_MONO:               return SoundChannel::Mono;
        case MA_CHANNEL_FRONT_LEFT:         return SoundChannel::FrontLeft;
        case MA_CHANNEL_FRONT_RIGHT:        return SoundChannel::FrontRight;
        case MA_CHANNEL_FRONT_CENTER:       return SoundChannel::FrontCenter;
        case MA_CHANNEL_FRONT_LEFT_CENTER:  return SoundChannel::FrontLeftOfCenter;
        case MA_CHANNEL_FRONT_RIGHT_CENTER: return SoundChannel::FrontRightOfCenter;
        case MA_CHANNEL_LFE:                return SoundChannel::LowFrequencyEffects;
        case MA_CHANNEL_BACK_LEFT:          return SoundChannel::BackLeft;
        case MA_CHANNEL_BACK_RIGHT:         return SoundChannel::BackRight;
        case MA_CHANNEL_BACK_CENTER:        return SoundChannel::BackCenter;
        case MA_CHANNEL_SIDE_LEFT:          return SoundChannel::SideLeft;
        case MA_CHANNEL_SIDE_RIGHT:         return SoundChannel::SideRight;
        case MA_CHANNEL_TOP_CENTER:         return SoundChannel::TopCenter;
        case MA_CHANNEL_TOP_FRONT_LEFT:     return SoundChannel::TopFrontLeft;
        case MA_CHANNEL_TOP_FRONT_RIGHT:    return SoundChannel::TopFrontRight;
        case MA_CHANNEL_TOP_FRONT_CENTER:   return SoundChannel::TopFrontCenter;
        case MA_CHANNEL_TOP_BACK_LEFT:      return SoundChannel::TopBackLeft;
        case MA_CHANNEL_TOP_BACK_RIGHT:     return SoundChannel::TopBackRight;
            // clang-format on

        default:
            SFML_BASE_ASSERT(maChannel == MA_CHANNEL_TOP_BACK_CENTER);
            return SoundChannel::TopBackCenter;
    }
}


////////////////////////////////////////////////////////////
Time MiniaudioUtils::getPlayingOffset(ma_sound& sound)
{
    float cursor = 0.f;

    if (const ma_result result = ma_sound_get_cursor_in_seconds(&sound, &cursor); result != MA_SUCCESS)
    {
        fail("get sound cursor", result);
        return Time{};
    }

    return seconds(cursor);
}


////////////////////////////////////////////////////////////
base::U64 MiniaudioUtils::getFrameIndex(ma_sound& sound, const Time timeOffset)
{
    ma_uint32 sampleRate{};

    if (const ma_result result = ma_sound_get_data_format(&sound, nullptr, nullptr, &sampleRate, nullptr, 0);
        result != MA_SUCCESS)
        fail("get sound data format", result);

    const auto frameIndex = static_cast<base::U64>(timeOffset.asSeconds() * static_cast<float>(sampleRate));

    if (const ma_result result = ma_sound_seek_to_pcm_frame(&sound, frameIndex); result != MA_SUCCESS)
        fail("seek sound to pcm frame", result);

    return frameIndex;
}


////////////////////////////////////////////////////////////
bool MiniaudioUtils::fail(const char* const what, const int maResult)
{
    err() << "Failed to " << what << ": " << ma_result_description(static_cast<ma_result>(maResult));
    return false;
}

} // namespace sf::priv
