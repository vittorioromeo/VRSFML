#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Window/WindowHandle.hpp"

#include "SFML/Base/Span.hpp"


////////////////////////////////////////////////////////////
using VkInstance = struct VkInstance_T*;


////////////////////////////////////////////////////////////
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || \
    defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)

using VkSurfaceKHR = struct VkSurfaceKHR_T*;

#else

    #include "SFML/Base/IntTypes.hpp"

using VkSurfaceKHR = sf::base::U64;

#endif


////////////////////////////////////////////////////////////
struct VkAllocationCallbacks;


namespace sf
{
////////////////////////////////////////////////////////////
using VulkanFunctionPointer = void (*)();


////////////////////////////////////////////////////////////
/// \brief Vulkan helper functions
///
////////////////////////////////////////////////////////////
namespace Vulkan
{
////////////////////////////////////////////////////////////
/// \brief Bundle of references needed to create a Vulkan surface
///
/// `VulkanSurfaceData` groups the inputs and the output of
/// `sf::WindowBase::createVulkanSurface` into a single value
/// to keep that call concise.
///
/// All members are stored as references / pointers, so the
/// referred-to objects must outlive the `VulkanSurfaceData`
/// instance.
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

    const VkInstance&            instance;  //!< Vulkan instance to create the surface in
    VkSurfaceKHR&                surface;   //!< Out-parameter that will receive the created surface handle
    const VkAllocationCallbacks* allocator; //!< Optional Vulkan allocator callbacks (may be `nullptr`)
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
/// \param requireGraphics If `true`, also require the extensions necessary for graphics rendering
///
/// \return `true` if Vulkan is supported, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool isAvailable(bool requireGraphics = true);

////////////////////////////////////////////////////////////
/// \brief Get the address of a Vulkan function
///
/// \param name     Name of the function to get the address of
/// \param instance Vulkan instance to get the function for, or `VK_NULL_HANDLE` for global functions
///
/// \return Address of the Vulkan function, `nullptr` on failure
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API VulkanFunctionPointer getFunction(const char* name, VkInstance instance);

////////////////////////////////////////////////////////////
/// \brief Get Vulkan instance extensions required for graphics
///
/// Returns the set of extensions that must be enabled when
/// creating a Vulkan instance that will be used to render
/// into a `sf::Window` (or `sf::WindowBase`).
///
/// The returned span is owned by the implementation and
/// remains valid for the duration of the program.
///
/// \return Vulkan instance extensions required for graphics
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API base::Span<const char* const> getGraphicsRequiredInstanceExtensions();

////////////////////////////////////////////////////////////
/// \brief Create a Vulkan rendering surface
///
/// Internal helper used by `sf::WindowBase::createVulkanSurface`.
///
/// \param instance     Vulkan instance to create the surface in
/// \param windowHandle Handle to the underlying `SDL_Window` to create the surface for
/// \param surface      Out-parameter that will receive the created surface
/// \param allocator    Optional allocator callbacks (may be `nullptr`)
///
/// \return `true` if surface creation was successful, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool createVulkanSurface(const VkInstance&            instance,
                                                       void*                        windowHandle,
                                                       VkSurfaceKHR&                surface,
                                                       const VkAllocationCallbacks* allocator);

} // namespace Vulkan
} // namespace sf

////////////////////////////////////////////////////////////
/// \namespace sf::Vulkan
/// \ingroup window
///
/// This namespace contains the small set of helpers needed
/// to use SFML for windowing alongside hand-rolled Vulkan
/// rendering. Most users will only ever touch
/// `sf::Vulkan::isAvailable`,
/// `sf::Vulkan::getGraphicsRequiredInstanceExtensions`, and
/// `sf::WindowBase::createVulkanSurface`.
///
////////////////////////////////////////////////////////////
