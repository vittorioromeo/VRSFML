#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Base/Span.hpp"


using VkInstance = struct VkInstance_T*;

#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || \
    defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)

using VkSurfaceKHR = struct VkSurfaceKHR_T*;

#else

#include "SFML/Base/IntTypes.hpp"

using VkSurfaceKHR = sf::base::U64;

#endif

struct VkAllocationCallbacks;


namespace sf
{

using VulkanFunctionPointer = void (*)();

////////////////////////////////////////////////////////////
/// \brief Vulkan helper functions
///
////////////////////////////////////////////////////////////
namespace Vulkan
{
////////////////////////////////////////////////////////////
/// \brief Data associated with a Vulkan surface
///
////////////////////////////////////////////////////////////
struct VulkanSurfaceData
{
    explicit VulkanSurfaceData(const VkInstance&            theInstance,
                               VkSurfaceKHR&                theSurface,
                               const VkAllocationCallbacks* theAllocator = nullptr) :
    instance{theInstance},
    surface{theSurface},
    allocator{theAllocator}
    {
    }

    const VkInstance&            instance;
    VkSurfaceKHR&                surface;
    const VkAllocationCallbacks* allocator;
};

////////////////////////////////////////////////////////////
/// \brief Tell whether or not the system supports Vulkan
///
/// This function should always be called before using
/// the Vulkan features. If it returns `false`, then
/// any attempt to use Vulkan will fail.
///
/// If only compute is required, set `requireGraphics`
/// to `false` to skip checking for the extensions necessary
/// for graphics rendering.
///
/// \param requireGraphics
///
/// \return `true` if Vulkan is supported, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool isAvailable(bool requireGraphics = true);

////////////////////////////////////////////////////////////
/// \brief Get the address of a Vulkan function
///
/// \param name Name of the function to get the address of
///
/// \return Address of the Vulkan function, 0 on failure
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API VulkanFunctionPointer getFunction(const char* name);

////////////////////////////////////////////////////////////
/// \brief Get Vulkan instance extensions required for graphics
///
/// \return Vulkan instance extensions required for graphics
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API base::Span<const char* const> getGraphicsRequiredInstanceExtensions();

} // namespace Vulkan
} // namespace sf


////////////////////////////////////////////////////////////
/// \namespace sf::Vulkan
/// \ingroup window
///
/// This namespace contains functions to help you use SFML
/// for windowing and write your own Vulkan code for graphics
///
////////////////////////////////////////////////////////////
