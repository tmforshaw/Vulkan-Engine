#pragma once
#include "../Buffers/Buffers.hpp"
#include "../Buffers/CommandBuffer.hpp"
#include "../VulkanUtil/ImageView.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <stb_image.h>

static bool HasStencilComponent( const VkFormat& p_format )
{
	// If the format has a stencil component
	return p_format == VK_FORMAT_D32_SFLOAT_S8_UINT || p_format == VK_FORMAT_D24_UNORM_S8_UINT;
}

static void TransitionImageLayout( const VkDevice& p_logicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImage& p_image, const VkFormat& p_format, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout, const uint32_t& p_mipLevels )
{
	// Create a one-time command buffer
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands( p_logicalDevice, p_commandPool );

	// Declare the stage flags for the source and destination
	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// Create a memory barrier to transition the layout
	VkImageMemoryBarrier barrier {};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout						= p_oldLayout;
	barrier.newLayout						= p_newLayout;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= p_image;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= p_mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	// Set subresource range aspect mask according to the transition
	if ( p_newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
	{
		// Set aspect as depth
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		// Set aspect as stencil if it has a stencil component
		if ( HasStencilComponent( p_format ) ) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	// Set the barrier access mask according to the transition
	if ( p_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
	{
		// Set the masks
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		// Set the stages
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ( p_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
	{
		// Set the masks
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		// Set the stages
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if ( p_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
	{
		// Set the masks
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// Set the stages
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
		throw std::invalid_argument( "Unsupported layer transition" );

	// Submit the barrier
	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0, // VK_DEPENDENCY_BY_REGION_BIT, // (Consider this)
		0, nullptr,
		0, nullptr,
		1, &barrier );

	// End the command buffer recording and free the memory
	EndSingleTimeCommands( p_logicalDevice, p_graphicsQueue, p_commandPool, commandBuffer );
}

static void CopyBufferToImage( const VkDevice& p_logicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkBuffer& p_buffer, const VkImage& p_image, const uint32_t& p_width, const uint32_t& p_height )
{
	// Create a one-time command buffer
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands( p_logicalDevice, p_commandPool );

	// Specify the region to copy and to where
	VkBufferImageCopy region {};
	region.bufferOffset					   = 0;
	region.bufferRowLength				   = 0;
	region.bufferImageHeight			   = 0;
	region.imageSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel	   = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount	   = 1;
	region.imageOffset					   = { 0, 0, 0 };
	region.imageExtent					   = { p_width, p_height, 1 };

	// Queue the copy operation
	vkCmdCopyBufferToImage( commandBuffer, p_buffer, p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

	// End the command buffer recording and free the memory
	EndSingleTimeCommands( p_logicalDevice, p_graphicsQueue, p_commandPool, commandBuffer );
}

static void CreateImage( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const uint32_t& p_width, const uint32_t& p_height, const uint32_t& p_mipLevels, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, const VkSampleCountFlagBits& p_sampleCount, VkImage* p_image, VkDeviceMemory* p_imageMemory )
{
	// Setup the create information for the image
	VkImageCreateInfo imageCreateInfo {};
	imageCreateInfo.sType		  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType	  = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width  = p_width;
	imageCreateInfo.extent.height = p_height;
	imageCreateInfo.extent.depth  = 1;
	imageCreateInfo.mipLevels	  = p_mipLevels;
	imageCreateInfo.arrayLayers	  = 1;
	imageCreateInfo.format		  = p_format;
	imageCreateInfo.tiling		  = p_tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage		  = p_usage;
	imageCreateInfo.sharingMode	  = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.samples		  = p_sampleCount;
	imageCreateInfo.flags		  = 0;

	// Create the image
	if ( vkCreateImage( p_logicalDevice, &imageCreateInfo, nullptr, p_image ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create image" );

	// Get the memory requirements for the image
	VkMemoryRequirements memRequirements {};
	vkGetImageMemoryRequirements( p_logicalDevice, *p_image, &memRequirements );

	// Setup the allocation information for the image
	VkMemoryAllocateInfo imageAllocInfo {};
	imageAllocInfo.sType		   = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocInfo.allocationSize  = memRequirements.size;
	imageAllocInfo.memoryTypeIndex = FindMemoryType( p_physicalDevice, memRequirements.memoryTypeBits, p_properties );

	// Allocate memory for the image
	if ( vkAllocateMemory( p_logicalDevice, &imageAllocInfo, nullptr, p_imageMemory ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to allocate image memory" );

	// Bind the image memory
	vkBindImageMemory( p_logicalDevice, *p_image, *p_imageMemory, 0 );
}

static VkFormat FindSupportedFormat( const VkPhysicalDevice& p_physicalDevice, const std::vector<VkFormat>& p_candidates, const VkImageTiling& p_tiling, const VkFormatFeatureFlags& p_features )
{
	// Iterate the format candidates
	for ( const VkFormat& format : p_candidates )
	{
		// Get the properties of this format
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties( p_physicalDevice, format, &props );

		// Return a format based on the tiling features
		if ( ( p_tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & p_features ) == p_features ) ||	 // All requested features of linear tiling are supported
			 ( p_tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & p_features ) == p_features ) ) // All requested features of optimal tiling are supported
			return format;
	}

	// No format was found
	throw std::runtime_error( "Failed to find a supported format" );
}

static VkFormat FindDepthFormat( const VkPhysicalDevice& p_physicalDevice )
{
	// Return a format with the correct tiling and features
	return FindSupportedFormat(
		p_physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
}

class Image
{
protected:
	VkImage						 m_image;
	VkDeviceMemory				 m_imageMemory;
	std::shared_ptr<VkImageView> m_imageView;
	const VkDevice*				 m_logicalDevice;
	const VkFormat*				 m_format;

public:
	Image() : m_logicalDevice( nullptr ), m_format( nullptr ) {}

	void Init( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const uint32_t p_width, const uint32_t p_height, const uint32_t& p_mipLevels, const VkSampleCountFlagBits& p_sampleCount, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, const VkImageAspectFlags& p_aspectFlags )
	{
		// Set the member variables using the parameters
		m_logicalDevice = const_cast<VkDevice*>( &p_logicalDevice );
		m_format		= const_cast<VkFormat*>( &p_format );

		// Create the image
		CreateImage( *m_logicalDevice, p_physicalDevice, p_width, p_height, 1, *m_format, p_tiling, p_usage, p_properties, p_sampleCount, &m_image, &m_imageMemory );

		// Create image view
		m_imageView = std::make_shared<VkImageView>( CreateImageView( *m_logicalDevice, m_image, *m_format, p_aspectFlags, 1 ) );
	}

	virtual void TransitionLayout( const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout )
	{
		// Transition the layout of the image
		TransitionImageLayout( *m_logicalDevice, p_commandPool, p_graphicsQueue, m_image, *m_format, p_oldLayout, p_newLayout, 1 );
	}

	inline const VkImageView& GetImageView() const { return *m_imageView; }

	virtual void Cleanup()
	{
		// Destroy the image view
		vkDestroyImageView( *m_logicalDevice, *m_imageView, nullptr );

		// Destroy the pointer to the image view (This is a precautionary measure)
		m_imageView.reset();

		// Destroy the image and free its memory
		vkDestroyImage( *m_logicalDevice, m_image, nullptr );
		vkFreeMemory( *m_logicalDevice, m_imageMemory, nullptr );
	}
};