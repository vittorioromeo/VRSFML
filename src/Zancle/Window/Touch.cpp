// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Touch.hpp"

#include "Zancle/Window/SDLLayer.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "ZancleBase/Span.hpp"
#include "ZancleBase/Vector.hpp"

#include <SDL3/SDL_touch.h>


namespace za
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

} // namespace za
