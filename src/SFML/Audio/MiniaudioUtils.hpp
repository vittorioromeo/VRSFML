#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

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
