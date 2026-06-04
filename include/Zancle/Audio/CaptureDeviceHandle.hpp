#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/Export.hpp"

#include "Zancle/Audio/AudioDeviceHandle.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class CaptureDevice;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Lightweight handle identifying an audio capture device
///
/// `CaptureDeviceHandle` is a small, copyable value that
/// uniquely identifies a capture device (microphone, line-in,
/// virtual source, etc.) on the system. It exposes the device's
/// name and "is default" flag, but does not own or interact with
/// the device itself.
///
/// Handles are obtained from `za::AudioContext`:
/// \code
/// auto audioContext = za::AudioContext::create().value();
/// const auto handles = za::AudioContext::getAvailableCaptureDeviceHandles();
/// const auto defaultHandle = za::AudioContext::getDefaultCaptureDeviceHandle().value();
/// \endcode
///
/// They are then passed to `za::CaptureDevice` to actually open
/// and use the device for recording:
/// \code
/// za::CaptureDevice captureDevice{defaultHandle};
/// \endcode
///
/// The strong typing prevents accidentally mixing playback and
/// capture device handles.
///
/// \see `za::AudioContext`, `za::CaptureDevice`, `za::PlaybackDeviceHandle`
///
////////////////////////////////////////////////////////////
class ZA_AUDIO_API CaptureDeviceHandle : public priv::StronglyTypedDeviceHandle<CaptureDevice>
{
    using priv::StronglyTypedDeviceHandle<CaptureDevice>::StronglyTypedDeviceHandle;
};

} // namespace za
