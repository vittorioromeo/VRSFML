////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/EffectProcessor.hpp>
#include <SFML/Audio/MiniaudioUtils.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SavedSettings.hpp>
#include <SFML/Audio/SoundChannel.hpp>

#include <SFML/System/Assert.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Time.hpp>

#include <miniaudio.h>

#include <vector>

#include <cstring>


namespace sf::priv
{
struct MiniaudioUtils::SoundBase::Impl
{
    explicit Impl(PlaybackDevice& thePlaybackDevice) :
    dataSourceBase{}, // must be first member!
    playbackDevice(&thePlaybackDevice)
    {
    }

    struct EffectNode
    {
        ma_node_base base{};
        SoundBase*   impl{};
        ma_uint32    channelCount{};
    };

    ma_data_source_base dataSourceBase{}; //!< The struct that makes this object a miniaudio data source (must be first member)

    PlaybackDevice* playbackDevice;

    ma_node_vtable effectNodeVTable{};       //!< Vtable of the effect node
    EffectNode     effectNode;               //!< The engine node that performs effect processing
    std::vector<ma_channel> soundChannelMap; //!< The map of position in sample frame to sound channel (miniaudio channels)
    ma_sound        sound{};                 //!< The sound
    EffectProcessor effectProcessor;         //!< The effect processor
    PlaybackDevice::ResourceEntryIndex resourceEntryIndex; //!< Index of the resource entry registered with the PlaybackDevice
    SavedSettings savedSettings; //!< Saved settings used to restore ma_sound state in case we need to recreate it
};

////////////////////////////////////////////////////////////
MiniaudioUtils::SoundBase::SoundBase(PlaybackDevice&                         thePlaybackDevice,
                                     const void*                             dataSourceVTable,
                                     PlaybackDevice::ResourceEntry::InitFunc reinitializeFunc) :
impl(thePlaybackDevice)
{
    // Set this object up as a miniaudio data source
    ma_data_source_config config = ma_data_source_config_init();
    config.vtable                = static_cast<const ma_data_source_vtable*>(dataSourceVTable);

    if (const ma_result result = ma_data_source_init(&config, &impl->dataSourceBase); result != MA_SUCCESS)
        fail("initialize audio data source", result);

    impl->resourceEntryIndex = impl->playbackDevice->registerResource(
        this,
        [](void* ptr) { static_cast<SoundBase*>(ptr)->deinitialize(); },
        reinitializeFunc,
        [](void* ptr, PlaybackDevice& newPlaybackDevice, PlaybackDevice::ResourceEntryIndex newIndex)
        {
            static_cast<SoundBase*>(ptr)->impl->playbackDevice     = &newPlaybackDevice;
            static_cast<SoundBase*>(ptr)->impl->resourceEntryIndex = newIndex;
        });

