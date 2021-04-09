#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static VkSampleCountFlagBits GetMaxUsableSampleCount( const VkPhysicalDeviceProperties& p_physicalDeviceProperties )
{
	// Set the bitmask for the counts
	VkSampleCountFlags counts = p_physicalDeviceProperties.limits.framebufferColorSampleCounts & p_physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	// Pick the correct sample count
	if ( counts & VK_SAMPLE_COUNT_64_BIT ) return VK_SAMPLE_COUNT_64_BIT;
	if ( counts & VK_SAMPLE_COUNT_32_BIT ) return VK_SAMPLE_COUNT_32_BIT;
	if ( counts & VK_SAMPLE_COUNT_16_BIT ) return VK_SAMPLE_COUNT_16_BIT;
	if ( counts & VK_SAMPLE_COUNT_8_BIT ) return VK_SAMPLE_COUNT_8_BIT;
	if ( counts & VK_SAMPLE_COUNT_4_BIT ) return VK_SAMPLE_COUNT_4_BIT;
	if ( counts & VK_SAMPLE_COUNT_2_BIT ) return VK_SAMPLE_COUNT_2_BIT;
	return VK_SAMPLE_COUNT_1_BIT;
}