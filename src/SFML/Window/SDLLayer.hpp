#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "SFML/Window/WindowHandle.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/System/Vector2.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_touch.h>
#include <SDL3/SDL_video.h>


namespace sf::priv
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
[[nodiscard, gnu::const]] SDL_SystemCursor cursorTypeToSDLCursor(sf::Cursor::Type type) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard]] SDL_PropertiesID makeSDLWindowPropertiesFromHandle(WindowHandle handle);

////////////////////////////////////////////////////////////
[[nodiscard, gnu::pure]] SDL_WindowFlags makeSDLWindowFlagsFromWindowSettings(const WindowSettings& windowSettings) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] Mouse::Button getButtonFromSDLButton(base::U8 sdlButton) noexcept;

////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] base::U8 getSDLButtonFromSFMLButton(Mouse::Button button) noexcept;

////////////////////////////////////////////////////////////
struct SFML_BASE_TRIVIAL_ABI UniquePtrSDLDeleter
{
    template <typename T>
    [[gnu::always_inline]] void operator()(T* const ptr) const noexcept
    {
        static_assert(!SFML_BASE_IS_SAME(T, void), "can't delete pointer to incomplete type");

        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        static_assert(sizeof(T) > 0u, "can't delete pointer to incomplete type");

        SDL_free(static_cast<void*>(ptr));
    }
};

////////////////////////////////////////////////////////////
template <typename T>
using SDLUPtr = base::UniquePtr<T, UniquePtrSDLDeleter>;

////////////////////////////////////////////////////////////
struct SFML_BASE_TRIVIAL_ABI UniquePtrSDLSurfaceDeleter
{
    [[gnu::always_inline]] void operator()(SDL_Surface* const ptr) const noexcept
    {
        SDL_DestroySurface(ptr);
    }
};

////////////////////////////////////////////////////////////
using SDLSurfaceUPtr = base::UniquePtr<SDL_Surface, UniquePtrSDLSurfaceDeleter>;

////////////////////////////////////////////////////////////
template <typename T>
struct SDLAllocatedArray
{
    ////////////////////////////////////////////////////////////
    SDLUPtr<T>  ptr;
    base::SizeT count;


    ////////////////////////////////////////////////////////////
    explicit SDLAllocatedArray(SDLUPtr<T>&& thePtr, const base::SizeT theCount) :
    ptr{SFML_BASE_MOVE(thePtr)},
    count{theCount}
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
    [[nodiscard]] T& operator[](const base::SizeT index) noexcept
    {
        SFML_BASE_ASSERT(ptr != nullptr);
        SFML_BASE_ASSERT(index < count);

        return ptr.get()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const T& operator[](const base::SizeT index) const noexcept
    {
        SFML_BASE_ASSERT(ptr != nullptr);
        SFML_BASE_ASSERT(index < count);

        return ptr.get()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool valid() const noexcept
    {
        return ptr != nullptr;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT size() const noexcept
    {
        return count;
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
    [[nodiscard]] const SDL_PixelFormatDetails* getPixelFormatDetails(SDL_PixelFormat format);

    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_DisplayMode* getDesktopDisplayMode(SDL_DisplayID displayId);

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
    [[nodiscard]] Vector2i getGlobalMousePosition() const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setGlobalMousePosition(Vector2i position) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] String getClipboardString() const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setClipboardString(const String& string) const noexcept;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getDisplayContentScale() const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getWindowDisplayScale(SDL_Window& window) const;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLSurfaceUPtr createSurfaceFromPixels(Vector2u size, const base::U8* pixels) const;
};

////////////////////////////////////////////////////////////
[[nodiscard]] SDLLayer& getSDLLayerSingleton();

} // namespace sf::priv
