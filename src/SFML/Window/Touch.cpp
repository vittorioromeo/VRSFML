#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/InputImpl.hpp"
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/Touch.hpp"

#include "SFML/Base/TrivialVector.hpp"

#include <string> // TODO P0:


namespace sf::Touch
{
////////////////////////////////////////////////////////////
bool isDown(unsigned int finger)
{
    return priv::InputImpl::isTouchDown(finger);
}


////////////////////////////////////////////////////////////
Vector2i getPosition(unsigned int finger)
{
    return priv::InputImpl::getTouchPosition(finger);
}


////////////////////////////////////////////////////////////
Vector2i getPosition(unsigned int finger, const WindowBase& relativeTo)
{
    return priv::InputImpl::getTouchPosition(finger, relativeTo);
}


////////////////////////////////////////////////////////////
base::Span<Device> getDevices()
{
    thread_local static base::TrivialVector<Device> devices;
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

// TODO P0:
const char* testsdl()
{
    thread_local std::string out;
    out.clear();

    auto& sdlLayer = priv::getSDLLayerSingleton();

    auto d = getDevices();

    for (const Device& device : d)
    {
        for (auto fp : sdlLayer.getTouchFingers(device.id))
        {
            const auto& [id, x, y, pressure] = *fp;

            out += " Finger: " + std::to_string(id) + " x: " + std::to_string(x) + " y: " + std::to_string(y) +
                   " pressure: " + std::to_string(pressure) + "\n";
        }
    }

    return out.data();
}

} // namespace sf::Touch
