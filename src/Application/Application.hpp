#pragma once
#include "DebugMessenger.hpp"
#include "QueueFamilies.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

const uint16_t winWidth	 = 700;
const uint16_t winHeight = 700;

const char*	   validationLayers[]	= { "VK_LAYER_KHRONOS_validation" }; // The names of the validation layers
const uint32_t validationLayerCount = 1;

const char*	   deviceExtensions[]	= { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // The names of the required extensions
const uint32_t deviceExtensionCount = 1;

#define ENABLE_VALIDATION_LAYERS 1 // When in debug mode enable validation layers

class Application
{
private:
	GLFWwindow*				 m_window;
	VkInstance				 m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice		 m_physicalDevice;
	VkDevice				 m_logicalDevice;
	VkQueue					 m_graphicsQueue;
	VkQueue					 m_presentQueue;
	VkSurfaceKHR			 m_surface;

	void InitVulkan()
	{
		// Create a Vulkan instance
		CreateVulkanInstance();

		// Setup the debug messenger
		InitDebugMessenger();

		// Create a window surface
		CreateSurface();

		// Setup the graphics card to use
		m_physicalDevice = VK_NULL_HANDLE; // Set a default value for m_physicalDevice
		PickPhysicalDevice();			   // Pick a suitable device

		// Initialise the logical device
		CreateLogicalDevice();
	}

	void InitWindow()
	{
		// Setup a GLFW window
		glfwInit();

		// Don't create an OpenGL context
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

		glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

		// Set the window variable
		m_window = glfwCreateWindow( winWidth, winHeight, "Vulkan", nullptr, nullptr );
	}

	void InitDebugMessenger()
	{
		// If validation layers aren't enabled then do nothing
		if ( !ENABLE_VALIDATION_LAYERS ) return;

		// Set the create info
		VkDebugUtilsMessengerCreateInfoEXT createInfo {};
		PopulateDebugMessengerCreateInfo( &createInfo );

		// Create the messenger and throw an error if it fails
		if ( CreateDebugUtilsMessengerEXT( m_instance, &createInfo, nullptr, &m_debugMessenger ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to initialise debug messenger" );
	}

	void PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT* createInfo )
	{
		*createInfo					= {};
		createInfo->sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		createInfo->messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo->pfnUserCallback = DebugCallback;
		createInfo->pUserData		= nullptr; // Optional data to send to DebugCallback
	}

	void CreateVulkanInstance()
	{
		// Enable validation layers
		if ( ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport() )
			throw std::runtime_error( "One or more requested validation layer was not supported" );

		// Get the required extensions
		uint32_t				 extensionCount = 0;
		std::vector<const char*> extensions		= GetRequiredExtensions( &extensionCount );

		// Define the instance application information
		VkApplicationInfo appInfo {};
		appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName   = "Application Name";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.pEngineName		   = "No Name Vulkan Engine";
		appInfo.engineVersion	   = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion		   = VK_API_VERSION_1_0;

		// Define the instance create information
		VkInstanceCreateInfo createInfo {};
		createInfo.sType				   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo		   = &appInfo;
		createInfo.enabledExtensionCount   = extensionCount;
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount	   = 0;

		// Define the debug messenger create information
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; // This will only last the lifetime of this function

		if ( ENABLE_VALIDATION_LAYERS )
		{
			// Set the validation layers if they are enabled
			createInfo.enabledLayerCount   = static_cast<uint32_t>( validationLayerCount );
			createInfo.ppEnabledLayerNames = validationLayers;

			// Set the debug messenger's create information, and enable it in the instance create information
			PopulateDebugMessengerCreateInfo( &debugCreateInfo );
			createInfo.pNext = (VkDebugUtilsMessengerEXT*)&debugCreateInfo;
		}
		else
		{
			// Set the enabled validation layers to zero
			createInfo.enabledLayerCount = 0;

			// Set that there is no debug messenger to use when instantiating the application or Vulkan
			createInfo.pNext = nullptr;
		}

		// Create the instance and throw an error if it hasn't been instantiated
		if ( vkCreateInstance( &createInfo, nullptr, &m_instance ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create Vulkan instance" );

		// Check extension support
		uint32_t supportedExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties( nullptr, &supportedExtensionCount, nullptr );

		// Create an array to hold the supported extension properties and then store the properties
		VkExtensionProperties supportedExtensions[supportedExtensionCount];
		vkEnumerateInstanceExtensionProperties( nullptr, &supportedExtensionCount, supportedExtensions );

		// Output the supported extensions
		std::cout << "Supported extensions (" << supportedExtensionCount << "):" << std::endl;
		for ( const auto& extension : supportedExtensions )
			std::cout << '\t' << extension.extensionName << std::endl;

		std::cout << std::endl; // Padding
	}

	bool CheckValidationLayerSupport()
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

	std::vector<const char*> GetRequiredExtensions( uint32_t* glfwExtensionCount )
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

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* p_callbackData, void* p_userData )
	{
		std::cerr << "Validation Layer: " << p_callbackData->pMessage << std::endl
				  << std::endl;

		return VK_FALSE;
	}

	void PickPhysicalDevice()
	{
		// Get the amount of GPUs available
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices( m_instance, &deviceCount, nullptr );

		// If there is no GPUs then throw an error
		if ( deviceCount == 0 )
			throw std::runtime_error( "Failed to find a GPU with support for Vulkan" );

		// Store the physical devices
		VkPhysicalDevice devices[deviceCount];
		vkEnumeratePhysicalDevices( m_instance, &deviceCount, devices );

		// Check if any physical devices are suitable
		for ( const auto& device : devices )
		{
			// Set the class physical device to the first suitable device
			if ( IsDeviceSuitable( device ) )
			{
				m_physicalDevice = device;
				break;
			}
		}

		// Check if a device was found
		if ( m_physicalDevice == VK_NULL_HANDLE )
			throw std::runtime_error( "Failed to find a suitable GPU" ); // Device wasn't found
	}

	bool IsDeviceSuitable( const VkPhysicalDevice& p_device )
	{
		// Get the queue family indices
		QueueFamilyIndices indices = FindQueueFamilies( p_device, m_surface );

		// Check if the queue family can process the commands we want
		return indices.IsComplete();

		// // Get the properties of the device
		// VkPhysicalDeviceProperties deviceProperties;
		// vkGetPhysicalDeviceProperties( p_device, &deviceProperties );

		// // Get the features of the device
		// VkPhysicalDeviceFeatures deviceFeatures;
		// vkGetPhysicalDeviceFeatures( p_device, &deviceFeatures );

		// // The device is a discrete GPU and it has a geometry shader
		// return ( deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader );
	}

	void CreateLogicalDevice()
	{
		// Get the queue family indices
		QueueFamilyIndices indices = FindQueueFamilies( m_physicalDevice, m_surface );

		// Create a set of unique queue families
		std::set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily.value(), indices.presentFamily.value()
		};

		// Set the queue priority
		float queuePriority = 1.0f;

		// Create an array of create infos
		VkDeviceQueueCreateInfo queueCreateInfos[uniqueQueueFamilies.size()];

		// Iterate through unique queue families and set the create info
		{ // In a scope to destroy i
			uint32_t i = 0;
			for ( uint32_t family : uniqueQueueFamilies )
			{
				VkDeviceQueueCreateInfo queueCreateInfo {};
				queueCreateInfo.sType			 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = family;
				queueCreateInfo.queueCount		 = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos[i]				 = queueCreateInfo;

				i++; // Increment i
			}
		}

		// Specify the device features to use
		VkPhysicalDeviceFeatures deviceFeatures {};

		// Create the logical device
		VkDeviceCreateInfo createInfo {};
		createInfo.sType				 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos	 = queueCreateInfos;
		createInfo.queueCreateInfoCount	 = uniqueQueueFamilies.size();
		createInfo.pEnabledFeatures		 = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		// Set the validation layers (For compatability with older versions)
		if ( ENABLE_VALIDATION_LAYERS )
		{
			createInfo.enabledLayerCount   = static_cast<uint32_t>( validationLayerCount );
			createInfo.ppEnabledLayerNames = validationLayers;
		}
		else
			createInfo.enabledLayerCount = 0;

		// Instantiate the logical device
		if ( vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_logicalDevice ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create logical device" ); // Throw an error if it failed

		// Get the queue handle for the graphics queue
		vkGetDeviceQueue( m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue );

		// Get the queue handle for the presentation queue
		vkGetDeviceQueue( m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue );
	}

	void CreateSurface()
	{
		// Create a platform agnostic window surface using GLFW
		if ( glfwCreateWindowSurface( m_instance, m_window, nullptr, &m_surface ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create window surface" );
	}

	void MainLoop()
	{
		while ( !glfwWindowShouldClose( m_window ) ) // Loop until the window is supposed to close
		{
			glfwPollEvents(); // Check for events and then call the correct callback
		}
	}

	void Cleanup()
	{
		// Destroy the logical device
		vkDestroyDevice( m_logicalDevice, nullptr );

		// Destroy the window surface
		vkDestroySurfaceKHR( m_instance, m_surface, nullptr );

		// Destroy debug messenger if it exists
		if ( ENABLE_VALIDATION_LAYERS )
			DestroyDebugUtilsMessengerEXT( m_instance, m_debugMessenger, nullptr );

		// Destroy the Vulkan instance
		vkDestroyInstance( m_instance, nullptr );

		// Destroy the GLFW window and then the context
		glfwDestroyWindow( m_window );
		glfwTerminate();
	}

public:
	void Run()
	{
		std::cout << "Starting Application" << std::endl;

		// Initialise variables
		InitWindow();
		InitVulkan();

		MainLoop();

		// Destruct variables
		Cleanup();
	}
};