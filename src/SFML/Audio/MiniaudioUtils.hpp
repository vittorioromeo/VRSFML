#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundChannel.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct ma_sound;
using ma_sound_end_proc = void (*)(void*, ma_sound*);


namespace sf
{
class Time;
} // namespace sf


namespace sf::priv::MiniaudioUtils
{
////////////////////////////////////////////////////////////
[[nodiscard]] base::U8                  soundChannelToMiniaudioChannel(SoundChannel soundChannel);
[[nodiscard]] SoundChannel              miniaudioChannelToSoundChannel(base::U8 soundChannel);
[[nodiscard]] base::Optional<Time>      getPlayingOffset(ma_sound& sound);
[[nodiscard]] base::Optional<base::U64> getFrameIndex(ma_sound& sound, Time timeOffset);
[[gnu::cold]] bool                      fail(const char* what, int maResult);

} // namespace sf::priv::MiniaudioUtils
