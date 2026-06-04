#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/AudioDeviceHandle.hpp"
#include "Zancle/Audio/Export.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class PlaybackDevice;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Lightweight handle identifying an audio playback device
///
/// `PlaybackDeviceHandle` is a small, copyable value that
/// uniquely identifies a playback device (speakers, headphones,
/// virtual sink, etc.) on the system. It exposes the device's
/// name and "is default" flag, but does not own or interact with
/// the device itself.
///
/// Handles are obtained from `za::AudioContext`:
/// \code
/// auto audioContext = za::AudioContext::create().value();
/// const auto handles = za::AudioContext::getAvailablePlaybackDeviceHandles();
/// const auto defaultHandle = za::AudioContext::getDefaultPlaybackDeviceHandle().value();
/// \endcode
///
/// They are then passed to `za::PlaybackDevice` to actually open
/// and use the device:
/// \code
/// za::PlaybackDevice playbackDevice{defaultHandle};
/// \endcode
///
/// The strong typing prevents accidentally mixing playback and
/// capture device handles.
///
/// \see `za::AudioContext`, `za::PlaybackDevice`, `za::CaptureDeviceHandle`
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API PlaybackDeviceHandle : public priv::StronglyTypedDeviceHandle<PlaybackDevice>
{
    using priv::StronglyTypedDeviceHandle<PlaybackDevice>::StronglyTypedDeviceHandle;
};

} // namespace za
