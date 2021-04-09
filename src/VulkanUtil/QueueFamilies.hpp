#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

static QueueFamilyIndices FindQueueFamilies( const VkPhysicalDevice& p_physicalDevice, const VkSurfaceKHR& p_surface )
{
	QueueFamilyIndices indices;

	// Get the amount of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( p_physicalDevice, &queueFamilyCount, nullptr );

	// Get the queue families
	VkQueueFamilyProperties queueFamilies[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties( p_physicalDevice, &queueFamilyCount, queueFamilies );

	{ // Inside of a scope to destroy presentSupport
		VkBool32 presentSupport = false;
		for ( uint32_t i = 0; i < queueFamilyCount; i++ ) // Iterate over queue families
		{
			// Look for a graphics queue
			if ( queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
				indices.graphicsFamily = i; // Set the graphics family to this index

			// Query the queue family for window surface support
			vkGetPhysicalDeviceSurfaceSupportKHR( p_physicalDevice, i, p_surface, &presentSupport );
			if ( presentSupport ) indices.presentFamily = i; // Set the presentation family to this index

			// Exit loop if the indices are complete
			if ( indices.IsComplete() ) return indices;
		}
	}

	// This is an incomplete set of indices
	return indices;
}