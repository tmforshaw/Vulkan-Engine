#pragma once
#include "DebugMessenger.hpp"
#include "QueueFamilies.hpp"
#include "Shaders.hpp"
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
#define MAX_FRAMES_IN_FLIGHT	 2 // Maximum number of frames to process concurrently

class Application
{
private:
	GLFWwindow*					 m_window;
	VkInstance					 m_instance;
	VkDebugUtilsMessengerEXT	 m_debugMessenger;
	VkPhysicalDevice			 m_physicalDevice;
	VkDevice					 m_logicalDevice;
	VkQueue						 m_graphicsQueue;
	VkQueue						 m_presentQueue;
	VkSurfaceKHR				 m_surface;
	VkSwapchainKHR				 m_swapchain;
	std::vector<VkImage>		 m_swapchainImages;
	VkFormat					 m_swapchainImageFormat;
	VkExtent2D					 m_swapchainExtent;
	std::vector<VkImageView>	 m_swapchainImageViews;
	VkRenderPass				 m_renderPass;
	VkPipelineLayout			 m_pipelineLayout;
	VkPipeline					 m_graphicsPipeline;
	std::vector<VkFramebuffer>	 m_swapchainFramebuffers;
	VkCommandPool				 m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore>	 m_imageAvailableSemaphores;
	std::vector<VkSemaphore>	 m_renderFinishedSemaphores;
	std::vector<VkFence>		 m_inFlightFences;
	std::vector<VkFence>		 m_inFlightImages;
	size_t						 m_currentFrame;

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

		// Create a render pass
		CreateRenderPass();

		// Create the graphics pipeline
		CreateGraphicsPipeline();

		// Create the framebuffers
		CreateFramebuffers();

		// Create the command pool
		CreateCommandPool();

		// Create the command buffers
		CreateCommandBuffers();

		// Create the semaphores and fences
		CreateSyncObjects();
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
		std::cerr << "Validation layer: " << p_callbackData->pMessage << std::endl
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

