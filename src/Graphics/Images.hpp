#pragma once
#include "../Buffers/Buffers.hpp"
#include "../Buffers/CommandBuffer.hpp"
#include "../VulkanUtil/ImageView.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

static void CreateImage( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const uint32_t& p_width, const uint32_t& p_height, const uint32_t& p_mipLevels, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, VkImage* p_image, VkDeviceMemory* p_imageMemory )
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

static void GenerateMipmaps( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImage& p_image, const VkFormat& p_format, const uint32_t& p_width, const uint32_t& p_height, const uint32_t& p_mipLevels )
{
	// Check if the image format allows linear filtering
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties( p_physicalDevice, p_format, &formatProperties );

	if ( !( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ) )
		throw std::runtime_error( "Image format does not support linear blitting" );

	// Start a command buffer
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands( p_logicalDevice, p_commandPool );

	// Create a reusable barrier
	VkImageMemoryBarrier barrier {};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image							= p_image;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;
	barrier.subresourceRange.levelCount		= 1;

	// Setup the width of each mip level
	int32_t mipWidth  = (int32_t)p_width;
	int32_t mipHeight = (int32_t)p_height;

	// Iterate the mip levels to record the VkCmdBlitImage commands
	for ( uint32_t i = 1; i < p_mipLevels; i++ )
	{
		// Modify the barrier to transition from dst to src
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout					  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout					  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask				  = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask				  = VK_ACCESS_TRANSFER_READ_BIT;

		// Record the barrier to wait for the i-1 level to be filled
		vkCmdPipelineBarrier( commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							  0, nullptr,
							  0, nullptr,
							  1, &barrier );

		// Create the blit
		VkImageBlit blit {};
		blit.srcOffsets[0]				   = { 0, 0, 0 };
		blit.srcOffsets[1]				   = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel	   = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount	   = 1;
		blit.dstOffsets[0]				   = { 0, 0, 0 };
		blit.dstOffsets[1]				   = { mipWidth > 1 ? ( mipWidth / 2 ) : 1, mipHeight > 1 ? ( mipHeight / 2 ) : 1, 1 };
		blit.dstSubresource.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel	   = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount	   = 1;

		// Record the blit command
		vkCmdBlitImage( commandBuffer,
						p_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1, &blit,
						VK_FILTER_LINEAR );

		// Modify the barrier again to transition from src to shader optimal
		barrier.oldLayout	  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout	  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		// Record the barrier transition
		vkCmdPipelineBarrier( commandBuffer,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							  0, nullptr,
							  0, nullptr,
							  1, &barrier );

		// Half the mip dimensions
		if ( mipWidth > 1 ) mipWidth /= 2;
		if ( mipHeight > 1 ) mipHeight /= 2;
	}

	// Modify the barrier again to transition the final mip level to shader optimal
	barrier.subresourceRange.baseMipLevel = p_mipLevels - 1;
	barrier.oldLayout					  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout					  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask				  = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask				  = VK_ACCESS_SHADER_READ_BIT;

	// Record the barrier
	vkCmdPipelineBarrier( commandBuffer,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						  0, nullptr,
						  0, nullptr,
						  1, &barrier );

	// End the command buffer
	EndSingleTimeCommands( p_logicalDevice, p_graphicsQueue, p_commandPool, commandBuffer );
}

class Image
{
private:
	VkImage		   m_image;
	VkDeviceMemory m_imageMemory;
	VkImageView	   m_imageView;
	uint32_t	   m_mipLevels;
	VkDevice	   m_logicalDevice;

public:
	void Init( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
			   const uint32_t p_width, const uint32_t p_height, const uint32_t& p_mipLevels, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage,
			   const VkMemoryPropertyFlags& p_properties, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout, const VkImageAspectFlags& p_aspectFlags )
	{
		// Set the member variables using the parameters
		m_logicalDevice = p_logicalDevice;
		m_mipLevels		= p_mipLevels;

		// Create the image
		CreateImage( m_logicalDevice, p_physicalDevice, p_width, p_height, m_mipLevels, p_format, p_tiling, p_usage, p_properties, &m_image, &m_imageMemory );

		// Transition the layout of the image to an optimal layout
		TransitionImageLayout( m_logicalDevice, p_commandPool, p_graphicsQueue, m_image, p_format, p_oldLayout, p_newLayout, m_mipLevels );

		// Modify the generate mipmaps function to allow multiple image layouts

		// GenerateMipmaps( m_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, m_image, p_format, texWidth, texHeight, m_mipLevels );

		// Create image view
		m_imageView = CreateImageView( m_logicalDevice, m_image, p_format, p_aspectFlags, m_mipLevels );
	}

	void InitFromFile( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
					   const char* path, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
					   const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout, const VkImageLayout& p_finalLayout, const VkImageAspectFlags& p_aspectFlags )
	{
		// Set the member variables using the parameters
		m_logicalDevice = p_logicalDevice;

		// Get the pixels
		int		 texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load( path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

		// Calculate the number of mip levels
		m_mipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max( texWidth, texHeight ) ) ) ) + 1;

		// Get the size of the image
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		// Throw an error if the image wasn't loaded
		if ( !pixels )
			throw std::runtime_error( "Failed to load image" );

		// Create a staging buffer and some memory for the image
		VkBuffer	   stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer( m_logicalDevice, p_physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );

		// Copy the data (Map memory to CPU accessible memory, copy, un-map CPU accessible memory)
		void* mappedMemPtr;
		vkMapMemory( m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &mappedMemPtr );
		memcpy( mappedMemPtr, pixels, static_cast<uint32_t>( imageSize ) );
		vkUnmapMemory( m_logicalDevice, stagingBufferMemory );

		// Free the original pixel array
		stbi_image_free( pixels );

		// Create the image
		CreateImage( m_logicalDevice, p_physicalDevice, texWidth, texHeight, m_mipLevels, p_format, p_tiling, p_usage, p_properties, &m_image, &m_imageMemory );

		// Transition the layout to the first format
		TransitionImageLayout( m_logicalDevice, p_commandPool, p_graphicsQueue, m_image, p_format, p_oldLayout, p_newLayout, m_mipLevels );

		// Copy the buffer to the image
		CopyBufferToImage( m_logicalDevice, p_commandPool, p_graphicsQueue, stagingBuffer, m_image, static_cast<uint32_t>( texWidth ), static_cast<uint32_t>( texHeight ) );

		// // Transition the layout to the second format
		// TransitionImageLayout( m_logicalDevice, p_commandPool, p_graphicsQueue, m_image, p_format, p_newLayout, p_finalLayout, m_mipLevels );

		GenerateMipmaps( m_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, m_image, p_format, texWidth, texHeight, m_mipLevels );

		// Destroy the staging buffer and free its memory
		vkDestroyBuffer( m_logicalDevice, stagingBuffer, nullptr );
		vkFreeMemory( m_logicalDevice, stagingBufferMemory, nullptr );

		// Create image view
		m_imageView = CreateImageView( m_logicalDevice, m_image, p_format, p_aspectFlags, m_mipLevels );
	}

	inline const VkImageView& GetImageView() const { return m_imageView; }
	inline const uint32_t&	  GetMipLevels() const { return m_mipLevels; }

	~Image()
	{
		// Destroy the image view
		vkDestroyImageView( m_logicalDevice, m_imageView, nullptr );

		// Destroy the image and free its memory
		vkDestroyImage( m_logicalDevice, m_image, nullptr );
		vkFreeMemory( m_logicalDevice, m_imageMemory, nullptr );
	}
};