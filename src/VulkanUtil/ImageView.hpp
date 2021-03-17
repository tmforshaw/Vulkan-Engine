#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static VkImageView CreateImageView( const VkDevice& p_logicalDevice, const VkImage& p_image, const VkFormat& p_format, VkImageAspectFlags p_aspectFlags )
{
	// Setup the creation information for the image view
	VkImageViewCreateInfo viewCreateInfo {};
	viewCreateInfo.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image	= p_image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format	= p_format;
	// viewCreateInfo.components					   = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask	   = p_aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel   = 0;
	viewCreateInfo.subresourceRange.levelCount	   = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount	   = 1;

	// Create the image view
	VkImageView imageView;
	if ( vkCreateImageView( p_logicalDevice, &viewCreateInfo, nullptr, &imageView ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create texture image view" );

	return imageView;
}