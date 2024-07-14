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
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundChannel.hpp>

#include <SFML/System/LifetimeDependant.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <cfloat>
#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct ma_sound;
using ma_sound_end_proc = void (*)(void*, ma_sound*);

namespace sf
{
class EffectProcessor;
class Time;
} // namespace sf


namespace sf::priv::MiniaudioUtils
{
struct SoundBase
{
    explicit SoundBase(PlaybackDevice&                         thePlaybackDevice,
                       const void*                             dataSourceVTable,
                       PlaybackDevice::ResourceEntry::InitFunc reinitializeFunc);

    ~SoundBase();

    [[nodiscard]] bool initialize(ma_sound_end_proc endCallback);
    void               deinitialize();

    void processEffect(const float** framesIn, std::uint32_t& frameCountIn, float** framesOut, std::uint32_t& frameCountOut) const;
    void connectEffect(bool connect);

    ma_sound& getSound();

    void clearSoundChannelMap();
    void addToSoundChannelMap(std::uint8_t maChannel);
    void refreshSoundChannelMap();

    void setAndConnectEffectProcessor(EffectProcessor effectProcessor);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 2048> impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(PlaybackDevice);
};

[[nodiscard]] std::uint8_t  soundChannelToMiniaudioChannel(SoundChannel soundChannel);
[[nodiscard]] SoundChannel  miniaudioChannelToSoundChannel(std::uint8_t soundChannel);
[[nodiscard]] Time          getPlayingOffset(ma_sound& sound);
[[nodiscard]] std::uint64_t getFrameIndex(ma_sound& sound, Time timeOffset);
[[gnu::cold]] bool          fail(const char* what, int maResult);

} // namespace sf::priv::MiniaudioUtils
