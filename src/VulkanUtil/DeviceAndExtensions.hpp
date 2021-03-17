#pragma once
#include "QueueFamilies.hpp"
#include "Swapchain.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <set>
#include <string>

#define ENABLE_VALIDATION_LAYERS 1 // When in debug mode enable validation layers

const char*	   validationLayers[]	= { "VK_LAYER_KHRONOS_validation" }; // The names of the validation layers
const uint32_t validationLayerCount = 1;

const char*	   deviceExtensions[]	= { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // The names of the required extensions
const uint32_t deviceExtensionCount = 1;

static bool CheckDeviceExtensionSupport( const VkPhysicalDevice& p_device )
{
	// Get the amount of extensions
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( p_device, nullptr, &extensionCount, nullptr );

	// Get the extension properties
	VkExtensionProperties supportedExtensions[extensionCount];
	vkEnumerateDeviceExtensionProperties( p_device, nullptr, &extensionCount, supportedExtensions );

	// Create a set from the required extensions
	std::set<std::string> requiredExtensions( std::begin( deviceExtensions ), std::end( deviceExtensions ) );

	for ( const auto& extension : supportedExtensions )
		requiredExtensions.erase( extension.extensionName ); // Remove it from the set if it is supported

	// If all of the extensions were removed then they are all supported
	return requiredExtensions.empty();
}

static bool IsDeviceSuitable( const VkPhysicalDevice& p_device, const VkSurfaceKHR& p_surface )
{
	// Get the queue family indices
	QueueFamilyIndices indices = FindQueueFamilies( p_device, p_surface );

	// Check if the device is supported by the extensions
	bool extensionSupported = CheckDeviceExtensionSupport( p_device );

	// Check if the swap chain is sufficiently supported
	bool swapchainSufficient = false;

	if ( extensionSupported )
	{
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport( p_device, p_surface );

		// The swap chain has formats and present modes
		swapchainSufficient = !( swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty() );
	}

	// Get the physical device features
	VkPhysicalDeviceFeatures supportedFeatures {};
	vkGetPhysicalDeviceFeatures( p_device, &supportedFeatures );

	// Check if the queue family can process the commands we want, and the extentions and features we want are supported
	return indices.IsComplete() && extensionSupported && swapchainSufficient && supportedFeatures.samplerAnisotropy;
}

static bool CheckValidationLayerSupport()
{
	// Get the amount of instance layers supported
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

	// Get all the supported instance layers
	VkLayerProperties supportedLayers[layerCount];
	vkEnumerateInstanceLayerProperties( &layerCount, supportedLayers );

	// Output the supported layers to the console
	std::cout << "Supported instance layers (" << layerCount << "):" << std::endl;
	for ( const auto& layer : supportedLayers )
		std::cout << '\t' << layer.layerName << std::endl;

	std::cout << std::endl; // Padding

	// Check if the defined validation layers are in the set of supported layers
	for ( const char* layerName : validationLayers ) // Iterate defined layers
	{
		bool layerFound = false;

		for ( const auto& layerProperties : supportedLayers )		   // Iterate supported layers
			if ( strcmp( layerName, layerProperties.layerName ) == 0 ) // Check for equality
			{
				layerFound = true;
				break;
			}

		// No supported layer was found for this layer
		if ( !layerFound ) return false;
	}

	// All layers were supported
	return true;
}

static std::vector<const char*> GetRequiredExtensions( uint32_t* glfwExtensionCount )
{
	// Get the extensions that GLFW is using
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions( glfwExtensionCount );

	// Create a vector with the extension names
	std::vector<const char*> extensions( glfwExtensions, glfwExtensions + *glfwExtensionCount );

	// Add the debug utilities extension
	if ( ENABLE_VALIDATION_LAYERS )
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	// Increment glfwExtensionCount because another extension was added
	*glfwExtensionCount += 1;

	return extensions;
}
