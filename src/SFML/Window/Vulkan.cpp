// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Vulkan.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Assert.hpp"

#define VK_NO_PROTOTYPES
#include <vulkan.h>

// Must be included after Vulkan
#include <SDL3/SDL_vulkan.h>


namespace sf::Vulkan
{
////////////////////////////////////////////////////////////
bool isAvailable([[maybe_unused]] bool requireGraphics)
{
    static bool checked           = false;
    static bool computeAvailable  = false;
    static bool graphicsAvailable = false;

    if (!checked)
    {
        checked          = true;
        computeAvailable = SDL_Vulkan_LoadLibrary(nullptr);

        if (!computeAvailable)
        {
            priv::err() << "Failed to load Vulkan library: " << SDL_GetError();
            return false;
        }

        graphicsAvailable = true;

        Uint32             count      = 0;
        const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);

        if (extensions == nullptr)
        {
            priv::err() << "Failed to get Vulkan extensions: " << SDL_GetError();
            graphicsAvailable = false;
        }

        if (count == 0)
        {
            priv::err() << "No Vulkan extensions available";
            graphicsAvailable = false;
        }
    }

    return requireGraphics ? graphicsAvailable : computeAvailable;
}


////////////////////////////////////////////////////////////
VulkanFunctionPointer getFunction(const char* name, const VkInstance instance)
{
    SFML_BASE_ASSERT(name != nullptr && "Name cannot be a null pointer");

    if (!isAvailable(/* requireGraphics */ false))
    {
        priv::err() << "Tried to get Vulkan function pointer when Vulkan is not available";
        return nullptr;
    }

    // Retrieve the global vkGetInstanceProcAddr function from SDL
    auto vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());

    if (vkGetInstanceProcAddr == nullptr)
    {
        priv::err() << "Failed to get Vulkan function pointer: " << SDL_GetError();
        return nullptr;
    }

    return reinterpret_cast<VulkanFunctionPointer>(vkGetInstanceProcAddr(instance, name));
}


////////////////////////////////////////////////////////////
base::Span<const char* const> getGraphicsRequiredInstanceExtensions()
{
    if (!isAvailable(/* requireGraphics */ true))
    {
        priv::err() << "Tried to get graphics required instance extensions when Vulkan is not available";
        return {};
    }

    Uint32             count      = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    if (extensions == nullptr)
    {
        priv::err() << "Failed to get Vulkan extensions: " << SDL_GetError();
        return {};
    }

    return {extensions, static_cast<base::SizeT>(count)};
}

////////////////////////////////////////////////////////////
bool createVulkanSurface(const VkInstance&            instance,
                         void*                        sdlWindowHandle,
                         VkSurfaceKHR&                surface,
                         const VkAllocationCallbacks* allocator)
{
    SFML_BASE_ASSERT(sdlWindowHandle != nullptr && "SDL window handle cannot be a null pointer");

    if (!isAvailable(true))
    {
        priv::err() << "Tried to create Vulkan surface when Vulkan is not available";
        return false;
    }

    if (!SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(sdlWindowHandle), instance, allocator, &surface))
    {
        priv::err() << "Failed to create Vulkan surface: " << SDL_GetError();
        return false;
    }

    return true;
}


} // namespace sf::Vulkan
