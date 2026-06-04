#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/SoundChannel.hpp"

#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct ma_sound;
using ma_sound_end_proc = void (*)(void*, ma_sound*);


namespace za
{
class Time;
} // namespace za


namespace za::priv::MiniaudioUtils
{
////////////////////////////////////////////////////////////
[[nodiscard]] zb::U8                soundChannelToMiniaudioChannel(SoundChannel soundChannel);
[[nodiscard]] SoundChannel          miniaudioChannelToSoundChannel(zb::U8 soundChannel);
[[nodiscard]] zb::Optional<Time>    getPlayingOffset(ma_sound& sound);
[[nodiscard]] zb::Optional<zb::U64> getFrameIndex(ma_sound& sound, Time timeOffset);
[[gnu::cold]] bool                  fail(const char* what, int maResult);

} // namespace za::priv::MiniaudioUtils
