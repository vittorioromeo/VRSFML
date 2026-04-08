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
class PlaybackDevice;
} // namespace sf


namespace sf
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
/// Handles are obtained from `sf::AudioContext`:
/// \code
/// auto audioContext = sf::AudioContext::create().value();
/// const auto handles = sf::AudioContext::getAvailablePlaybackDeviceHandles();
/// const auto defaultHandle = sf::AudioContext::getDefaultPlaybackDeviceHandle().value();
/// \endcode
///
/// They are then passed to `sf::PlaybackDevice` to actually open
/// and use the device:
/// \code
/// sf::PlaybackDevice playbackDevice{defaultHandle};
/// \endcode
///
/// The strong typing prevents accidentally mixing playback and
/// capture device handles.
///
/// \see `sf::AudioContext`, `sf::PlaybackDevice`, `sf::CaptureDeviceHandle`
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API PlaybackDeviceHandle : public priv::StronglyTypedDeviceHandle<PlaybackDevice>
{
    using priv::StronglyTypedDeviceHandle<PlaybackDevice>::StronglyTypedDeviceHandle;
};

} // namespace sf
