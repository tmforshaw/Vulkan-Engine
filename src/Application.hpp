#pragma once
#include "Buffers/Buffers.hpp"
#include "Buffers/UniformBuffers.hpp"
#include "Buffers/Vertex.hpp"
#include "Graphics/Images.hpp"
#include "Graphics/Models.hpp"
#include "Graphics/Shaders.hpp"
#include "VulkanUtil/DebugMessenger.hpp"
#include "VulkanUtil/DeviceAndExtensions.hpp"
#include "VulkanUtil/ImageView.hpp"
#include "VulkanUtil/QueueFamilies.hpp"
#include "VulkanUtil/Swapchain.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // Ensure the data is correctly aligned
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

// Constants
const uint16_t WINDOW_WIDTH	 = 700;
const uint16_t WINDOW_HEIGHT = 700;

const std::string MODEL_PATH   = "resources/models/viking_room.obj";
const std::string TEXTURE_PATH = "resources/textures/viking_room.png";

#define MAX_FRAMES_IN_FLIGHT 2	   // Maximum number of frames to process concurrently
#define FOV					 45.0f // The camera field of view

class Application
{
private:
	GLFWwindow*					 m_window;
	VkInstance					 m_instance;
	VkDebugUtilsMessengerEXT	 m_debugMessenger;
	VkPhysicalDevice			 m_physicalDevice;
	VkPhysicalDeviceProperties	 m_physicalDeviceProperties;
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
	VkDescriptorSetLayout		 m_descriptorSetLayout;
	VkDescriptorPool			 m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;
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
	Model						 m_environmentModel;
	VkBuffer					 m_vertexBuffer;
	VkDeviceMemory				 m_vertexBufferMemory;
	VkBuffer					 m_indexBuffer;
	VkDeviceMemory				 m_indexBufferMemory;
	std::vector<VkBuffer>		 m_uniformBuffers;
	std::vector<VkDeviceMemory>	 m_uniformBuffersMemory;
	Image						 m_textureImage;
	VkSampler					 m_textureSampler;
	Image						 m_depthImage;

	bool m_framebufferResized;

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

		// Initialise the swapchain
		CreateSwapchain();

		// Create image views for the swapchain images
		CreateImageViews();

		// Create a render pass
		CreateRenderPass();

		// Create the descriptor set layout
		CreateDescriptorSetLayout();

		// Create the graphics pipeline
		CreateGraphicsPipeline();

		// Create the command pool
		CreateCommandPool();

		// Create the depth buffer resources
		CreateDepthResources();

		// Create the framebuffers
		CreateFramebuffers();

		// Create a texture
		CreateTextureImage();

		// Create an texture sampler
		CreateTextureSampler();

		// Load the model
		m_environmentModel.LoadFromFile( MODEL_PATH.c_str() );

		// Create a vertex buffer
		CreateVertexBuffer();

		// Create an index buffer
		CreateIndexBuffer();

		// Create the uniform buffers
		CreateUniformBuffers();

		// Create a descriptor pool
		CreateDescriptorPool();

		// Create the descriptor sets
		CreateDescriptorSets();

		// Create the command buffers
		CreateCommandBuffers();

		// Set the current frame to zero
		m_currentFrame = 0;

