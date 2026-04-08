#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/Audio/AudioDeviceHandle.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CaptureDevice;
} // namespace sf


namespace sf
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
/// Handles are obtained from `sf::AudioContext`:
/// \code
/// auto audioContext = sf::AudioContext::create().value();
/// const auto handles = sf::AudioContext::getAvailableCaptureDeviceHandles();
/// const auto defaultHandle = sf::AudioContext::getDefaultCaptureDeviceHandle().value();
/// \endcode
///
/// They are then passed to `sf::CaptureDevice` to actually open
/// and use the device for recording:
/// \code
/// sf::CaptureDevice captureDevice{defaultHandle};
/// \endcode
///
/// The strong typing prevents accidentally mixing playback and
/// capture device handles.
///
/// \see `sf::AudioContext`, `sf::CaptureDevice`, `sf::PlaybackDeviceHandle`
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API CaptureDeviceHandle : public priv::StronglyTypedDeviceHandle<CaptureDevice>
{
    using priv::StronglyTypedDeviceHandle<CaptureDevice>::StronglyTypedDeviceHandle;
};

} // namespace sf
