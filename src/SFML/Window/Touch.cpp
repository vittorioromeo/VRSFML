// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Touch.hpp"

#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/WindowContext.hpp"

#include "SFML/Base/Span.hpp"
#include "SFML/Base/Vector.hpp"

#include <SDL3/SDL_touch.h>


namespace sf
{
////////////////////////////////////////////////////////////
base::Span<Touch::Device> Touch::getDevices()
{
    static thread_local base::Vector<Device> devices;
    devices.clear();

    {
        auto& sdlLayer = WindowContext::getSDLLayer();

        priv::SDLAllocatedArray<SDL_TouchID> touchDeviceIds = sdlLayer.getTouchDevices();
        if (!touchDeviceIds.valid()) // invalid device array
            return {};

        for (const auto touchDeviceId : touchDeviceIds)
        {
            if (touchDeviceId == 0) // invalid id
                continue;

            const SDL_TouchDeviceType type = sdlLayer.getTouchDeviceType(touchDeviceId);
            if (type == SDL_TOUCH_DEVICE_INVALID) // invalid device type
                continue;

            const auto* const name = sdlLayer.getTouchDeviceName(touchDeviceId);
            if (name == nullptr) // invalid device name
                continue;

            devices.pushBack(Device{
                .id   = touchDeviceId,
                .type = static_cast<DeviceType>(type),
                .name = name,
            });
        }
    }

    return {devices.data(), devices.size()};
}

} // namespace sf
