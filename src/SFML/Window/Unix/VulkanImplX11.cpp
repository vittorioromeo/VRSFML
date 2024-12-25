#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Unix/Display.hpp"
#include "SFML/Window/VulkanImpl.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/StringView.hpp"

#include <dlfcn.h>
#include <vector>

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>


namespace
{
struct VulkanLibraryWrapper
{
    ~VulkanLibraryWrapper()
    {
        if (library)
            dlclose(library);
    }

    // Try to load the library and all the required entry points
    bool loadLibrary()
    {
        if (library)
            return true;

        library = dlopen("libvulkan.so.1", RTLD_LAZY);

        if (!library)
            return false;

        if (!loadEntryPoint(vkGetInstanceProcAddr, "vkGetInstanceProcAddr"))
        {
            dlclose(library);
            library = nullptr;
            return false;
        }

        if (!loadEntryPoint(vkEnumerateInstanceLayerProperties, "vkEnumerateInstanceLayerProperties"))
        {
            dlclose(library);
            library = nullptr;
            return false;
        }

        if (!loadEntryPoint(vkEnumerateInstanceExtensionProperties, "vkEnumerateInstanceExtensionProperties"))
        {
            dlclose(library);
            library = nullptr;
            return false;
        }

        return true;
    }

    template <typename T>
    bool loadEntryPoint(T& entryPoint, const char* name)
    {
        entryPoint = reinterpret_cast<T>(dlsym(library, name));

        return entryPoint != nullptr;
    }

    void* library{};

    PFN_vkGetInstanceProcAddr                  vkGetInstanceProcAddr{};
    PFN_vkEnumerateInstanceLayerProperties     vkEnumerateInstanceLayerProperties{};
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties{};
};

VulkanLibraryWrapper wrapper;
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
bool VulkanImpl::isAvailable(bool requireGraphics)
{
    static bool checked           = false;
    static bool computeAvailable  = false;
    static bool graphicsAvailable = false;

    if (!checked)
    {
        checked = true;

        // Check if the library is available
        computeAvailable = wrapper.loadLibrary();

        // To check for instance extensions we don't need to differentiate between graphics and compute
        graphicsAvailable = computeAvailable;

        if (graphicsAvailable)
        {
            // Retrieve the available instance extensions
            std::vector<VkExtensionProperties> extensionProperties;

            base::U32 extensionCount = 0;

            wrapper.vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

            extensionProperties.resize(extensionCount);

            wrapper.vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

            // Check if the necessary extensions are available
            bool hasVkKhrSurface         = false;
            bool hasVkKhrPlatformSurface = false;

            for (const VkExtensionProperties& properties : extensionProperties)
            {
                if (base::StringView(properties.extensionName) == VK_KHR_SURFACE_EXTENSION_NAME)
                {
                    hasVkKhrSurface = true;
                }
                else if (base::StringView(properties.extensionName) == VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
                {
                    hasVkKhrPlatformSurface = true;
                }
            }

            if (!hasVkKhrSurface || !hasVkKhrPlatformSurface)
                graphicsAvailable = false;
        }
    }

    if (requireGraphics)
        return graphicsAvailable;

    return computeAvailable;
}


////////////////////////////////////////////////////////////
VulkanFunctionPointer VulkanImpl::getFunction(const char* name)
{
    if (!isAvailable(false))
        return nullptr;

    return reinterpret_cast<VulkanFunctionPointer>(dlsym(wrapper.library, name));
}


////////////////////////////////////////////////////////////
base::Span<const char* const> VulkanImpl::getGraphicsRequiredInstanceExtensions()
{
    static constexpr const char* extensions[]{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME};
    return {extensions, 2u};
}


////////////////////////////////////////////////////////////
bool VulkanImpl::createVulkanSurface(const VkInstance&            instance,
                                     WindowHandle                 windowHandle,
                                     VkSurfaceKHR&                surface,
                                     const VkAllocationCallbacks* allocator)
{
    if (!isAvailable())
        return false;

    // Make a copy of the instance handle since we get it passed as a reference
    VkInstance inst = instance;

    auto vkCreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(
        wrapper.vkGetInstanceProcAddr(inst, "vkCreateXlibSurfaceKHR"));

    if (!vkCreateXlibSurfaceKHR)
        return false;

    // Since the surface is basically attached to the window, the connection
    // to the X display will stay open even after we open and close it here
    const auto                 display           = openDisplay();
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = VkXlibSurfaceCreateInfoKHR();
    surfaceCreateInfo.sType                      = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.dpy                        = display.get();
    surfaceCreateInfo.window                     = windowHandle;

    const bool result = (vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, allocator, &surface) == VK_SUCCESS);

    return result;
}

} // namespace sf::priv