    // TODO:
    // SFML_UPDATE_LIFETIME_DEPENDANT(PlaybackDevice, SoundBase, playbackDevice);
}


////////////////////////////////////////////////////////////
MiniaudioUtils::SoundBase::~SoundBase()
{
    impl->playbackDevice->unregisterResource(impl->resourceEntryIndex);

    ma_sound_uninit(&impl->sound);
    ma_node_uninit(&impl->effectNode, nullptr);
    ma_data_source_uninit(&impl->dataSourceBase);
}


////////////////////////////////////////////////////////////
bool MiniaudioUtils::SoundBase::initialize(ma_sound_end_proc endCallback)
{
    // Get the engine
    auto* engine = static_cast<ma_engine*>(impl->playbackDevice->getMAEngine());

    // Initialize the sound
    ma_sound_config soundConfig;

    soundConfig                      = ma_sound_config_init();
    soundConfig.pDataSource          = this;
    soundConfig.pEndCallbackUserData = this;
    soundConfig.endCallback          = endCallback;

    if (const ma_result result = ma_sound_init_ex(engine, &soundConfig, &impl->sound); result != MA_SUCCESS)
        return fail("initialize sound", result);

    // Initialize the custom effect node
    impl->effectNodeVTable.onProcess =
        [](ma_node* node, const float** framesIn, ma_uint32* frameCountIn, float** framesOut, ma_uint32* frameCountOut)
    { static_cast<Impl::EffectNode*>(node)->impl->processEffect(framesIn, *frameCountIn, framesOut, *frameCountOut); };
    impl->effectNodeVTable.onGetRequiredInputFrameCount = nullptr;
    impl->effectNodeVTable.inputBusCount                = 1;
    impl->effectNodeVTable.outputBusCount               = 1;
    impl->effectNodeVTable.flags = MA_NODE_FLAG_CONTINUOUS_PROCESSING | MA_NODE_FLAG_ALLOW_NULL_INPUT;

    const auto     nodeChannelCount = ma_engine_get_channels(engine);
    ma_node_config nodeConfig       = ma_node_config_init();
    nodeConfig.vtable               = &impl->effectNodeVTable;
    nodeConfig.pInputChannels       = &nodeChannelCount;
    nodeConfig.pOutputChannels      = &nodeChannelCount;

    if (const ma_result result = ma_node_init(ma_engine_get_node_graph(engine), &nodeConfig, nullptr, &impl->effectNode);
        result != MA_SUCCESS)
        return fail("initialize effect node", result);

    impl->effectNode.impl         = this;
    impl->effectNode.channelCount = nodeChannelCount;

    // Route the sound through the effect node depending on whether an effect processor is set
    connectEffect(bool{impl->effectProcessor});

    impl->savedSettings.applyOnto(impl->sound);
    return true;
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::deinitialize()
{
    impl->savedSettings = SavedSettings{impl->sound};

    ma_sound_uninit(&impl->sound);
    ma_node_uninit(&impl->effectNode, nullptr);
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::processEffect(const float**  framesIn,
                                              std::uint32_t& frameCountIn,
                                              float**        framesOut,
                                              std::uint32_t& frameCountOut) const
{
    // If a processor is set, call it
    if (impl->effectProcessor)
    {
        if (!framesIn)
            frameCountIn = 0;

        impl->effectProcessor(framesIn ? framesIn[0] : nullptr,
                              frameCountIn,
                              framesOut[0],
                              frameCountOut,
                              impl->effectNode.channelCount);
        return;
    }

    // Otherwise just pass the data through 1:1
    if (framesIn == nullptr)
    {
        frameCountIn  = 0;
        frameCountOut = 0;
        return;
    }

    const auto toProcess = std::min(frameCountIn, frameCountOut);
    std::memcpy(framesOut[0], framesIn[0], toProcess * impl->effectNode.channelCount * sizeof(float));
    frameCountIn  = toProcess;
    frameCountOut = toProcess;
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::connectEffect(bool connect)
{
    auto* engine = static_cast<ma_engine*>(impl->playbackDevice->getMAEngine());

    if (connect)
    {
        // Attach the custom effect node output to our engine endpoint
        if (const ma_result result = ma_node_attach_output_bus(&impl->effectNode, 0, ma_engine_get_endpoint(engine), 0);
            result != MA_SUCCESS)
        {
            fail("attach effect node output to endpoint", result);
            return;
        }
    }
    else
    {
        // Detach the custom effect node output from our engine endpoint
        if (const ma_result result = ma_node_detach_output_bus(&impl->effectNode, 0); result != MA_SUCCESS)
        {
            fail("detach effect node output from endpoint", result);
            return;
        }
    }

    // Attach the sound output to the custom effect node or the engine endpoint
    if (const ma_result result = ma_node_attach_output_bus(&impl->sound,
                                                           0,
                                                           connect ? &impl->effectNode : ma_engine_get_endpoint(engine),
                                                           0);
        result != MA_SUCCESS)
    {
        fail("attach sound node output to effect node", result);
        return;
    }
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
void MiniaudioUtils::SoundBase::addToSoundChannelMap(std::uint8_t maChannel)
{
    impl->soundChannelMap.push_back(maChannel);
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::refreshSoundChannelMap()
{
    impl->sound.engineNode.spatializer.pChannelMapIn = impl->soundChannelMap.data();
}


////////////////////////////////////////////////////////////
void MiniaudioUtils::SoundBase::setAndConnectEffectProcessor(EffectProcessor effectProcessor)
{
    impl->effectProcessor = SFML_MOVE(effectProcessor);
    connectEffect(bool{impl->effectProcessor});
}


////////////////////////////////////////////////////////////
std::uint8_t MiniaudioUtils::soundChannelToMiniaudioChannel(SoundChannel soundChannel)
{
    switch (soundChannel)
    {
            // clang-format off
        case SoundChannel::Unspecified:         return std::uint8_t{MA_CHANNEL_NONE};
        case SoundChannel::Mono:                return std::uint8_t{MA_CHANNEL_MONO};
        case SoundChannel::FrontLeft:           return std::uint8_t{MA_CHANNEL_FRONT_LEFT};
        case SoundChannel::FrontRight:          return std::uint8_t{MA_CHANNEL_FRONT_RIGHT};
        case SoundChannel::FrontCenter:         return std::uint8_t{MA_CHANNEL_FRONT_CENTER};
        case SoundChannel::FrontLeftOfCenter:   return std::uint8_t{MA_CHANNEL_FRONT_LEFT_CENTER};
        case SoundChannel::FrontRightOfCenter:  return std::uint8_t{MA_CHANNEL_FRONT_RIGHT_CENTER};
        case SoundChannel::LowFrequencyEffects: return std::uint8_t{MA_CHANNEL_LFE};
        case SoundChannel::BackLeft:            return std::uint8_t{MA_CHANNEL_BACK_LEFT};
        case SoundChannel::BackRight:           return std::uint8_t{MA_CHANNEL_BACK_RIGHT};
        case SoundChannel::BackCenter:          return std::uint8_t{MA_CHANNEL_BACK_CENTER};
        case SoundChannel::SideLeft:            return std::uint8_t{MA_CHANNEL_SIDE_LEFT};
        case SoundChannel::SideRight:           return std::uint8_t{MA_CHANNEL_SIDE_RIGHT};
        case SoundChannel::TopCenter:           return std::uint8_t{MA_CHANNEL_TOP_CENTER};
        case SoundChannel::TopFrontLeft:        return std::uint8_t{MA_CHANNEL_TOP_FRONT_LEFT};
        case SoundChannel::TopFrontRight:       return std::uint8_t{MA_CHANNEL_TOP_FRONT_RIGHT};
        case SoundChannel::TopFrontCenter:      return std::uint8_t{MA_CHANNEL_TOP_FRONT_CENTER};
        case SoundChannel::TopBackLeft:         return std::uint8_t{MA_CHANNEL_TOP_BACK_LEFT};
        case SoundChannel::TopBackRight:        return std::uint8_t{MA_CHANNEL_TOP_BACK_RIGHT};
            // clang-format on

        default:
            SFML_ASSERT(soundChannel == SoundChannel::TopBackCenter);
            return std::uint8_t{MA_CHANNEL_TOP_BACK_CENTER};
    }
}


////////////////////////////////////////////////////////////
SoundChannel MiniaudioUtils::miniaudioChannelToSoundChannel(std::uint8_t soundChannel)
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
            SFML_ASSERT(maChannel == MA_CHANNEL_TOP_BACK_CENTER);
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
std::uint64_t MiniaudioUtils::getFrameIndex(ma_sound& sound, Time timeOffset)
{
    ma_uint32 sampleRate{};

    if (const ma_result result = ma_sound_get_data_format(&sound, nullptr, nullptr, &sampleRate, nullptr, 0);
        result != MA_SUCCESS)
        fail("get sound data format", result);

    const auto frameIndex = static_cast<std::uint64_t>(timeOffset.asSeconds() * static_cast<float>(sampleRate));

    if (const ma_result result = ma_sound_seek_to_pcm_frame(&sound, frameIndex); result != MA_SUCCESS)
        fail("seek sound to pcm frame", result);

    return frameIndex;
}


////////////////////////////////////////////////////////////
bool MiniaudioUtils::fail(const char* what, int maResult)
{
    err() << "Failed to " << what << ": " << ma_result_description(static_cast<ma_result>(maResult)) << errEndl;
    return false;
}

} // namespace sf::priv
