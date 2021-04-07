#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>

#define PREFERRED_COLOUR_SPACE	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define PREFERRED_COLOUR_FORMAT VK_FORMAT_B8G8R8A8_SRGB

#define PREFERRED_PRESENT_MODE VK_PRESENT_MODE_MAILBOX_KHR

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR		capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>	presentModes;
};

static SwapchainSupportDetails QuerySwapchainSupport( const VkPhysicalDevice &p_device, const VkSurfaceKHR &p_surface )
{
	SwapchainSupportDetails details;

	// Get the surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( p_device, p_surface, &details.capabilities );

	// Query the supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR( p_device, p_surface, &formatCount, nullptr );

	if ( formatCount > 0 ) // If there are formats
	{
		// Set the format vector
		details.formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( p_device, p_surface, &formatCount, details.formats.data() );
	}

	// Query the supported surface presentation modes
	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( p_device, p_surface, &presentCount, nullptr );

	if ( presentCount > 0 ) // If there are present modes
	{
		// Set the present modes vector
		details.presentModes.resize( presentCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( p_device, p_surface, &presentCount, details.presentModes.data() );
	}

	return details;
}

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR> &supportedFormats )
{
	for ( const auto &supportedFormat : supportedFormats ) // Iterate formats
	{
		// If the surface format has the correct colour format and colour space
		if ( supportedFormat.format == PREFERRED_COLOUR_FORMAT && supportedFormat.colorSpace == PREFERRED_COLOUR_SPACE )
			return supportedFormat;
	}

	// If an ideal format wasn't found, use the first
	return supportedFormats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode( const std::vector<VkPresentModeKHR> &supportedPresentModes )
{
	for ( const auto &supportedPresentMode : supportedPresentModes )
	{
		// If the present mode matches the preferred present mode
		if ( supportedPresentMode == PREFERRED_PRESENT_MODE )
			return supportedPresentMode;
	}

	// If an ideal present mode wasn't found, use the one guaranteed to be supported
	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR &p_capabilities, GLFWwindow *p_window )
{
	if ( p_capabilities.currentExtent.width != (uint32_t)-1 ) // If it isnt equal to the uint32_t value
		return p_capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize( p_window, &width, &height );

	VkExtent2D actualExtent = {
		static_cast<uint32_t>( width ),
		static_cast<uint32_t>( height )
	};

	// Clamp the width between the max and min width
	actualExtent.width = std::max( p_capabilities.minImageExtent.width, std::min( p_capabilities.maxImageExtent.width, actualExtent.width ) );

	// Clamp the height between the max and min height
	actualExtent.height = std::max( p_capabilities.minImageExtent.height, std::min( p_capabilities.maxImageExtent.height, actualExtent.height ) );

	return actualExtent;
}