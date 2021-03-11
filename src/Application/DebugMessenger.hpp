#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance								  p_instance,
	const VkDebugUtilsMessengerCreateInfoEXT* p_createInfo,
	const VkAllocationCallbacks*			  p_allocator,
	VkDebugUtilsMessengerEXT*				  p_debugMessenger )
{
	// Create the function via the address
	PFN_vkCreateDebugUtilsMessengerEXT function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( p_instance, "vkCreateDebugUtilsMessengerEXT" );

	// Call the function if it exists
	if ( function != nullptr )
		return function( p_instance, p_createInfo, p_allocator, p_debugMessenger );
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT; // Couldn't find function
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance					 p_instance,
	VkDebugUtilsMessengerEXT	 p_debugMessenger,
	const VkAllocationCallbacks* p_allocator )
{
	// Create the function via the address
	PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( p_instance, "vkDestroyDebugUtilsMessengerEXT" );

	// Call the function if it exists
	if ( function != nullptr )
		return function( p_instance, p_debugMessenger, p_allocator );
	else
		return; // Couldn't find function
}