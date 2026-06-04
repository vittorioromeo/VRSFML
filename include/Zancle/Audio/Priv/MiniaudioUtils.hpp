#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


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
[[nodiscard]] base::U8                  soundChannelToMiniaudioChannel(SoundChannel soundChannel);
[[nodiscard]] SoundChannel              miniaudioChannelToSoundChannel(base::U8 soundChannel);
[[nodiscard]] base::Optional<Time>      getPlayingOffset(ma_sound& sound);
[[nodiscard]] base::Optional<base::U64> getFrameIndex(ma_sound& sound, Time timeOffset);
[[gnu::cold]] bool                      fail(const char* what, int maResult);

} // namespace za::priv::MiniaudioUtils
