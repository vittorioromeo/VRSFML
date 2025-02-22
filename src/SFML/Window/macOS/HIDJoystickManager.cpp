#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/macOS/HIDInputManager.hpp"
#include "SFML/Window/macOS/HIDJoystickManager.hpp"

#include <array>

////////////////////////////////////////////////////////////
// Private data
////////////////////////////////////////////////////////////
namespace
{
// Using a custom run loop mode solve some issues that appears when SFML
// is used with Cocoa.
const CFStringRef runLoopMode = CFSTR("SFML_RUN_LOOP_MODE");
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
HIDJoystickManager& HIDJoystickManager::getInstance()
{
    static HIDJoystickManager manager;
    return manager;
}


////////////////////////////////////////////////////////////
unsigned int HIDJoystickManager::getJoystickCount()
{
    update();
    return m_joystickCount;
}


////////////////////////////////////////////////////////////
CFSetRef HIDJoystickManager::copyJoysticks()
{
    CFSetRef devices = IOHIDManagerCopyDevices(m_manager);
    return devices;
}


////////////////////////////////////////////////////////////
HIDJoystickManager::HIDJoystickManager()
{
    m_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    CFDictionaryRef mask0 = HIDInputManager::copyDevicesMask(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);

    CFDictionaryRef mask1 = HIDInputManager::copyDevicesMask(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);

    std::array maskArray = {mask0, mask1};
    CFArrayRef mask = CFArrayCreate(nullptr, reinterpret_cast<const void**>(maskArray.data()), maskArray.size(), nullptr);

    IOHIDManagerSetDeviceMatchingMultiple(m_manager, mask);
    CFRelease(mask);
    CFRelease(mask1);
    CFRelease(mask0);


    IOHIDManagerRegisterDeviceMatchingCallback(m_manager, pluggedIn, this);
    IOHIDManagerRegisterDeviceRemovalCallback(m_manager, pluggedOut, this);

    IOHIDManagerScheduleWithRunLoop(m_manager, CFRunLoopGetCurrent(), runLoopMode);

    IOHIDManagerOpen(m_manager, kIOHIDOptionsTypeNone);
}


////////////////////////////////////////////////////////////
HIDJoystickManager::~HIDJoystickManager()
{
    IOHIDManagerUnscheduleFromRunLoop(m_manager, CFRunLoopGetCurrent(), runLoopMode);

    IOHIDManagerRegisterDeviceMatchingCallback(m_manager, nullptr, nullptr);
    IOHIDManagerRegisterDeviceRemovalCallback(m_manager, nullptr, nullptr);

    IOHIDManagerClose(m_manager, kIOHIDOptionsTypeNone);
}


////////////////////////////////////////////////////////////
void HIDJoystickManager::update()
{
    SInt32 status = kCFRunLoopRunHandledSource;

    while (status == kCFRunLoopRunHandledSource)
    {
        status = CFRunLoopRunInMode(runLoopMode, 0, true);
    }
}


////////////////////////////////////////////////////////////
void HIDJoystickManager::pluggedIn(void* context, IOReturn, void*, IOHIDDeviceRef)
{
    auto* manager = static_cast<HIDJoystickManager*>(context);
    ++manager->m_joystickCount;
}


////////////////////////////////////////////////////////////
void HIDJoystickManager::pluggedOut(void* context, IOReturn, void*, IOHIDDeviceRef)
{
    auto* manager = static_cast<HIDJoystickManager*>(context);
    --manager->m_joystickCount;
}


} // namespace sf::priv
