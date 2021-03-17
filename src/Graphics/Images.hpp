#pragma once
#include "../Buffers/Buffers.hpp"
#include "../Buffers/CommandBuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <USER/stb_image.h>

static void TransitionImageLayout( const VkDevice& p_logicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImage& p_image, const VkFormat& p_format, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout )
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
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	// Set the barrier according to the transition
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

static void CreateImage( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const uint32_t& p_width, const uint32_t& p_height, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, VkImage* p_image, VkDeviceMemory* p_imageMemory )
{
	// Setup the create information for the image
	VkImageCreateInfo imageCreateInfo {};
	imageCreateInfo.sType		  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType	  = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width  = p_width;
	imageCreateInfo.extent.height = p_height;
	imageCreateInfo.extent.depth  = 1;
	imageCreateInfo.mipLevels	  = 1;
	imageCreateInfo.arrayLayers	  = 1;
	imageCreateInfo.format		  = p_format;
	imageCreateInfo.tiling		  = p_tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage		  = p_usage;
	imageCreateInfo.sharingMode	  = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.samples		  = VK_SAMPLE_COUNT_1_BIT;
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