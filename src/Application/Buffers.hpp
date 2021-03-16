#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>

static uint32_t FindMemoryType( const VkPhysicalDevice& p_physicalDevice, const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties )
{
	// Get the GPU memory properties
	VkPhysicalDeviceMemoryProperties memProperities;
	vkGetPhysicalDeviceMemoryProperties( p_physicalDevice, &memProperities );

	// Find a suitable memory type
	for ( uint32_t i = 0; i < memProperities.memoryTypeCount; i++ )
		if ( typeFilter & ( 1 << i ) &&													  // The type of memory is suitable
			 ( memProperities.memoryTypes[i].propertyFlags & properties ) == properties ) // The memory has the correct properties
			return i;																	  // Return the index of a suitable memory type

	// No memory type was found
	throw std::runtime_error( "Failed to find suitable memory type" );
}

void CreateBuffer( const VkDevice& p_logicalDevice, const VkPhysicalDevice& p_physicalDevice, const VkDeviceSize& p_size, const VkBufferUsageFlags& p_usage, const VkMemoryPropertyFlags& p_properties, VkBuffer* p_buffer, VkDeviceMemory* p_bufferMemory )
{
	// Setup the create information for the buffer
	VkBufferCreateInfo bufferCreateInfo {};
	bufferCreateInfo.sType		 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size		 = p_size;
	bufferCreateInfo.usage		 = p_usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.flags		 = 0;

	// Create the buffer
	if ( vkCreateBuffer( p_logicalDevice, &bufferCreateInfo, nullptr, p_buffer ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create buffer" );

	// Setup the memory requirements for the buffer
	VkMemoryRequirements memRequirements {};
	vkGetBufferMemoryRequirements( p_logicalDevice, *p_buffer, &memRequirements );

	// Setup the allocation information for the buffer
	VkMemoryAllocateInfo bufferAllocInfo {};
	bufferAllocInfo.sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocInfo.allocationSize	= memRequirements.size;
	bufferAllocInfo.memoryTypeIndex = FindMemoryType( p_physicalDevice, memRequirements.memoryTypeBits, p_properties ); // Find a memory type with the correct properties

	// Allocate the memory of the buffer
	if ( vkAllocateMemory( p_logicalDevice, &bufferAllocInfo, nullptr, p_bufferMemory ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to allocate buffer memory" );

	// Associate the memory with the buffer
	vkBindBufferMemory( p_logicalDevice, *p_buffer, *p_bufferMemory, 0 );
}

void CopyBuffer( const VkDevice& p_logicalDevice, const VkCommandPool& p_commandPool, const VkQueue& p_graphicsQueue, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, const VkDeviceSize& p_size )
{
	// Setup the allocation information for the command buffer
	VkCommandBufferAllocateInfo commandBufferAllocInfo {};
	commandBufferAllocInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocInfo.level			  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocInfo.commandPool		  = p_commandPool;
	commandBufferAllocInfo.commandBufferCount = 1;

	// Allocate the command buffer
	VkCommandBuffer commandBuffer;
	if ( vkAllocateCommandBuffers( p_logicalDevice, &commandBufferAllocInfo, &commandBuffer ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to allocate command buffer" );

	// Start recording to the command buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if ( vkBeginCommandBuffer( commandBuffer, &commandBufferBeginInfo ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to begin recording to command buffer" );

	// Setup the copy region
	VkBufferCopy copyRegion {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size		 = p_size;

	// Create the copy region
	vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

	// Finish recording to the command buffer
	if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to record command buffer" );

	// Setup the submission information for the command buffer
	VkSubmitInfo submitInfo {};
	submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers	  = &commandBuffer;

	// Execute the command buffer to complete the transfer of data
	vkQueueSubmit( p_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( p_graphicsQueue );

	// Free the command buffer
	vkFreeCommandBuffers( p_logicalDevice, p_commandPool, 1, &commandBuffer );
}