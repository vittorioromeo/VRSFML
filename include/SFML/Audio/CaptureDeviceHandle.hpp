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
class SFML_AUDIO_API CaptureDeviceHandle : public priv::StronglyTypedDeviceHandle<CaptureDevice>
{
    using priv::StronglyTypedDeviceHandle<CaptureDevice>::StronglyTypedDeviceHandle;
};

} // namespace sf
