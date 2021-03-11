#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
};

QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice p_device )
{
	QueueFamilyIndices indices;

	return indices;
}