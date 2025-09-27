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
class SFML_AUDIO_API PlaybackDeviceHandle : public priv::StronglyTypedDeviceHandle<PlaybackDevice>
{
    using priv::StronglyTypedDeviceHandle<PlaybackDevice>::StronglyTypedDeviceHandle;
};

} // namespace sf
