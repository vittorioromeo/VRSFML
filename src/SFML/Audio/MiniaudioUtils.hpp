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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/EffectProcessor.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundChannel.hpp>

#include <SFML/System/LifetimeDependant.hpp>

#include <miniaudio.h>

#include <vector>

#include <cfloat>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Time;
} // namespace sf


namespace sf::priv::MiniaudioUtils
{
struct SavedSettings
{
    float          pitch{1.f};
    float          pan{0.f};
    float          volume{1.f};
    ma_bool32      spatializationEnabled{MA_TRUE};
    ma_vec3f       position{0.f, 0.f, 0.f};
    ma_vec3f       direction{0.f, 0.f, -1.f};
    float          directionalAttenuationFactor{1.f};
    ma_vec3f       velocity{0.f, 0.f, 0.f};
    float          dopplerFactor{1.f};
    ma_positioning positioning{ma_positioning_absolute};
    float          minDistance{1.f};
    float          maxDistance{FLT_MAX};
    float          minGain{0.f};
    float          maxGain{1.f};
    float          rollOff{1.f};
    ma_bool32      playing{MA_FALSE};
    ma_bool32      looping{MA_FALSE};
    float          innerAngle{6.283185f}; // 360° in radians
    float          outerAngle{6.283185f}; // 360° in radians
    float          outerGain{0.f};
};

struct SoundBase
{
    SoundBase(PlaybackDevice&                         thePlaybackDevice,
              const ma_data_source_vtable&            dataSourceVTable,
              PlaybackDevice::ResourceEntry::InitFunc reinitializeFunc);

    ~SoundBase();

    [[nodiscard]] bool initialize(ma_sound_end_proc endCallback);
    void               deinitialize();

    void processEffect(const float** framesIn, ma_uint32& frameCountIn, float** framesOut, ma_uint32& frameCountOut) const;
    void connectEffect(bool connect);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
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
    MiniaudioUtils::SavedSettings savedSettings; //!< Saved settings used to restore ma_sound state in case we need to recreate it

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(PlaybackDevice);
};

[[nodiscard]] ma_channel   soundChannelToMiniaudioChannel(SoundChannel soundChannel);
[[nodiscard]] SoundChannel miniaudioChannelToSoundChannel(ma_channel soundChannel);
[[nodiscard]] Time         getPlayingOffset(ma_sound& sound);
[[nodiscard]] ma_uint64    getFrameIndex(ma_sound& sound, Time timeOffset);

} // namespace sf::priv::MiniaudioUtils
