#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Cursor.hpp"
#include "Zancle/Window/DisplayOrientation.hpp"
#include "Zancle/Window/Keyboard.hpp"
#include "Zancle/Window/Mouse.hpp"
#include "Zancle/Window/WindowHandle.hpp"

#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Rect2.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Trait/IsSame.hpp"
#include "ZancleBase/TrivialAbi.hpp"
#include "ZancleBase/UniquePtr.hpp"

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_touch.h>
#include <SDL3/SDL_video.h>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct SDL_Joystick;
struct SDL_GUID;

namespace za
{
class Utf8String;
struct ContextSettings;
struct VideoMode;
struct WindowSettings;
} // namespace za


namespace za::priv
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Keyboard::Scan mapSDLScancodeToSFML(SDL_Scancode sdlCode) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] SDL_Scancode mapSFMLScancodeToSDL(Keyboard::Scan scanCode) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Keyboard::Key mapSDLKeycodeToSFML(SDL_Keycode sdlKey) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] SDL_Keycode mapSFMLKeycodeToSDL(Keyboard::Key key) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] SDL_SystemCursor cursorTypeToSDLCursor(za::Cursor::Type type) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard]] SDL_PropertiesID makeSDLWindowPropertiesFromHandle(const char* currentVideoDriver, WindowHandle handle);

////////////////////////////////////////////////////////////
[[nodiscard]] SDL_PropertiesID makeSDLWindowPropertiesFromWindowSettings(const WindowSettings& windowSettings);

////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure]] SDL_WindowFlags makeSDLWindowFlagsFromWindowSettings(const WindowSettings& windowSettings) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Mouse::Button getButtonFromSDLButton(zb::U8 sdlButton) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] zb::U8 getSDLButtonFromSFMLButton(Mouse::Button button) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] DisplayOrientation mapSDLDisplayOrientationToSFML(SDL_DisplayOrientation displayOrientation);

////////////////////////////////////////////////////////////
struct ZB_TRIVIAL_ABI UniquePtrSDLDeleter
{
    template <typename T>
    [[gnu::always_inline]] void operator()(T* const ptr) const noexcept
    {
        static_assert(!ZB_IS_SAME(T, void), "can't delete pointer to incomplete type");

        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        static_assert(sizeof(T) > 0u, "can't delete pointer to incomplete type");

        SDL_free(static_cast<void*>(ptr));
    }
};

////////////////////////////////////////////////////////////
template <typename T>
using SDLUPtr = zb::UniquePtr<T, UniquePtrSDLDeleter>;

////////////////////////////////////////////////////////////
struct ZB_TRIVIAL_ABI UniquePtrSDLSurfaceDeleter
{
    [[gnu::always_inline]] void operator()(SDL_Surface* const ptr) const noexcept
    {
        SDL_DestroySurface(ptr);
    }
};

////////////////////////////////////////////////////////////
using SDLSurfaceUPtr = zb::UniquePtr<SDL_Surface, UniquePtrSDLSurfaceDeleter>;

////////////////////////////////////////////////////////////
template <typename T>
struct SDLAllocatedArray
{
    ////////////////////////////////////////////////////////////
    SDLUPtr<T> ptr;
    zb::SizeT  count;


    ////////////////////////////////////////////////////////////
    explicit SDLAllocatedArray(SDLUPtr<T>&& thePtr, const zb::SizeT theCount) : ptr{ZB_MOVE(thePtr)}, count{theCount}
    {
    }


    ////////////////////////////////////////////////////////////
    SDLAllocatedArray(decltype(nullptr)) : ptr{nullptr}, count{0u}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T* get() noexcept
    {
        return ptr.get();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T* get() const noexcept
    {
        return ptr.get();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T& operator[](const zb::SizeT index) noexcept
    {
        ZB_ASSERT(ptr != nullptr);
        ZB_ASSERT(index < count);

        return ptr.get()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T& operator[](const zb::SizeT index) const noexcept
    {
        ZB_ASSERT(ptr != nullptr);
        ZB_ASSERT(index < count);

        return ptr.get()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool valid() const noexcept
    {
        return ptr != nullptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::SizeT size() const noexcept
    {
        return count;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool empty() const noexcept
    {
        return count == 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T* begin() noexcept
    {
        return ptr.get();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] T* end() noexcept
    {
        return ptr.get() + count;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T* begin() const noexcept
    {
        return ptr.get();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T* end() const noexcept
    {
        return ptr.get() + count;
    }
};


////////////////////////////////////////////////////////////
class SDLLayer
{
public:
    ////////////////////////////////////////////////////////////
    explicit SDLLayer();

    ////////////////////////////////////////////////////////////
    ~SDLLayer();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_DisplayID> getDisplays() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_DisplayMode*> getFullscreenDisplayModesForDisplay(SDL_DisplayID displayId);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_PixelFormatDetails* getPixelFormatDetails(SDL_PixelFormat format) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_DisplayMode* getDesktopDisplayMode(SDL_DisplayID displayId) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_DisplayMode* getPrimaryDisplayDesktopDisplayMode() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_TouchID> getTouchDevices();

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_Finger*> getTouchFingers(SDL_TouchID touchDeviceId);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDL_TouchDeviceType getTouchDeviceType(SDL_TouchID touchDeviceId);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getTouchDeviceName(SDL_TouchID touchDeviceId);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Keyboard::Key localizeScancode(Keyboard::Scancode code) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Keyboard::Scancode delocalizeScancode(Keyboard::Key key) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isKeyPressedByScancode(Keyboard::Scancode code) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getScancodeDescription(Keyboard::Scancode code) const noexcept;

    ////////////////////////////////////////////////////////////
    void setVirtualKeyboardVisible(bool visible) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2i getGlobalMousePosition() const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setGlobalMousePosition(Vec2i position) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Utf8String getClipboardString() const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setClipboardString(const Utf8String& string) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDisplayContentScale(SDL_DisplayID displayID) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Utf8String getDisplayName(SDL_DisplayID displayID) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Rect2i getDisplayBounds(SDL_DisplayID displayID) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Rect2i getDisplayUsableBounds(SDL_DisplayID displayID) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] DisplayOrientation getNaturalDisplayOrientation(SDL_DisplayID displayID) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] DisplayOrientation getCurrentDisplayOrientation(SDL_DisplayID displayID) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getPrimaryDisplayContentScale() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDisplayScale(SDL_Window& window) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLSurfaceUPtr createSurfaceFromPixels(const zb::U8* pixels, Vec2u size) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getJoystickButtonCount(SDL_Joystick& handle);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getJoystickAxisCount(SDL_Joystick& handle);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getJoystickHatCount(SDL_Joystick& handle);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getJoystickName(SDL_Joystick& handle);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getJoystickVendor(SDL_Joystick& handle);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getJoystickProduct(SDL_Joystick& handle);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool areGUIDsEqual(const SDL_GUID& a, const SDL_GUID& b);

    ////////////////////////////////////////////////////////////
    void setWindowSize(SDL_Window& window, Vec2u size) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2u getWindowSize(SDL_Window& window) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] VideoMode getVideoModeFromSDLDisplayMode(const SDL_DisplayMode& mode) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setGLAttribute(int attribute, int value) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool applyGLContextSettings(const ContextSettings& settings) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const char* getCurrentVideoDriver() const;
};

} // namespace za::priv
