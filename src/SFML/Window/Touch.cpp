#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/Touch.hpp"

#include "SFML/Base/Vector.hpp"


namespace sf::Touch
{
////////////////////////////////////////////////////////////
bool isDown(unsigned int finger)
{
    // TODO P0: implement
    return false;
}


////////////////////////////////////////////////////////////
Vector2i getPosition(unsigned int finger)
{
    // TODO P0: implement
    return {};
}


////////////////////////////////////////////////////////////
Vector2i getPosition(unsigned int finger, const WindowBase& relativeTo)
{
    // TODO P0: implement
    return {};
}


////////////////////////////////////////////////////////////
base::Span<Device> getDevices()
{
    static thread_local base::Vector<Device> devices;
    devices.clear();

    {
        auto& sdlLayer = priv::getSDLLayerSingleton();

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

} // namespace sf::Touch