		// Create the semaphores and fences
		CreateSyncObjects();
	}

	void InitWindow()
	{
		// Setup a GLFW window
		glfwInit();

		// Don't create an OpenGL context
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

		// Set the window variable
		m_window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr );

		// State that the window hasn't been resized yet
		m_framebufferResized = false;

		// Get the window to point to this application
		glfwSetWindowUserPointer( m_window, this );

		// Create a window resize callback
		glfwSetFramebufferSizeCallback( m_window, FramebufferResizeCallback );
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

	static void FramebufferResizeCallback( GLFWwindow* p_window, int width, int height )
	{
		// Get the app from the window and set the framebufferResized variable
		auto app				  = reinterpret_cast<Application*>( glfwGetWindowUserPointer( p_window ) );
		app->m_framebufferResized = true;
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
			if ( IsDeviceSuitable( device, m_surface ) )
			{
				m_physicalDevice = device;
				break;
			}
		}

		// Check if a device was found
		if ( m_physicalDevice == VK_NULL_HANDLE )
			throw std::runtime_error( "Failed to find a suitable GPU" ); // Device wasn't found

		// Query the device properties
		vkGetPhysicalDeviceProperties( m_physicalDevice, &m_physicalDeviceProperties );
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;

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

	void CreateSwapchain()
	{
		// Get the swapchain support details
		SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport( m_physicalDevice, m_surface );

		// Pick the best properties for the swapchain from the available settings
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( swapchainSupport.formats );
		VkPresentModeKHR   presentMode	 = ChooseSwapPresentMode( swapchainSupport.presentModes );
		VkExtent2D		   extent		 = ChooseSwapExtent( swapchainSupport.capabilities, m_window );

		// Specify the minimum number of images for the swapchain to function
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

		// Ensure the image count doesn't exceed the maximum number allowed (If it is zero there is no maximum)
		if ( swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount )
			imageCount = swapchainSupport.capabilities.maxImageCount;

		// Setup the create info for the swapchain
		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface			= m_surface;
		createInfo.minImageCount	= imageCount;
		createInfo.imageFormat		= surfaceFormat.format;
		createInfo.imageColorSpace	= surfaceFormat.colorSpace;
		createInfo.imageExtent		= extent;
		createInfo.imageArrayLayers = 1;								   // Number of layers that each image consists of (useful for stereoscopic rendering)
		createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // The way in which the swapchain is used

		// Define how the swapchain handles images across multiple queue families
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

		// The swapchain requires a reference to the old swapchain incase it needs recreated
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Create the swapchain
		if ( vkCreateSwapchainKHR( m_logicalDevice, &createInfo, nullptr, &m_swapchain ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create swapchain" );

		// Retrieve the swapchain images
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

		// Iterate over the swapchain images and create an image view
		for ( size_t i = 0; i < m_swapchainImages.size(); i++ )
			m_swapchainImageViews[i] = CreateImageView( m_logicalDevice, m_swapchainImages[i], m_swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1 );
	}

	void CreateRenderPass()
	{
		// Set colour attachment settings
		VkAttachmentDescription colourAttatchment {};
		colourAttatchment.format		 = m_swapchainImageFormat;
		colourAttatchment.samples		 = VK_SAMPLE_COUNT_1_BIT;
		colourAttatchment.loadOp		 = VK_ATTACHMENT_LOAD_OP_CLEAR;	 // Clear the framebuffer to black each frame
		colourAttatchment.storeOp		 = VK_ATTACHMENT_STORE_OP_STORE; // Store the contents into memory
		colourAttatchment.stencilLoadOp	 = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttatchment.initialLayout	 = VK_IMAGE_LAYOUT_UNDEFINED;
		colourAttatchment.finalLayout	 = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Set colour attachment settings
		VkAttachmentDescription depthAttachment {};
		depthAttachment.format		   = FindDepthFormat( m_physicalDevice );
		depthAttachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp		   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout	   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Setup the colour attachment reference
		VkAttachmentReference colourAttatchmentRef {};
		colourAttatchmentRef.attachment = 0;										// Which attachment to reference (by index in attachment descriptions array)
		colourAttatchmentRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout for the subpass

		// Setup the colour attachment reference
		VkAttachmentReference depthAttachmentRef {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout	  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Create a subpass description
		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount	= 1; // Index of colour attachment
		subpass.pColorAttachments		= &colourAttatchmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// Set a dependency for the image to be acquired before the subpass starts
		VkSubpassDependency dependency {};
		dependency.srcSubpass	 = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass	 = 0;
		dependency.srcStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Wait on this operation
		dependency.srcAccessMask = 0;
		dependency.dstStageMask	 = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // The operations that should wait
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;		   // The operations that should wait

		// Create an array of attachment descriptions
		std::array<VkAttachmentDescription, 2> attachments = { colourAttatchment, depthAttachment };

		// Set the render pass create info
		VkRenderPassCreateInfo renderPassCreateInfo {};
		renderPassCreateInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = static_cast<uint32_t>( attachments.size() );
		renderPassCreateInfo.pAttachments	 = attachments.data();
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

			// Create an array with the shader stage information
			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			// Get the vertex binding and attribute descriptions
			auto bindingDescription	   = Vertex::GetBindingDescription();
			auto attributeDescriptions = Vertex::GetAttributeDescriptions();

			// Setup structure of the vertex data using create information
			VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
			vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount	= 1;
			vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescriptions.size() );
			vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();

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
			rasteriserCreateInfo.cullMode				 = 0;								// Cull back faces
			rasteriserCreateInfo.frontFace				 = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Anti-clockwise because of the flipped y axis
			rasteriserCreateInfo.depthBiasEnable		 = VK_FALSE;						// Enable alteration of depth values
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

			VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo {};
			depthStencilCreateInfo.sType				 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilCreateInfo.depthTestEnable		 = VK_TRUE;
			depthStencilCreateInfo.depthWriteEnable		 = VK_TRUE;
			depthStencilCreateInfo.depthCompareOp		 = VK_COMPARE_OP_LESS;
			depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilCreateInfo.minDepthBounds		 = 0.0f;
			depthStencilCreateInfo.maxDepthBounds		 = 1.0f;
			depthStencilCreateInfo.stencilTestEnable	 = VK_FALSE;
			depthStencilCreateInfo.front				 = {};
			depthStencilCreateInfo.back					 = {};

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
			pipelineLayoutCreateInfo.setLayoutCount			= 1;
			pipelineLayoutCreateInfo.pSetLayouts			= &m_descriptorSetLayout;
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
			graphicsPipelineCreateInfo.pDepthStencilState  = &depthStencilCreateInfo;
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
		// Resize the framebuffer vector to hold all the framebuffers
		m_swapchainFramebuffers.resize( m_swapchainImageViews.size() );

		// Create framebuffers from the image views
		for ( size_t i = 0; i < m_swapchainImageViews.size(); i++ )
		{
			// Create an image view array
			std::array<VkImageView, 2> attachments = { m_swapchainImageViews[i], m_depthImage.GetImageView() };

			// Setup the framebuffer create information
			VkFramebufferCreateInfo framebufferCreateInfo {};
			framebufferCreateInfo.sType			  = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass	  = m_renderPass;
			framebufferCreateInfo.attachmentCount = static_cast<uint32_t>( attachments.size() );
			framebufferCreateInfo.pAttachments	  = attachments.data();
			framebufferCreateInfo.width			  = m_swapchainExtent.width;
			framebufferCreateInfo.height		  = m_swapchainExtent.height;
			framebufferCreateInfo.layers		  = 1;

			// Create the framebuffer
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

		// Create an array of clear values
		std::array<VkClearValue, 2> clearValues {};
		clearValues[0].color		= { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

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
				throw std::runtime_error( "Failed to begin recording to command buffer[" + std::to_string( i ) + "]" );

			// Setup the begin informatio for the render pass
			VkRenderPassBeginInfo renderPassBeginInfo {};
			renderPassBeginInfo.sType			  = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass		  = m_renderPass;
			renderPassBeginInfo.framebuffer		  = m_swapchainFramebuffers[i];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
			renderPassBeginInfo.clearValueCount	  = static_cast<uint32_t>( clearValues.size() );
			renderPassBeginInfo.pClearValues	  = clearValues.data();

			// Record the beginning of a render pass
			vkCmdBeginRenderPass( m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			// Record the binding of the graphics pipeline
			vkCmdBindPipeline( m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline );

			// Bind the vertex buffers
			VkBuffer	 vertexBuffers[] = { m_vertexBuffer };
			VkDeviceSize offsets[]		 = { 0 };
			vkCmdBindVertexBuffers( m_commandBuffers[i], 0, 1, vertexBuffers, offsets );

			// Bind the index buffers
			vkCmdBindIndexBuffer( m_commandBuffers[i], m_indexBuffer, 0, INDEX_BUFFER_TYPE );

			// Bind the descriptor sets
			vkCmdBindDescriptorSets( m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr );

			// Record the drawing of the triangle
			vkCmdDrawIndexed( m_commandBuffers[i], static_cast<uint32_t>( m_environmentModel.GetIndices().size() ), 1, 0, 0, 0 );

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
				throw std::runtime_error( "Failed to create syncronisation objects for a frame" );
		}
	}

	void CreateVertexBuffer()
	{
		// Get the vertices from the model
		auto vertices = m_environmentModel.GetVertices();

		// Create a vertex buffer using a staging buffer
		CreateBufferViaStagingBuffer( m_logicalDevice, m_physicalDevice, m_commandPool, m_graphicsQueue, vertices.size() * sizeof( Vertex ), vertices.data(),
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_vertexBuffer, &m_vertexBufferMemory );
	}

	void CreateIndexBuffer()
	{
		// Get the indices from the model
		auto indices = m_environmentModel.GetIndices();

		// Create an index buffer using a staging buffer
		CreateBufferViaStagingBuffer( m_logicalDevice, m_physicalDevice, m_commandPool, m_graphicsQueue, indices.size() * sizeof( indices[0] ), indices.data(),
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_indexBuffer, &m_indexBufferMemory );
	}

	void CreateDescriptorSetLayout()
	{
		// Setup the descriptor set layout binding for the model view projection matrix
		VkDescriptorSetLayoutBinding uboLayoutBinding {};
		uboLayoutBinding.binding			= 0;
		uboLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount	= 1;
		uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		// Setup the descriptor set layout binding
		VkDescriptorSetLayoutBinding samplerLayoutBinding {};
		samplerLayoutBinding.binding			= 1;
		samplerLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount	= 1;
		samplerLayoutBinding.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		// Create an array of bindings
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		// Setup the descriptor set layout create information
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo {};
		layoutCreateInfo.sType		  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<uint32_t>( bindings.size() );
		layoutCreateInfo.pBindings	  = bindings.data();

		// Create the descriptor set layout
		if ( vkCreateDescriptorSetLayout( m_logicalDevice, &layoutCreateInfo, nullptr, &m_descriptorSetLayout ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create descriptor set layout" );
	}

	void CreateUniformBuffers()
	{
		// Resize the buffer and memory vectors
		VkDeviceSize bufferSize = sizeof( UniformBufferObject );
		m_uniformBuffers.resize( m_swapchainImages.size() );
		m_uniformBuffersMemory.resize( m_swapchainImages.size() );

		// Create a buffer for each swapchain image
		for ( size_t i = 0; i < m_swapchainImages.size(); i++ )
			CreateBuffer( m_logicalDevice, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_uniformBuffers[i], &m_uniformBuffersMemory[i] );
	}

	void CreateDescriptorPool()
	{
		// Setup the descriptor pool sizes
		std::array<VkDescriptorPoolSize, 2> poolSizes {};
		poolSizes[0].type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>( m_swapchainImages.size() );
		poolSizes[1].type			 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>( m_swapchainImages.size() );

		// Setup the create information for the decriptor pool
		VkDescriptorPoolCreateInfo poolCreateInfo {};
		poolCreateInfo.sType		 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
		poolCreateInfo.pPoolSizes	 = poolSizes.data();
		poolCreateInfo.maxSets		 = static_cast<uint32_t>( m_swapchainImages.size() );
		poolCreateInfo.flags		 = 0;

		// Create the descriptor pool
		if ( vkCreateDescriptorPool( m_logicalDevice, &poolCreateInfo, nullptr, &m_descriptorPool ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create descriptor pool" );
	}

	void CreateDescriptorSets()
	{
		// Create a vector of of descriptor set layouts and fill with m_descriptorSetLayout
		std::vector<VkDescriptorSetLayout> layouts( m_swapchainImages.size(), m_descriptorSetLayout );

		// Setup the allocation information for the descriptor sets
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo {};
		descriptorSetAllocInfo.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool	  = m_descriptorPool;
		descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>( m_swapchainImages.size() );
		descriptorSetAllocInfo.pSetLayouts		  = layouts.data();

		// Resize the descriptor set vector
		m_descriptorSets.resize( m_swapchainImages.size() );

		// Create the descriptor sets
		if ( vkAllocateDescriptorSets( m_logicalDevice, &descriptorSetAllocInfo, m_descriptorSets.data() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate descriptor sets" );

		// Populate the descriptor sets
		for ( size_t i = 0; i < m_swapchainImages.size(); i++ )
		{
			// Configure the descriptors using buffer information
			VkDescriptorBufferInfo bufferInfo {};
			bufferInfo.buffer = m_uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range  = sizeof( UniformBufferObject );

			// Configure the descriptors using image information
			VkDescriptorImageInfo imageInfo {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView	  = m_textureImage.GetImageView();
			imageInfo.sampler	  = m_textureSampler;

			// Create an array of the write descriptor sets
			std::array<VkWriteDescriptorSet, 2> descriptorsWrites {};

			// Configure the uniform buffer write descriptor set
			descriptorsWrites[0].sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrites[0].dstSet			  = m_descriptorSets[i];
			descriptorsWrites[0].dstBinding		  = 0;
			descriptorsWrites[0].dstArrayElement  = 0;
			descriptorsWrites[0].descriptorType	  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorsWrites[0].descriptorCount  = 1;
			descriptorsWrites[0].pBufferInfo	  = &bufferInfo;
			descriptorsWrites[0].pImageInfo		  = nullptr;
			descriptorsWrites[0].pTexelBufferView = nullptr;

			// Configure the sampler write descriptor set
			descriptorsWrites[1].sType			 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrites[1].dstSet			 = m_descriptorSets[i];
			descriptorsWrites[1].dstBinding		 = 1;
			descriptorsWrites[1].dstArrayElement = 0;
			descriptorsWrites[1].descriptorType	 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorsWrites[1].descriptorCount = 1;
			descriptorsWrites[1].pImageInfo		 = &imageInfo;

			// Update the descriptor sets
			vkUpdateDescriptorSets( m_logicalDevice, static_cast<uint32_t>( descriptorsWrites.size() ), descriptorsWrites.data(), 0, nullptr );
		}
	}

	void CreateTextureImage()
	{
		// Initialise an Image object from an image file using the correct parameters
		m_textureImage.InitFromFile( m_logicalDevice, m_physicalDevice, m_commandPool, m_graphicsQueue, TEXTURE_PATH.c_str(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
									 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT );
	}

	void CreateTextureSampler()
	{
		// Setup the create information for the texture sampler
		VkSamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.sType					  = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter				  = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter				  = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU			  = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerCreateInfo.addressModeV			  = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerCreateInfo.addressModeW			  = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerCreateInfo.anisotropyEnable		  = VK_TRUE;
		samplerCreateInfo.maxAnisotropy			  = m_physicalDeviceProperties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.borderColor			  = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable			  = VK_FALSE;
		samplerCreateInfo.compareOp				  = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode			  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias			  = 0.0f;
		samplerCreateInfo.minLod				  = 0.0f;
		samplerCreateInfo.maxLod				  = static_cast<float>( m_textureImage.GetMipLevels() );

		// Create the texture sampler
		if ( vkCreateSampler( m_logicalDevice, &samplerCreateInfo, nullptr, &m_textureSampler ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create texture sampler" );
	}

	void CreateDepthResources()
	{
		// Find a suitable depth format
		VkFormat depthFormat = FindDepthFormat( m_physicalDevice );

		// Initialise an Image object using the correct parameters
		m_depthImage.Init( m_logicalDevice, m_physicalDevice, m_commandPool, m_graphicsQueue, m_swapchainExtent.width, m_swapchainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL,
						   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						   VK_IMAGE_ASPECT_DEPTH_BIT );
	}

	void RecreateSwapchain()
	{
		// Get the width and height of the framebuffer
		int width = 0, height = 0;
		// glfwGetFramebufferSize( m_window, &width, &height );

		// Wait until the framebuffer has a size
		while ( width == 0 || height == 0 )
		{
			// Get the width and height of the framebuffer
			glfwGetFramebufferSize( m_window, &width, &height );
			glfwWaitEvents(); // Wait until there is a GLFW event
		}

		// Wait for the logical device has completed its operations
		vkDeviceWaitIdle( m_logicalDevice );

		// Destroy all the old versions
		CleanupSwapchain();

		// Call the creation functions to recreate the swapchain and all dependencies of it
		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline(); // This can be avoided by using dynamic states for the scissor and viewport
		CreateDepthResources();
		CreateFramebuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
	}

	void DrawFrame()
	{
		// Wait for the frame to be finished before accessing it again
		vkWaitForFences( m_logicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, (uint64_t)-1 );

		// Acquire the image from the swapchain (gets the index from the the swapchainImages array)
		// And recreate the swapchain if it is out of date
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR( m_logicalDevice, m_swapchain, (uint64_t)-1, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex );
		if ( result == VK_ERROR_OUT_OF_DATE_KHR )
		{
			RecreateSwapchain();
			return;
		}
		else if ( !( result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ) )
			throw std::runtime_error( "Failed to acquire swapchain image" );

		// Update the uniform buffer
		UpdateUniformBuffer( imageIndex );

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

		// Give the present image to the swapchain and recreate swapchain if it is out of date
		result = vkQueuePresentKHR( m_presentQueue, &presentInfo );
		if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized )
		{
			m_framebufferResized = false;
			RecreateSwapchain();
			return;
		}
		else if ( result != VK_SUCCESS )
			throw std::runtime_error( "Failed to present swapchain image" );

		// Increment the frames (Which of the in flight frames are being rendered)
		m_currentFrame = ( m_currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
	}

	void UpdateUniformBuffer( const uint32_t& currentImage )
	{
		// Set the current time at the start of the function
		static auto startTime = std::chrono::high_resolution_clock::now();

		// Set the current time at the end of the function
		auto currentTime = std::chrono::high_resolution_clock::now();

		// Get the elapsed time
		float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

		// Set the uniform buffer object
		UniformBufferObject ubo {};
		ubo.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 45.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
		ubo.view  = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
		ubo.proj  = glm::perspective( glm::radians( FOV ), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, 10.0f );

		// Flip the y axis of the projection matrix
		ubo.proj[1][1] *= -1;

		// Copy the data into the uniform buffer
		void* mappedMemPtr;
		vkMapMemory( m_logicalDevice, m_uniformBuffersMemory[currentImage], 0, sizeof( ubo ), 0, &mappedMemPtr );
		memcpy( mappedMemPtr, &ubo, sizeof( ubo ) );
		vkUnmapMemory( m_logicalDevice, m_uniformBuffersMemory[currentImage] );
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

	void CleanupSwapchain()
	{
		// Destroy the depth buffer image
		m_depthImage.~Image();

		// Destroy the framebuffers
		for ( const auto& framebuffer : m_swapchainFramebuffers )
			vkDestroyFramebuffer( m_logicalDevice, framebuffer, nullptr );

		// Destroy the command buffers (As opposed to destroying the command pool)
		vkFreeCommandBuffers( m_logicalDevice, m_commandPool, static_cast<uint32_t>( m_commandBuffers.size() ), m_commandBuffers.data() );

		// Destroy the graphics pipeline
		vkDestroyPipeline( m_logicalDevice, m_graphicsPipeline, nullptr );

		// Destroy the pipeline layout
		vkDestroyPipelineLayout( m_logicalDevice, m_pipelineLayout, nullptr );

		// Destroy the render pass
		vkDestroyRenderPass( m_logicalDevice, m_renderPass, nullptr );

		// Destroy the image views
		for ( const auto& imageView : m_swapchainImageViews )
			vkDestroyImageView( m_logicalDevice, imageView, nullptr );

		// Destroy the swapchain
		vkDestroySwapchainKHR( m_logicalDevice, m_swapchain, nullptr );

		// Destroy the uniform buffers and free the memory
		for ( size_t i = 0; i < m_swapchainImages.size(); i++ )
		{
			vkDestroyBuffer( m_logicalDevice, m_uniformBuffers[i], nullptr );
			vkFreeMemory( m_logicalDevice, m_uniformBuffersMemory[i], nullptr );
		}

		// Destroy the descriptor pool
		vkDestroyDescriptorPool( m_logicalDevice, m_descriptorPool, nullptr );
	}

	void Cleanup()
	{
		// Destroy the swapchain and all dependencies
		CleanupSwapchain();

		// Destroy the texture sampler
		vkDestroySampler( m_logicalDevice, m_textureSampler, nullptr );

		// Destroy the texture image
		m_textureImage.~Image();

		// Destroy the descriptor set layout
		vkDestroyDescriptorSetLayout( m_logicalDevice, m_descriptorSetLayout, nullptr );

		// Destroy the vertex buffer and free its memory
		vkDestroyBuffer( m_logicalDevice, m_vertexBuffer, nullptr );
		vkFreeMemory( m_logicalDevice, m_vertexBufferMemory, nullptr );

		// Destroy the index buffer an free its memory
		vkDestroyBuffer( m_logicalDevice, m_indexBuffer, nullptr );
		vkFreeMemory( m_logicalDevice, m_indexBufferMemory, nullptr );

		// Destroy the syncronisation objects for all frames
		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			vkDestroySemaphore( m_logicalDevice, m_imageAvailableSemaphores[i], nullptr );
			vkDestroySemaphore( m_logicalDevice, m_renderFinishedSemaphores[i], nullptr );
			vkDestroyFence( m_logicalDevice, m_inFlightFences[i], nullptr );
		}

		// Destroy the command pool
		vkDestroyCommandPool( m_logicalDevice, m_commandPool, nullptr );

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
