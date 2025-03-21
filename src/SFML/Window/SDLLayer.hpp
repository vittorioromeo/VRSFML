#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/UniquePtr.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>


namespace sf::priv
{
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
    explicit SDLLayer()
    {
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            err() << "`SDL_Init` failed: " << SDL_GetError();
            sf::base::abort();
        }
    }


    ////////////////////////////////////////////////////////////
    ~SDLLayer()
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_DisplayID> getDisplays()
    {
        int            displayCount = 0;
        SDL_DisplayID* displays     = SDL_GetDisplays(&displayCount);

        if (displays == nullptr)
        {
            err() << "`SDL_GetDisplays` failed: " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_DisplayID>{SDLUPtr<SDL_DisplayID>{displays}, static_cast<base::SizeT>(displayCount)};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_DisplayMode*> getFullscreenDisplayModesForDisplay(const SDL_DisplayID displayId)
    {
        int               modeCount = 0;
        SDL_DisplayMode** modes     = SDL_GetFullscreenDisplayModes(displayId, &modeCount);

        if (modes == nullptr)
        {
            err() << "`SDL_GetFullscreenDisplayModes` failed for display " << displayId << ": " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_DisplayMode*>{SDLUPtr<SDL_DisplayMode*>(modes), static_cast<base::SizeT>(modeCount)};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_PixelFormatDetails* getPixelFormatDetails(const SDL_PixelFormat format)
    {
        const auto* result = SDL_GetPixelFormatDetails(format);

        if (result == nullptr)
        {
            err() << "`SDL_GetPixelFormatDetails` failed for format " << static_cast<int>(format) << ": "
                  << SDL_GetError();
            return nullptr;
        }

        return result;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] const SDL_DisplayMode* getDesktopDisplayMode(const SDL_DisplayID displayId)
    {
        const auto* result = SDL_GetDesktopDisplayMode(displayId);

        if (result == nullptr)
        {
            err() << "`SDL_GetDesktopDisplayMode` failed for display " << displayId << ": " << SDL_GetError();
            return nullptr;
        }

        return result;
    }
};


////////////////////////////////////////////////////////////
[[nodiscard]] inline SDLLayer& getSDLLayerSingleton()
{
    static SDLLayer sdlLayer;
    return sdlLayer;
}

} // namespace sf::priv
