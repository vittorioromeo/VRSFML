#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Vulkan.hpp"
#include "SFML/Window/WindowHandle.hpp"


namespace sf::priv::VulkanImpl
{
////////////////////////////////////////////////////////////
/// \brief Tell whether or not the system supports Vulkan
///
/// This function should always be called before using
/// the Vulkan features. If it returns false, then
/// any attempt to use Vulkan will fail.
///
/// If only compute is required, set \a requireGraphics
/// to false to skip checking for the extensions necessary
/// for graphics rendering.
///
/// \param requireGraphics
///
/// \return True if Vulkan is supported, false otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool isAvailable(bool requireGraphics = true);

////////////////////////////////////////////////////////////
/// \brief Get the address of a Vulkan function
///
/// \param name Name of the function to get the address of
///
/// \return Address of the Vulkan function, `nullptr` on failure
///
////////////////////////////////////////////////////////////
[[nodiscard]] VulkanFunctionPointer getFunction(const char* name);

////////////////////////////////////////////////////////////
/// \brief Get Vulkan instance extensions required for graphics
///
/// \return Vulkan instance extensions required for graphics
///
////////////////////////////////////////////////////////////
[[nodiscard]] base::Span<const char* const> getGraphicsRequiredInstanceExtensions();

////////////////////////////////////////////////////////////
/// \brief Create a Vulkan rendering surface
///
/// \param instance     Vulkan instance
/// \param windowHandle Handle to the window to create the surface for
/// \param surface      Created surface
/// \param allocator    Allocator to use
///
/// \return True if surface creation was successful, false otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool createVulkanSurface(const VkInstance&            instance,
                                       WindowHandle                 windowHandle,
                                       VkSurfaceKHR&                surface,
                                       const VkAllocationCallbacks* allocator);

} // namespace sf::priv::VulkanImpl
