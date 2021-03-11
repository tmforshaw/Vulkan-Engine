#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool IsComplete() { return graphicsFamily.has_value(); }
};

QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice p_device )
{
	QueueFamilyIndices indices;

	// Get the amount of queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( p_device, &queueFamilyCount, nullptr );

	// Get the queue families
	VkQueueFamilyProperties queueFamilies[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties( p_device, &queueFamilyCount, queueFamilies );

	for ( uint32_t i = 0; i < queueFamilyCount; i++ ) // Iterate over queue families
	{
		if ( queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) // If the graphics bit is set
			indices.graphicsFamily = i;							   // Set the graphics family to this index

		// Exit loop if the indices are complete
		if ( indices.IsComplete() ) return indices;
	}

	// This is an incomplete set of indices
	return indices;
}