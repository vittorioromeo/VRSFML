#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/System/LifetimeDependant.hpp"

#include "SFML/Base/InPlaceVector.hpp"
#include "SFML/Base/IntTypes.hpp"

#include <miniaudio.h>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class ChannelMap;
class EffectProcessor;
} // namespace sf


namespace sf::priv::MiniaudioUtils
{
////////////////////////////////////////////////////////////
struct SoundBase
{
    ////////////////////////////////////////////////////////////
    explicit SoundBase(PlaybackDevice& thePlaybackDevice, const void* dataSourceVTable, const ChannelMap& channelMap);

    ////////////////////////////////////////////////////////////
    ~SoundBase();

    ////////////////////////////////////////////////////////////
    SoundBase(const SoundBase&) = delete;
    SoundBase(SoundBase&&)      = delete;

    ////////////////////////////////////////////////////////////
    SoundBase& operator=(const SoundBase&) = delete;
    SoundBase& operator=(SoundBase&&)      = delete;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool initialize(ma_sound_end_proc endCallback);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool connectEffect(bool connect);

    ////////////////////////////////////////////////////////////
    ma_sound& getSound();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setAndConnectEffectProcessor(const EffectProcessor& effectProcessor);

    ////////////////////////////////////////////////////////////
    void applyAudioSettings(const AudioSettings& audioSettings);

    ////////////////////////////////////////////////////////////
    static void nodeOnProcess(ma_node*      node,
                              const float** framesIn,
                              ma_uint32*    frameCountIn,
                              float**       framesOut,
                              ma_uint32*    frameCountOut);

    ////////////////////////////////////////////////////////////
    void processEffect(const float** framesIn, base::U32& frameCountIn, float** framesOut, base::U32& frameCountOut) const;

    ////////////////////////////////////////////////////////////
    void setChannelMap(const ChannelMap& channelMap);

    ////////////////////////////////////////////////////////////
    static inline constexpr ma_node_vtable effectNodeVTable{
        .onProcess                    = &nodeOnProcess,
        .onGetRequiredInputFrameCount = nullptr,
        .inputBusCount                = 1,
        .outputBusCount               = 1,
        .flags                        = MA_NODE_FLAG_CONTINUOUS_PROCESSING | MA_NODE_FLAG_ALLOW_NULL_INPUT,
    };

    ////////////////////////////////////////////////////////////
    struct EffectNode
    {
        ma_node_base base{}; // must be first member
        ma_uint32    channelCount{};
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ma_data_source_base dataSourceBase{}; //!< The struct that makes this object a miniaudio data source (must be first member)

    PlaybackDevice* playbackDevice;

    EffectNode effectNode; //!< The engine node that performs effect processing

    base::InPlaceVector<ma_channel, MA_CHANNEL_POSITION_COUNT> soundChannelMap; //!< The map of position in sample frame to sound channel

    ma_sound        sound{};         //!< The sound
    EffectProcessor effectProcessor; //!< The effect processor

    [[maybe_unused]] bool effectNodeUninitialized{}; //!< Failsafe debug boolean to check if `onProcess` is called after destruction

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(PlaybackDevice);
};

} // namespace sf::priv::MiniaudioUtils
