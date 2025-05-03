#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
class SFML_AUDIO_API PlaybackDeviceHandle : public priv::StronglyTypedDeviceHandle<PlaybackDevice>
{
    using priv::StronglyTypedDeviceHandle<PlaybackDevice>::StronglyTypedDeviceHandle;
};

} // namespace sf
