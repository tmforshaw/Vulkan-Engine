#pragma once
#include "Images.hpp"

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

class Texture : public Image
{
private:
	uint32_t  m_mipLevels;
	VkSampler m_sampler;
	uint32_t  m_samplerID;

public:
	void
	Init( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const uint32_t p_width, const uint32_t p_height, const uint32_t& p_mipLevels,
		  const VkSampleCountFlagBits& p_sampleCount, const VkFormat& p_format, const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage,
		  const VkMemoryPropertyFlags& p_properties, const VkImageAspectFlags& p_aspectFlags ) = delete;

	void Init( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue,
			   const VkPhysicalDeviceProperties& p_physicalDeviceProperties, const char* path, const VkSampleCountFlagBits& p_sampleCount, const VkFormat& p_format,
			   const VkImageTiling& p_tiling, const VkImageUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties,
			   const VkImageAspectFlags& p_aspectFlags, const uint32_t& p_samplerID )
	{
		// Set the member variables using the parameters
		m_logicalDevice = const_cast<VkDevice*>( &p_logicalDevice );
		m_format		= const_cast<VkFormat*>( &p_format );
		m_samplerID		= p_samplerID;

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
		CreateBuffer( *m_logicalDevice, p_physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );

		// Copy the data (Map memory to CPU accessible memory, copy, un-map CPU accessible memory)
		void* mappedMemPtr;
		vkMapMemory( *m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &mappedMemPtr );
		memcpy( mappedMemPtr, pixels, static_cast<uint32_t>( imageSize ) );
		vkUnmapMemory( *m_logicalDevice, stagingBufferMemory );

		// Free the original pixel array
		stbi_image_free( pixels );

		// Create the image
		CreateImage( *m_logicalDevice, p_physicalDevice, texWidth, texHeight, m_mipLevels, p_format, p_tiling, p_usage, p_properties, p_sampleCount, &m_image, &m_imageMemory );

		// Transition the layout to the first format
		TransitionLayout( p_commandPool, p_graphicsQueue, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

		// Copy the buffer to the image
		CopyBufferToImage( *m_logicalDevice, p_commandPool, p_graphicsQueue, stagingBuffer, m_image, static_cast<uint32_t>( texWidth ), static_cast<uint32_t>( texHeight ) );

		if ( m_mipLevels > 1 )
		{
			// Generate the mipmaps for the image (LOD)
			GenerateMipmaps( *m_logicalDevice, p_physicalDevice, p_commandPool, p_graphicsQueue, m_image, p_format, texWidth, texHeight, m_mipLevels );
		}

		// Destroy the staging buffer and free its memory
		vkDestroyBuffer( *m_logicalDevice, stagingBuffer, nullptr );
		vkFreeMemory( *m_logicalDevice, stagingBufferMemory, nullptr );

		// Create image view
		m_imageView = std::make_unique<VkImageView>( CreateImageView( *m_logicalDevice, m_image, p_format, p_aspectFlags, m_mipLevels ) );

		// Generate the texture sampler
		CreateSampler( p_physicalDeviceProperties );
	}

	void TransitionLayout( const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkImageLayout& p_oldLayout, const VkImageLayout& p_newLayout ) override
	{
		// Transition the layout of the image
		TransitionImageLayout( *m_logicalDevice, p_commandPool, p_graphicsQueue, m_image, *m_format, p_oldLayout, p_newLayout, m_mipLevels );
	}

	void CreateSampler( const VkPhysicalDeviceProperties& p_physicalDeviceProperties )
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
		samplerCreateInfo.maxAnisotropy			  = p_physicalDeviceProperties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.borderColor			  = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable			  = VK_FALSE;
		samplerCreateInfo.compareOp				  = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode			  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias			  = 0.0f;
		samplerCreateInfo.minLod				  = 0.0f;
		samplerCreateInfo.maxLod				  = static_cast<float>( m_mipLevels );

		// Create the texture sampler
		if ( vkCreateSampler( *m_logicalDevice, &samplerCreateInfo, nullptr, &m_sampler ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create texture sampler" );
	}

	inline const uint32_t&	GetMipLevels() const { return m_mipLevels; }
	inline const VkSampler& GetSampler() const { return m_sampler; }
	inline const uint32_t&	GetSamplerID() const { return m_samplerID; }

	void Cleanup() override
	{
		// Destroy the sampler
		vkDestroySampler( *m_logicalDevice, m_sampler, nullptr );

		// Destroy the image view
		vkDestroyImageView( *m_logicalDevice, *m_imageView, nullptr );

		// Destroy the pointer to the image view (This is a precautionary measure)
		m_imageView.reset();

		// Destroy the image and free its memory
		vkDestroyImage( *m_logicalDevice, m_image, nullptr );
		vkFreeMemory( *m_logicalDevice, m_imageMemory, nullptr );
	}
};