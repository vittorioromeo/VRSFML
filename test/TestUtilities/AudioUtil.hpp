#pragma once

#ifdef ZA_RUN_AUDIO_DEVICE_TESTS
inline constexpr bool skipAudioDeviceTests = false;
#else
inline constexpr bool skipAudioDeviceTests = true;
#endif