	void CreateRenderPass()
	{
		// Set the rendering settings
		VkAttachmentDescription colourAttatchment {};
		colourAttatchment.format		 = m_swapchainImageFormat;
		colourAttatchment.samples		 = VK_SAMPLE_COUNT_1_BIT;
		colourAttatchment.loadOp		 = VK_ATTACHMENT_LOAD_OP_CLEAR;	 // Clear the frame buffer to black each frame
		colourAttatchment.storeOp		 = VK_ATTACHMENT_STORE_OP_STORE; // Store the contents into memory
		colourAttatchment.stencilLoadOp	 = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttatchment.initialLayout	 = VK_IMAGE_LAYOUT_UNDEFINED;
		colourAttatchment.finalLayout	 = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Setup subpasses and attatchment references
		VkAttachmentReference colourAttatchmentRef {};
		colourAttatchmentRef.attachment = 0;										// Which attachment to reference (by index in attachment descriptions array)
		colourAttatchmentRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout for the subpass

		// Create a subpass description
		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint	 = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1; // Index of colour attachment
		subpass.pColorAttachments	 = &colourAttatchmentRef;

		// Set a dependency for the image to be acquired before the subpass starts
		VkSubpassDependency dependency {};
		dependency.srcSubpass	 = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass	 = 0;
		dependency.srcStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait on this operation
		dependency.srcAccessMask = 0;
		dependency.dstStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // The operations that should wait
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;		  // The operations that should wait

		// Set the render pass create info
		VkRenderPassCreateInfo renderPassCreateInfo {};
		renderPassCreateInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments	 = &colourAttatchment;
		renderPassCreateInfo.subpassCount	 = 1;
		renderPassCreateInfo.pSubpasses		 = &subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies	 = &dependency;

		// Create the render pass
		if ( vkCreateRenderPass( m_logicalDevice, &renderPassCreateInfo, nullptr, &m_renderPass ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create render pass" );
	}

	void CreateGraphicsPipeline()
	{
		// Read the shader files
		auto vertShaderCode = ReadFile( "lib/shaders/SimpleShader.vert.spv" );
		auto fragShaderCode = ReadFile( "lib/shaders/SimpleShader.frag.spv" );

		// If they have data
		if ( !vertShaderCode.empty() && !fragShaderCode.empty() )
		{
			std::cout << "Vertex shader read from file (" << vertShaderCode.size() << " bytes)" << std::endl;
			std::cout << "Fragment shader read from file (" << fragShaderCode.size() << " bytes)" << std::endl;

			// Create the shader modules
			VkShaderModule vertShaderModule = CreateShaderModule( vertShaderCode, m_logicalDevice );
			VkShaderModule fragShaderModule = CreateShaderModule( fragShaderCode, m_logicalDevice );

			// Set the create info for the vertex shader stage
			VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
			vertShaderStageInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage				= VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module				= vertShaderModule;
			vertShaderStageInfo.pName				= "main";  // Entry point
			vertShaderStageInfo.pSpecializationInfo = nullptr; // Set shader constants

			// Set the create info for the fragment shader stage
			VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
			fragShaderStageInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module				= fragShaderModule;
			fragShaderStageInfo.pName				= "main";  // Entry point
			fragShaderStageInfo.pSpecializationInfo = nullptr; // Set shader constants

			// Create an array with the structs
			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			// Setup structure of the vertex data using create information
			VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
			vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount	= 0;
			vertexInputInfo.pVertexBindingDescriptions		= nullptr; // Array of structs to describe vertex data
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions	= nullptr;

			// Describe the primitive which will be drawn and if primitive restart should be enabled
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {};
			inputAssemblyCreateInfo.sType				   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyCreateInfo.topology			   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

			// Setup the viewport
			VkViewport viewport {};
			viewport.x		  = 0.0f;
			viewport.y		  = 0.0f;
			viewport.width	  = (float)m_swapchainExtent.width;
			viewport.height	  = (float)m_swapchainExtent.height;
			viewport.minDepth = 0.0f; // Minimum depth value to use
			viewport.maxDepth = 1.0f; // Maxmimum depth value to use

			// Define the clipping rectangle (scissor rectangle)
			VkRect2D scissor {};
			scissor.offset = { 0, 0 };
			scissor.extent = m_swapchainExtent;

			// Combine viewport and clipping rectangle
			VkPipelineViewportStateCreateInfo viewportStateCreateInfo {};
			viewportStateCreateInfo.sType		  = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pViewports	  = &viewport;
			viewportStateCreateInfo.scissorCount  = 1;
			viewportStateCreateInfo.pScissors	  = &scissor;

			// Setup the rasteriser
			VkPipelineRasterizationStateCreateInfo rasteriserCreateInfo {};
			rasteriserCreateInfo.sType					 = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasteriserCreateInfo.depthClampEnable		 = VK_FALSE;
			rasteriserCreateInfo.rasterizerDiscardEnable = VK_FALSE; // Disable output to geometry shader (disable output to framebuffer)
			rasteriserCreateInfo.polygonMode			 = VK_POLYGON_MODE_FILL;
			rasteriserCreateInfo.lineWidth				 = 1.0f;
			rasteriserCreateInfo.cullMode				 = VK_CULL_MODE_BACK_BIT; // Cull back faces
			rasteriserCreateInfo.frontFace				 = VK_FRONT_FACE_CLOCKWISE;
			rasteriserCreateInfo.depthBiasEnable		 = VK_FALSE; // Enable alteration of depth values
			rasteriserCreateInfo.depthBiasConstantFactor = 0.0f;
			rasteriserCreateInfo.depthBiasClamp			 = 0.0f;
			rasteriserCreateInfo.depthBiasSlopeFactor	 = 0.0f;

			// Configure multisampling
			VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo {};
			multisamplingCreateInfo.sType				  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisamplingCreateInfo.sampleShadingEnable	  = VK_FALSE;
			multisamplingCreateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
			multisamplingCreateInfo.minSampleShading	  = 1.0f;
			multisamplingCreateInfo.pSampleMask			  = nullptr;
			multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
			multisamplingCreateInfo.alphaToOneEnable	  = VK_FALSE;

			// This is where I should configure depth and stencil buffers

			// Setup the colour blending stage (Per Framebuffer)
			VkPipelineColorBlendAttachmentState colourBlendAttatchment {};
			colourBlendAttatchment.colorWriteMask	   = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colourBlendAttatchment.blendEnable		   = VK_TRUE;
			colourBlendAttatchment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colourBlendAttatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colourBlendAttatchment.colorBlendOp		   = VK_BLEND_OP_ADD;
			colourBlendAttatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colourBlendAttatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colourBlendAttatchment.alphaBlendOp		   = VK_BLEND_OP_ADD;

			// Setup the create information
			VkPipelineColorBlendStateCreateInfo colourBlendCreateInfo {};
			colourBlendCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colourBlendCreateInfo.logicOpEnable		= VK_FALSE; // Enable bitwise blending
			colourBlendCreateInfo.logicOp			= VK_LOGIC_OP_COPY;
			colourBlendCreateInfo.attachmentCount	= 1;
			colourBlendCreateInfo.pAttachments		= &colourBlendAttatchment;
			colourBlendCreateInfo.blendConstants[0] = 0.0f;
			colourBlendCreateInfo.blendConstants[1] = 0.0f;
			colourBlendCreateInfo.blendConstants[2] = 0.0f;
			colourBlendCreateInfo.blendConstants[3] = 0.0f;

			// Set the dynamic states for the graphics pipeline
			VkDynamicState dynamicStates[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_LINE_WIDTH
			};

			// Create the dynamic state pipeline create information
			VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
			dynamicStateCreateInfo.sType			 = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateCreateInfo.dynamicStateCount = 2;
			dynamicStateCreateInfo.pDynamicStates	 = dynamicStates;

			// Set the pipeline layout
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
			pipelineLayoutCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount			= 0;
			pipelineLayoutCreateInfo.pSetLayouts			= nullptr;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges	= nullptr;

			// Create the pipeline layout
			if ( vkCreatePipelineLayout( m_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to create pipeline lauout" );

			// Setup the graphics pipeline create information
			VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {};
			graphicsPipelineCreateInfo.sType			   = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfo.stageCount		   = 2;
			graphicsPipelineCreateInfo.pStages			   = shaderStages;
			graphicsPipelineCreateInfo.pVertexInputState   = &vertexInputInfo;
			graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
			graphicsPipelineCreateInfo.pViewportState	   = &viewportStateCreateInfo;
			graphicsPipelineCreateInfo.pRasterizationState = &rasteriserCreateInfo;
			graphicsPipelineCreateInfo.pMultisampleState   = &multisamplingCreateInfo;
			graphicsPipelineCreateInfo.pDepthStencilState  = nullptr;
			graphicsPipelineCreateInfo.pColorBlendState	   = &colourBlendCreateInfo;
			graphicsPipelineCreateInfo.pDynamicState	   = nullptr;
			graphicsPipelineCreateInfo.layout			   = m_pipelineLayout;
			graphicsPipelineCreateInfo.renderPass		   = m_renderPass;
			graphicsPipelineCreateInfo.subpass			   = 0;
			graphicsPipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
			graphicsPipelineCreateInfo.basePipelineIndex   = -1;

			// Create the graphics pipeline
			if ( vkCreateGraphicsPipelines( m_logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_graphicsPipeline ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to create graphics pipeline" );

			// Destroy the shader modules (Once the graphics pipeline is created they aren't needed)
			vkDestroyShaderModule( m_logicalDevice, vertShaderModule, nullptr );
			vkDestroyShaderModule( m_logicalDevice, fragShaderModule, nullptr );
		}
	}

	void CreateFramebuffers()
	{
		// Resize the framebuffer vector to hold all the frame buffers
		m_swapchainFramebuffers.resize( m_swapchainImageViews.size() );

		// Create frame buffers from the image views
		for ( size_t i = 0; i < m_swapchainImageViews.size(); i++ )
		{
			// Create an image view array
			VkImageView attachments[] = {
				m_swapchainImageViews[i]
			};

			// Setup the framebuffer create information
			VkFramebufferCreateInfo framebufferCreateInfo {};
			framebufferCreateInfo.sType			  = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass	  = m_renderPass;
			framebufferCreateInfo.attachmentCount = 1;
			framebufferCreateInfo.pAttachments	  = attachments;
			framebufferCreateInfo.width			  = m_swapchainExtent.width;
			framebufferCreateInfo.height		  = m_swapchainExtent.height;
			framebufferCreateInfo.layers		  = 1;

			// Create the frame buffer
			if ( vkCreateFramebuffer( m_logicalDevice, &framebufferCreateInfo, nullptr, &m_swapchainFramebuffers[i] ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to create framebuffer" );
		}
	}

	void CreateCommandPool()
	{
		// Get the family indices
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies( m_physicalDevice, m_surface );

		// Setup the create information for the command pool
		VkCommandPoolCreateInfo poolCreateInfo {};
		poolCreateInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolCreateInfo.flags			= 0;

		// Create the command pool
		if ( vkCreateCommandPool( m_logicalDevice, &poolCreateInfo, nullptr, &m_commandPool ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create command pool" );
	}

	void CreateCommandBuffers()
	{
		// Resize the command buffers vector
		m_commandBuffers.resize( m_swapchainFramebuffers.size() );

		// Setup the allocation information for the command buffer
		VkCommandBufferAllocateInfo commandBufferAllocInfo {};
		commandBufferAllocInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.commandPool		  = m_commandPool;
		commandBufferAllocInfo.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		// Create the command buffers
		if ( vkAllocateCommandBuffers( m_logicalDevice, &commandBufferAllocInfo, m_commandBuffers.data() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate command buffers" );

		// Begin recording to the command buffers
		for ( size_t i = 0; i < m_commandBuffers.size(); i++ )
		{
			// Setup the begin information for the command buffer
			VkCommandBufferBeginInfo commandBufferBeginInfo {};
			commandBufferBeginInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.flags			= 0;
			commandBufferBeginInfo.pInheritanceInfo = nullptr;

			// Create the command buffer
			if ( vkBeginCommandBuffer( m_commandBuffers[i], &commandBufferBeginInfo ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to begin recording command buffer[" + std::to_string( i ) + "]" );

			// Set a clear colour
			VkClearValue clearColour = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };

			// Setup the begin informatio for the render pass
			VkRenderPassBeginInfo renderPassBeginInfo {};
			renderPassBeginInfo.sType			  = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass		  = m_renderPass;
			renderPassBeginInfo.framebuffer		  = m_swapchainFramebuffers[i];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
			renderPassBeginInfo.clearValueCount	  = 1;
			renderPassBeginInfo.pClearValues	  = &clearColour;

			// Record the beginning of a render pass
			vkCmdBeginRenderPass( m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			// Record the binding of the graphics pipeline
			vkCmdBindPipeline( m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline );

			// Record the drawing of the triangle
			vkCmdDraw( m_commandBuffers[i], 3, 1, 0, 0 );

			// Record the end of the render pass
			vkCmdEndRenderPass( m_commandBuffers[i] );

			// Finish the recording and check for errors
			if ( vkEndCommandBuffer( m_commandBuffers[i] ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to record command buffer" );
		}
	}

	void CreateSyncObjects()
	{
		// Resize the semaphore and fence vectors
		m_imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
		m_renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
		m_inFlightFences.resize( MAX_FRAMES_IN_FLIGHT );
		m_inFlightImages.resize( m_swapchainImages.size(), VK_NULL_HANDLE ); // Initialise to no fence

		// Setup the semaphore create information
		VkSemaphoreCreateInfo semaphoreCreateInfo {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Setup the fence create information
		VkFenceCreateInfo fenceCreateInfo {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start in the signaled state

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) // Create the semaphores for each frame
		{
			// Create the semaphores and the fence
			if ( vkCreateSemaphore( m_logicalDevice, &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i] ) != VK_SUCCESS ||
				 vkCreateSemaphore( m_logicalDevice, &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i] ) != VK_SUCCESS ||
				 vkCreateFence( m_logicalDevice, &fenceCreateInfo, nullptr, &m_inFlightFences[i] ) != VK_SUCCESS )
				throw std::runtime_error( "Failed to create syncronization objects for a frame" );
		}
	}

	void DrawFrame()
	{
		// Wait for the frame to be finished before accessing it again
		vkWaitForFences( m_logicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, (uint64_t)-1 );

		// Acquire the image from the swap chain (gets the index from the the swapchainImages array)
		uint32_t imageIndex;
		vkAcquireNextImageKHR( m_logicalDevice, m_swapchain, (uint64_t)-1, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex );

		// Wait on any frame that is using the assigned image
		if ( m_inFlightImages[imageIndex] != VK_NULL_HANDLE ) // Is in use
			vkWaitForFences( m_logicalDevice, 1, &m_inFlightImages[imageIndex], VK_TRUE, (uint64_t)-1 );

		// Mark the image as being used by this frame
		m_inFlightImages[imageIndex] = m_inFlightFences[m_currentFrame];

		// Which semaphores and stages to wait on before execution
		VkSemaphore			 waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[]	  = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		// Specify the signal semaphores
		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] }; // Semaphores to signal when the execution ends

		// Submit the command buffer
		VkSubmitInfo submitInfo {};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount	= 1;
		submitInfo.pWaitSemaphores		= waitSemaphores;
		submitInfo.pWaitDstStageMask	= waitStages;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &m_commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores	= signalSemaphores;

		// Reset fence to an unsignaled state
		vkResetFences( m_logicalDevice, 1, &m_inFlightFences[m_currentFrame] );

		// Submit the command buffer to the queue
		if ( vkQueueSubmit( m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame] ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to submit draw command buffer" );

		// Specify the swapchain
		VkSwapchainKHR swapchains[] = { m_swapchain };

		// Setup the present information
		VkPresentInfoKHR presentInfo {};
		presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores	   = signalSemaphores;
		presentInfo.swapchainCount	   = 1;
		presentInfo.pSwapchains		   = swapchains;
		presentInfo.pImageIndices	   = &imageIndex;
		presentInfo.pResults		   = nullptr;

		// Give the present image to the swap chain
		vkQueuePresentKHR( m_presentQueue, &presentInfo );

		// Increment the frames (Which of the in flight frames are being rendered)
		m_currentFrame = ( m_currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
	}

	void MainLoop()
	{
		while ( !glfwWindowShouldClose( m_window ) ) // Loop until the window is supposed to close
		{
			glfwPollEvents(); // Check for events and then call the correct callback

			// Draw the frame
			DrawFrame();
		}

		// Wait until the logical device has finished all operations
		vkDeviceWaitIdle( m_logicalDevice );
	}

	void Cleanup()
	{
		// Destroy the syncronization objects for all frames
		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			vkDestroySemaphore( m_logicalDevice, m_imageAvailableSemaphores[i], nullptr );
			vkDestroySemaphore( m_logicalDevice, m_renderFinishedSemaphores[i], nullptr );
			vkDestroyFence( m_logicalDevice, m_inFlightFences[i], nullptr );
		}

		// Destroy the command pool
		vkDestroyCommandPool( m_logicalDevice, m_commandPool, nullptr );

		// Destroy the framebuffers
		for ( const auto& framebuffer : m_swapchainFramebuffers )
			vkDestroyFramebuffer( m_logicalDevice, framebuffer, nullptr );

		// Destroy the graphics pipeline
		vkDestroyPipeline( m_logicalDevice, m_graphicsPipeline, nullptr );

		// Destroy the pipeline layout
		vkDestroyPipelineLayout( m_logicalDevice, m_pipelineLayout, nullptr );

		// Destroy the render pass
		vkDestroyRenderPass( m_logicalDevice, m_renderPass, nullptr );

		// Destroy the image views
		for ( const auto& imageView : m_swapchainImageViews )
			vkDestroyImageView( m_logicalDevice, imageView, nullptr );

		// Destroy the swap chain
		vkDestroySwapchainKHR( m_logicalDevice, m_swapchain, nullptr );

		// Destroy the logical device
		vkDestroyDevice( m_logicalDevice, nullptr );

		// Destroy debug messenger if it exists
		if ( ENABLE_VALIDATION_LAYERS )
			DestroyDebugUtilsMessengerEXT( m_instance, m_debugMessenger, nullptr );

		// Destroy the window surface
		vkDestroySurfaceKHR( m_instance, m_surface, nullptr );

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