#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

static VkResult CreateDebugUtilsMessengerEXT(
	const VkInstance&						  p_instance,
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

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* p_callbackData, void* p_userData )
{
	std::cerr << "Validation layer: " << p_callbackData->pMessage << std::endl
			  << std::endl;

	return VK_FALSE;
}

static void PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT* createInfo )
{
	*createInfo					= {};
	createInfo->sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
	createInfo->messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo->pfnUserCallback = DebugCallback;
	createInfo->pUserData		= nullptr; // Optional data to send to DebugCallback
}

static void DestroyDebugUtilsMessengerEXT(
	const VkInstance&				p_instance,
	const VkDebugUtilsMessengerEXT& p_debugMessenger,
	const VkAllocationCallbacks*	p_allocator )
{
	// Create the function via the address
	PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( p_instance, "vkDestroyDebugUtilsMessengerEXT" );

	// Call the function if it exists
	if ( function != nullptr )
		return function( p_instance, p_debugMessenger, p_allocator );
	else
		return; // Couldn't find function
}