#pragma once
#include "DebugMessenger.hpp"
#include "QueueFamilies.hpp"
#include "Swapchain.hpp"

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
	VkSwapchainKHR			 m_swapchain;
	std::vector<VkImage>	 m_swapchainImages;
	VkFormat				 m_swapchainImageFormat;
	VkExtent2D				 m_swapchainExtent;
	std::vector<VkImageView> m_swapchainImageViews;

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

		// Initialise the swap chain
		CreateSwapchain();

		// Create image views for the swap chain images
		CreateImageViews();

		// Create the graphics pipeline
		CreateGraphicsPipeline();
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

		// Check if the device is supported by the extensions
		bool extensionSupported = CheckDeviceExtensionSupport( p_device );

		// Check if the swap chain is sufficiently supported
		bool swapchainSufficient = false;

		if ( extensionSupported )
		{
			SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport( p_device, m_surface );

			// The swap chain has formats and present modes
			swapchainSufficient = !( swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty() );
		}

		// Check if the queue family can process the commands we want
		return indices.IsComplete() && extensionSupported && swapchainSufficient;
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
		createInfo.sType				   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos	   = queueCreateInfos;
		createInfo.queueCreateInfoCount	   = uniqueQueueFamilies.size();
		createInfo.pEnabledFeatures		   = &deviceFeatures;
		createInfo.enabledExtensionCount   = static_cast<uint32_t>( deviceExtensionCount );
		createInfo.ppEnabledExtensionNames = deviceExtensions;

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

	bool CheckDeviceExtensionSupport( const VkPhysicalDevice& p_device )
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

	void CreateSwapchain()
	{
		// Get the swap chain support details
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport( m_physicalDevice, m_surface );

		// Pick the best properties for the swap chain from the available settings
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( swapchainSupport.formats );
		VkPresentModeKHR   presentMode	 = ChooseSwapPresentMode( swapchainSupport.presentModes );
		VkExtent2D		   extent		 = ChooseSwapExtent( swapchainSupport.capabilities, m_window );

		// Specify the minimum number of images for the swap chain to function
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

		// Ensure the image count doesn't exceed the maximum number allowed (If it is zero there is no maximum)
		if ( swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount )
			imageCount = swapchainSupport.capabilities.maxImageCount;

		// Setup the create info for the swap chain
		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface			= m_surface;
		createInfo.minImageCount	= imageCount;
		createInfo.imageFormat		= surfaceFormat.format;
		createInfo.imageColorSpace	= surfaceFormat.colorSpace;
		createInfo.imageExtent		= extent;
		createInfo.imageArrayLayers = 1;								   // Number of layers that each image consists of (useful for stereoscopic rendering)
		createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // The way in which the swap chain is used

		// Define how the swap chain handles images across multiple queue families
		QueueFamilyIndices indices				= FindQueueFamilies( m_physicalDevice, m_surface );
		uint32_t		   queueFamilyIndices[] = {
			  indices.graphicsFamily.value(), indices.presentFamily.value()
		};

		if ( indices.graphicsFamily != indices.presentFamily ) // If the graphics and present queues are different
		{
			createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices	 = queueFamilyIndices;
		}
		else // They are the same
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Specify the tranformations to apply to the images
		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;

		// Should blending between windows be used
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Don't blend between windows

		// Set the present mode and if obscured pixels should be clipped
		createInfo.presentMode = presentMode;
		createInfo.clipped	   = VK_TRUE;

		// The swap chain requires a reference to the old swap chain incase it needs recreated
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Create the swap chain
		if ( vkCreateSwapchainKHR( m_logicalDevice, &createInfo, nullptr, &m_swapchain ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create swap chain" );

		// Retrieve the swap chain images
		vkGetSwapchainImagesKHR( m_logicalDevice, m_swapchain, &imageCount, nullptr );
		m_swapchainImages.resize( imageCount );
		vkGetSwapchainImagesKHR( m_logicalDevice, m_swapchain, &imageCount, m_swapchainImages.data() );

		// Set the member variables image format and extent
		m_swapchainImageFormat = surfaceFormat.format;
		m_swapchainExtent	   = extent;
	}

	void CreateImageViews()
	{
		// Resize the vector
		m_swapchainImageViews.resize( m_swapchainImages.size() );

		// Declare a create information struct for the image views
		VkImageViewCreateInfo createInfo {};

		// Iterate over the swap chain images
		for ( size_t i = 0; i < m_swapchainImages.size(); i++ )
		{
			// Set the create info
			createInfo			= {};
			createInfo.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image	= m_swapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format	= m_swapchainImageFormat;

			// Colour mapping for the colour channels
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			// Describe the image's purpose
			createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel   = 0;
			createInfo.subresourceRange.levelCount	   = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount	   = 1;

			// Create the image view
			if ( vkCreateImageView( m_logicalDevice, &createInfo, nullptr, &m_swapchainImageViews[i] ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to create image view" );
		}
	}

	void CreateGraphicsPipeline()
	{
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
		// Destroy the image views
		for ( const auto& imageView : m_swapchainImageViews )
			vkDestroyImageView( m_logicalDevice, imageView, nullptr );

		// Destroy the swap chain
		vkDestroySwapchainKHR( m_logicalDevice, m_swapchain, nullptr );

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